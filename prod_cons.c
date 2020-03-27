/*
 *	File	: pc.c
 *
 *	Title	: Demo Producer/Consumer.
 *
 *	Short	: A solution to the producer consumer problem using
 *		pthreads.	
 *
 *	Long 	:
 *
 *	Author	: Andrae Muys
 *
 *	Date	: 18 September 1997
 *
 *	Revised	:
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
 #include <sys/time.h> 

#define QUEUESIZE 20
#define LOOP 1000
#define EXP 1500
void *producer (void *args);
void *consumer (void *args);

//struct with arguments for the functions.Contains arguments for each type of function, and 
//depending on the type of function chosen, the suitable arguments are used.
//the busy variable is used to prevent a producer to alter a struct of arguments that is being used of the struct array
//the functionType variable is used to define the type of the function that will be executed from a consumer
//the variables startTime and endTime are used to store the time that the struct workFunction
//was added to the queue and the time that was picked up from the queue.
 struct AllArgs{    
  int x;
  int y;
  double theta;
  int functionType;
  int  busy;
  double startTime;
  double endTime;
} ;

//struct that contains a pointer to a function and a pointer to a struct of arguments for the function
struct workFunction { 
  void * (*work)();
  struct AllArgs (* arg);

};

typedef struct {
  struct workFunction buf[QUEUESIZE];
  long head, tail;  
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q,struct  workFunction in);
void queueDel (queue *q,struct workFunction *out);
void PrintHello(void);
double Tan(double theta);int Sum(int x,int y);


int numofProd;   //number of producers 
void* (*Functions[3])();   //array of pointers to three types of functions. Sum,Tan , and simple hello message
                           //Each function takes different number and types of arguments 
struct AllArgs A[20];      //array of structs with the arguments.
struct timeval tv;          
long double  elapseTime[1000];  //array with all time elapses of one experiment
int timeIndex;     //index to the array of  time elapses in one experiment
int prodExit[1];   //array with all exit states of producers
int exitIndex;     //index to array with exit states of the producers
int finished=0;    //is set when all producers have terminated



int main ()
{
  long double wholeSum=0.0;     //sum of the experiments median elapse time
  long double  wholeMed=0.0;    //final median elapse time,of all the experiments
  long double  medElapseTime=0.0; //median elapse time of one experiment
  long double   sum=0.0;          //sum of one experiment's median elapse time



  for(int i=0;i<EXP;i++){



    queue *fifo;
    Functions[0]=Sum;
    Functions[1]=Tan;
    Functions[2]=PrintHello;
    numofProd=1;  
    timeIndex=0;  
    exitIndex=0;  
    finished=0;   

    medElapseTime=0.0;
    sum=0.0;
    for(int i=0; i<1000;i++){//allazei
      elapseTime[i]=0.0;
    }
    for(int i=0;i<numofProd;i++){
      prodExit[i]=0;
    }
    for( int i =0 ;i<20;i++){
      A[i].x=i;
      A[i].y=i+1;
      A[i].theta=(double)(i/2);
      A[i].busy=0;
    }

    pthread_t pro,pro1,pro2,pro3,con,con1,con2,con3,con4,con5,con6,con7,con8,con9,con10,con11,con12,con13,con14,con15;;

    fifo = queueInit ();
    if (fifo ==  NULL) {
      fprintf (stderr, "main: Queue Init failed.\n");
      exit (1);
    }
  
    pthread_create (&pro, NULL, producer, fifo);
    pthread_create (&con, NULL, consumer, fifo);
    // pthread_create (&pro1, NULL, producer, fifo);
    pthread_create (&con1, NULL, consumer, fifo);
    // pthread_create (&pro2, NULL, producer, fifo);
    pthread_create (&con2, NULL, consumer, fifo);
    // pthread_create (&pro3, NULL, producer, fifo);
    //pthread_create (&con3, NULL, consumer, fifo);
    //  pthread_create (&con4, NULL, consumer, fifo);
    // pthread_create (&con5, NULL,consumer, fifo);
    // pthread_create (&con6, NULL, consumer, fifo);
    // pthread_create (&con7, NULL, consumer, fifo);
    // pthread_create (&con8, NULL, consumer, fifo);
    // pthread_create (&con9, NULL, consumer, fifo);
    // pthread_create (&con10, NULL, consumer, fifo);
    // pthread_create (&con11, NULL, consumer, fifo);
    //  pthread_create (&con12, NULL, consumer, fifo);
    // pthread_create (&con13, NULL,consumer, fifo);
    // pthread_create (&con14, NULL, consumer, fifo);
    //  pthread_create (&con15, NULL, consumer, fifo);



    pthread_join (pro, NULL);
    // pthread_join (pro1, NULL);
    // pthread_join (pro2, NULL);
    // pthread_join (pro3, NULL);
    // pthread_join (pro4, NULL);
    pthread_join (con, NULL);
    pthread_join (con1, NULL);
    pthread_join (con2, NULL);
    //pthread_join (con3, NULL);
    // pthread_join (con4, NULL);
    // pthread_join (con5, NULL);
    // pthread_join (con6, NULL);
    // pthread_join (con7, NULL);
   
    // pthread_join (con8, NULL);
    // pthread_join (con9, NULL);
    // pthread_join (con10, NULL);
    // pthread_join (con11, NULL);
    // pthread_join (con12, NULL);

    // pthread_join (con13, NULL);
    // pthread_join (con14, NULL);
    // pthread_join (con15, NULL);

    for(int i=0;i<timeIndex;i++){
       sum=sum+elapseTime[i];
    }
    if (sum<0)
       sum=15000;
    medElapseTime=sum/((long double)timeIndex) ;   
    wholeSum=wholeSum + medElapseTime;

    // printf("medElapseTime= %Lf \n",medElapseTime);
    // printf("wholeSum=%Lf",wholeSum);
    queueDelete (fifo);
  }

  wholeMed=wholeSum/((long double)EXP);
  printf("final median time=%Lf",wholeMed);

  return 0;
}

void *producer (void *q)
{
  queue *fifo;
  int i;
  fifo = (queue *)q;
  struct workFunction wrk;  
  int funcChoice=0;   //choice of function type
  int argChoice=0;    //choice of arguments
  int term=0;         //termination variable 
  srand(time(NULL));


  for (i = 0; i < LOOP; i++) {

    funcChoice=rand()%3+0;   //random choice of function type
    //depending on the function type choice, the producer sets the variables of the workFunction structure.
    if(funcChoice==0){
      wrk.work=Functions[0];
      do{
        argChoice=rand()%20+0;   //random choice of arguments 
      }while((A[argChoice]).busy==1);
      wrk.arg=&(A[argChoice]);
      A[argChoice].busy=1;
      (*(wrk.arg)).functionType=0;
     
   }

   if(funcChoice==1){

     wrk.work=Functions[1];
     do{
       argChoice=rand()%20+0;
     }while((A[argChoice]).busy==1);
     wrk.arg=&(A[argChoice]);
     A[argChoice].busy=1;
     (*(wrk.arg)).functionType=1;
    

   }

   if(funcChoice==2){
     wrk.work=Functions[2];
     do{
        argChoice=rand()%20+0;
     }while((A[argChoice]).busy==1);
     wrk.arg=&(A[argChoice]);
     A[argChoice].busy=1;

     (*(wrk.arg)).functionType=2;

   }
   pthread_mutex_lock (fifo->mut);    //mutex locks for using the queue
   
    while (fifo->full) {
      printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
   gettimeofday(&tv,NULL);
   (*(wrk.arg)).startTime=tv.tv_usec;

   queueAdd (fifo, wrk);          
   pthread_mutex_unlock (fifo->mut);
   pthread_cond_signal (fifo->notEmpty);
   
  }
 
  prodExit[exitIndex]=1;  //sets one position of the exitArray when it terminates and inreases the exitIndex for the next producer that terminates 
  exitIndex++;
	
  //check if all the producers have terminated. If all the cells of the prodExit array are set, then all the producers have terminated
  //and the finished variable is set.Now the consumers can terminate.
  for(int j=0;j<numofProd;j++){
     if( prodExit[j]==1){
         term++;
         if(term==numofProd){
             finished=1;          
          }
      }
   }

  pthread_cond_signal (fifo->notEmpty);     
  return (NULL);
}

void *consumer (void *q)
{
  queue *fifo;
  int i,funcType;
  struct workFunction d;
  fifo = (queue *)q;

    while(1) {
        pthread_mutex_lock (fifo->mut);
        if(finished==1){                   //consumer termination condition,when all the producers have terminated
           pthread_mutex_unlock (fifo->mut);
           pthread_cond_signal (fifo->notEmpty);    
           return(NULL);
         }
     while (fifo->empty) {                
        if(finished==1){                       //consumer termination condition ,when all the producers have terminated
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);   ////in that case fifo is empty but we send the signal to let the other consumers terminate
        return(NULL);
      }
 
      printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
      }
       

      d.work =( fifo->buf[fifo->head]).work;     //save the index of the function 
      d.arg=(( fifo->buf[fifo->head]).arg);      //save the args for the function
      gettimeofday(&tv,NULL);              
      (*(d.arg)).endTime=tv.tv_usec;          
      elapseTime[timeIndex]=(*(d.arg)).endTime-(*(d.arg)).startTime;    //subtruct start and end time to get the elapse time
      timeIndex++;    //increase index for the next time measurement

      queueDel (fifo,&d);
      pthread_mutex_unlock (fifo->mut);      //unlock the mutex before the execution of the functions , so this part can be done in parallel
      pthread_cond_signal (fifo->notFull);

      funcType=(*(d.arg)).functionType;
    

      if(funcType==0){
     
        (*(d.work))((*(d.arg)).x,(*(d.arg)).y);    //if function type is 0 , then consumer uses the arguments for Sum function.
        (*(d.arg)).busy=0;   //free the specific struct for next consumer
       }

      if(funcType==2){  
        (*(d.work))();      //if function type is 2 , then the consumer uses no arguments
        (*(d.arg)).busy=0;
       }
      if(funcType==1){   
        (*(d.work))((*(d.arg)).theta);    //if the function type is 1, then the consumer uses the argument for Tan function.
        (*(d.arg)).busy=0;

       }


   // printf ("consumer: recieved %d.\n", d);


  }
 
  return(NULL);
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

void queueDel (queue *q,struct workFunction *out)
{
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

                     
