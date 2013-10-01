#include "type.h"
PROC *getproc()
{
	PROC *tmp;
	tmp = dequeue(&freeList);
	return tmp;
}
int goUmode();
int copy_image(u16 srcSegment, u16 dstSegment, u16 size)
{
	u16 offset;
	int word;
	offset = 0;
	while (offset < size)
	{
		// Read in the word from the current segment
		word = get_word(srcSegment, offset);
		// Write word in the child segment
		put_word(word, dstSegment, offset);
		// Increment offset (it seems to work with +1 or +2....)
		offset++;
		//offset += 2;
	}
	return 0;
}
/****************************************************************
  fork creates a child process ready to run in Umode
  In addition, it loads a copy of the parents image 
 ****************************************************************/
int do_fork()
{
	PROC *child;
	int i, childPid;
	u16 word, segment, *offset;
	childPid = kfork(0);		// kfork() but do NOT load any Umode image for child
	
	if (childPid < 0)
	{					// kfork failed 
		return -1;
	}

	child = &proc[childPid];	// we can do this because of static pid
	segment = (childPid + 1) * 0x1000;
	child->usp = running->usp;
//printf("running->uss = %d, child->uss = %d\n", running->uss, segment);
	copy_image(running->uss, segment, 0x1000);//copyImage(running->uss, segment, (32 * 1024));
	// Make the child runnable in User mode
	child->kstack[SSIZE-1]=(int)goUmode;	// called tswitch() from goUmode
	for (i = 1; i < 13; i++)	// i from 1 to 12
	{
		offset = 0x1000 - (2 * i); // offset RELATIVE to current segment: end of segment - number*2 because stored as 2bytes size
		switch (i)	// Set word that will be passed into put_word()
		{
			case 1 : word = 0x0200;		// flag
				break;
			case 2 : word = segment;	// uCS
				break;
			case 3 : word = 0;			// uPC
				break;
			case 4 : word = 0;			// ax
				break;
			case 5 : word = 0;			// bx
				break;
			case 6 : word = 0;			// cx
				break;
			case 7 : word = 0;			// dx
				break;
			case 8 : word = 0;			// bp
				break;
			case 9 : word = 0;			// si
				break;
			case 10 : word = 0;			// di
				break;
			case 11 : word = segment;	// uES
				break;
			case 12 : word = segment;	// uDS
				child->usp = offset;	// Set PROC->uSS
				break;
		}
		put_word(word, segment, offset); // write the WORD to whatever SEGMENT, relative to the OFFSET
	}
	/**** ADD these : copy file descriptors ****
	for (i=0; i<NFD; i++)
	{
		child->fd[i] = running->fd[i];
		if (p->fd[i] != 0)
		{
				child->fd[i]->refCount++;
				if (child->fd[i]->mode == READ_PIPE)
					child->fd[i]->pipe_ptr->nreader++;
				if (child->fd[i]->mode == WRITE_PIPE)
					child->fd[i]->pipe_ptr->nwriter++;
		}
	}*/
	return childPid;
}
int do_exec(int b)
{
	char filename[64];
	char tmp;
	int i, returnValue;
	u16 segment, word, *offset;
	do
	{
		tmp = get_byte(running->uss, b);
		filename[i] = tmp;
		i++;
		b++;
	} while ((tmp != 0) && (i < 64));
	// check the parameter
	if (filename[0] == 0)
	{
		printf("do_exec(): Parameter cannot be null, aborting.\n");
		return -1;
	}
	// load file into segment
	returnValue = load(filename, running->uss);
	if (0 == returnValue)
	{
		printf("load returned 0.\n", returnValue);
		return -1;
	}
	// (a). re-establish ustack to the very high end of the segment.
	for (i = 1; i < 13; i++)	// i from 1 to 12
	{
		offset = 0x1000 - (2 * i); // offset RELATIVE to current segment: end of segment - number*2 because stored as 2bytes size
		switch (i)	// Set word that will be passed into put_word()
		{
			case 1 : word = 0x0200;		// flag
				break;
			case 2 : word = segment;	// uCS
				break;
			case 3 : word = 0;			// uPC
				break;
			case 4 : word = 0;			// ax
				break;
			case 5 : word = 0;			// bx
				break;
			case 6 : word = 0;			// cx
				break;
			case 7 : word = 0;			// dx
				break;
			case 8 : word = 0;			// bp
				break;
			case 9 : word = 0;			// si
				break;
			case 10 : word = 0;			// di
				break;
			case 11 : word = segment;	// uES
				break;
			case 12 : word = segment;	// uDS
				running->usp = offset;	// Set PROC->uSS
				break;
		}
		put_word(word, segment, offset); // write the WORD to whatever SEGMENT, relative to the OFFSET
	}
	return 0;
}
/****************************************************************
  kfork creates a child process ready to run in Kmode from body()
  In addition, it loads u1 file to the child's Umode segment 
 ****************************************************************/
int kfork(char *filename)
{
	u16 segment, word, *offset;
	int i;
	PROC *child;
	printf("\n*****************************************\n"); 
	printf("Kfork\n");
	printf("*****************************************\n");
	//child = getproc(); // get a FREE proc from freeList
	child = dequeue(&freeList);
	if (child == 0)
	{
		printf("No more processes in the free list.\n");
		return -1;  // no more PROCs in freeList
	}
	child->ppid = running->pid; // set parent pid ptr
	printf("PROC %d's parent is PROC %d.\n", child->pid, child->ppid);
	child->parent = running; // set parent PROC pointer to parent
	child->priority = 1;
	// initialize kstack[ ]
	for (i = 1; i < 10; i++) // i = 1 to 9
    {
    	child->kstack[SSIZE - i] = 0;
    }
	child->ksp = &(child->kstack[SSIZE-9]);	// ksp -> kstack top
	child->status = READY;
	printf("kfork: child = %d(%d)\n", child->pid, child->priority);
	//enter p into readyQueue (by priority)
	enqueueReadyQueue(&readyQueue, child); //enqueue(&readyQueue, child);
	segment = (child->pid + 1)*0x1000; // set equal to address where this PROC's segment begins
	child->uss = segment;	// Set PROC->uss to childs segment
    if (filename != 0)
    {
//printf("kfork(): filename != 0\n");
    	child->kstack[SSIZE-1]=(int)body;		// called tswitch() from body
		// call load(filename, segment) in mtxlib to load filename (/bin/u1) to segment. Thus,
		// every newly forked proc has u1 as its Umode image BUT in its own segment.
		load(filename, segment);
		// initialize ustack[ ]
/**************************************************************************
  (LOW) |uSP       by save in int80h ----->| by INT 80  |     HIGH
 --------|-----------------------------------------------------------------
       |uDS|uES| di| si| bp| dx| cx| bx| ax|uPC|uCS|flag|XXXXXXXXXXXXXXXXX
 --------------------------------------------------------------------------
        -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1 | 0 
 **************************************************************************/
		for (i = 1; i < 13; i++)	// i from 1 to 12
		{
			offset = 0x1000 - (2 * i); // offset RELATIVE to current segment: end of segment - number*2 because stored as 2bytes size
			switch (i)	// Set word that will be passed into put_word()
			{
				case 1 : word = 0x0200;		// flag
					break;
				case 2 : word = segment;	// uCS
					break;
				case 3 : word = 0;			// uPC
					break;
				case 4 : word = 0;			// ax
					break;
				case 5 : word = 0;			// bx
					break;
				case 6 : word = 0;			// cx
					break;
				case 7 : word = 0;			// dx
					break;
				case 8 : word = 0;			// bp
					break;
				case 9 : word = 0;			// si
					break;
				case 10 : word = 0;			// di
					break;
				case 11 : word = segment;	// uES
					break;
				case 12 : word = segment;	// uDS
					child->usp = offset;	// Set PROC->uSS
					break;
			}
			put_word(word, segment, offset); // write the WORD to whatever SEGMENT, relative to the OFFSET
		}
	}
	else
	{
//printf("kfork(): filename == 0\n");
	}
	return child->pid;
}
int body()
{
	char c;
	printf("proc %d resumes to body()\n", running->pid);
	while(1)
	{
		printf("-----------------------------------------\n");
		printList("freelist  ", freeList);
		printList("readyQueue", readyQueue);
		printList("sleepList ", sleepList); // if you use a sleepList
		printf("-----------------------------------------\n");
		printf("s=switch   f=fork   w=wait   q=quit   u=goUmode\n");
		printf("proc %d running: parent = %d  enter a char [s|f|w|q|u] : ", running->pid, running->parent->pid);
		c = getc();
		printf("%c\n", c);
		switch(c)
		{
			case 's' : do_tswitch();   break;
			case 'f' : do_kfork();     break;
			case 'w' : do_wait();      break;
			case 'q' : do_exit(99);      break;
			case 'u' : goUmode();      break;
		}
	}
}
int do_tswitch()
{
	tswitch();
	return 0;
}
int do_kfork()
{
	int value;
	value = kfork("/bin/u1"); // is this the correct parameter to pass to kfork()?
	return value;
}
int do_wait()
{
	int status, returnValue;
	returnValue = kwait(&status);
	printf("kwait() returned on pid=%d\n", returnValue);
	return 0;
}
int do_exit(int bx)
{
	int returnValue;
	char input;
	int exitCode;
	if (bx != 99) // If called from umode then bx will not be 99 (it will be 0-9).
	{
		returnValue = kexit(bx);
	}
	else	// Exit was called from kmode, so we need to get exit value.
	{
		exitCode = -1;
		while ((exitCode < 0) || (exitCode > 10))
		{
			printf("Enter exit value (0-9): ");
			input = getc();
			printf("%c\n", input);
			exitCode = (int)(input-'0');
		}
		returnValue = kexit(exitCode);
	}
	return 0;
}
int do_ps()
{
	int i;
	/*// Print out running PROC, freelist, readyqueue, sleeplist
	printf("Running PROC = %s: %d(%d).\n", running->name, running->pid, running->priority);
	printf("Free List:\t");
	printQueue(&freeList);
	printf("Ready Queue:\t");
	printQueue(&readyQueue);
	printf("Sleep List:\t");
	printQueue(&sleepList);*/
	i = 0;
	while (i < NPROC)
	{
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		i++;
	}
}
int kmode()
{
	running->kstack[SSIZE-1]=(int)body;
	return 0;
}
int chname(int s)
{
	char name[64];
	char tmp;
	int i;
	tmp = '>';
	while ((tmp != 0) && (i < 64))
	{
		tmp = get_byte(running->uss, s);
		name[i] = tmp;
		i++;
		s++;
	}
	printf("Changing name of PROC %d(%d) to \"%s\"\n", running->pid, running->priority, &name[0]);
	strcpy(&(running->name[0]), &name[0]);
	return 0;
}