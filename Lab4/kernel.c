

/***********************************************************
    kfork creates a child process ready to run in Kmode from body()
    In addition, it loads u1 file to the child's Umode segment 
************************************************************/




int get_proc(PROC **list){
  return (dequeue(list));
}

int copy_image(u16 child_segment){
  u16 end, offset;
  int word;

  end = 0x1000;
  offset = 0;
 
  while(offset < end){
    word = get_word(running->uss, offset);
   
    put_word(word, child_segment, offset);
    offset++;
  }

//load("/bin/u1", child_segment);
    return 1;


}
int goUmode();

int do_exec(int val){

  u16 end, cursor, segment;
  int j;
  char str[64];
  char c;
  int i = 0;

  printf("------- exec --------\n");
   if(!val){
      printf("No file was given to exec()\n");
      return 0;
    }

  do
  {
    c = get_byte(running->uss, val);
    str[i] = c;
    val++;
    i++;
  }while(c!='\0');

      

   

    // Load the executable image into the running segment
    load(str, running->uss);

    segment = running->uss;


     end = cursor = segment + 0x1000; // Get to the HIGH end of the ustack

            j=0;
            while(j<=12){

              cursor = 0x1000 - j*2;
           
              switch(j){
                case 1: // FLAG
                
                  put_word(0x0200, segment, cursor);

                  break;
                case 2: // uCS current segment value
                 
                   put_word(segment, segment, cursor);
                  break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10: // Set all these values to 0
                   put_word(0x000, segment, cursor);
                  break;
                case 11: // Set the last 2 uDS and uES
                case 12:
                   put_word(segment, segment, cursor);
                  break;
              }
              j++;

            }


            running->usp = cursor;


          printf("------------- uSTACK : Exec --------------\n");
          printf("| uDS |  uES  | di | si | bp | dx | cx | bx | ax | uPC | uCS  | flag |\n");
          j=0;

          while(j<12){
             printf("%x ", get_word(segment, cursor));
             cursor+=1;
             j++;
          }
          printf("\n\n");

}



PROC * kfork(char * file){
       int j;
      u16 segment, *cursor, *end;
      PROC *p;
      
      printf("------ kfork ------\n");

      p = get_proc(&freeList);  // Set up a new PROC that points to the next free PROC in the list
      
      if(p==0) {
        printf("FreeList is NULL\n");
        return -1;
      }

      p->ppid = running->pid; // Set the parent PID to the PID of the running proc
      p->parent = running;
      p->status = READY;
      p->priority = 1;
      p->next = 0;

      //clear all saved registers on stack
      for(j=1; j<10; j++){
          p->kstack[SSIZE-j] = 0;
      }

      segment = (p->pid + 1)*0x1000;
      end = cursor = segment + 0x1000; // Get to the HIGH end of the ustack


      if(file != 0){

       p->kstack[SSIZE-1]=(int)body;

       load(file, segment);

          j=0;
            while(j<=12){
              cursor = 0x1000 - j*2;
           
              switch(j){
                case 1: // FLAG
                
                  put_word(0x0200, segment, cursor);

                  break;
                case 2: // uCS current segment value
                 
                   put_word(segment, segment, cursor);
                  break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10: // Set all these values to 0
                   put_word(0x0000, segment, cursor);
                  break;
                case 11: // Set the last 2 uDS and uES
                case 12:
                   put_word(segment, segment, cursor);
                  break;
              }
              j++;

            }
          
          
          p->uss = segment; // Set the current segment
          p->usp = cursor; // Set the USP to the cursor which is currently pointing to the TOP of the stack which is the uDS

           

     }
     enqueue(p, &readyQueue);

     return p;
}

int do_fork()
{
         /* This do_fork() function is called only when fork is called from kmode */

            PROC *p;
            u16 segment, cursor;
            int j;

            printf("------- do_fork -------\n");

            p = kfork(0); // kfork from kernel

            if(p != 0){
            p->kstack[SSIZE-1] = (int)goUmode;
            p->ksp = &(p->kstack[SSIZE-9]);

            segment = (p->pid + 1)*0x1000;

            copy_image(segment);

            p->kstack[SSIZE-1] = (int)goUmode;

           
             j=0;
            while(j<=12){
              cursor = 0x1000 - j*2;
           
              switch(j){
                case 1: // FLAG
                
                  put_word(0x0200, segment, cursor);

                  break;
                case 2: // uCS current segment value
                 
                   put_word(segment, segment, cursor);
                  break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10: // Set all these values to 0
                   put_word(0x0000, segment, cursor);
                  break;
                case 11: // Set the last 2 uDS and uES
                case 12:
                   put_word(segment, segment, cursor);
                  break;
              }
              j++;

            }


           
          
          /* PRINT THE STACK 

          printf("------------- uSTACK --------------\n");
          printf("| uDS |  uES  | di | si | bp | dx | cx | bx | ax | uPC | uCS  | flag |\n");
          j=0;

          while(j<12){
             printf("%x ", get_word(segment, cursor));
             cursor+=1;
             j++;
          }
          printf("\n\n");*/

            }else{
              return -1;
            }
         


           return p->pid;
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

int body()
{
  char c;
  PROC * temp;

  printf("Proc %d resumes to body()\n", running->pid);
  while(1){
    
   
    printList("\nReadyQueue", readyQueue);
    printList("FreeList", freeList);
    printList("SleepList", sleepList);
    temp = procList;
   /* printf("AllProcs: ");
    while(temp){
      printf("[ %d| %d]->", temp->pid, (temp->parent ? temp->parent->pid : -1));
      temp = temp->pnext;
    }
    printf("NULL");
    printf("\n\n");*/

    printf("Proc %d running: Parent = %d\nEnter a char [s|f|w|q|u] : ", 
     running->pid, running->parent->pid);
    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : tswitch();      break;
       case 'f' : kfork("/bin/u1");     break;
       case 'w' : do_wait();      break;
       case 'q' : do_exit(-1);    break;
       case 'u' : goUmode();      break;
    }
  }
}