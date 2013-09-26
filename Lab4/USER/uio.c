typedef unsigned char    u8;
typedef unsigned short  u16;
typedef unsigned long   u32;

char *ctable = "0123456789ABCDEF";
u16  BASE = 10;

int rpu(int x)
{
   char c;
   if (x){ 
      c = ctable[x % BASE]; 
      rpu(x / BASE);
      putc(c);
   }
}

int printu(int x)
{   
  if (x==0) 
     putc('0');
  else 
     rpu(x);
  
} 

int printd(int x)
{   
  if (x==0) 
     putc('0');
  else if(x<0){
   putc('-');
   rpu(-1*x);
  }else{
  	  rpu(x);
  }
  
} 

void prints(char *s){
	while(*s != '\0'){
		putc(*s++);
	}
}

void printf(char * fmt, ...){

	char *cp = fmt;              // let cp point at the fmt string
	u16  *ip = (int *)&fmt + 1;  // ip points at first item to be printed on stack
	u32  *up;                    // for getting LONG parameter off stack
	char flag;

	while(*cp != '\0'){

		if(*cp == '%'){
			// We need to format print
			flag = *(cp+1);
			// Now switch on the flag character to see what type of variable we are printing
			switch(flag){
				case 'c':
						putc(*ip);
					break;
				case 's':
						
						prints(*ip);

					break;
				case 'u':
						if(*ip >= 0){
							printu(*ip);
						}else{
							prints("The %d format must be SIGNED\n\r");
						}
					break;
				case 'd':
						printd(*ip);
					break;
				case 'x':
						BASE = 16;
						printd(*ip);
						BASE = 10;
					break;
				case 'l':
						up = (u32 *)ip; // Since this is a long, we cast it as a u32
						printd(*up);

						// Increment an extra 2 bytes for LONGS
						ip++;
					break;
			}

			ip = (u16 *)ip + 1; // Increment the stack watcher by 2-bytes every time			
			cp++;	
		}else if(*cp == '\n'){
			putc(*cp);
			putc('\r');
		}else{
			putc(*cp); // Just print the raw character
		}
		cp++; // Advance to the next character in fmt
	}	
	
	return;
}
