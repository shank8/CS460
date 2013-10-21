#include "uio.c"
#include "ucode.c"

main()
{ 
  char name[64]; 
  int pid, cmd;


  while(1){

       printf("==============================================\n");
       printf("\nIch bin Prozess %d in der U Weise: das laufen im Segment=%x\n",
                getpid(), getcs());

       
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

           case 6 : die();      break;
           case 7 : ufork();     break;
           case 8 : uexec();     break;

           case 9 : chcolor();  break;

           default: invalid(name); break;
       } 
  }
}


