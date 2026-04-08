#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  printf("=== fork_stats_25 Demo by Prathik Reddy ===\n");
  printf("System Call #25\n\n");

  int pid1, pid2, pid3;

  // Test 1: Normal fork with priority 1, max 3 children
  printf("--- Test 1: Forking with priority=1, max_children=3 ---\n");
  pid1 = fork_stats_25(1, 3);
  if(pid1 == 0){
    printf("Child 1: Running with priority=1\n");
    exit(0);
  }
  wait(0);
  printf("Parent: Child 1 done\n\n");

  // Test 2: Fork with priority 2, max 3 children
  printf("--- Test 2: Forking with priority=2, max_children=3 ---\n");
  pid2 = fork_stats_25(2, 3);
  if(pid2 == 0){
    printf("Child 2: Running with priority=2\n");
    exit(0);
  }
  wait(0);
  printf("Parent: Child 2 done\n\n");

  // Test 3: Fork two children simultaneously, max=1 (second should block)
  printf("--- Test 3: max_children=1, forking twice (2nd should BLOCK) ---\n");

  pid1 = fork_stats_25(1, 1);
  if(pid1 == 0){
    for(volatile int i = 0; i < 10000000; i++); // busy wait to stay alive
    printf("Child A: Done\n");
    exit(0);
  }

  // While Child A is alive, try forking again with max=1 (should block)
  pid2 = fork_stats_25(1, 1);
  if(pid2 < 0){
    printf("Parent: Second fork BLOCKED as expected! max_children=1 reached\n");
  } else if(pid2 == 0){
    printf("Child B: Created (unexpected)\n");
    exit(0);
  } else {
    wait(0);
  }

  wait(0); // Wait for Child A

  // Test 4: Fork with high priority
  printf("\n--- Test 4: Forking with priority=5, max_children=3 ---\n");
  pid3 = fork_stats_25(5, 3);
  if(pid3 == 0){
    printf("Child 3: Running with high priority=5\n");
    exit(0);
  }
  wait(0);
  printf("Parent: Child 3 done\n");

  printf("\n=== Demo Complete ===\n");
  exit(0);
}