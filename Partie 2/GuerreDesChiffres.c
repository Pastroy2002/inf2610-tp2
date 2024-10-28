#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
//..

int sizeTamp;
int* tamp;
sem_t prod;
sem_t cons;
sem_t mutex;
int flag_de_fin = 0;
int produced = 0;
int consumed = 0;

void produceNumber(int number) {
    sem_wait(&prod);
    sem_wait(&mutex);
    int index = produced % sizeTamp;
    tamp[index] = number;
    produced++;
    sem_post(&mutex);
    sem_post(&cons);
}

int consumeNumber() {
    sem_wait(&cons);
    sem_wait(&mutex);
    int index = consumed % sizeTamp;
    int value = tamp[index];
    consumed++;
    sem_post(&mutex);
    sem_post(&prod);
    return value;
}

void* producteur(void* pid) {
    int* sum = malloc(sizeof(int)); 
    *sum = 0;
    while (flag_de_fin != 1) {
        int r = (rand ()%9) + 1;
        *sum += r;
        produceNumber(r);
    }
    pthread_exit((void*) sum);
}


void* consommateur(void* cid) {
    int* sum = malloc(sizeof(int)); 
    *sum = 0;
    while(1) {
        int value = consumeNumber();
        *sum += value;
        if (value == 0) break;
        
    }
    pthread_exit((void*) sum);
}

void setToTrue(int sig) {
    flag_de_fin = 1;
}

void setUp() {
    signal(SIGALRM, setToTrue);
    srand(time(NULL));
    
    sem_init(&prod,0, sizeTamp);
    sem_init(&cons, 0, 0);
    sem_init(&mutex,0, 1);

    tamp = (int*)malloc(sizeTamp*sizeof(int));
}

int getSum(int nbThreads, pthread_t threads[]) {
    int sum = 0;
    for (int i = 0; i < nbThreads; i++) {
        int* value;
        pthread_join(threads[i], (void**) &value);
        sum += *value;
        free(value);
    }
    return sum;
}

int main(int argc, char* argv[]) {
    // Les paramètres du programme sont, dans l'ordre :
    // le nombre de producteurs, le nombre de consommateurs
    // et la taille du tampon.
    
    const int nbProd = atoi(argv[1]);
    const int nbCons = atoi(argv[2]);
    sizeTamp = atoi(argv[3]);
    setUp();
    pthread_t consThread[nbCons];
    pthread_t prodThread[nbProd];

    for (int i = 0; i < nbProd; i++) {
        pthread_create(&prodThread[i], NULL, producteur, (void*)i);
    }

    for (int i = 0; i < nbCons; i++) {
        pthread_create(&consThread[i], NULL, consommateur, (void*)i);
    }

    alarm(1);

    int prodSum = getSum(nbProd, prodThread);

    for (int i = 0; i < nbCons; i++) {
        produceNumber(0);
    }


    int consSum = getSum(nbCons, consThread);

    free(tamp);

    printf("Somme des nombres produits: %d\t Somme des nombres consommes: %d\n", prodSum, consSum);
    printf("Nombre de chiffres produits: %d\t Nombre de chiffres consommes: %d\n", produced, consumed);
    return 0;
}
