#include "types.h"
#include "stat.h"
#include "user.h"


int main (int argc, char *argv[]) {
    int number;
    int register_data;
    int status;
    int i;

    number = 0;
    for (i = 0; argv[1][i] != '\0'; i++) {
        number = (number * 10) + (argv[1][i] - 48);
    }

    asm ("movl %%ebx, %0" : "=r" (register_data));
    asm ("movl %0, %%ebx" : : "r" (number));

    status = finddigitalroot ();

    asm ("movl %0, %%ebx" : : "r" (register_data));

    if (status == -1) {
        write (1, "error\n", 6);
    }

    exit ();
}