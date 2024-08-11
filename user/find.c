#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *search_expr)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0)
  {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0)
  {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(path);
  *p = '/';
  p++;

  while (read(fd, &de, sizeof(de)) == sizeof(de))
  {
    if (de.inum == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0)
    {
      printf("find: cannot stat %s\n", buf);
      continue;
    }
    if (st.type == T_FILE)
    {
      if (strcmp(fmtname(buf), search_expr) == 0)
      {
        printf("%s\n", buf);
      }
    }
    else if (st.type == T_DIR)
    {
      if (strcmp(fmtname(buf), ".") != 0 && strcmp(fmtname(buf), "..") != 0)
      {
        find(buf, search_expr);
      }
    }
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  find(argv[1], argv[2]);
  exit(0);
}
