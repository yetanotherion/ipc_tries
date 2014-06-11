#include <semaphore.h>
#include <sys/mman.h>

#include "sem_utils.h"

int main() {
  sem_unlink(sem_election_name);
  sem_unlink(sem_shift_queue_name);
  shm_unlink(shift_queue_name);
  return 0;
}
