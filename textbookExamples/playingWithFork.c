#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int count = 0;
    printf("hello world (pid:%d)\n", (int) getpid());
    fork();
    count++;
    printf("count now %d; (pid:%d)\n", count, (int) getpid());
    fork();
    count++;
    printf("count now %d; (pid:%d)\n", count, (int) getpid());
    fork();
    count++;
    printf("count now %d; (pid:%d)\n", count, (int) getpid());

}