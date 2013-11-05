#define KEYBD   0x60	/* I/O port for keyboard data */
#define PORT_B  0x61    /* port_B of 8255 */
#define KBIT	0x80	/* bit used to ack characters to keyboard */

#define KBSIZE    64    // size of input buffer in bytes
#define N_SCAN   105	/* Number of scan codes */

/* Scan codes to ASCII for unshifted keys; unused keys are left out */
char unshift[N_SCAN] = { 
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','[',']', '\r', 0,'a','s',
 'd','f','g','h','j','k','l',';',       0, 0, 0, 0,'z','x','c','v',
 'b','n','m',',','.','/', 0,'*',        0, ' '       
};

/* Scan codes to ASCII for shifted keys; unused keys are left out */
char shift[N_SCAN] = {
 0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
 'Q','W','E','R','T','Y','U','I',      'O','P','{','}', '\r', 0,'A','S',
 'D','F','G','H','J','K','L',':',       0,'~', 0,'|','Z','X','C','V',
 'B','N','M','<','>','?',0,'*',         0, ' '  
};

int kbdata;            // has KBD input flag
char kbc;              // data char
char inbuf[128];
int inhead, intail;
int indata;

int kbinit()
{
  printf("kbinit() : ");
  indata = 0;          // flag = 0 initially
  inhead = 0;
  intail = 0;
  enable_irq(1);
  out_byte(0x20, 0x20);
  printf("kbinit done\n\r");
}

int kbhandler()
{
  int scode, value, c;

  /* Fetch the character from the keyboard hardware and acknowledge it. */
  scode = in_byte(KEYBD);/* get the scan code of the key struck */
  value = in_byte(PORT_B);/* strobe the keyboard to ack the char */
  out_byte(PORT_B, value | KBIT);/* first, strobe the bit high */
  out_byte(PORT_B, value); /* then strobe it low */

   kbc = unshift[scode];                // translate scan code into ASCII char

  if(indata != 128){
  	inbuf[inhead++] = (char)kbc;
  	inhead %= 128;
  	indata++;
  	kwakeup(&inbuf);

  }else{
  	//sound beep;
  	printf("beep");
  }

  printf("kb interrupt %x\n", scode);  // should see the scand code
  if (scode & 0x80) {
  		out_byte(0x20, 0x20);
  }                   // ignore key release
    
   out_byte(0x20, 0x20); 
}

// process call getc() to return a char from KBD, by busy waiting loop
int getc()
{
  char c;
 
  unlock();          // syscall to MTX kernel MAY have masked out interrupts

  if(indata == 0){
  	printf("SLEEEEEEEP");
  	ksleep(&inbuf);
  	lock();
  	c = inbuf[intail++];
  	intail %= 128;
  	indata--;
  	unlock();
  	return c;
  }
  printf("NOOOO SLEEEEP");
  lock();
  	c = inbuf[intail++];
  	intail %= 128;
  	indata--;
  	unlock();

  return c;
}