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
       
       show_menu();
       printf("Command ? ");
       gets(name); 
       if (name[0]==0) 
           continue;

       cmd = find_cmd(name);
       printf("find_cmd = %d\n", cmd);
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
            case 13: close();     break;
           default: invalid(name); break;
       } 

  }
}



