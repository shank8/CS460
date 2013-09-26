#include "uio.c"
#include "ucode.c"
int color;

main()
{ 
  char name[64]; int pid, cmd;
  pid = getpid();

  while(1){
       color = 0x0000B + pid;

       printf("==============================================\n");
       printf("\nIch bin Prozess %d in der U Weise: das laufen im Segment=%x\n",
                getpid(), getcs());
       #include "ubody.c"
  }
}



