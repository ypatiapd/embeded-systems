

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define QUEUESIZE 20

//arguments for the TimerFcn execution
struct AllArgs{
    int x;
    int y;
    double theta;
    int functionType;
    int  busy;
    double startTime;
    double endTime;
};

//struct that contains a pointer to the function that will be executed and one to the args struct
struct workFunction {
    void * (*TimerFcn)();
    struct AllArgs (* arg);
};

//timer struct
typedef struct{
    int Period;
    int TasksToExecute;
    int StartDelay;
    int ArgChoice;
    struct workFunction wrk;  //borei na thelei deikti des to
}Timer;

//queue struct
typedef struct {
  struct workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

//thread execution args
typedef struct {
    Timer *timer;
    queue *fifo;
}ThreadArgs;

void *producer (void *args);
void *consumer (void *fifo);

Timer *timerInit(int Per,int Tasks,int Delay,int FunChoice,int Args);
void startFcn (int* FunChoice, int* ArgChoice);
void StopFcn(int ArgChoice , queue *q) ;
void Start( void *args,pthread_t prod);
void Startat(int t,int y,int m,int h,int min ,int sec,void *args,pthread_t prod);
queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q,struct  workFunction in);
void queueDel (queue *q,struct workFunction *out);
long int ErrorFcn(queue *q);
int cmpfunc (const void * a, const void * b) ;
void calculate_metrics(int procon,int arraySize ,long int elapse[]);
void PrintHello(void);
double Tan(double theta);
int Sum(int x,int y);

void* (*Functions[3])();   //array of pointers to three types of functions. Sum,Tan , and simple hello message
                           //Each function takes different number and types of arguments
struct AllArgs A[20];      //array of structs with the arguments.
int timer_finished=0;      //variable that when is equal to 0 all timers have terminated so the consumers can terminate also

int main(){

    Timer *timer1;
    Timer *timer2;
    Timer *timer3;
    queue *fifo;
    ThreadArgs *args1;
    ThreadArgs *args2;
    ThreadArgs *args3;
    int funChoice;
    int argChoice;
    pthread_t pro1,pro2,pro3,con1,con2;

    fifo = queueInit ();
    if (fifo ==  NULL) {
        fprintf (stderr, "main: Queue Init failed.\n");
        exit (1);
    }
    args1 = (ThreadArgs*)malloc (sizeof (ThreadArgs));
    args2 = (ThreadArgs*)malloc (sizeof (ThreadArgs));
    args3 = (ThreadArgs*)malloc (sizeof (ThreadArgs));

    Functions[0]=Sum;
    Functions[1]=Tan;
    Functions[2]=PrintHello;

    for( int i =0 ;i<20;i++){
      A[i].x=i;
      A[i].y=i+1;
      A[i].theta=(double)(i/2);
      A[i].busy=0;
    }

    startFcn (&funChoice, &argChoice);
    timer1=timerInit(10,10,10,funChoice,argChoice);
    startFcn (&funChoice, &argChoice);
    timer2=timerInit(100,10,10,funChoice,argChoice);
    startFcn (&funChoice, &argChoice);
    timer3=timerInit(1000,10,10,funChoice,argChoice);

    args1->fifo=fifo;
    args1->timer=timer1;
    args2->fifo=fifo;
    args2->timer=timer2;
    args3->fifo=fifo;
    args3->timer=timer3;
    Start(args1,pro1);
    Start(args2,pro2);
    Start(args3,pro3);
    Start(args,pro3);
    //Startat(2020,12,17,23,6,17,args,pro1);

    pthread_create (&con1,NULL,consumer,fifo);
    //pthread_create (&con2,NULL,consumer,fifo);

    pthread_join(con1,NULL);
    //pthread_join(con2,NULL);
    printf("teleiwsaaaaaaaaaaa");
    queueDelete (fifo);
}

void *producer (void *args)
{
  long double median_elapse=0;  //median time drift
  long int min_elapse=10000000; //minimun time drift
  long int max_elapse=0;       //maximum time drift
  long double average_elapse=0; //average time drift
  long double st_dev=0;        //standar deviation of time drift
  long int start_Delay=0;      //delay when timer starts
  //variables for time measurements with gettimeofday()
  long int start_time=0;
  long int end_time=0;
  long int start_time_sec=0;
  long int start_time_usec=0;
  long int end_time_sec=0;
  long int end_time_usec=0;
  long int start_drift=0;
  long int end_drift=0;
  long int start_drift_sec=0;
  long int start_drift_usec=0;
  long int end_drift_sec=0;
  long int end_drift_usec=0;
  long int interval=0;       //the interval that the thread slept for
  long int apominari=0;      // the interval that remains to complete the period
  long int counter=0;
  long int time_passed=0;    // time passed in case that queue was full until it wasnt any more
  long int time_drift=0;     //time drift of one period
  struct timeval tv;
  ThreadArgs *Arg;
  Arg=args ;
  int startDelay=(Arg->timer->StartDelay)*1000000;
  long int timer_period=(long int)(Arg->timer->Period);
  long int sleepVar=0;      // the argument for the sleep of thread, less than the period
  long int elapseTime[Arg->timer->TasksToExecute];  //array with all time drifts of consumer
  int error=0;           //sets if the ErrorFcn was called
  int error_size=0;      // number of lost periods, 0 is the period is not lost.
  long int sleep_time=0;
  int tasks_executed=0;

  // depending on the size of the period, we define the interbal that the thread will sleep
  if(timer_period<=99)
      sleepVar= (long int)(timer_period*1000-timer_period*1000/2);
  else if(timer_period<=999)
      sleepVar= (long int)(timer_period*1000-timer_period*1000/10);
  else
      sleepVar= (long int)(timer_period*1000-timer_period*1000*3/100);

  timer_finished++;   // increases at the start of each timer and decreases when timer terimnates
  usleep(startDelay);

  for (int i = 0; i < Arg->timer->TasksToExecute; i++) {

   sleep_time= sleepVar;    // renew everytime the sleep_time variable in case it changed due to error
   printf("sleep_vaaar= %ld\n",sleep_time);
   pthread_mutex_lock (Arg->fifo->mut);    //mutex locks for using the queue
   while (Arg->fifo->full) {
      printf ("producer: queue FULL.\n");
      time_passed=ErrorFcn(Arg->fifo);
      printf("time_passed= %ld\n",time_passed);
      error=1;
      // error handle  upologizei xrono pou perase , kanei skip tin queueadd kai metraei oso menei gia to epomeno vima
   }

   if(error==1){
     error_size=time_passed/(timer_period*1000);  //compute the lost periods
     printf("error_size= %d\n",error_size);
     sleep_time=sleepVar+(timer_period*1000*error_size)-time_passed;
     
   }
   if((error==0)&&(i!=0)){ // if there are lost periods
     gettimeofday(&tv,NULL);
     end_drift_sec=tv.tv_sec;
     end_drift_sec*=1000000;
     end_drift_usec=tv.tv_usec;
     end_drift=end_drift_sec+end_drift_usec;
     time_drift+=(end_drift-start_drift);
     elapseTime[i]=time_drift;    // time drift for one period
     printf("time_for_queue2= %ld\n",time_drift);
     gettimeofday(&tv,NULL);
     start_time_sec=tv.tv_sec;
     start_time_sec*=1000000;
     start_time_usec=tv.tv_usec;
     start_time=start_time_sec+start_time_usec;
     Arg->timer->wrk.arg->startTime=start_time;
     queueAdd (Arg->fifo, Arg->timer->wrk);
     printf("added a job");
     tasks_executed++;
   }
   pthread_mutex_unlock (Arg->fifo->mut);
   pthread_cond_signal (Arg->fifo->notEmpty);
   printf("fifo=%ld",Arg->fifo->tail);

   gettimeofday(&tv,NULL);
   start_time_sec=tv.tv_sec;
   start_time_sec*=1000000;
   start_time_usec=tv.tv_usec;
   start_time=start_time_sec+start_time_usec;

   usleep(sleep_time);    //sleep for less than a period
   gettimeofday(&tv,NULL);
   end_time_sec=tv.tv_sec;
   end_time_sec*=1000000;
   end_time_usec=tv.tv_usec;
   end_time=end_time_sec+end_time_usec;

   interval=end_time-start_time;     //actual time that thread slept
   printf("period= %ld\n",interval);

   // time left to complete a period in case of error and not.
   if (error==0)
      apominari=(timer_period*1000)-interval;
   else{
      apominari=(timer_period*(error_size+1)*1000)-time_passed-interval;
   }
   printf("apominari= %ld\n",apominari);

   gettimeofday(&tv,NULL);
   start_time_usec=tv.tv_usec;
   end_time_usec=0;
   counter=end_time_usec-start_time_usec;
   while(counter<apominari){
     if (apominari<0 )break;
     gettimeofday(&tv,NULL);
     end_time_usec=tv.tv_usec;
     counter=end_time_usec-start_time_usec;

   }
   interval=interval+counter;

   //handle the drift measurements in case of errror and not
   if(error==0){
      if(apominari>0){
        time_drift=(long int)interval%10000;
        printf("time_for_queue1= %ld\n",time_drift);
        printf("period_final= %ld\n",interval);
      }
      else{
        time_drift= abs(apominari);
        printf("time_for_queue1= %ld\n",time_drift);
      }
      gettimeofday(&tv,NULL);
      start_drift_sec=tv.tv_sec;
      start_drift_sec*=1000000;
      start_drift_usec=tv.tv_usec;
      start_drift=start_drift_sec+start_drift_usec;
   }
   else{
     if(apominari>0){
       time_drift+=((long int)(interval+time_passed)%10000);
       printf("time_for_queue1= %ld\n",time_drift);
       printf("period_final= %ld\n",interval+time_passed);
     }
     else{ //in case that the thread sleeping exceeded the period
       time_drift+= abs(apominari) ;
       printf("time_for_queue1= %ld\n",time_drift);
     }
   }

   error=0;
   error_size=0;
  }
  printf("tasks_executed= %d\n",tasks_executed);
  calculate_metrics( 0,Arg->timer->TasksToExecute,elapseTime);
  pthread_cond_signal (Arg->fifo->notEmpty);
  StopFcn(Arg->timer->ArgChoice,Arg->fifo);
  return (NULL);
}


void *consumer (void *q)
{
  queue *fifo;
  int i,funcType;
  long int end_time_sec=0;
  long int end_time_usec=0;
  long int end_time=0;
  struct workFunction d;
  fifo = (queue *)q;
  struct timeval tv;
  long int  elapseTime[10000];  //array with all time elapses of consumer
  int timeIndex;
  long double median_elapse=0;
  long int min_elapse=10000000;
  long int max_elapse=0;
  long double average_elapse=0;
  long double st_dev=0;

  for(i=0;i<10000;i++){
    elapseTime[i]=0;
  }
  sleep(5);
  while(timer_finished!=0) {
    pthread_mutex_lock (fifo->mut);

    while (fifo->empty) {

        printf ("consumer: queue EMPTY.\n");
        pthread_cond_wait (fifo->notEmpty, fifo->mut);
        if(timer_finished==0){                       //consumer termination condition ,when all the producers have terminated
        printf ("bika2.\n");
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);   ////in that case fifo is empty but we send the signal to let the other consumers terminate
        break;
        }
      }
      if(timer_finished==0) break;

      d.TimerFcn =( fifo->buf[fifo->head]).TimerFcn;     //save the index of the function
      d.arg=(( fifo->buf[fifo->head]).arg);      //save the args for the function
      gettimeofday(&tv,NULL);
      end_time_sec=tv.tv_sec;
      end_time_sec*=1000000;
      end_time_usec=tv.tv_usec;
      end_time=end_time_sec+end_time_usec;
      (*(d.arg)).endTime=end_time;
      elapseTime[timeIndex]=(*(d.arg)).endTime-(*(d.arg)).startTime;    //subtruct start and end time to get the elapse time
      timeIndex++;    //increase index for the next time measurement

      queueDel (fifo,&d);
      pthread_mutex_unlock (fifo->mut);      //unlock the mutex before the execution of the functions , so this part can be done in parallel
      pthread_cond_signal (fifo->notFull);

      funcType=(*(d.arg)).functionType;


      if(funcType==0){
        (*(d.TimerFcn))((*(d.arg)).x,(*(d.arg)).y);    //if function type is 0 , then consumer uses the arguments for Sum function.
           //free the specific struct for next consumer
       }
      if(funcType==2){
        (*(d.TimerFcn))();      //if function type is 2 , then the consumer uses no arguments
       }
      if(funcType==1){
        (*(d.TimerFcn))((*(d.arg)).theta);    //if the function type is 1, then the consumer uses the argument for Tan function
       }

  }

  calculate_metrics(1,timeIndex,elapseTime);
  printf("teleiwsaaaa");
    //wholeSum=wholeSum + medElapseTime;
  return(NULL);
}

void calculate_metrics(int procon,int arraySize , long int elapse[]){

  long double median_elapse=0;
  long int min_elapse=10000000;
  long int max_elapse=0;
  long double average_elapse=0;
  long double st_dev=0;
  elapse[0]=0;
  double sum=0;
  char ch;
  if (procon==0)
    ch='p';
  else ch='c';

  for(int i=1;i<arraySize;i++){
       sum=sum+elapse[i];
       if(elapse[i]<min_elapse)
          min_elapse=elapse[i];
       if(elapse[i]>max_elapse)
          max_elapse=elapse[i];
    }
  if (sum<0)
      sum=15000;
  average_elapse=sum/(long double)arraySize ;
  sum=0;
  for(int i=1;i<arraySize;i++){
        sum=sum+(elapse[i]-average_elapse)*(elapse[i]-average_elapse);
  }
  st_dev=sum/((long double)arraySize);
  st_dev=sqrt(st_dev);
  qsort(elapse, arraySize, sizeof(long int), cmpfunc);
  median_elapse=elapse[(int)(arraySize/2)];
  printf("average_%c_elapse= %Lf\n",ch,average_elapse);
  printf("median_%c_elapse= %Lf\n",ch,median_elapse);
  printf("st_dev_%c_elapse= %Lf\n",ch,st_dev);
  printf("max_%c_elapse= %ld\n",ch,max_elapse);
  printf("min_%c_elapse= %ld\n",ch,min_elapse);
}

//function that starts the timer
void Start(void *args,pthread_t prod ){

    pthread_create (&prod, NULL,producer,args);
    return;

}

//function that starts the timer at a specific date and time
void Startat(int y,int m,int d,int h,int min ,int sec,void *args,pthread_t prod){


  time_t t= time(NULL);
  struct tm tm = *localtime(&t);
  printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("%d\n",tm.tm_year+1900);
  printf("%d\n",tm.tm_mon+1);
  printf("%d\n",tm.tm_mday);
  printf("%d\n",tm.tm_hour);
  printf("%d\n",tm.tm_min);
  printf("%d\n",tm.tm_sec);

  int current_year=tm.tm_year+1900;
  int current_month=tm.tm_mon+1;
  int current_day=tm.tm_mday;
  int current_hour=tm.tm_hour;
  int current_min=tm.tm_min;
  int current_sec=tm.tm_sec;
  int year_sec=(y-current_year)*365*24*3600;//disekto
  printf("%d\n",year_sec);
  int counter=current_month+1;
  int sum=0;
  while(counter<m){
    if(counter==2){
      sum+=28;
      counter++;
      continue;
    }
    if (counter%2==0){
      sum+=30;
      counter++;
      continue;
    }
    if(counter%2==1){
      sum+=31;
      counter++;
      continue;
    }

  }
  printf("%d\n",sum);
  int month_sec=sum*24*3600;// 30 meres kai 28
  printf("%d\n",month_sec);
  int day_sec=(d-current_day)*24*3600;
  printf("%d\n",day_sec);
  int hour_sec=(h-current_hour)*3600;
  printf("%d\n",hour_sec);
  int min_sec=(min-current_min)*60;
  printf("%d\n",min_sec);
  int sec_sec=(sec-current_sec);
  printf("%d\n",sec_sec);
  int total_secs=year_sec+month_sec+day_sec+hour_sec+min_sec+sec_sec;
  printf("%d\n",total_secs);
  sleep(total_secs);
  pthread_create (&prod, NULL,producer,args);
  return;

}

//function that is called when a timer terminates
void StopFcn(int ArgChoice,queue *q) {

    timer_finished--; //each timer that terminates decreases the global variable
    printf("timer_finished= %d\n",timer_finished);
    A[ArgChoice].busy=0; // free the argument from the array
    if(timer_finished==0)
      sleep(3);// gia na exei prolavei o consumer na ektelesei kai tin teleutaia wrk, se periptwsi pou einai enas
      pthread_cond_signal (q->notEmpty);//signal to let the consumers terminate
    return;
}

//initialization of timer function choice and her args
void startFcn (int* FunChoice, int* ArgChoice){
  int term=0;         //termination variable
  srand(time(NULL));
  *FunChoice=rand()%3+0;   //random choice of function type
    //depending on the function type choice, the producer sets the variables of the workFunction structure.
  do{
    *ArgChoice=rand()%20+0;   //random choice of arguments
  }while((A[*ArgChoice]).busy==1);
  A[*ArgChoice].busy=1;   // set the busy variable of the args for not letting an other time object use the same
  //printf("funcchoice=%d\n",*FuncChoice);
  //printf("argchoice=%d\n",*ArgChoice);
  return;
}

// function that is called in case the queue is full
long int ErrorFcn(queue *q){
    long int start_time=0;
    long int start_time_sec=0;
    long int start_time_usec=0;
    long int end_time=0;
    long int end_time_sec=0;
    long int end_time_usec=0;
    long int t_passed=0;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    start_time_sec=tv.tv_sec;
    start_time_sec*=1000000;
    start_time_usec=tv.tv_usec;
    start_time=start_time_sec+start_time_usec;
    printf(" error start_time = %ld\n",start_time);
    pthread_cond_wait (q->notFull, q->mut);//wait until the queue is not full
    //sleep(5);
    gettimeofday(&tv,NULL);
    end_time_sec=tv.tv_sec;
    end_time_sec*=1000000;
    end_time_usec=tv.tv_usec;
    end_time=end_time_sec+end_time_usec;
    printf("end_period= %ld\n",end_time);
    t_passed=end_time-start_time;  // time passed
    return t_passed;
}

//function that initializes the timer
Timer *timerInit (int Per,int Tasks,int Delay,int FunChoice,int ArgChoice)
{
  Timer *t;

  t = (Timer *)malloc (sizeof (Timer));
  if (t== NULL) return (NULL);

  t->Period=Per;
  t->TasksToExecute=Tasks;
  t->StartDelay=Delay;
  t->ArgChoice=ArgChoice;
  t->wrk.TimerFcn=Functions[FunChoice];
  t->wrk.arg=&(A[ArgChoice]);
  t->wrk.arg->functionType=FunChoice;
  return (t);
}

//function needed for quicksort (metrics)
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);

  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q,struct workFunction in)
{
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q,struct workFunction *out){
  *out = q->buf[q->head];

  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}
int Sum(int x , int y){
  int sum=0;
  sum=x+y;
  printf("sum= %d     \n ",sum);
  return sum;
}

double Tan(double theta){
  double t=0.0;
  t=sin(theta)/cos(theta);
  printf("tan= %lf     \n ",t);
  return t;
}

void PrintHello(void){
  printf("Hello ! I am a lonely thread\n");
}
