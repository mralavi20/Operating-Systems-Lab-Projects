#include "types.h"
#include "stat.h"
#include "user.h"


int main (int argc, char *argv[]) {
    int status;

    status = copyfile (argv[1], argv[2]);

    if (status == -1) {
        write (1, "error\n", 6);
    }

    exit ();
}