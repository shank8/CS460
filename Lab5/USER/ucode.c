// ucode.c file
char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", 
             "fork", "exec", "pipe", "pfd", "read", "write", "close", 0};

int show_menu()
{
   printf("******************** Menu ***************************\n");
   printf("*  ps  chname  kmode  switch  wait  exit  fork  exec *\n");
          //   1     2      3       4      5     6    7     8 
   printf("*  pipe  pfd   read   write   close                 *\n");
	  //   9     10    11      12     13    
   printf("*****************************************************\n");
}

int find_cmd(name) char *name;
{
   int i = 0;   
   char *p = cmd[0];

   while (p){
 
     if (!strcmp(p, name)){
   
        return i;
      }
     p = cmd[++i];
   } 
   return(-1);
}


int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   syscall(1, 0, 0);
}

int chname()
{
    char s[64];
    printf("\ninput new name : ");
    gets(s);
    syscall(2, s, 0);
}

int kmode()
{
    printf("kmode : enter Kmode via INT 80\n");
    printf("proc %d going K mode ....\n", getpid());
        syscall(3, 0, 0);
    printf("proc %d back from Kernel\n", getpid());
}    

int kswitch()
{
    printf("proc %d enter Kernel to switch proc\n", getpid());
        syscall(4,0,0);
    printf("proc %d back from Kernel\n", getpid());
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n");
    return child; 
} 

int exit()
{
   char exitValue;
   printf("enter an exitValue (0-9) : ");
   exitValue=(getc()&0x7F) - '0';
   printf("enter kernel to die with exitValue=%d\n",exitValue);
   _kexit(exitValue);
}

int _kexit(int exitValue)
{
  syscall(6,exitValue,0);
}

int fork()
{
  int child;
  child = syscall(7,0,0,0);
  if (child)
    printf("parent % return form fork, child=%d\n", getpid(), child);
  else
    printf("child %d return from fork, child=%d\n", getpid(), child);
}

int exec()
{
  int r;
  char filename[32];
  printf("enter exec filename : ");
  gets(filename);
  r = syscall(8,filename,0,0);
  printf("exec failed\n");
}

int pd[2];

int pipe()
{
   printf("pipe syscall\n");
   syscall(30, pd, 0);
   printf("proc %d created a pipe with fd = %d %d\n", 
           getpid(), pd[0], pd[1]);
}

int pfd()
{
  syscall(34,0,0,0);
}
  
int read_pipe()
{
   int fd;
   int n;
  int nbytes;
  char byteString[32];
  char data[1024];

  pfd();
  printf("What fd would you like to read from? ");
  fd = (getc()&0x7F) - '0';
  putc(fd+'0');
  getc();
  printf("\n");

  printf("How many bytes should be read? ");
  gets(byteString);
  nbytes = atoi(byteString);
  printf("\n");

  n = syscall(31, fd, data, nbytes);

  if (n>=0){
     printf("proc %d back to Umode, read %d bytes from pipe : ",
             getpid(), n);
     data[n]=0;
     printf("%s\n", data);
  }
  else
    printf("read pipe failed\n");
}

int write_pipe()
{
     int fd, nbytes, n;
    char data[1024];

    pfd();
    printf("What fd would you like to write to? ");
    fd = (getc()&0x7F) - '0';
    putc(fd+'0');
    getc();
    printf("\n");
    
    printf("Enter a string to write: ");
    gets(data);

    nbytes = strlen(data);
    printf("fd=%d nbytes=%d : %s\n", fd, nbytes, data);

    n = syscall(32, fd, data, nbytes);


  if (n>=0){
     printf("\nproc %d back to Umode, wrote %d bytes to pipe\n", getpid(),n);
  }
  else
    printf("write pipe failed\n");
}

int close_pipe()
{
  int fd;

  pfd();
  printf("What fd would you like to close? ");
  fd = (getc()&0x7F) - '0';
  printf("%d", fd);
  getc();

  syscall(33, fd, 0);
}

int invalid(name) char *name;
{
    printf("Invalid command : %s\n", name);
}