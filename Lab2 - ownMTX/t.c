/************ t.c file **********************************/
#define NPROC     9        
#define SSIZE  1024                /* kstack int size */

#define DEAD      0                /* proc status     */
#define READY     1      
#define FREE      2

typedef struct proc{
    struct proc *next;   
  
           int  ksp;               /* saved sp; offset = 2 */
           int  pid;
           int ppid;
           int  priority;
          struct proc *parent;
           int  status;            /* READY|DEAD, etc */
           int  kstack[SSIZE];     // kmode stack of task
}PROC;

#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/

PROC proc[NPROC], *running, *readyQueue, *freeList;

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
    p->next = &proc[i+1];
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
  freeList = &proc[0];


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
              printf("NO MORE LEFT TO GIVE\n");
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

  printf("running = %d\n", running->pid);

  printf("readyProcs:");
  printQueue(readyQueue);
printf("\n\n");
  printf("freeProcs:");
  printQueue(freeList);
}

int body()
{  char c;
   while(1){
     ps();
     
      printf("I am Proc %d in body()\n", running->pid); 
      printf("Input a char : [s|q|f] ");
       c=getc();
       switch(c){
            case 's': tswitch(); break;
            case 'q': grave();   break;
            case 'f': kfork();   break;
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
      printf("ENQUEUING RUNNING PROC\n");
      enqueue(running, &readyQueue);
   }
    running = dequeue(&readyQueue);
    if(running){
    printf("\n-----------------------------\n");
    printf("next running proc = %d\n", running->pid);
    printf("-----------------------------\n");
  }
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
    printf("[ %d-%d ] -> ", p->pid, p->priority);
    p = p->next;
  }
  prints("NULL\n\n\r");

  return 1;
}