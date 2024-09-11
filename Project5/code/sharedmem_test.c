#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "spinlock.h"
#include "uspinlock.h"




int main(int argc, char* argv[]){
    int* shared_mem;
    iplock();
    open_sharedmem(1,(void*) &shared_mem);
    char* value = (char*) shared_mem;
    *value = 0;
    for (int i = 0; i < 10; i++){
        int pid = fork();
        if ( pid== 0){
            int * shared_mem;
            aplock();
            open_sharedmem(1,(void*)&shared_mem);
            char* value = (char*) shared_mem;
            *value += 1;
            printf(1, "Values in child: %d\n", *value);
            close_sharedmem(1);
            rplock();
            exit();
            return 0;
        }
    }
    for(int i =0; i < 10;i++){
        wait();
    }
    printf(1, "Final Value: %d\n", *value);
    
    close_sharedmem(1);
        
    exit();
}