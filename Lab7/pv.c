struct semaphore{
  int value;
  PROC *queue;
};

int P(s) struct semaphore *s;
{
  int sr = int_off();
   s->value--;
   if (s->value < 0){
      running->status=BLOCK;
      enqueue(&s->queue, running);
      tswitch();
   }
   int_on(sr);

}

int V(s) struct semaphore *s;
{
    PROC *p;
    int sr = int_off();
    s->value++;
    if (s->value <= 0){
        p = dequeue(&s->queue);
        p->status = READY;
        enqueue(&readyQueue, p);
    }
    int_on(sr);
}
