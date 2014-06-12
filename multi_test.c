#include <assert.h>
#include "sim_time.h"

int main() {
  sim_time* st;
  st = init_sim_time();
  set_shift(st, 3.0);
  set_idle(st);
  assert(get_time(st) == 3.0);
  set_shift(st, 9.0);
  set_shift(st, 4.0);
  set_idle(st);
  assert(get_time(st) == 7.0);
  set_idle(st);
  assert(get_time(st) == 12.0);
  free_sim_time(st);
  printf("%d finished\n", st->sd->curr_pid);
  return 0;
}
