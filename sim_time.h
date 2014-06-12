#ifndef _UTILS_H_
#define _UTILS_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#define NUM_MASTER 100
#define BUFF_SIZE 1024
#define SHIFT_QUEUE_SIZE 1024

typedef struct {
  float queue[SHIFT_QUEUE_SIZE];
  int nb_element;
} shift_request_queue;

typedef struct {
  sem_t *sem_election;
  sem_t *sem_shift_queue;
  int pids[NUM_MASTER];
  int curr_pid;
} sync_data;

typedef struct {
  shift_request_queue queue;
  float curr_time;
} time_data;

typedef struct {
  sync_data *sd;
  time_data *td;
  char* progname;
} sim_time;

sim_time *init_sim_time(const char *progname);
void set_idle(sim_time *st);
float get_time(sim_time *st);
void set_shift(sim_time *st, float curr_shift);
void set_idle(sim_time *st);
void free_sim_time(sim_time *st);
void print_queue(sim_time *st);

#endif /* _UTILS_H_ */
