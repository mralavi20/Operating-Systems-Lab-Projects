struct prioritylock {
    uint locked;
    uint validnum;
    struct spinlock lk;

    int pid_queue[1000];
    int queue_size;

    char *name;
    int pid;
};

// void initprioritylock (struct prioritylock *lk, char *name);

// int find_min_pid (struct prioritylock *lk);

// void add_pid (struct prioritylock *lk, int pid);

// void remove_first_pid (struct prioritylock *lk);

// void acquirepriority (struct prioritylock *lk);

// void releasepriority (struct prioritylock *lk);