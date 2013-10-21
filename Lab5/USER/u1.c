#include "ucode.c"
int color;
main()
{ 
  char name[64]; int pid, cmd;

  while(1){
    pid = getpid();
    color = 0x000B + (pid % 6);
       
    printf("----------------------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
      printf("find_cmd = %d\n", cmd);
    /*char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", 
             "fork", "exec", "pipe", "pfd", "read", "write", "close", 0};*/
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;
           case 7 : fork();     break;
           case 8 : exec();     break;
         
           case 9 : pipe();    break;
           case 10: pfd();       break;
          
           case 11 : read_pipe(); break;
           case 12 : write_pipe();break;
            case 13: close_pipe();     break;
           default: invalid(name); break;
    }
  }
}



