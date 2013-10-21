

/***********************************************************
    kfork creates a child process ready to run in Kmode from body()
    In addition, it loads u1 file to the child's Umode segment 
************************************************************/




PROC * get_proc(PROC **list){
  return (dequeue(list));
}

int copy_image(u16 child_segment){
  u16 end, offset;
  int word;

  end = 32*1024;
  offset = 0;
 
  while(offset < end){
    word = get_word(running->uss, offset);
   
    put_word(word, child_segment, offset);
    offset+=1;
  }

//load("/bin/u1", child_segment);
    return 1;


}
int goUmode();

int exec(int val){

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
  }while(c!='\0' && i<64);

  if(str[0]==0){
    printf("do_exec() file is null\n");
    return -1;
  }
    // Load the executable image into the running segment
    if(load(str, running->uss) == 0){
      printf("load did not work correctly..File: %s\n", str);
      return -1;
    }

    segment = running->uss;

            j=0;
            while(j<=12){

              cursor =  32*1024 - j*2;
           
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
            return 1;        

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
      printf("pid from freelist: %d\n", p->pid);

      p->ppid = running->pid; // Set the parent PID to the PID of the running proc
      p->parent = running;
      p->status = READY;
      p->priority = 1;
      p->next = 0;

      //Copy all file descriptors from parent
      for(j=0;j<NFD;j++){
        p->fd[j] = running->fd[j];
        if(p->fd[j] != 0){
          p->fd[j]->refCount++;
          if(p->fd[j]->mode == READ_PIPE){
            p->fd[j]->pipe_ptr->nreader++;
          }
          if(p->fd[j]->mode == WRITE_PIPE){
            p->fd[j]->pipe_ptr->nwriter++;
          }
        }
        
      }
      
      //clear all saved registers on stack
      for(j=1; j<10; j++){
          p->kstack[SSIZE-j] = 0;
      }

      p->ksp = &(p->kstack[SSIZE-9]);

      segment = (p->pid + 1)*0x1000;
     // end = cursor = segment + 0x1000; // Get to the HIGH end of the ustack

      enqueue(p, &readyQueue);

      if(file != 0){

      printf("File exists.. setting return address to body and loading file\n");

       p->kstack[SSIZE-1]=(int)body;

       load(file, segment);

          j=0;
          printf("Setting up the stack...\n");
            while(j<=12){
              cursor = 32*1024 - j*2;
           
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
     

     return p;
}

int fork()
{
         /* This do_fork() function is called only when fork is called from kmode */

            PROC *p;
            u16 segment, cursor;
            int j;

            printf("------- do_fork -------\n");

            p = kfork(0); // kfork from kernel

            if(p != 0){
              printf("Got a new proc.. PROC %d\n", p->pid);


              segment = (p->pid + 1)*0x1000;
              
              p->uss = segment;
              copy_image(segment);
            p->kstack[SSIZE-1] = (int)goUmode;
           // p->ksp = &(p->kstack[SSIZE-9]);
           
             j=0;
            while(j<=12){
              cursor = 32*1024 - j*2;
           
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

            }else{
              return -1;
            }
         
            p->usp = cursor;

           return p->pid;
}

int do_ps(){
    PROC * temp = procList;
    int count = 0;
    char status[32];

    printf("---------------- Processes ----------------\n");

    while(temp != 0){
      switch(temp->status){
        case 0:
        strcpy(&status, "FREE");
        break;
        case 1:
        strcpy(&status, "READY");
        break;
        case 2:
        strcpy( &status, "RUNNING");
        break;
        case 3:
        strcpy(&status, "STOPPED");
        break;
        case 4:
        strcpy( status, "SLEEP");
        break;
        case 5:
        strcpy(status, "ZOMBIE");
        break;
      }
      printf("%s [ %d]\nParent: %d\nStatus: %s\n\n\n", temp->name, temp->pid, (temp->parent == 0 ? -1 : temp->parent->pid), status, (temp->uss == 0 ? 0 : temp->uss));
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

int printStack(u16 segment){
        int j;
        u16 cursor = 0x1000 - 24;

          printf("------------- uSTACK : (%x) --------------\n", segment);
          printf("| uDS |  uES  | di | si | bp | dx | cx | bx | ax | uPC | uCS  | flag |\n");

          j=0;

          while(j<12){
             printf("%x ", get_word(segment, cursor));
             cursor+=2;
             j++;
          }
          printf("\n\n");

          return 1;
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
    printf("\n");
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