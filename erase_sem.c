#include <semaphore.h>
#include "utils.h"
#include "sem_utils.h"

int main() {
  sem_unlink(sem_election_name);
  sem_unlink(sem_shift_queue_name);
}
