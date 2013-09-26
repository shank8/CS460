
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/
#define PA 13
#define PB 14
#define PC 15
#define PD 16
#define AX  8

/****************** syscall handler in C ***************************/
int kcinth()
{
   u16    segment, offset;
   int    a,b,c,d, r;
   segment = running->uss; 
   offset = running->usp;

   /** get syscall parameters from ustack **/
   a = get_word(segment, offset + 2*PA);
   b = get_word(segment, offset + 2*PB);
   c = get_word(segment, offset + 2*PC);
   d = get_word(segment, offset + 2*PD);

   switch(a){
       case 0 : r = running->pid;     break;
       case 1 : r = do_ps();          break;
       case 2 : r = chname(b);        break;
       case 3 : r = kmode();          break;
       case 4 : r = tswitch();        break;
       case 5 : r = do_wait(b);       break;
       case 6 : r = do_exit(b);       break;
       
       case 90: r =  getc();          break;
       case 91: //color=running->pid+11;
                r =  putc(b);         break;       
       case 99: do_exit(b);           break;
       default: printf("invalid syscall # : %d\n", a); 
   }
   put_word(r, segment, offset + 2*AX);
}

int do_ps(){
    PROC * temp = procList;
    int count = 0;
    printf("---------------- Processes ----------------\n");

    while(temp != 0){
      printf("%s %d[ %d]\nStatus: %d\nSegment:%x\n\n", temp->name, temp->pid, (temp->parent == 0 ? -1 : temp->parent->pid), temp->status, (temp->uss == 0 ? 0 : temp->uss));
      temp = temp->pnext;
      count++;
      if(count % 4==0){
        getc();
      }
    }
    printf("\n\n");
};
int chname(int val){
  char str[64];
    char c;
    int i = 0;
   
    do{
       c = get_byte(running->uss, val);
      str[i] = c;
      val++;
      i++;
    }while(c!='\0');

    printf("Changing name from %s to %s\n", running->name, str);
    strcpy(running->name, str);
};
int kmode(){
  //setds(0x1000);
  running->kstack[SSIZE-1] = (int)body;
};

int do_wait(){

 int status, ret;
  printf("Running proc %d is now waiting...\n", running->pid);

  ret = kwait(&status);
 
  if(ret != -1){
    printf("Finished waiting on Zombie child PROC %d\n", ret);
  }else{
    printf("Error: no children exist\n");
  }

  return ret;
};

int do_exit(int exitvalue){
  int val = exitvalue;
  

  if(val==-1){
    printf("Enter an exit value: ");
     val=(getc()&0x7F) - '0';
 
   printf("%d", val);
  }
  kexit(val);

  return val;
}

int printList(char * header, PROC * list){
  PROC * temp = list;

  printf("%s: ", header);

  while(temp!=0){
    printf("[ %d]-> ", temp->pid);
    temp = temp->next;
  }

  printf("NULL\n");
}