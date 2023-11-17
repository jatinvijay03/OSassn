#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define buf 1000*sizeof(int)

key_t key;

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

void *makeModify(void *fileName)
{
    key_t newkey=ftok("client.c", 'B');
    int shmid = shmget(newkey, buf, IPC_CREAT | 0666);
    if (shmid == -1) 
    {
        perror("Error getting shared memory key for numNodess");
        pthread_exit(0);
    }
    int *shmptr = (int *)shmat(shmid,NULL,0);
    int numNode = *shmptr;
    FILE * file = fopen(fileName,"w");
    if (file == NULL) {
        perror("Error opening file");
        exit(0);
    }
    shmptr++;
    int adjMatrix[numNode][numNode];
    fprintf(file,"%d\n",numNode);
    for (int i = 0; i < numNode; i++) {
        for (int j = 0; j < numNode; j++) {
            adjMatrix[i][j]=*shmptr;
            fprintf(file,"%d ",adjMatrix[i][j]);
            shmptr++;
        }
        fprintf(file,"\n");
    }
    fclose(file);
    shmdt(shmptr);
    pthread_exit(0);
}

int main() {
    int msqid, shmid;

    key = ftok("load_balancer.c", 'A');
    if (key == -1) {
        perror("Error generating key");
        exit(EXIT_FAILURE);
    }

    while(1) {
        msqid = msgget(key, IPC_CREAT | 0666);
        if (msqid == -1) {
        perror("Error creating/opening message queue");
        exit(EXIT_FAILURE);
        }
        message readRecMessage;

        if (msgrcv(msqid, &readRecMessage, sizeof(readRecMessage.mtext), 2, 0) == -1) 
        {
            perror("Error receiving message from the load balancer");
            exit(EXIT_FAILURE);
        }
        int operation;
        char graphFileName[50];
        if (sscanf(readRecMessage.mtext, "%d %49s", &operation, graphFileName) != 2) 
        {
            fprintf(stderr, "Error parsing the received message\n");
            exit(EXIT_FAILURE);
        }
        printf("Received operation from load balancer: %d\n", operation);
        printf("Received graph file name: %s\n", graphFileName);
        pthread_t tid;
        pthread_create(&tid,NULL,makeModify,graphFileName);
        pthread_join(tid,NULL);
        message successMessage;
        successMessage.mtype = 4;
        strcpy(successMessage.mtext,"Succesfully Modified file\n");
        if (msgsnd(msqid, &successMessage, sizeof(successMessage.mtext), 0) == -1) 
        {
            perror("Error sending message to the client");
            exit(EXIT_FAILURE);
        }

    }
}