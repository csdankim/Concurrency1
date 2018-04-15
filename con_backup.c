#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <zconf.h>
#include "mt19937ar.h"

typedef int buffer_item;
#define BUFFER_SIZE 32

struct item {
    int num;
    int sec;
} buffer[BUFFER_SIZE];

//int insert_item(buffer_item item);
//int remove_item(buffer_item *item);


//buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int insertPointer = 0, removePointer = 0;
int items_num = 0;
int isRdrand = 0;

void *producer(void *param);
void *consumer(void *param);

// =============== Produce Random Number ===============
// check if to use rdrand
void isRdrandCheck() {
    unsigned int eax, ebx, ecx, edx;
    eax = 0x01;
    __asm__ __volatile__(
    "cpuid;"
    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
    : "a"(eax)
    );

    isRdrand = ecx & 0x40000000;
}

// return random number by rdrand
int rdrand(int min, int max) {
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

// return random number by mt19937
int mt19937(int min, int max) {
    init_genrand(time(NULL));
    int a = (int)genrand_int32();

    return (abs(a) % (max - 1)) + min;
}

// choose random method
int randomNum(int min, int max) {
    if (isRdrand)
        return rdrand(min, max);
    else
        return mt19937(min, max);
}

void insert_item(int val1, int val2)
{
//    //Acquire Empty Semaphore
//    sem_wait(&empty);
//
//    //Acquire mutex lock to protect buffer
//    pthread_mutex_lock(&mutex);
    buffer[insertPointer].num = val1;
    buffer[insertPointer].sec = val2;
    insertPointer = (insertPointer+1) % BUFFER_SIZE;
    items_num++;

//    //Release mutex lock and full semaphore
//    pthread_mutex_unlock(&mutex);
//    sem_post(&full);
}

struct item *remove_item()
{
//    //Acquire Full Semaphore
//    sem_wait(&full);
//
//    //Acquire mutex lock to protect buffer
//    pthread_mutex_lock(&mutex);
    removePointer = removePointer % BUFFER_SIZE;
    items_num--;
    return &(buffer[removePointer++]);

    //Release mutex lock and empty semaphore
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
}

int main(int argc, char *argv[])
{
    int producerThreads, consumerThreads;
    int i, j;

    if(argc !=3)
    {
        fprintf(stderr, "Usage: <producer threads> <consumer threads>₩\n");
        return -1;
    }

    //sleepTime = atoi(argv[1]);
    producerThreads = atoi(argv[1]);
    consumerThreads = atoi(argv[2]);

    isRdrandCheck();

    //Initialize the locks
    printf("%d", pthread_mutex_init(&mutex, NULL));

//    char *a = "semaphore 1";
//    char *b = "semaphore 2";
    printf("%d", sem_init(&empty, 0, 32));
    printf("%d", sem_init(&full, 0, 0));
    //srand(time(NULL));

    pthread_t tid[producerThreads+consumerThreads];
    //Create the producer and consumer threads
    for(i =0; i < producerThreads; i++)
    {
//        pthread_t tid;
//        pthread_attr_t attr;
//        pthread_attr_init(&attr);
        pthread_create(&tid[i], NULL, producer, NULL);
    }

    for(j =producerThreads; j < (producerThreads+consumerThreads); j++)
    {
//        pthread_t tid;
//        pthread_attr_t attr;
//        pthread_attr_init(&attr);
        pthread_create(&tid[j], NULL, consumer, NULL);
    }
    int k;
    for ( k = 0; k < (producerThreads+consumerThreads); ++k) {
        pthread_join(tid[k],NULL);
    }
//    //Sleep for user specified time
//    sleep(randomNum(3, 8));

    pthread_mutex_destroy(&mutex);

    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}


void *producer(void *param)
{
//    buffer_item random;
//    int r;
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

//        //Acquire Empty Semaphore
//        sem_wait(&empty);

        val1 = randomNum(1, 10);

//        sleep(randomNum(3, 8));

        val2 = randomNum(2, 9);

        insert_item(val1, val2);

//        printf("num: %d, ", items_num);
//        printf("Producer produced %d \n", val1);
//        fflush(stdout);

//        sem_post(&full);

        //Release mutex lock and full semaphore
        pthread_mutex_unlock(&mutex);
        sem_post(&full);

        sleep(randomNum(3, 8));

//        printf("num: %d, ", items_num);
//        printf("Producer produced %d \n", val1);
//        fflush(stdout);

    }
}

void *consumer(void *param)
{
//    buffer_item random;
//    int r;

    while(1)
    {
        //Acquire Full Semaphore
        sem_wait(&full);

        //Acquire mutex lock to protect buffer
        pthread_mutex_lock(&mutex);

//        //Acquire Full Semaphore
//        sem_wait(&full);

        struct item *con_item = remove_item();

//        printf("num: %d, ", items_num);
//        printf("Consumer consumed %d \n", con_item->num);
//        //printf("Consumer consumed %d \n", con_item->val2);
//        fflush(stdout);

//        sem_post(&empty);

        //Release mutex lock and empty semaphore
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        printf("num: %d, ", items_num);
        printf("Consumer consumed %d \n", con_item->num);
        //printf("Consumer consumed %d \n", con_item->val2);
        fflush(stdout);

        sleep(con_item->sec);
    }
}



//void *producer(void *param)
//{
//    buffer_item random;
//    int r;
//
//    while(1)
//    {
//        r = rand() % BUFFER_SIZE;
//        sleep(r);
//        random = rand();
//
//        if(insert_item(random))
//            fprintf(stderr, "Error");
//
//        printf("Producer produced %d \n", random);
//    }
//}
//
//void *consumer(void *param)
//{
//    buffer_item random;
//    int r;
//
//    while(1)
//    {
//        r = rand() % BUFFER_SIZE;
//        sleep(r);
//
//        if(remove_item(&random))
//            fprintf(stderr, "Error Consuming");
//
//        printf("Consumer consumed %d \n", random);
//    }
//}