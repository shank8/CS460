typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC    9
#define SSIZE 1024

#define PIPE_SIZE 10
#define NPIPE 10
#define NFD 10
#define NOFT 20

// OFTE Modes
#define READ 0
#define WRITE 1
#define READ_PIPE 2
#define WRITE_PIPE 3

#define PROC_NAME_SIZE 64

// PIPE Status
//#define FREE 0
#define IN_USE 1


/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5



typedef struct pipe{
     char  buf[PIPE_SIZE];
     int   head, tail, data, room;
     int   nreader, nwriter; // number of readers, writers
     int   busy;   /* IN_USE or FREE */
}PIPE;

typedef struct ofte{
     int mode;        /* READ, WRITE, READ_PIPE, WRITE_PIPE, etc */  
     int refCount;    

     struct pipe *pipe_ptr;

     // INODE    *indoePtr;
     //long offset;   /* for ordinary files */      
}OFTE;

typedef struct proc
{
    struct proc *next;
    u16  *ksp; /* offset = 2 bytes */
    int   uss, usp;  // at 4, 6
    u16   pid;       /* pid = 0 to NPROC-1 */
    u16   status; 
    u16   priority;      /* scheduling priority */
    u16   ppid;     /* parent pid */
    struct proc *pnext;
    struct proc *parent;
    u16    event;
    u16    exitCode;
    char   name[PROC_NAME_SIZE];
    OFTE    *fd[NFD];  
    u16    kstack[SSIZE];   // Kmode per process stack
}PROC;