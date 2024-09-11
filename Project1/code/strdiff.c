#include "types.h"
#include "stat.h"
#include "user.h"


void strdiff (char* word1, char*word2) {
    char result[128];
    int size1;
    int size2;
    int fd;
    int i;

    size1 = strlen (word1);
    size2 = strlen (word2);

    i = 0;

    while (i < size1 || i < size2) {
        if (i < size1 && i < size2) {
            if (word2[i] <= word1[i]) {
                result[i] = '0';
            }
            else {
                result[i] = '1';
            }
        }
        else if (i < size1) {
            result[i] = '0';
        }
        else {
            result[i] = '1'; 
        }
        
        i = i + 1;
    }

    result[i] = '\n';

    fd = open ("strdiff_result.txt", 1);
    if (size1 > size2)
        write (fd, result, size1+1);
    else
        write(fd,result,size2+1);
    close (fd);
}

int main (int argc, char* argv[]) {

    strdiff (argv[1], argv[2]);

    exit ();
}