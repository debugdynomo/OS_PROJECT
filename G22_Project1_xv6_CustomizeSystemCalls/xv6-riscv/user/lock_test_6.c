#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Testing user-level locks (Purna - 6)...\n");
  initlock_6();
  
  int pid = fork();
  if(pid == 0) {
    // Child process context
    printf("Child: trying to acquire lock 0\n");
    acquire_6(0);
    printf("Child: acquired lock 0. Doing work...\n");
    pause(10); // simulate work 
    printf("Child: releasing lock 0\n");
    release_6(0);
    exit(0);
  } else {
    // Parent process context
    pause(2); // give child time to acquire
    printf("Parent: trying to acquire lock 0 (should wait)\n");
    acquire_6(0);
    printf("Parent: acquired lock 0. Lock system works!\n");
    release_6(0);
    wait(0);
  }

  printf("Test complete.\n");
  exit(0);
}
