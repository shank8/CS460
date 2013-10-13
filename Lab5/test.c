  //********************** Unix Pipe Example ***************************
// 1. Run this program under Linux.
// 2. See what happens if you let
//        the child  process (pipe reader) die first.
//        the parent process (pipe writer) die first.
//********************************************************************
int  pd[2], n, pid;
char line[256], *s="data from pipe";

main()
{
  pid = getpid();
  printf("parent=%d\n", pid);

  pipe(pd);

  if (fork()){
    printf("parent %d close pd[0]\n", pid);
    close(pd[0]);

    while(1){
      sleep(2);
      printf("parent %d writing pipe : %s\n", pid, s);
      write(pd[1], s, strlen(s));
    }
  }

  else{
    pid = getpid();
    printf("child  %d close pd[1]\n", pid);
    close(pd[1]);

    while(1){
      printf("child  %d reading pipe : ", pid);
      n = read(pd[0], line, 256);      
      line[n]=0;
      printf("%s\n", line);
    }
  }
}