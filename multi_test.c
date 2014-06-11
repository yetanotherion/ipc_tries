#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>

#include "utils.h"
#include "sem_utils.h"

typedef struct {
  sem_t* sem_election;
  sem_t* sem_shift_queue;
  int pids[NUM_MASTER];
  int curr_pid;
} sync_data;

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

typedef struct {
  shift_request_queue queue;
  float curr_time;
} time_data;

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

float get_time(time_data* td) {
  return td->curr_time;
}

void set_shift(sync_data* sd, time_data* td, float curr_shift) {
  sem_wait(sd->sem_shift_queue);
  enqueue(&(td->queue), curr_shift);
  sem_post(sd->sem_shift_queue);
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
    print_queue(&(td->queue));
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

void set_idle(sync_data* sd, time_data* td) {
  sync_processing(sd, td, &do_sync_shift);
}

void init_queue(sync_data* sd, time_data* td) {
  sync_processing(sd, td, &do_init_queue);
}

int main() {
  sync_data sd;
  time_data* td;

  signal(SIGUSR1, sighandler);
  /* this can be long, should find a better way to do so */
  load_sync_data(&sd);
  printf("master are synced\n");
  td = load_time_data();
  printf("shared mem loaded\n");
  init_queue(&sd, td);
  printf("queue initialized\n");
  set_shift(&sd, td, 3.0);
  set_idle(&sd, td);
  assert(get_time(td) == 3.0);
  set_shift(&sd, td, 9.0);
  set_shift(&sd, td, 4.0);
  set_idle(&sd, td);
  assert(get_time(td) == 7.0);
  set_idle(&sd, td);
  assert(get_time(td) == 12.0);
  unload_time_data(td);
  printf("%d finished\n", sd.curr_pid);
  return 0;
}
