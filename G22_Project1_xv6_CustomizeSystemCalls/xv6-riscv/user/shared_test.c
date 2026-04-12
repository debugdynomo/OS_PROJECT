#include "kernel/types.h"
#include "user/user.h"

int main(){
    int pid = fork();

    if(pid == 0){
        // child
        int val = sharedmem(-1);
        printf("Child received: %d\n", val);
        exit(0);
    } 
    else {
        // parent
        sharedmem(42);
        wait(0);   // wait for child
        printf("Parent wrote: 42\n");
    }

    exit(0);
}
