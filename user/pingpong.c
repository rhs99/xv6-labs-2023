
#include "kernel/types.h"
#include "user/user.h"

int
main()
{
  int pid;
  int fds[2];
  char buf[1];
  
  pipe(fds);

  pid = fork();

  if (pid == 0) {
    int child_pid = getpid();
    read(fds[0], buf, sizeof(buf));
    fprintf(1, "%d: received ping\n", child_pid);
    write(fds[1], buf, 1);
  } else {
    write(fds[1], buf, 1);
    int parent_pid = getpid();
    read(fds[0], buf, sizeof(buf));
    fprintf(1, "%d: received pong\n", parent_pid);
  }
  exit(0);
}