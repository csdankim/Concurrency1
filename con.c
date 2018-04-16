/*
NAME: Dongkyu kim
ONID: kimdongk
OSUID#: 933-296-408
DATE: April 15. 2018
*/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <zconf.h>
#include "mt19937ar.h"

#define BUFFER_SIZE 32

struct item {
    int num;
    int sec;
} buffer[BUFFER_SIZE];


pthread_mutex_t mutex;
sem_t empty;
sem_t full;


int insertPointer = 0, removePointer = 0;
int items_num = 0;
int isRdrand = 0;


void *producer(void *param);
void *consumer(void *param);


/* ====== Produce Random Number Start ====== */
// Check weather need to use rdrand
void isRdrandCheck()
{
    unsigned int eax, ebx, ecx, edx;
    eax = 0x01;
    __asm__ __volatile__(
    "cpuid;"
    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
    : "a"(eax)
    );

    isRdrand = ecx & 0x40000000;
}

// Return random number by rdrand
int rdrand(int min, int max)
{
    unsigned int a;
    unsigned char b;

    asm volatile("rdrand %0; setc %1"
    : "=r"(a), "=qm"(b)
    );
    if (b)
        return ((a % max) + min);
    else
        return 0;
}

// Return random number by mt19937
int mt19937(int min, int max)
{
    init_genrand(time(NULL));
    int a = (int)genrand_int32();

    return (abs(a) % (max - 1)) + min;
}

// Choose random number generation method
int randomNum(int min, int max)
{
    if (isRdrand)
        return rdrand(min, max);
    else
        return mt19937(min, max);
}

/* ===== Produce Random Number End ===== */


//Insert Item into queue
void insert_item(int val1, int val2)
{
    buffer[insertPointer].num = val1;
    buffer[insertPointer].sec = val2;
    insertPointer = (insertPointer+1) % BUFFER_SIZE;
    items_num++;
}

//Pick out Item from queue
struct item *remove_item()
{
    removePointer = removePointer % BUFFER_SIZE;
    items_num--;
    return &(buffer[removePointer++]);
}


int main(int argc, char *argv[])
{
    // argc for threads
    int producerThreads, consumerThreads;
    int i, j, k;

    if(argc !=3)
    {
        fprintf(stderr, "Usage: <producer threads> <consumer threads>₩\n");
        return -1;
    }

    producerThreads = atoi(argv[1]);
    consumerThreads = atoi(argv[2]);

    isRdrandCheck();

    //Initialize the locks
    printf("%d", pthread_mutex_init(&mutex, NULL));

    //Initialize Semaphores
    printf("%d", sem_init(&empty, 0, 32));
    printf("%d", sem_init(&full, 0, 0));


    //Create the producer and consumer threads
    pthread_t tid[producerThreads+consumerThreads];

    for(i = 0; i < producerThreads; i++)
    {
        pthread_create(&tid[i], NULL, producer, NULL);
    }

    for(j = producerThreads; j < (producerThreads+consumerThreads); j++)
    {
        pthread_create(&tid[j], NULL, consumer, NULL);
    }

    //Join threads
    for ( k = 0; k < (producerThreads+consumerThreads); ++k) {
        pthread_join(tid[k],NULL);
    }

    //Free mutex and semaphore
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}


void *producer(void *param)
{
    int val1, val2;

    while(1)
    {
        printf("num: %d, ", items_num);
        printf("Producer produced %d \n", val1);
        fflush(stdout);

        //Acquire Empty Semaphore
        sem_wait(&empty);

        //Acquire mutex lock to protect buffer
        pthread_mutex_lock(&mutex);

        val1 = randomNum(1, 10);

        sleep(randomNum(3, 8));

        val2 = randomNum(2, 9);

        insert_item(val1, val2);

        //Release mutex lock and full semaphore
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
}

void *consumer(void *param)
{
    while(1)
    {
        //Acquire Full Semaphore
        sem_wait(&full);

        //Acquire mutex lock to protect buffer
        pthread_mutex_lock(&mutex);

        struct item *con_item = remove_item();

        //Release mutex lock and empty semaphore
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        printf("num: %d, ", items_num);
        printf("Consumer consumed %d \n", con_item->num);
        fflush(stdout);

        sleep(con_item->sec);
    }
}