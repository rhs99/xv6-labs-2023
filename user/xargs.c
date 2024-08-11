#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int main(int argc, char *argv[]){
    char *args[MAXARG];
    int i;
    for(i=0;i<argc;i++){
        args[i] = argv[i];
    }

    char buf[256];

    while(1){
        int j = 0;
        while(read(0, buf+j, sizeof(char))!=0 && buf[j] != '\n'){
            j++;
        }
        if(j == 0){
            break;
        }
        buf[j] = 0;

        args[i] = buf;
        args[i+1] = 0;

        if(fork() == 0){
            exec(args[1], args+1);
        }else{
            wait((void*)0);
        }
    }
    exit(0);
}