PROC * dequeueSpecific(PROC ** queue, int pid){
    PROC * p = 0, * val = 0, *prev = 0;
    if(*queue == 0){
        val = 0;
    }else{

        p = *queue;
        
        while(p != 0){
            if(p->pid == pid){

                if(prev){
                  prev->next = p->next;
                }else{
                  (*queue) = (*queue)->next;
                }

                val = p;
                break;
            }

            prev = p;
            p = p->next;
        }

        
        return val;

    }
}
int enqueue(PROC *p, PROC **queue){
  
  PROC *temp = 0, *prev = 0;

 
  if(*queue == 0){
    *queue = p;
     p->next = 0;
  }else{
   
   

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



int ksleep(event) int event;
{
   	  running->event = event; // Record event in PROC
      running->status = SLEEP; // mark itself SLEEP
      enqueue(running, &sleepList); // Keep track in a SLEEP list
      printf("PROC %d is now sleeping on event %d\n", running->pid, event);
      tswitch();  // Switch to next ready PROC
}

/* wake up ALL procs sleeping on event */
int kwakeup(event) int event;
{
   PROC *list = procList;
   PROC * sleeper;
        while(list != 0){
        
          if(list->event == event && list->status == SLEEP){
              sleeper = dequeueSpecific(&sleepList, list->pid);
              list->status = READY; // Make it ready
              printf("Woke up PROC %d..\n", sleeper->pid);

              enqueue(list, &readyQueue); // Enter this sleeping proc into readyQueue
          }
          list = list->pnext;
        }
        return 1;
}


int kexit(value) int value;
{
   PROC * temp;
  
  if(running->pid == 1 && hasChildren(running)){
    printf("\nCannot kEXIT on Proc 1\n");
    return -1;
  }
  printf("KEXIT called on PROC %d with value %d...\n", running->pid, value);

  running->status = ZOMBIE;
  running->exitCode = value;
  
  temp = procList;

  // Check each proc to see if its parent is running
  while(temp!=0){
      if(temp->parent == running){
        temp->parent = running->parent; // Give away children of running to P1 (which should be the grandparent)
      }
      temp = temp->pnext;
  }
  
  // Now wake up the parent of the running proc before it dies

  kwakeup(running->parent);
 
  running->parent = 0;

  tswitch();
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

int kwait(status) int *status;
{
   PROC * zombie = 0;
    
      if(!hasChildren(running)){
       
        return -1;
      }

      while(1){
        zombie = getZombieChild(running);
        if(zombie != 0){
          printf("FOUND ZOMBIE.. KILLING Zombie PROC %d with exitValue %d\n", zombie->pid, zombie->exitCode);
          *status = zombie->exitCode;
          // free the ZOMBIE CHILD PROC ???
          zombie->status = FREE;
          zombie->parent = 0;
          enqueue(zombie, &freeList);

          return zombie->pid;
        }

      
        ksleep(running); // sleep on its own address
      }
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
    printf("\nThis process %s children\n", (value ? "has" : "has no"));

    return value;
}