/* External definitions for single-server queueing system. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define Q_LIMIT 100  /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */


void  initialize(void);
float random_variable_given_probabilities(float *vars, float *probs);
void  timing(void);
void  arrive(void);
void  depart(void);
void  report(void);
float interarrive_with_formula(float *vars, float *probs);
void  update_time_avg_stats(void);
float expon(float mean);
float  gen_next_interarrive(void);
float  gen_next_depart(void);
int num_in_q;
float time_last_event;
int num_custs_delayed;
float total_of_delays;
int next_event_type;
int completed_transactions;
float time_next_event[2];
int server_status;
float sim_time;
float area_num_in_q;
float area_server_status;
float service_times[6]={10.0,20.0,30.0,40.0,-1.0,-1.0};
float service_money[6]={5000.0,7000.0,9000.0,11000.0,-1.0,-1.0};
float service_probabilities[6]={0.15,0.25,0.4,0.2,-1.0,-1.0};
float interarrive_times[10]={5.0,10.0,15.0,20.0,25.0,30.0,35.0,40.0,-1.0,-1.0};
float interarrive_probabilities[10]={0.05,0.05,0.1,0.1,0.3,0.2,0.15,0.05,-1.0,-1.0};
float arrival_time[Q_LIMIT-1];
float service_time_total;
float time_last_service;
float time_in_business;
float last_queue_change;
int sum_num_in_q;
int total_steps;

//quiero: tiempo promedio que un cliente pasa en el comercio, promedio que pasan en la cola,
//numero clientes en comercio promedio, numero promedio clientes en cola, utilizacion de maquina

//a que se refiere con tiempo que pasa en el comercio? tiempo usando la maquina? o tiempo en cola + tiempo usando maquina?
//lo mismo con numero de gente en comercio

int main(int argc, char *argv[])  /* Main function. */
{

    /* Initialize the simulation. */

    float max_time=atof(argv[1]); // tiempo maximo en minutos

    initialize();

    /* Run the simulation until time runs out */

    while (sim_time < max_time) {

        /* Determine the next event. */

        timing();

        /* Update time-average statistical accumulators. */

        update_time_avg_stats();

        /* Invoke the appropriate event function. */

        switch (next_event_type) {
            case 1:
                arrive();
                break;
            case 2:
                depart();
                break;
        }
    }

    while (num_in_q>0 || server_status==BUSY){
        sim_time=time_next_event[1];
        depart();
    }

    printf("\n num in q: %d",num_in_q);

    /* Invoke the report generator and end the simulation. */

    report();


    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    server_status   = IDLE;
    num_in_q        = 0;
    time_last_event = 0.0;

    // Initialize the statistical counters. num_custs_delayed, total_of_delays, area_num_in_q, area_server_status

     num_custs_delayed=0;
     total_of_delays=0.0;
     area_num_in_q=0,0; //longitud promedio de cola
     area_server_status=0.0; //utilizacion promedio de server
     service_time_total=0.0;
     completed_transactions=0;
     service_time_total=0.0;
     time_last_service=0.0;
     time_in_business=0.0;
     last_queue_change=0.0;
     sum_num_in_q=0;
     total_steps=1; //sino se pasa de largo el de 0 a primer sim_time



    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[0] = sim_time + gen_next_interarrive() ; //Planificacion del primer arribo
    time_next_event[1] = 1.0e+30; //infinito
}


void timing(void)  /* Timing function. */
{
    float min_time_next_event = 1.0e+29;
    printf("%f",time_next_event[1]);

    next_event_type = 0; //Al final debe quedar en 1 en caso de que el proximo evento sea un arribo, 2: si es la salida de un servicio

    /* Determine the event type of the next event to occur. */
    //printf("\n tiempo arrive: %f", time_next_event[0]);
    //printf("\n tiempo depart: %f", time_next_event[1]);
    if (time_next_event[0]<=time_next_event[1]){
        next_event_type=1;
        //printf("\n sim_time: %f", sim_time);
        sim_time=time_next_event[0];
    }
    else{
        next_event_type=2;
        sim_time=time_next_event[1];
    }
}


void arrive(void)  /* Arrival event function. */
{
    float delay;

    /* Schedule next arrival. */

    sum_num_in_q=sum_num_in_q+num_in_q;
    total_steps++;

    printf("\n Llego en %f",sim_time);

    completed_transactions++;
    time_next_event[0] = sim_time + gen_next_interarrive();

    printf("\n Proximo arrivo en %f",time_next_event[0]);

    /* Check to see whether server is busy. */

    if (server_status == BUSY) {

        /* Server is busy, so increment number of customers in queue. */
    //printf("\n num_in_q: %d",num_in_q);
    total_of_delays=total_of_delays+((sim_time - last_queue_change )* num_in_q);
	num_in_q++;
	last_queue_change=sim_time;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q > Q_LIMIT) {

            exit(2);
        }

        /* Guardar el tiempo de arribo de esta entidad para los calculos estadisticos */

        arrival_time[num_in_q-1]=sim_time;


    }

    else {

        /* Server libre, tener en  cuenta la entidad que pasa directamente al server para los calculos estadisticos */

	//...

        //...


        /* Schedule a departure (service completion). */
        server_status = BUSY;
        time_last_service=sim_time;
        time_next_event[1] = sim_time + gen_next_depart();
        printf("\n Proxima salida en %f",time_next_event[1]);
    }
}


void depart(void)  /* Departure event function. */
{
    printf("\n Salio en %f",sim_time);
    int   i;
    float delay;

    sum_num_in_q=sum_num_in_q+num_in_q;
    total_steps++;

    /* Check to see whether the queue is empty. */

    service_time_total=service_time_total + (sim_time - time_last_service);

    if (num_in_q == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status=IDLE;
        time_next_event[1]=1.0e+30;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */


        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */
        printf("\n num_in_q: %d", num_in_q);
        total_of_delays=total_of_delays+((sim_time - last_queue_change)* num_in_q);
        num_in_q--;
        last_queue_change=sim_time;

        //sim_time - arrival_time[num_in_q]

	//...

        /* Increment the number of customers delayed, and schedule departure. */

        //...
        time_next_event[1] = sim_time + gen_next_depart();
        printf("\n Proxima salida en %f",time_next_event[1]);
        server_status=BUSY;
        time_last_service=sim_time;

    }
}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    //Average delay in queue
    float avgdelays=total_of_delays/sim_time;
    float serverutilization=service_time_total/sim_time;
    float tiempopromencola=total_of_delays/completed_transactions;
    float numeroencolaprom=(float) sum_num_in_q/ (float) total_steps;
    printf("\n sum_num_in_q: %d",sum_num_in_q);
    printf("\n total_steps: %d",total_steps);
    printf("\n area bajo curva utilizacion server: %f",serverutilization);
    printf("\n area bajo curva cola: %f",avgdelays);
    printf("\n tiempo promedio en cola: %f",tiempopromencola);
    printf("\n numero de gente en cola promedio: %f",numeroencolaprom);
    //Average number in queue

    //Server utilization


}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */



    /* Update area under server-busy indicator function. */


}


float gen_next_interarrive()
{
  //float var=random_variable_given_probabilities(interarrive_times,interarrive_probabilities);
  float var=interarrive_with_formula(interarrive_times,interarrive_probabilities);
  //printf("\n var interarrivo: %f", var);
  return var;
}

float gen_next_depart()
{
  float var=random_variable_given_probabilities(service_times,service_probabilities);
  //printf("\n var depart: %f", var);
  return var;
}

float random_variable_given_probabilities(float *vars, float *probs)
{
    float random_number= lcgrand(4);
    //printf("\n numero aleatorio: %f", random_number);
    float prob_acum=0.0;
    int i=0;
    while(probs[i]>0.0){
        //printf("\n i: %d", i);
        //printf("\n prob acum: %f", prob_acum);
        //printf("\n probs[i]: %f", probs[i]);
       if (random_number>=prob_acum && random_number<prob_acum+probs[i]){
           //printf("\n variable aleatoria: %f", vars[i]);
           return vars[i];
       }
       prob_acum=prob_acum+probs[i];
       i=i+1;
    }
}

//es asi? Tengo que hacer lo de dos maquinas? que son los valores de performance teoricos, los estadisticos?
float interarrive_with_formula(float *vars, float *probs){
    float random_number= lcgrand(4);
    int lambda=3;
    float interarrive=(-1/(float) lambda) * (log(1-random_number));
    float prob_acum=0.0;
    int i=0;
    while(probs[i]>0.0){
        //printf("\n i: %d", i);
        //printf("\n prob acum: %f", prob_acum);
        //printf("\n probs[i]: %f", probs[i]);
       if (interarrive>=prob_acum && interarrive<prob_acum+probs[i]){
           //printf("\n variable aleatoria: %f", vars[i]);
           return vars[i];
       }
       prob_acum=prob_acum+probs[i];
       i=i+1;
    }
}
