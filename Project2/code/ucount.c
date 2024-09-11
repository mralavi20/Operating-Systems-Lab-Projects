#include "types.h"
#include "stat.h"
#include "user.h"

void pid4()
{
  int pid1 = fork();
  if (pid1 < 0)
  {
    printf(1, "Fork 4 failed.\n");
    exit();
  }

  if (pid1 == 0)
  {
    printf(1, "Uncle Numbers: %d\n", ucount(getpid()));
  }
  else
  {
    sleep(3);
    printf(1, "Father --- Siblings:\n");

    wait();
  }
}
// comment the function below and change pid3() in line 65 to pid4().. It is going to have 2 uncle. 
void pid3()
{
  int pid1 = fork();
  if (pid1 < 0)
  {
    printf(1, "Fork 3 failed.\n");
    exit();
  }

  if (pid1 == 0)
  {
    sleep(14);
    printf(1, "Siblin 3 arrived\n");
  }
  else
  {
    pid4();

    wait();
  }
}

void pid2()
{
  int pid1 = fork();
  if (pid1 < 0)
  {
    printf(1, "Fork 2 failed.\n");
    exit();
  }

  if (pid1 == 0)
  {
    sleep(12); 
    printf(1, "Siblin 2 arrived\n");
  }
  else
  {
    pid3();

    wait();
  }
}

int main(int argc, char *argv[])
{
  int pid1 = fork();
  if (pid1 < 0)
  {
    printf(1, "Fork 1 failed.\n");
    exit();
  }

  if (pid1 == 0)
  {
    sleep(10);
    printf(1, "Siblin 1 arrived\n");
  }
  else
  {

    pid2();

    wait();
  }

  // ucount(7);

  exit();
}