#include <stdio.h>
#include <unistd.h>

typedef int (*getpid_fun)();

getpid_fun global_getpid = (getpid_fun) getpid;

int main() {
    getpid_fun local_getpid = (getpid_fun) getpid;
    int pid = getpid();
    int local_pid = local_getpid();
    int global_pid = global_getpid();

    printf("direct call: %d, local call: %d, global call: %d\n",
           pid, local_pid, global_pid);

//    getchar();
    return 0;
}