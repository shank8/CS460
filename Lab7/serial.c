// serial.c file for SERIAL LAB ASSIGNEMNT
/**************** CONSTANTS ***********************/
#define INBUFLEN    80
#define OUTBUFLEN   80
#define EBUFLEN     10
#define NULLCHAR     0
#define BEEP         7
#define BACKSPACE    8

#define NR_STTY      2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */

/**** The serial terminal data structure ****/

struct stty {
   /* input buffer */
   char inbuf[INBUFLEN];
   int inhead, intail;
  struct semaphore inchars;

   /* output buffer */
   char outbuf[OUTBUFLEN];
   int outhead, outtail;
  struct semaphore outroom;

   int tx_on;
   
   /* echo buffer */
   char ebuf[EBUFLEN];
   int ehead, etail, e_count;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;
   
   /* I/O port base address */
   int port;
} stty[NR_STTY];


/********  bgetc()/bputc() by polling *********/
int bputc(port, c) int port, c;
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

int bgetc(port) int port;
{
    while ((in_byte(port+LSR) & 0x01) == 0);
    return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(irq_nr) unsigned irq_nr;
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}

   
/************ serial ports initialization ***************/
char *p = "\n\rSerial Port Ready\n\r\007";

int sinit()
{
  int i;  
  struct stty *t;
  char *q; 

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++){
    q = p;

    printf("sinit : port # %d\n", i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
          t->port = 0x3F8;    /* COM1 base address */
      else
          t->port = 0x2F8;    /* COM2 base address */  

      t->inchars.value  = 0;  t->inchars.queue = 0;
      t->outroom.value = OUTBUFLEN; t->outroom.queue = 0;

      t->inhead = t->intail = 0;
      t->ehead =  t->etail = t->e_count = 0;
      t->outhead =t->outtail = 0;

      t->tx_on = 0;

      // initialize control chars; NOT used in MTX but show how anyway
      t->ison = t->echo = 1;   /* is on and echoing */
      t->erase = '\b';
      t->kill  = '@';
      t->intr  = (char)0177;  /* del */
      t->quit  = (char)034;   /* control-C */
      t->x_on  = (char)021;   /* control-Q */
      t->x_off = (char)023;   /* control-S */
      t->eof   = (char)004;   /* control-D */

    lock();  // CLI; no interrupts

      //out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */ 
      out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */
      out_byte(t->port+DIVH, 0x00);
      out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */

      /******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/ 
      out_byte(t->port+LCR, 0x03); 

      /*******************************************************************
        Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        let modem respond as a DCE.  Here we must let the (crossed)
        cable tell the TVI terminal that the "DCE" has DSR and CTS.  
        So we turn the port's DTR and RTS on.
      ********************************************************************/

      out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    unlock();
    
    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    //    prints(q);

    while (*q){
      bputc(t->port, *q);
      q++;
    }
  }
}  
         
//======================== LOWER HALF ROUTINES ===============================
int s0handler()
{
  shandler(0);
}
int s1handler()
{
  shandler(1);
}

int shandler(int port)
{  
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;

   t = &stty[port];            /* IRQ 4 interrupt : COM1 = stty[0] */

   IntID     = in_byte(t->port+IIR);       /* read InterruptID Reg */
   LineStatus= in_byte(t->port+LSR);       /* read LineStatus  Reg */    
   ModemStatus=in_byte(t->port+MSR);       /* read ModemStatus Reg */

   intType = IntID & 7;     /* mask out all except the lowest 3 bits */
   switch(intType){
      case 6 : do_errors(t);  break;   /* 110 = errors */
      case 4 : do_rx(t);      break;   /* 100 = rx interrupt */
      case 2 : do_tx(t);      break;   /* 010 = tx interrupt */
      case 0 : do_modem(t);   break;   /* 000 = modem interrupt */
   }
   out_byte(0x20, 0x20);     /* reenable the 8259 controller */ 
}

int do_errors()
{ printf("assume no error\n"); }

int do_modem()
{  printf("don't have a modem\n"); }


/* The following show how to enable and disable Tx interrupts */

enable_tx(struct stty *t)
{
  lock();
  out_byte(t->port+IER, 0x03);   /* 0011 ==> both tx and rx on */
  t->tx_on = 1;
  unlock();
}

disable_tx(struct stty *t)
{ 
  lock();
  out_byte(t->port+IER, 0x01);   /* 0001 ==> tx off, rx on */
  t->tx_on = 0;
  unlock();
}


/******** echo char to RS232 **********/
int secho(struct stty *tty, int c)
{
   /* insert c into ebuf[]; turn on tx interrupt */
   lock();
   tty->ebuf[ehead++] = c;
   ehead %= EBUFLEN;
   enable_tx(tty);
   unlock();
}

int do_rx(struct stty *tty)   /* interrupts already disabled */
{ 
  int c;
  c = in_byte(tty->port) & 0x7F;  /* read the ASCII char from port */

  printf("port %x interrupt : c=%c\n", tty->port, c);

  /********* WRITE CODE ***************
   put char into stty[port]'s inbuf[ ]
  *************************************/

  /****** This code segment uses bput() to echo each input char ************
   It is for YOUR initial testing only. In YOUR serial port driver, MUST use
   the echo buffer to echo inputs.

   bputc(tty->port, c);
   if (c=='\r')
      bputc(tty->port, '\n');
  ************************************************************************/
  
  V(&tty->inchars);     /* unblock any process waiting in sgetc() */
}      

     
int sgetc(struct stty *tty)
{ 
  int c;
  P(&tty->inchars);   /* wait if no input char yet */
 lock();
  // WRITE CODE TO get a char c from tty->inbuf[]
  c = tty->inbuf[intail++];

  intail %= INBUFLEN;

 unlock();
  return(c);
}

int sgetline(int port, char *line)
{  
   struct stty *tty = &stty[port];
 
   char c;
   int i = 0;
   printf("sgetline from port %d\n", port);

   // WRITE CODE to get a line from serial port

   do{
      c = sgetc(tty);
      line[i] = c;

   }while(line[i++] != '\0');

   return strlen(line);
}


/******************************************************************/
int do_tx(struct stty *tty)
{
  int c;
  printf("tx interrupt ");


  /************** WRITE CODE TO DO THESE *********************
  (1). if (nothing to output){ 
          disable tx interrupt;
          return;
       }
  (2)  if (ebuf[] not empty){
          out_put a byte from ebuf[ ];
          return;    
       } 

  (3). // oubuf[ ] has chars
       out_put a char from outbuf[ ];
       V(&tty->outroom);
       return;
  ************************************************************/
}

int sputc(struct stty *tty, int c)
{
    P(&tty->outroom);   

    lock();             
    //  WRITE CODE to enter c into outbuf[ ];
    tty->outbuf[outhead++] = (char)c;
    outhead %= OUTBUFLEN;

    unlock();

    if (!tty->tx_on) 
       enable_tx(tty);
    return(1);
}

int sputline(int port, char *line)
{
  struct stty *tty = &stty[port];
  //WRITE CODE to output line to serial port
  int i = 0;

  do{
    sputc(tty, line[i]);
  }while(line[i++] != '\0');

}



/********** these are for system calls from Umode *****************/
char outline[64];
int sout(int port, char *z)
{
   // WRITE CODE to get a line from Umode and print it to serial port
   
    int i = 0;
    ushort c;

    // First get line from Umode
    do{
      c = get_byte(running->uss, z);
      outline[i] = c;
      z++;
      i++;
    }while(c != '\0');

    sputline(port, outline);

   return 0;
}

char inline[64];
int sin(int port, char *z)
{
  // WRITE CODE to get a line from serial port and copy it to Umode
  //  return length_of_line;
  int len;
  int i = 0;
  
  len = sgetline(port, inline);
  printf("Got %d chars from serial port %d\n", len, port);
  
  do{
    put_byte(inline[i++], running->uss, z++);
  }while(inline[i] != '\0');

  return len;
}

