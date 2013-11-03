typedef unsigned short ushort;
typedef unsigned long  ulong;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;


#define NFD 12


#define NULL      0
#define NPROC     9
#define SSIZE   512

#define FREE      0    /* PROC status */
#define READY     1 
#define SLEEP     2
#define ZOMBIE    3
#define BLOCK     4

typedef struct proc{
        struct proc *next;
        int    *ksp; 
        int    uss,usp;
        int    inkmode;

        struct proc *pnext;
        struct proc *parent;
        int     pid;
        int     ppid;
        int     status;
        int     exitCode;
        int     pri;          
        char    name[32];    
        int     event;

        int kstack[SSIZE];
}PROC;        
