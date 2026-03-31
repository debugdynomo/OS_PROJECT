#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pid = fork();

    if(pid == 0) {
        // CHILD
        printf("Child started (PID: %d)\n", getpid());

        for(int i = 0; i < 5; i++) {
            int val = recv();
            printf("Child received: %d\n", val);
        }

        printf("Child exiting\n");
    } else {
        // PARENT
        printf("Parent started (PID: %d)\n", getpid());

        for(int i = 1; i <= 5; i++) {
            printf("Parent sending: %d\n", i * 10);
            send(i * 10);
        }

        wait(0);  // wait for child
        printf("Parent exiting\n");
    }

    exit(0);
}
