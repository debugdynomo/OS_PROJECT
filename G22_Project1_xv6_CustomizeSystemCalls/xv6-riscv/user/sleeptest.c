#include "kernel/types.h"
#include "user/user.h"

int main() {
	printf("start\n");

	sleep(100);

	printf("end\n");
	exit(0);
}
