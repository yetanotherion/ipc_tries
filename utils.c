#include "utils.h"

int parsePid(char* buff) {
  char res[BUFF_SIZE];
  int j=0, i;
  bool started = false;
  for (i=0; i < strlen(buff); i++) {
    if (!started) {
      if (buff[i] != 0x20) {
        started=true;
        res[j] = buff[i];
        j++;
      }
    }
    else {
      if (buff[i] != 0x20) {
        res[j] = buff[i];
        j++;
      }
      else {
        res[j] = '\0';
        return atoi(res);
      }
    }
  }
  return 0;
}

void parsePids(int* pids) {
   char cmd[] = "ps -A | grep multi_test | grep -v grep";
   FILE * p = popen(cmd, "r");
   char buff[BUFF_SIZE];
   int i = 0;
   while (fgets(buff, BUFF_SIZE - 1, p) != NULL) {
     if (i == NUM_MASTER) {
       printf("too many masters\n");
       exit(-1);
     }
     pids[i] = parsePid(buff);
     i++;
   }
   pclose(p);
   if (i < NUM_MASTER) {
     printf("waiting for other masters");
     sleep(1);
     parsePids(pids);
   }
}
