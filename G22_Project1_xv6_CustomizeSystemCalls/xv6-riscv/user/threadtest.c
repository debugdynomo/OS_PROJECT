#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Global variable to prove memory is shared
int shared_var = 0; 

int main(void) {
  printf("Main: Allocating stack...\n");
  
  // 1. Allocate 1 page (4096 bytes) of memory for the thread's stack
  void *stack = malloc(4096);
  if(stack == 0){
    printf("Main: malloc failed\n");
    exit(1);
  }

  // CRITICAL: In RISC-V, the stack grows DOWNWARDS. 
  // malloc gives us the bottom address. We must pass the TOP address to clone.
  void *stack_top = stack + 4096;

  printf("Main: Calling clone()...\n");
  
  // 2. Create the thread
  int pid = clone(stack_top);

  if (pid < 0) {
    printf("Main: Clone failed!\n");
    exit(1);
  } 
  
  if (pid == 0) {
    // ------------------------------------------------
    // THIS IS THE THREAD
    // ------------------------------------------------
    printf("Thread: Hello from the new thread!\n");
    
    // Modify the shared variable. 
    // If memory is shared, the main thread will see this change.
    shared_var = 42; 
    
    printf("Thread: Exiting now.\n");
    exit(0); 
  } 
  else {
    // ------------------------------------------------
    // THIS IS THE MAIN PROGRAM
    // ------------------------------------------------
    void *joined_stack;
    
    // Wait for the thread to finish
    int joined_pid = join(&joined_stack);
    
    printf("Main: Thread %d successfully joined.\n", joined_pid);
    
    // Verify the shared memory worked
    if (shared_var == 42) {
      printf("Main: SUCCESS! Shared memory works. shared_var = %d\n", shared_var);
    } else {
      printf("Main: FAILED! Memory was not shared. shared_var = %d\n", shared_var);
    }

    // Verify the stack pointer was returned correctly
    if (joined_stack == stack_top) {
      printf("Main: SUCCESS! Stack pointer returned correctly.\n");
    }

    // Free the stack memory that we allocated earlier
    free(stack); 
  }

  exit(0);
}