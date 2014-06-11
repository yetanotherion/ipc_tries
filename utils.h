#ifndef _UTILS_H_
#define _UTILS_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define NUM_MASTER 100
#define BUFF_SIZE 1024
#define SHIFT_QUEUE_SIZE 1024

int parse_pid(char* buff);
void parse_pids(int* pids);

typedef struct {
  float queue[SHIFT_QUEUE_SIZE];
  int nb_element;
} shift_request_queue;

void enqueue(shift_request_queue* shift_queue, float curr_shift);
void print_queue(shift_request_queue* shift_queue);
float implement_shift(shift_request_queue* shift_queue);

#endif /* _UTILS_H_ */
