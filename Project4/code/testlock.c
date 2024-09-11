#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


#define NPROC 8
    
int main(){
    int i;
    int pid;
    iplock();
    for (i = 0; i < NPROC; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            printf(2, "Fork failed\n");
            exit();
        }
        else if (pid == 0)
        {
            aplock();
            sleep(200);
            printf(1,"done\n");
            rplock();
            exit();
        }
        
    }

    for (i = 0; i < NPROC; i++)
    {
        wait();
        // if (i!= NPROC-1){
        //     exit();
        // }
    }

    // countsyscall();
    exit();
}