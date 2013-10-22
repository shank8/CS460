/* timer parameters. */
#define LATCH_COUNT     0x00	/* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	/* ccaammmb, a = access, m = mode, b = BCD */

/************************ NOTICE THE DIVISOR VALUE ***********************/
#define TIMER_FREQ   1193182L	/* timer frequency for timer in PC and AT */
#define TIMER_COUNT ((unsigned) (TIMER_FREQ/60)) /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

int seconds;

int enable_irq(irq_nr) unsigned irq_nr;
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

/*===========================================================================*
 *				timer_init				     *
 *===========================================================================*/

ushort tick;

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = 0;
  seconds = 0;

  out_byte(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
  out_byte(TIMER0, TIMER_COUNT);	/* load timer low byte */
  out_byte(TIMER0, TIMER_COUNT >> 8);	/* load timer high byte */
  enable_irq(TIMER_IRQ); 
}

/*===========================================================================*
 *				timer_handler				     *
 *===========================================================================*/

int thandler()
{
    ushort floppy;
   

    PROC * list;

    tick++; 
    tick %= 60;

    if (tick == 0)
       {
        
        seconds++;

        // 1) Display Time
        print_clock();

        //2) Change IFF Umode every 5 seconds
        if(inkmode == 1){
          // WE ARE IN UMODE
         
          running->time--;
           printf("Switching in %d...\n", running->time);
          if(running->time == 0){
             out_byte(0x20, 0x20);  
             tswitch();
          }
        }

       // 3) Turn ON/OFF Floppy Disk port
        if(seconds % 5 == 0){
         
          printf("Switching FD...");
          if(in_byte(0x3F2) == 0x0C){ 

            out_byte(0x3F2, 0x1C);

          }else if(in_byte(0x3F2)  == 0x1C){
             out_byte(0x3F2, 0x0C);
          }else if(in_byte(0x3F2) == 0x2C){
            // printf("fd = %x\n", in_byte(0x3F2));
             //out_byte(0x3F2, 0x1C);
          }
        }

        // 4) Sleep function

        list = sleepList;
        while(list != 0){
          if(list->time > 0){
            list->time--;
            if(list->time == 0){
              printf("WAKEUP LIST+1\n");
              wakeup(list+1);
            }
          }
          list = list->next;
        }

       }

    out_byte(0x20, 0x20);  

}

int tsleep(timer) int timer; {
    if(running->pid <= 1){
      printf("Cannot sleep PROC 1\n");
      return -1;
    }
    running->time = timer;
    printf("tsleep for %d seconds\n", timer);
    sleep(running+1); // Sleep on some unique value

}





