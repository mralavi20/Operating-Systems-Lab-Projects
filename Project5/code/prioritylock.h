
// #include "spinlock.h"
struct prioritylock {
    uint locked;
    uint validnum;
    struct spinlock lk;

    int pid_queue[1000];
    int queue_size;

    char *name;
    int pid;
};
