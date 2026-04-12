#include "logger.h"

int main() {
    logger_init("log.txt");

    log_operation("READ", "file1.txt", "SUCCESS", "Read completed");
    log_operation("WRITE", "file2.txt", "SUCCESS", "Write completed");

    logger_close();

    return 0;
}
