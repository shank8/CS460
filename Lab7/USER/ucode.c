typedef unsigned short ushort;

int pid;
char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "die", 
             "fork", "exec", "in", "out", 0};

int show_menu()
{
   printf("\n************************* Menu *********************\n");
   printf("* ps chname kmode switch wait die fork exec in out *\n");
   /*        1    2      3     4     5    6   7    8   9  10    */
   printf("******************************************************\n");
}

int find_cmd(name) char *name;
{
   int i;   char *p;
   i = 0;   p = cmd[0];
   while (p){
         if (strcmp(p, name)==0)
            return i;
         i++;  p = cmd[i];
   } 
   return(-1);
}

int get_pid()
{
   return int80(0,0,0);
}

int ps()
{
   int80(1, 0, 0);
}

int chname()
{
    char s[64];
    printf("input new name : ");
    mgets(s);
    int80(2, s, 0);
}

int kmode()
{
    int80(3, 0, 0);
}    

int kswitch()
{
    int80(4,0,0);
}

int mywait()
{
    int child, status;
    child = int80(5,&status,0);
    printf("%d back from wait, dead child=%d\n", child);
}
  
int myexit()
{
    int80(6,0,0);
}

int ufork()
{  
   int child;
   child = int80(7, 0, 0);
   printf("%d forked a child %d\n", getpid(), child);
}

int myexec(char *file)
{
    int80(8, file, 0);
}

int sout()
{
  int port;
  printf("input port number:[0|1]");
  port = getc()&0x7F - '0';
  int80(9, port, "serial line from Umode\n");
}

char uline[64];

int sin()
{
  int port;
  printf("input port number:[0|1]");
  port = getc()&0x7F - '0';
  int80(10, port, uline, 0);
  printf("uline=%s\n", uline);
}

int invalid(char *name)
{
  printf("Invalid command %s\n",name);
}
