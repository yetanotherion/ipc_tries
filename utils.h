#ifndef _UTILS_H_
#define _UTILS_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define NUM_MASTER 100
#define BUFF_SIZE 1024

int parsePid(char* buff);
void parsePids(int* pids);

#endif /* _UTILS_H_ */
