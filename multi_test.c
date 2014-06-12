#include <assert.h>
#include "sim_time.h"

int main() {
  sim_time* st;
  st = init_sim_time("./multi_test");
  set_shift(st, 3.0);
  set_idle(st);
  assert(get_time(st) == 3.0);
  set_shift(st, 9.0);
  set_shift(st, 4.0);
  set_idle(st);
  assert(get_time(st) == 7.0);
  set_idle(st);
  assert(get_time(st) == 12.0);
  printf("%d finished\n", st->sd->curr_pid);
  free_sim_time(st);
  return 0;
}
