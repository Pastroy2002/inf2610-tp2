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

void* producteur(void* pid) {
    int* sum = malloc(sizeof(int)); 
    *sum = 0;
    while (flag_de_fin != 1) {
        int r = (rand ()%9) + 1;
        *sum += r;
        sem_wait(&prod);
        sem_wait(&mutex);
        int index = produced % sizeTamp;
        tamp[index] = r;
        produced++;
        sem_post(&mutex);
        sem_post(&cons);
    }
    pthread_exit((void*) sum);
}

void* consommateur(void* cid) {
    int* sum = malloc(sizeof(int)); 
    *sum = 0;
    while(1) {
        sem_wait(&cons);
        sem_wait(&mutex);
        int index = consumed % sizeTamp;
        int value = tamp[index];
        if (value == 0) break;
        *sum += value;
        consumed++;
        sem_post(&mutex);
        sem_post(&prod);
    }
    pthread_exit((void*) sum);
}

void setToTrue(int sig) {
    flag_de_fin = 1;
}

int main(int argc, char* argv[]) {
    // Les paramètres du programme sont, dans l'ordre :
    // le nombre de producteurs, le nombre de consommateurs
    // et la taille du tampon.
    signal(SIGALRM, setToTrue);
    srand(time(NULL));
    const int nbProd = atoi(argv[1]);
    const int nbCons = atoi(argv[2]);
    sizeTamp = atoi(argv[3]);
    sem_init(&prod,0, sizeTamp);
    sem_init(&cons, 0, 0);
    sem_init(&mutex,0, 1);
    pthread_t consThread[nbCons];
    pthread_t prodThread[nbProd];
    tamp = (int*)malloc(sizeTamp*sizeof(int));
    int pid = 0;
    int cid = 0;

    for (int i = 0; i < nbProd; i++) {
        pthread_create(&prodThread[i], NULL, producteur, (void*)i);
    }

    for (int i = 0; i < nbCons; i++) {
        pthread_create(&consThread[i], NULL, consommateur, (void*)i);
    }

    alarm(1);

    int prodSum = 0;
    for (int i = 0; i < nbProd; i++) {
        int* value;
        pthread_join(prodThread[i], (void**) &value);
        prodSum += *value;
        free(value);
    }

    sem_wait(&mutex);
    for (int i = 0; i < sizeTamp; i++) {
        tamp[i] = 0;
    }

    for (int i = 0; i < nbCons; i++) {
        sem_post(&cons);
        sem_post(&mutex);
    }

    int consSum = 0;
    for (int i = 0; i < nbCons; i++) {
        int* value;
        pthread_join(consThread[i], (void**) &value);
        consSum += *value;
        free(value);
    }

    free(tamp);

    printf("Somme des nombres produits: %d\t Somme des nombres consommes: %d\n", prodSum, consSum);
    printf("Nombre de chiffres produits: %d\t Nombre de chiffres consommes: %d\n", produced, consumed);
    return 0;
}
