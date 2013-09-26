/*
Matt Hintke
CS460
Lab #2
MultiTasking System - w/Sleep, Wait, Wakeup
*/

//************ t.c file **********************************/
#define NPROC     9        
#define SSIZE  1024                /* kstack int size */

#define DEAD      0                /* proc status     */
#define READY     1      
#define FREE      2
#define ZOMBIE    3
#define SLEEP     4

typedef struct proc{
           struct proc *next;   
           int  ksp;               /* saved sp; offset = 2 */
           int  pid;
           int  ppid;
           int  priority;
           struct proc *parent;
           struct proc* pnext;
           int  exitValue;
           int  event;
           int  status;            /* READY|DEAD, etc */
           int  kstack[SSIZE];     // kmode stack of task
}PROC;

#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/

PROC proc[NPROC], *running, *readyQueue, *freeList, *procList, *sleepList;

int  procSize = sizeof(PROC);

/****************************************************************
 Initialize the proc's as shown:
        running ---> proc[0] -> proc[1];

        proc[1] to proc[N-1] form a circular list:

        proc[1] --> proc[2] ... --> proc[NPROC-1] -->
          ^                                         |
          |<---------------------------------------<-

        Each proc's kstack contains:
        retPC, ax, bx, cx, dx, bp, si, di, flag;  all 2 bytes
*****************************************************************/

int body();  
PROC * dequeue(PROC **queue);
PROC * getproc();

PROC* getproc(){
  return (dequeue(&freeList));
}


int initialize()
{
  int i, j;
  PROC *p;
  

  for (i=0; i < NPROC; i++){
    p = &proc[i];
    if(i==NPROC-1){
      p->next=0;
      p->pnext=0;
    }else{
     p->next = p->pnext = &proc[i+1];
    }
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    
    /*if (i){     // initialize kstack[ ] of proc[1] to proc[N-1]
      for (j=1; j<10; j++)
          p->kstack[SSIZE - j] = 0;          // all saved registers = 0
      p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
      p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    }*/
  }
  procList = freeList = &proc[0];

  running = getproc();

  running->pid = 0;
  running->status = READY;
  running->priority = 0;

  readyQueue = 0;
  
   //proc[NPROC-1].next = freeList; // The last PROC item points to the front of the freeList

  printf("initialization complete\n"); 
}

char *gasp[NPROC]={
     "Oh! You are killing me .......",
     "Oh! I am dying ...............", 
     "Oh! I am a goner .............", 
     "Bye! Bye! World...............",      
};

int kfork(){
           /*
            PROC *p = getproc();         // get a FREE proc from freeList
            if (p==0) return -1;  // no more PROCs in freeList
            --------------------------------------------------------
            initialize p's ppid
            initialize p's kstack[] AS IF it called tswitch() before
            enter p into readyQueue (by priority)
            return p->pid
            */
            int j;
            PROC *p = getproc();  // Set up a new PROC that points to the next free PROC in the list
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

            //fill in resume
            p->kstack[SSIZE-1] = (int)body;
            p->ksp = &(p->kstack[SSIZE-9]);

    
           enqueue(p, &readyQueue);
             
           return p->pid;
}
int do_exit(){
  int val;
  char c;

  printf("Enter an exit value (0-9): ");
  c = getc();
  val = c-'0';

  kexit(val);
}
int kexit(int value){
  PROC * temp;

  printf("KEXIT called on PROC with value %d...\n", running->pid, value);

  running->status = ZOMBIE;
  running->exitValue = value;

  temp = procList;

  // Check each proc to see if its parent is running
  while(temp!=0){
      if(temp->parent == running){
        temp->parent = running->parent; // Give away children of running to P1 (which should be the grandparent)
      }
      temp = temp->pnext;
  }
  
  // Now wake up the parent of the running proc before it dies

  wakeup(running->parent);
  printf("Woke up parent %d..", running->parent->pid);
  tswitch();
}

int hasChildren(PROC * process){
    PROC * list;
    int value = 0;

    list = procList;
    while(list!=0){
      if(list->parent == process){

        value = 1;
        break;
      }

      list = list->pnext;
    }

    return value;
}

int do_wait(){
  int status, ret;
  printf("Running proc %d is now waiting...\n", running->pid);

  ret = wait(&status);
 
  if(ret != -1){
    printf("Finished waiting on Zombie child PROC %d\n", ret);
  }else{
    printf("Error: no children exist\n");
  }

}
PROC *  getZombieChild(PROC * parent){
   PROC * list;
    PROC * value = 0;

    list = procList;
    while(list!=0){
      if(list->parent == parent && list->status == ZOMBIE){
        value = list;
        break;
      }

      list = list->pnext;
    }

    return value;
}

int wait(int * status){
  /* int wait(status) int *status;
           {
              if (no child)
                 return -1 for ERROR;
              while(1){
                 if (found a ZOMBIE child){
                    copy child's exitValue to *status;
                    free the ZOMBIE child PROC;
                    return dead child's pid;
                 }
                 sleep(running);    // sleep at its own &PROC
              }
            }*/
      PROC * zombie = 0;
    
      

      if(!hasChildren(running)){
       
        return -1;
      }

      while(1){
        zombie = getZombieChild(running);
        if(zombie != 0){
          printf("FOUND ZOMBIE.. KILLING PROC %d with exitValue %d\n", zombie->pid, zombie->exitValue);
          *status = zombie->exitValue;
          // free the ZOMBIE CHILD PROC ???
          zombie->status = FREE;
          enqueue(zombie, &freeList);

          return zombie->pid;
        }

      
        sleep(running); // sleep on its own address
      }
}



int do_sleep(){
  char c;
  int event;

  printf("Enter an event to sleep on (0-9): ");
  c = getc();
  event = c-'0';

  sleep(event);

  printf("..completed sleeping\n");

}
int sleep(int event){
  /*  sleep(int event)
       {
          running->event = event;      //Record event in PROC
          running->status = SLEEP;     // mark itself SLEEP
          // For fairness, put running into a FIFO sleepList so that they will wakeup in order
          tswitch();                   // not in readyQueue anymore
       } */

      running->event = event; // Record event in PROC
      running->status = SLEEP; // mark itself SLEEP
      enqueue(running, &sleepList); // Keep track in a SLEEP list
      printf("PROC %d is now sleeping on event %d\n", running->pid, event);
      tswitch();  // Switch to next ready PROC

}
int do_wakeup(){
  char c;
  int event;

  printf("Enter a wakeup event (0-9): ");
  c = getc();


  event = (int)(c-'0');
  printf("Waking up processes sleeping on event [%d]..\n", event);
  wakeup(event);

  printf("..completed wakeup\n");

}
int wakeup(int event){
  /* wakeup(int event)
       {
          for every proc p SLEEPing on this event do{
              // remove p from sleepList if you implement a sleepList
              p->status = READY;        // make it READY
              enqueue(&readyQueue, p)   // enter p into readyQueue (by pri)
          }
       }*/
        PROC *list = procList;
        while(list != 0){
        
          if(list->event == event && list->status == SLEEP){
              dequeue(&sleepList);
              list->status = READY; // Make it ready
              printf("Woke up PROC %d..\n", list->pid);

              enqueue(list, &readyQueue); // Enter this sleeping proc into readyQueue
          }
          list = list->pnext;
        }
        return 1;
}
int grave(){
  printf("\n*****************************************\n"); 
  printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
  printf("*****************************************\n");
  running->status = DEAD;
  enqueue(running, &freeList);

  tswitch();   /* journey of no return */        
}

int ps()
{
  printf("----------------------------------------------------------\n");
  printf("readyProcs = ");
  printQueue(readyQueue);
  printf("freeProcs = ");
  printQueue(freeList);
  printf("sleepProcs = ");
  printQueue(sleepList);
}

int body()
{  char c;
   while(1){
      printf("\n\n\n");
     ps();
      printf("I am Proc %d in body()\n", running->pid); 
      printf("Input a char : [s|q|f|z|a|w]: ");
       c=getc();
     
       switch(c){
            case 's': tswitch(); break;
            case 'q': do_exit();   break;
            case 'f': kfork();   break;
            case 'z': do_sleep();   break;
            case 'a': do_wakeup();  break;
            case 'w': do_wait();    break;
            default :            break;
       }
   }
}


main()
{
 /* initialize();
 PROC p[5];

 int i=0;
 for(i=0;i<5;i++){
  p[i].priority = i;
  p[i].pid = 5-i;
  enqueue(&p[i], &readyQueue);
 }
*/
 

 printf("\nWelcome to the 460 Multitasking System\n");
    initialize();
    printf("P0 forks P1\n");

  kfork();  // fork P1 
  printf("P0 switches to P1... calling tswitch()\n"); 
    tswitch();  // switches running to P1

  // Switch, Quit & Fork processes until all of them are dead except P0
  printf("Almost done... running is P%d\n", running->pid); 
  printf("P0 resumes: all dead, happy ending\n");

 // printf("Lets go to the %s to get %d %s for %d %ss\n", "store", 5, "steaks", 8, "dinner");
}


int scheduler()
{
   
   if(running->status == READY){
      enqueue(running, &readyQueue);
   }
    running = dequeue(&readyQueue);
    
}

 int enqueue(PROC *p, PROC **queue){
  
  PROC *temp = 0, *prev = 0;

  if(*queue == 0){
    *queue = p;
     p->next = 0;
  }else{
   
  // printf("temp: %d - p: %d\n", temp->priority, p->priority);

    if(p->priority > (*queue)->priority){
      p->next = (*queue);
      *queue = p;
    }else{

       temp = prev = *queue;
    while((temp) && temp->priority >= p->priority){
      prev = temp;
      temp = temp->next;
    }

    prev->next = p;
    p->next = temp;
   
   }


  }

  return 1;
 }


PROC * dequeue(PROC **queue) {
  PROC * p = 0;

  if(*queue == 0){
    p = 0;
  }else{

  p = *queue;
  *queue = (*queue)->next;

  }

  return p;
}

int printQueue(PROC *queue) {

  PROC *p = queue, *front = queue;
  //prints("\n------------- Print Queue -------------\n\r");

  while(p!=0)
  {
    printf("[ %d ] -> ", p->pid);
    p = p->next;
  }
  prints("NULL\n\n\r");

  return 1;
}
