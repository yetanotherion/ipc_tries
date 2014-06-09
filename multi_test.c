#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>

#include "utils.h"
#include "sem_utils.h"

const char process_name[] = "multi_test";

typedef struct {
  sem_t* sem_election;
  sem_t* sem_shift_queue;
  int pids[NUM_MASTER];
  int curr_pid;
} sync_data;


bool signalCalled = false;

void loadSyncData(sync_data* sd) {
  sd->sem_election = sem_open(sem_election_name, O_CREAT, 0644, NUM_MASTER - 1);
  sd->sem_shift_queue = sem_open(sem_shift_queue_name, O_CREAT, 0644, 1);
  sd->curr_pid = getpid();
  parsePids(sd->pids);
}

void sighandler(int unused) {
  signalCalled = true;
}

void waitForChosenSignal() {
  sigset_t mask, oldmask;

  /* Set up the mask of signals to temporarily block. */
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);

  /* Wait for a signal to arrive. */
  sigprocmask (SIG_BLOCK, &mask, &oldmask);
  while (!signalCalled) {
    sigsuspend (&oldmask);
  }
  sigprocmask (SIG_UNBLOCK, &mask, NULL);
  signalCalled = false;
}

void setIdle(sync_data* sd) {
  if (sem_trywait(sd->sem_election) != 0) {
    int i;
    printf("%d is chosen\n", sd->curr_pid);
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
    printf("%d is waiting\n", sd->curr_pid);
    waitForChosenSignal();
    printf("%d finished waiting\n", sd->curr_pid);
  }
}

int main() {
  sync_data d;
  signal(SIGUSR1, sighandler);
  // this can be long, should find a better way to do so
  loadSyncData(&d);
  printf("master synced\n");
  setIdle(&d);
  setIdle(&d);
  setIdle(&d);
  setIdle(&d);
  printf("%d finished\n", d.curr_pid);
}
