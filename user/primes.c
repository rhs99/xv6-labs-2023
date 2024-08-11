#include "kernel/types.h"
#include "user/user.h"

void
filter_primes(int *p_in)
{
    int p_out[2];
    int min, buf;
    pipe(p_out);
    close(p_in[1]);

    if(read(p_in[0], &min, sizeof(int)) > 0){
        printf("prime %d\n", min);

        if(fork() == 0){
            filter_primes(p_out);
            exit(0);
        }else{
            close(p_out[0]);
            while(read(p_in[0], &buf, sizeof(int)) > 0){
                if(buf % min != 0){
                    write(p_out[1], &buf, sizeof(int));
                }
            }
            close(p_out[1]);
            wait(0);
        }
    }
    exit(0);
}

int
main()
{
    int p[2];
    pipe(p);

    if(fork() == 0){
        filter_primes(p);
        exit(0);
    }else{
        close(p[0]);
        for(int i = 2;i <= 35;i++){
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
        wait(0);
    }

    exit(0);
}
