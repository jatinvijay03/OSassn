#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

int main() {
    key_t key;
    int msqid;

    key = ftok("load_balancer.c", 'A');
    if (key == -1) {
        perror("Error generating key");
        exit(EXIT_FAILURE);
    }

    msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("Error creating/opening message queue");
        exit(EXIT_FAILURE);
    }
    message writeRecMessage;

    if (msgrcv(msqid, &writeRecMessage, sizeof(writeRecMessage.mtext), 3, 0) == -1) {
        perror("Error receiving message from the load balancer");
        exit(EXIT_FAILURE);
    }
    int operation;
    char graphFileName[50];

    if (sscanf(writeRecMessage.mtext, "%d %49s", &operation, graphFileName) != 2) {
        fprintf(stderr, "Error parsing the received message\n");
        exit(EXIT_FAILURE);
    }

    printf("Received operation from load balancer: %d\n", operation);
    printf("Received graph file name: %s\n", graphFileName);

    while(1){
        int shmid = shmget(key, sizeof(int), 0666);
        if (shmid == -1) {
            perror("Error getting shared memory");
            exit(EXIT_FAILURE);
        }
        int *sharedData = (int *)shmat(shmid, NULL, 0);
        if ((intptr_t)sharedData == -1) {
            perror("Error attaching to shared memory");
            exit(EXIT_FAILURE);
        }
        printf("Data read from shared memory: %d\n", *sharedData);
        if (shmdt(sharedData) == -1) {
            perror("Error detaching from shared memory");
            exit(EXIT_FAILURE);
        }
        break;
    }
    

    return 0;
}
