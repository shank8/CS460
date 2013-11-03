int kmode()
{
  body();
}

char *pStatus[] = {"FREE","READY","SLEEP","ZOMBIE","BLOCK",0};
int kps()
{
  int i;
  for (i=0; i<NPROC; i++){
    printf("Task %d ", i+1);
      printf("ppid="); 
         if (i>0) prints("  ");
         if (proc[i].status)
	   printf("%d", proc[i].ppid); 
         else
            printf("--");
         if (running==&proc[i])
	   printf("%s", "  RUNNING");
         else
	   printf("  %s",pStatus[proc[i].status]);
      printf("\n");
  }
  return(0);
}

int chname(y) char *y;
{
    prints("chname : not implemented\n\r");
}
   
int kcinth() 
{
   ushort x, y, z, w, r;   
   ushort seg, off;

   seg = running->uss; off = running->usp;

   x = get_word(seg, off+13*2);
   y = get_word(seg, off+14*2);
   z = get_word(seg, off+15*2);
   w = get_word(seg, off+16*2);
 
   switch(x){
       case 0 : r = running->pid;    break;
       case 1 : r = kps();           break;
       case 2 : r = chname(y);       break;
       case 3 : r = kmode();         break;
       case 4 : r = tswitch();       break;
       case 5 : r = kwait();         break;
       case 6 : r = kexit();         break;
       case 7 : r = fork();          break;
       case 8 : r = kexec(y);        break;

       case 9 : r = sout(y,z);       break;
       case 10: r = sin(y,z);        break;

       case 99: r = kexit();         break;

       default: printf("invalid syscall # %d \n", x);

   }
   put_word(r, seg, off+2*8);
}
