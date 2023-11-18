#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

struct message
{
    long mtype;
    char mtext[100];
};
typedef struct message message;

typedef struct
{
    pthread_t tid;
} ThreadInfo;

char graphFileName[50];
int maxNodes;
int adjMatrix[30][30];
int msqid;
char sequenceNumber[4];
int visited[30];


void * DFS(void * fileName);
void * DFS2(void * arg);
void * BFS(void * fileName);

char result[60];

int main()
{
    key_t key;
    

    key = ftok("load_balancer.c", 'A');
    if (key == -1)
    {
        perror("Error generating key");
        exit(EXIT_FAILURE);
    }

    msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1)
    {
        perror("Error creating/opening message queue");
        exit(EXIT_FAILURE);
    }
    message writeRecMessage;
    int evenodd;
    printf("Is this instance even or odd? Enter 0 for even or 1 for odd \n");
    scanf("%d", &evenodd);
    int whichmtype;
    if (evenodd == 0) whichmtype = 108;
    else whichmtype = 103;
    while (1)
        {
            if (msgrcv(msqid, &writeRecMessage, sizeof(writeRecMessage.mtext), whichmtype, 0) == -1)
            {
                perror("Error receiving message from the load balancer");
                exit(EXIT_FAILURE);
            }
            if (strcmp(writeRecMessage.mtext, "Terminate") == 0)
            {
                printf("bye");
                exit(0);
            }
            int operation;
            

            if (sscanf(writeRecMessage.mtext, "%d %s %49s", &operation, sequenceNumber, graphFileName) != 3)
            {
                fprintf(stderr, "Error parsing the received message\n");
                exit(EXIT_FAILURE);
            }

            printf("Received operation from load balancer: %d\n", operation);
            printf("Received graph file name: %s\n", graphFileName);

            while (1)
            {
                if (operation == 3)
                {
                    // DFS
                    pthread_t tid;
                    pthread_create(&tid, NULL, DFS, graphFileName);
                    pthread_join(tid, NULL);
                }
                else if (operation == 4)
                {
                    // // BFS
                    // pthread_t tid;
                    // pthread_create(&tid, NULL, BFS, graphFileName);
                    // pthread_join(tid, NULL);
                }
                else
                {
                    printf("Invalid operation number\n");
                }
                
                break;
            }
        }

        return 0;
    }
      
void *populateMatrix(void *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(0);
    }

    fscanf(file, "%d", &maxNodes);

    for (int i = 0; i < maxNodes; i++)
    {
        for (int j = 0; j < maxNodes; j++)
        {
            fscanf(file, "%d", &adjMatrix[i][j]);
        }
    }
    fclose(file);

    pthread_exit(NULL);
}

void *DFS(void *fileName)
{   
    for(int i = 0;i<30;i++){
        visited[i] = 0;
    }
    key_t newkey = ftok("client.c", 'S');
    int shmid = shmget(newkey, sizeof(int), 0666);
    if (shmid == -1)
    {
        perror("Error getting shared memory");
        exit(EXIT_FAILURE);
    }
    int *sharedData = (int *)shmat(shmid, NULL, 0);
    if ((intptr_t)sharedData == -1)
    {
        perror("Error attaching to shared memory");
        exit(EXIT_FAILURE);
    }
    int* startVer = malloc(sizeof(int*));
    *startVer = (*sharedData) -1;

    printf("Data read from shared memory: %d\n",*startVer );
    printf("yo");
    if (shmdt(sharedData) == -1)
    {
        perror("Error detaching from shared memory");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(fileName, "r");
    printf("file opened");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(0);
    }



    fscanf(file, "%d", &maxNodes);

    for (int i = 0; i < maxNodes; i++)
    {
        for (int j = 0; j < maxNodes; j++)
        {
            fscanf(file, "%d", &adjMatrix[i][j]);
        }
    }
    fclose(file);
    printf("file closed");
    

    printf("%d\n",maxNodes);
    for(int i = 0;i<maxNodes;i++){
        for(int j = 0;j<maxNodes;j++){
            printf("%d ",adjMatrix[i][j]);
        }
        printf("\n");
    }

    pthread_t pid2;
    pthread_create(&pid2, NULL, DFS2, startVer);
    pthread_join(pid2, NULL);
    printf("output done");
    sleep(1);
    message writeMessage;
    writeMessage.mtype = atoi(sequenceNumber) + 1000;
    strcpy(writeMessage.mtext, result);
    if (msgsnd(msqid, &writeMessage, sizeof(writeMessage.mtext), 0) == -1)
    {
        perror("Error sending message to the client");
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}

void *DFS2(void *arg) {
    int i = *((int *)arg);
    free(arg);

    ThreadInfo *info = (ThreadInfo *)malloc(sizeof(ThreadInfo));
    int flag = 1;
    visited[i] = 1;

    for (int j = 0; j < maxNodes; ++j) {
        if (adjMatrix[i][j] && !visited[j]) {
            flag = 0;

            int *start = malloc(sizeof(*start));
            *start = j;

            pthread_create(&info->tid, NULL, DFS2, start);
            pthread_join(info->tid, NULL);

            
        }
    }
    if (flag == 1) {
        printf("%d ", i + 1);
        char temp[30];
        sprintf(temp, "%d ", i + 1);
        strcat(result, temp);
    }
    free(info);
    pthread_exit(NULL);
}
