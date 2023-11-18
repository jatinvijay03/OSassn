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
int fileFlag = 0;

void *DFS(void *fileName)
{
    int maxNodes;
    int adjMatrix[maxNodes][maxNodes];
    int startVer;
    if (fileFlag == 0)
    {

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
        printf("Data read from shared memory: %d\n", *sharedData);
        if (shmdt(sharedData) == -1)
        {
            perror("Error detaching from shared memory");
            exit(EXIT_FAILURE);
        }
        startVer = *sharedData;
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
        int fileFlag = 1;
    }
    else {
        ThreadInfo *info = (ThreadInfo *)malloc(sizeof(ThreadInfo));
        int flag = 1;
        int i = startVer;
        for (int j = i + 1; j < maxNodes; ++j)
        {
            if (adjMatrix[i][j])
            {
                flag = 0;

                int *start = malloc(sizeof(*start));
                *start = j;
                pthread_create(&info->tid, NULL, DFS, start);
                pthread_join(info->tid, NULL);
            }
        }
        if (flag == 1)
        {
            printf("%d ", i + 1);
        }
        free(info);
        pthread_exit(NULL);
    }  
}

void *BFS(void *fileName)
{
    // key_t newkey = ftok("client.c", 'S');

    // int shmid = shmget(newkey, sizeof(int), 0666);
    // if (shmid == -1)
    // {
    //     perror("Error getting shared memory");
    //     exit(EXIT_FAILURE);
    // }
    // int *sharedData = (int *)shmat(shmid, NULL, 0);
    // int startVer = *sharedData;
    // if ((intptr_t)sharedData == -1)
    // {
    //     perror("Error attaching to shared memory");
    //     exit(EXIT_FAILURE);
    // }
    // printf("Data read from shared memory: %d\n", startVer);
    // if (shmdt(sharedData) == -1)
    // {
    //     perror("Error detaching from shared memory");
    //     exit(EXIT_FAILURE);
    // }
}

int main()
{
    key_t key;
    int msqid;

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
    if (evenodd == 0)
    {
        // even instance
        while (1)
        {
            if (msgrcv(msqid, &writeRecMessage, sizeof(writeRecMessage.mtext), 108, 0) == -1)
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
            char sequenceNumber[4];

            if (sscanf(writeRecMessage.mtext, "%d %s %49s", &operation, sequenceNumber, graphFileName) != 3)
            {
                fprintf(stderr, "Error parsing the received message\n");
                exit(EXIT_FAILURE);
            }

            printf("Received operation from load balancer: %d\n", operation);
            printf("Received graph file name: %s\n", graphFileName);

            while (1)
            {
                printf("45");
                printf("54");
                if (operation == 3)
                {
                    // DFS
                    printf("0\n");
                    pthread_t tid;
                    printf("1\n");
                    pthread_create(&tid, NULL, DFS, graphFileName);
                    printf("2\n");
                    pthread_join(tid, NULL);
                    printf("3\n");
                }
                else if (operation == 4)
                {
                    // BFS
                    pthread_t tid;
                    pthread_create(&tid, NULL, BFS, graphFileName);
                    pthread_join(tid, NULL);
                }
                else
                {
                    printf("Invalid operation number\n");
                }
                message writeMessage;
                writeMessage.mtype = atoi(sequenceNumber) + 1000;
                strcpy(writeMessage.mtext, "Fuck yeah this shit works (even instance)\n");
                if (msgsnd(msqid, &writeMessage, sizeof(writeMessage.mtext), 0) == -1)
                {
                    perror("Error sending message to the client");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }
    else
    {
        while (1)
        {
            // odd instance
            if (msgrcv(msqid, &writeRecMessage, sizeof(writeRecMessage.mtext), 103, 0) == -1)
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
            char sequenceNumber[4];

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
                    printf("0\n");
                    pthread_t tid;
                    printf("1\n");
                    pthread_create(&tid, NULL, DFS, graphFileName);
                    printf("2\n");
                    pthread_join(tid, NULL);
                    printf("3\n");
                }
                else if (operation == 4)
                {
                    // BFS
                    pthread_t tid;
                    pthread_create(&tid, NULL, BFS, graphFileName);
                    pthread_join(tid, NULL);
                }
                else
                {
                    printf("Invalid operation number\n");
                }
                message writeMessage;
                writeMessage.mtype = atoi(sequenceNumber) + 1000;
                strcpy(writeMessage.mtext, "Fuck yeah this shit works (odd instance)\n");
                if (msgsnd(msqid, &writeMessage, sizeof(writeMessage.mtext), 0) == -1)
                {
                    perror("Error sending message to the client");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }

    return 0;
}