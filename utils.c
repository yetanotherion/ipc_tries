#include "utils.h"


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

void print_queue(shift_request_queue* shift_queue) {
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
