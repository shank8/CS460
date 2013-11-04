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
      enqueue(running, &s->queue);
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
        enqueue(p, &readyQueue);
    }
    int_on(sr);
}
