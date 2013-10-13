#include "type.h"



/********* in type.h ***********
typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NULL     0
#define NPROC    9
#define SSIZE 1024

//******* PROC status ********
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

typedef struct proc{
    struct proc *next;
    int    *ksp;

    int    uss, usp;
    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;

    char   name[32];
    int    kstack[SSIZE];      // per proc stack area
}PROC;
*******************************/
PIPE pipe[NPIPE];
OFTE ofte[NOFT];

PROC proc[NPROC], MainProc, *running, *freeList, *readyQueue, *sleepList, *procList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */
  
#include "int.c"
#include "wait.c"
#include "kernel.c"
#include "pipe.c"


/*PROC * dequeue(PROC **queue) {
  PROC * p = 0;

  if(*queue == 0){
    p = 0;
  }else{

  p = *queue;
  *queue = (*queue)->next;

  }

  return p;
}*/



int init()
{
    PROC *p; int i, j;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        p->parent = 0;
        for(j=0;j<NFD;j++){
           p->fd[j] = 0;
        }
       
        strcpy(proc[i].name, pname[i]);
   
        p->pnext = p->next = &proc[i+1];
    }
    procList = freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = proc[NPROC-1].pnext = 0;
    readyQueue = sleepList = 0;


    /**** create P0 as running ******/
    p = get_proc(&freeList);


    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{

  u16 segment, cursor;
  int j;

    if (running->status == RUNNING){
    	
     	
        running->status = READY;
        enqueue(running, &readyQueue);
    }
    

    running = dequeue(&readyQueue);
    running->status = RUNNING;

}

int int80h();

int set_vec(vector, handler) u16 vector, handler;
{
     // put_word(word, segment, offset) in mtxlib
     put_word(handler, 0, vector<<2);
     put_word(0x1000,  0, (vector<<2) + 2);
}
            
main()
{
    printf("MTX starts in main()\n");

 
    init();      // initialize and create P0 as running
    printList("ReadyQueue", readyQueue);
    set_vec(80, int80h);
    printf("PROC 0: %d\n", running->priority);

    kfork("/bin/u1");     // P0 kfork() P1
  	

    while(1){
      printf("P0 running\n");
      if (nproc==2 && proc[1].status != READY)
	  printf("no runable process, system halts\n");
      while(!readyQueue){
       
      }

      printf("P0 switch process\n");
    
      tswitch();   // P0 switch to run P1
   }
}
