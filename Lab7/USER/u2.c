#include "ucode.c"
main()
{ 
  char name[64]; int cmd;
  /* resetVideo();*/
  while(1){
       pid = get_pid();
       prints("==============================================\n\r");
       prints("\n\rIch bin Aufgabe "); printi(pid);
       prints(" im U Weise: laufen vom Teil=0x");
       printx(getcs()); prints("\n\r");
       show_menu();
       prints("Command ? ");
       mgets(name); 
       if (name[0]==0) 
           continue;

       cmd = find_cmd(name);
       switch(cmd){
           case 0 : get_pid();  break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : mywait();   break;
           case 6 : myexit();   break;
           case 7 : ufork();    break;
           case 8 : myexec("/u1"); break; 
           case 9 : sin(); break;
           case 10: sout(); break;
          default: invalid(name); break;
       }
  }
}


