char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

show_pipe(PIPE *pp)
{
   int i, j;

   printf("------------ PIPE CONTENTS ------------\n");     
   // print pipe information
    printf("<");
    for(i=0;i<PIPE_SIZE;i++){
      // PRINT the contents where a '.' denotes an empty cell
      if(pp->buf[i] == 0){
        putc('.');
      }else{
        putc(pp->buf[i]);
      }
    }
   printf(">\n");
   printf("\n----------------------------------------\n");

   printf("Room: %d\n", pp->room);
   printf("Data: %d\n", pp->data);
}



int pfd()
{
  // print running process' opened file descriptors
  int i = 0;
  char mode[32], type[32];
  printf("   fd      type      mode  \n");
  printf("  ----    ------    ------\n");

  while(i < NFD){
    if(running->fd[i]!=0){
      // SET THE MODE
      switch(running->fd[i]->mode){
        case READ_PIPE:
        strcpy(mode, "READ");
        break;
        case WRITE_PIPE:
        strcpy(mode, "WRITE");
        break;
        default: 
        strcpy(mode, "N/A");
        break;
      }
    

      printf("   %d     %s    %s(%d)\n", i, "PIPE", mode, running->fd[i]->mode);
    }

    i++;
  }

  return 1;
}

int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe()
  PIPE *pipe;
  char * offset;

  int rbytes = 0;
  u8 byte;

  offset = buf;
  pipe = running->fd[fd]->pipe_ptr;


    
    if(running->fd[fd] ==0){
      printf("Error: fd is not open\n");
      return -1;
    }

    if(running->fd[fd]->mode != READ_PIPE){
      printf("Error: fd mode is not read\n");
      return -2;
    }

    printf("BEFORE READ:\n");
    show_pipe(pipe);
    while(rbytes < n){
          if(pipe->nwriter == 0){
      /*  read as much as it can; either nbytes or until no more data.
                                       return ACTUAL number of bytes read. */

            // Put the byte into umode, then destroy it in the pipe
            printf("NO WRITER\n");
            if(pipe->data != 0){

              byte = pipe->buf[pipe->tail];
               printf("READ BYTE: %c[%d]\n", byte, pipe->tail);
              put_byte(byte, running->uss, offset);
              pipe->buf[pipe->tail] = 0;
              pipe->room++;
              pipe->data--;
              rbytes++;
              offset++;
              pipe->tail++;
              pipe->tail %= PIPE_SIZE;
            }else{
              printf("AFTER READ:\n");
              show_pipe(pipe);
              return rbytes;
            }

           
           // ksleep(&(pipe->data));
      }else{
          if(pipe->data != 0){

              byte = pipe->buf[pipe->tail];
              printf("READ BYTE: %c[%d]\n", byte, pipe->tail);
              put_byte(byte, running->uss, offset);
              pipe->buf[pipe->tail] = 0;
              pipe->room++;
              pipe->data--;
              rbytes++;
              offset++;
              pipe->tail++;
              pipe->tail %= PIPE_SIZE;
              kwakeup(&(pipe->room));
          }else{
            kwakeup(&(pipe->room));
            ksleep(&(pipe->data));
          }

      }

    }
    printf("AFTER READ:\n");
              show_pipe(pipe);
    return rbytes;
}

int write_pipe(int fd, char *buf, int n)
{
  // your code for write_pipe()
   PIPE *pipe;
  char * offset;

  int wbytes = 0;
  u8 byte;

  offset = buf;
  pipe = running->fd[fd]->pipe_ptr;

       
    if(running->fd[fd] ==0){
      printf("Error: fd is not open\n");
      return -1;
    }

    if(running->fd[fd]->mode != WRITE_PIPE){
      printf("Error: fd mode is not write\n");
      return -2;
    }
     printf("BEFORE WRITE:\n");
        show_pipe(pipe);
        
    while(wbytes < n){

        if(pipe->nreader == 0){
          printf("NO READER\n");
            return BROKEN_PIPE_ERROR;
         }else{
            if(pipe->room != 0){
              byte = get_byte(running->uss, offset);
               printf("WRITE BYTE: %c[%d]\n", byte, pipe->head);
              pipe->buf[pipe->head] = byte;
              pipe->head++;
              offset++;
              wbytes++;
              pipe->room--;
              pipe->data++;
              pipe->head %= PIPE_SIZE;
              kwakeup(&(pipe->data));
            }else{
               kwakeup(&(pipe->data));
               ksleep(&(pipe->room));
            }
         }
          

    }
     printf("AFTER WRITE:\n");
        show_pipe(pipe);
    return wbytes;
   
}
PIPE * getPipe(){
  int i = 0;
  while(i < NPIPE){
    if(pipe[i].busy == 0){
      return &pipe[i];
    }
    i++;
  }
  printf("No FREE pipes available...Sorry\n");
  return 0;
  
}
int getOFT(OFTE **read, OFTE **write){
  int i = 0;
  int num = 0;
  while(i < NOFT){
    if(ofte[i].refCount == 0){
      switch(num){
        case 0:
              printf("oft[%d]: %x\n", i, &ofte[i]);
               *read = &ofte[i];
               break;
        case 1:
             printf("oft[%d]: %x\n", i, &ofte[i]);
              *write = &ofte[i];
              break;
        case 2:
              return 0;
      }
      num++;
    }
    i++;

  }

  return 1;
}
int kpipe(int pd)
{
  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  OFTE * read, * write;
  PIPE *newpipe;
  u16 i = 0;

  newpipe = getPipe();
  if(newpipe == 0){
    return 0;
  }
  if(!getOFT(&read, &write)){
    printf("getOFT success");
  }
  printf("ref: %d\n", read->refCount);

  printf("About to pipe running PROC %d\n", running->pid);
  // Get the next open FD spot and fill it with the reader OFTE
  while(running->fd[i] != 0){
    i++;
  }
  running->fd[i] = read;
  printf("Putting word (%d) into address %x\n", i, pd);
  put_word(i, running->uss, pd);

  // Continue to the next open spot and fill it with the writer OFTE
  pd+=2;

  while(running->fd[i] != 0){
    i++;
  }
  running->fd[i] = write;
    printf("Putting word (%d) into address %x\n", i, pd);
  put_word(i, running->uss, pd);

  // Set both the reader and writer to the pipe
  read->pipe_ptr = write->pipe_ptr = pipe;

  // Init the OFTE structs
  read->mode = READ_PIPE;
  write->mode = WRITE_PIPE;
  printf("mode: %d\n", read->mode);
  
  printf("OFT = %x\n", read);
  printf("OFT = %x\n", write);
  read->refCount = 1;
  write->refCount = 1;

  // Init our pipe
  for(i=0;i<PIPE_SIZE;i++){
    pipe->buf[i] = 0;
  }
  pipe->head = 0;
  pipe->tail = 0;
  pipe->room = PIPE_SIZE;
  pipe->data = 0;
  pipe->nreader = 1;
  pipe->nwriter = 1;
  pipe->busy = 1;

}

int close_pipe(int fd)
{
  OFTE *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry 

  if (op->mode == READ_PIPE){
      pp = op->pipe_ptr;
      pp->nreader--;                   // dec n reader by 1

      if (--op->refCount == 0){        // last reader
	if (pp->nwriter <= 0){         // no more writers
	     pp->busy = 0;             // free the pipe   
             return;
        }
      }
      kwakeup(&pp->room); 
      return;
  }

  if (op->mode == WRITE_PIPE){
      pp = op->pipe_ptr;
      pp->nwriter--;                   // dec nwriter by 1

      if (--op->refCount == 0){        // last writer 
	if (pp->nreader <= 0){         // no more readers 
	    pp->busy = 0;              // free pipe also 
            return;
        }
      }
      kwakeup(&pp->data);
      return;
  }
}