#include "kernel/types.h"
#include "user/user.h"

int main(){
    int pid = fork();

    if(pid == 0){
        // child process
        while(1){
            printf("Child waiting for signal...\n");
            sleep(20);
        }
    } 
    else {
        // parent process
        sleep(100);
        sendsignal(pid);
        printf("Signal sent to child!\n");
    }

    exit(0);
}

