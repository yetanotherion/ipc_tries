#include "sim_time.h"
#include "ipc_const.h"

int parse_pid(char* buff) {
  const char slash_multi_test[] = "./multi_test", delim[] = " ";
  int pid = 0, nber_call = 0;
  char *last_buffer = NULL, *res;
  res = strtok(buff, delim);
  nber_call = 1;
  while ((res = strtok(NULL, delim)) != NULL) {
    nber_call ++;
    if (nber_call == 2) pid = atoi(res);
    if (last_buffer != NULL) free(last_buffer);
    last_buffer = strdup(res);
  }
  if (strncmp(last_buffer, slash_multi_test,
              strlen(slash_multi_test)) != 0) pid = 0;
  free(last_buffer);
  return pid;
}

void parse_pids(int* pids) {
  char cmd[] = "ps aux | grep multi_test | grep -v bash | grep -v ps | grep -v grep";
  FILE * p = popen(cmd, "r");
  char buff[BUFF_SIZE];
  int i = 0, res;
  while (fgets(buff, BUFF_SIZE - 1, p) != NULL) {
    if ((res = parse_pid(buff)) != 0) {
      if (i == NUM_MASTER) {
        printf("too many masters\n");
        exit(-1);
      }
      else {
        printf("got pid %d\n", res);
        pids[i] = res;
        i++;
      }
    }
  }
  pclose(p);
  if (i < NUM_MASTER) {
    int sleep_time = NUM_MASTER / 10;
    printf("waiting for other masters\n");
    sleep_time = (sleep_time > 0) ? sleep_time : 1;
    sleep(sleep_time);
    parse_pids(pids);
  }
}

int comp(const void* e1, const void* e2) {
  float f1 = *((float*)e1);
  float f2 = *((float*)e2);
  if (f1 < f2) return -1;
  if (f1 > f2) return  1;
  return 0;
}

void enqueue(shift_request_queue* shift_queue, float curr_shift) {
  int nb_element = shift_queue->nb_element;
  if (nb_element == SHIFT_QUEUE_SIZE) {
    exit(1);
  }
  shift_queue->queue[nb_element] = curr_shift;
  shift_queue->nb_element = nb_element + 1;
  qsort(shift_queue->queue, shift_queue->nb_element,
        sizeof(float), comp);
}

void _print_queue(shift_request_queue* shift_queue) {
  int i;
  for (i = 0; i < shift_queue->nb_element -1; i++) {
    printf("%f,", shift_queue->queue[i]);
  }
  printf("\n");
}

float implement_shift(shift_request_queue* shift_queue) {
  int i, nb_zero=0;
  float next_shift;
  if (shift_queue->nb_element == 0) {
    return 0.0;
  }
  next_shift = shift_queue->queue[0];
  for (i = 0; i < shift_queue->nb_element - 1; i++) {
    shift_queue->queue[i] = shift_queue->queue[i + 1] - next_shift;
    if (shift_queue->queue[i] == 0) {
      nb_zero++;
    }
  }
  shift_queue->nb_element -= 1;
  /* let's filter trailing 0 */
  for (i = nb_zero; i < shift_queue->nb_element - 1; i++) {
    shift_queue->queue[i - nb_zero] = shift_queue->queue[i];
  }
  shift_queue -> nb_element -= nb_zero;
  /* we can return now */
  return next_shift;
}

sem_t* ensure_sem_open(const char* sem_name, int initial_value) {
  sem_t * res;
  if ((res = sem_open(sem_name, O_CREAT, 0644, initial_value)) == SEM_FAILED) {
    perror("sem_open");
    exit(-1);
  }
  return res;
}

void load_sync_data(sync_data* sd) {
  sd->sem_election = ensure_sem_open(sem_election_name, NUM_MASTER - 1);
  sd->sem_shift_queue = ensure_sem_open(sem_shift_queue_name, 1);
  sd->curr_pid = getpid();
  parse_pids(sd->pids);
}

time_data* load_time_data(void) {
  void* attach;
  int shm_fd;
  if ((shm_fd = shm_open(shift_queue_name, O_CREAT | O_RDWR, 0600)) == -1) {
    perror("shm_open");
    exit(-1);
  }
  if (ftruncate(shm_fd, sizeof(time_data)) == -1) {
    perror("ftruncate");
    exit(-1);
  }
  if ((attach = mmap(NULL, sizeof(time_data), PROT_READ | PROT_WRITE,
                     MAP_SHARED, shm_fd, (off_t)0)) == MAP_FAILED) {
    perror("mmap failed");
    exit(-1);
  }
  return (time_data*) attach;
}

void unload_time_data(time_data* td) {
  if (munmap((void*) td, sizeof(time_data)) != 0) {
    printf("could not unloadTime data");
    exit(-1);
  }
}

bool signal_called = false;

void sighandler(int unused) {
  signal_called = true;
}

void wait_for_chosen_signal() {
  sigset_t mask, oldmask;

  /* Set up the mask of signals to temporarily block. */
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);

  /* Wait for a signal to arrive. */
  sigprocmask (SIG_BLOCK, &mask, &oldmask);
  while (!signal_called) {
    sigsuspend (&oldmask);
  }
  sigprocmask (SIG_UNBLOCK, &mask, NULL);
  signal_called = false;
}

void sync_processing(sync_data* sd, time_data* td,
                     void (*sync_processing) (sync_data*, time_data*)) {
  if (sem_trywait(sd->sem_election) != 0) {
    int i;
    printf("%d is chosen\n", sd->curr_pid);
    (*sync_processing)(sd, td);
    for (i=0; i < NUM_MASTER - 1; i++) {
      sem_post(sd->sem_election);
    }
    printf("%d wakes up others\n", sd->curr_pid);
    for (i=0; i < NUM_MASTER; i++) {
      int pid = sd->pids[i];
      if (pid != sd->curr_pid) {
        kill(pid, SIGUSR1);
      }
    }
  }
  else {
    printf("%d starts waiting, queue: ", sd->curr_pid);
    _print_queue(&(td->queue));
    wait_for_chosen_signal();
    printf("%d finished waiting\n", sd->curr_pid);
  }
}

void do_sync_shift(sync_data* sd, time_data* td) {
   td->curr_time += implement_shift(&(td->queue));
}

void do_init_queue(sync_data* sd, time_data* td) {
  td->queue.nb_element = 0;
}

void init_queue(sync_data* sd, time_data* td) {
  sync_processing(sd, td, &do_init_queue);
}


sim_time* init_sim_time() {
  sync_data *sd;
  time_data *td;
  sim_time* st;

  if ((sd = (sync_data*) malloc(sizeof(sync_data))) == NULL) {
    perror("malloc");
    exit(-1);
  }
  if ((st = (sim_time*) malloc(sizeof(sim_time))) == NULL) {
    perror("malloc");
    exit(-1);
  }
  signal(SIGUSR1, sighandler);
  /* this can be long, should find a better way to do so */
  load_sync_data(sd);
  printf("master are synced\n");
  td = load_time_data();
  printf("shared mem loaded\n");
  init_queue(sd, td);
  printf("queue initialized\n");
  st->sd = sd;
  st->td = td;
  return st;
}

float get_time(sim_time *st) {
  return st->td->curr_time;
}

void set_shift(sim_time *st, float curr_shift) {
  sem_wait(st->sd->sem_shift_queue);
  enqueue(&(st->td->queue), curr_shift);
  sem_post(st->sd->sem_shift_queue);
}

void set_idle(sim_time *st) {
  sync_processing(st->sd, st->td, &do_sync_shift);
}

void free_sim_time(sim_time *st) {
  sync_data* sd = st->sd;
  unload_time_data(st->td);
  sem_close(sd->sem_election);
  sem_close(sd->sem_shift_queue);
  free(sd);
  free(st);
}

void print_queue(sim_time *st) {
  _print_queue(&(st->td->queue));
}
