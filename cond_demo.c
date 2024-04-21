#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

pthread_cond_t cd1;
pthread_cond_t cd2;
pthread_cond_t cd3;
char bufferA[1024];
char bufferB[1024];
pthread_mutex_t lock1;
pthread_mutex_t lock2;

void* pthread_jobA(void* arg) {
    FILE* fp = fopen("ERROR.log", "rt");
    while (1) {
        pthread_mutex_lock(&lock1);
        if (strlen(bufferA) != 0) {
            pthread_cond_wait(&cd1, &lock1);
        }
        if (fgets(bufferA, sizeof(bufferA), fp) == NULL) {
            exit(0);
        }
        pthread_cond_signal(&cd2);
        pthread_mutex_unlock(&lock1);
    }
}

void* pthread_jobB(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock1);
        if (strlen(bufferA) == 0) {
            pthread_cond_wait(&cd2, &lock1);
        }
        char* ret;
        if ((ret = strstr(bufferA, "E CHIUSECASE")) != NULL) {

            pthread_mutex_lock(&lock2);
            if (strlen(bufferB) != 0) {
                pthread_cond_wait(&cd2, &lock2);
            }
            strncpy(bufferB, bufferA, strlen(bufferA));
            bzero(bufferA, sizeof(bufferA));
            pthread_cond_signal(&cd1);
            pthread_cond_signal(&cd3);
            pthread_mutex_unlock(&lock2);

        } else {
            bzero(bufferA, sizeof(bufferA));
            pthread_cond_signal(&cd1);
        }
        pthread_mutex_unlock(&lock1);
    }
}

void* pthread_jobC(void* arg) {
    int fd = open("result.log", O_WRONLY | O_APPEND | O_CREAT, 0666);
    while (1) {
        pthread_mutex_lock(&lock2);
        if (strlen(bufferB) == 0) {
            pthread_cond_wait(&cd3, &lock2);
        }
        write(fd, bufferB, sizeof(bufferB));
        bzero(bufferB, sizeof(bufferB));
        pthread_cond_signal(&cd2);
        pthread_mutex_unlock(&lock2);
    }
}

int main() {

    pthread_t tid;
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_cond_init(&cd1, NULL);
    pthread_cond_init(&cd2, NULL);
    pthread_cond_init(&cd3, NULL);

    pthread_create(&tid, NULL, pthread_jobA, NULL);
    pthread_create(&tid, NULL, pthread_jobB, NULL);
    pthread_create(&tid, NULL, pthread_jobC, NULL);
    while (1) {
        sleep(1);
    }
    return 0;
}