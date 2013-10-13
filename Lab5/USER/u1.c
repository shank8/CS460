#include "ucode.c"
int color;
main()
{ 
  char name[64]; int pid, cmd;

  while(1){
    pid = getpid();
    color = 0x000A + (pid % 6);
       
    printf("----------------------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : fork();     break;
           case 7 : exec();     break;
           case 8 : exit();     break;
           case 9 : pipe();    break;
           case 10: pfd();       break;
           case 11: close();     break;
           case 12 : read_pipe(); break;
           case 13 : write_pipe();break;
           default: invalid(name); break;
    }
  }
}



