// #include "types.h"
// #include "stat.h"
// #include "user.h"


// int main (int argc, char *argv[]) {
//     int fd;

//     fd = open ("sample.txt", 1);
//     write (fd, "hello\n", 6);
//     close (fd);

//     countsyscall();
//     exit();
// }

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NPROC 8

const char* names[NPROC] = {"1.txt","2.txt","3.txt","4.txt","5.txt","6.txt","7.txt","8.txt"};





int main()
{
    int i;
    int pid;

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
            int fd = open(names[i],O_WRONLY|O_CREATE);
            write(fd,names[i],20);
            // printf(1,"%s",names[i]);
            close(fd);
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

    countsyscall();
    exit();
}