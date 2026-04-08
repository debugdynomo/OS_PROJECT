#include "kernel/types.h"
#include "user/user.h"

int main() {

    for(int i = 0; i < 3; i++){
        if(fork() == 0){
            for(volatile int j = 0; j < 100000000; j++);

            exit(0);
        }
    }

    for(volatile int k = 0; k < 10000000; k++);
   
    int count = getchildcount();
    printf("Child count: %d\n", count);

    for(int i = 0; i < 3; i++){
        wait(0);
    }

    exit(0);
}
