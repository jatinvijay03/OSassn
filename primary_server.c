#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define buf 1000*sizeof(int)

key_t key;

sem_t mutex;

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

void *makeModify(void *fileName)
{
    key_t newkey=ftok("client.c", 'B');
    int shmid = shmget(newkey, buf, IPC_CREAT | 0666);
    printf("Shared mem created\n");
    if (shmid == -1) 
    {
        perror("Error getting shared memory key for numNodess");
        pthread_exit(0);
    }
    int *shmptr = (int *)shmat(shmid,NULL,0);
    int numNode = *shmptr;
    printf("%d\n",numNode);
    sem_wait(&mutex);
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
            printf("%d",*shmptr);
            adjMatrix[i][j]= *shmptr;
            fprintf(file,"%d ",adjMatrix[i][j]);
            shmptr++;
        }
        fprintf(file,"\n");
    }
    printf("File modified successfully\n");
    fclose(file);
    sem_post(&mutex);
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
    sem_init(&mutex, 0, 1); 
    while(1) {
        msqid = msgget(key, IPC_CREAT | 0666);
        if (msqid == -1) {
        perror("Error creating/opening message queue");
        exit(EXIT_FAILURE);
        }
        message readRecMessage;
        if (msgrcv(msqid, &readRecMessage, sizeof(readRecMessage.mtext), 102, 0) == -1) 
        {
            perror("Error receiving message from the load balancer");
            exit(EXIT_FAILURE);
        }
        if(strcmp(readRecMessage.mtext,"Terminate") == 0)
        {
            printf("bye");
            exit(0);
        }
        int operation;
        char seqNum[4];
        char graphFileName[50];
        if (sscanf(readRecMessage.mtext, "%d %s %49s", &operation, seqNum ,graphFileName) != 3) 
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
        successMessage.mtype = 1000 + atoi(seqNum);
        strcpy(successMessage.mtext,"Succesfully Modified file\n");
        if (msgsnd(msqid, &successMessage, sizeof(successMessage.mtext), 0) == -1) 
        {
            perror("Error sending message to the client");
            exit(EXIT_FAILURE);
        }
        
    }
    sem_destroy(&mutex);
}