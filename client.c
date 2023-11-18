#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define buf 1000*sizeof(int)

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
    printf("Message queue created/opened successfully by the client.\n");

    message sendMessage;
    char sequenceNumber[4];
    int operationNumber;
    char *graphFileName = (char *)malloc(50 * sizeof(char));

    while (1) {
        printf("1. Add a new graph to the database\n2. Modify an existing graph of the database\n3. Perform DFS on an existing graph of the database\n4. Perform BFS on an existing graph of the database\n");
        printf("Enter the sequence number: ");
        scanf("%s", sequenceNumber);
        printf("Enter the operation number: ");
        scanf("%d", &operationNumber);
        printf("Enter the graph file name: ");
        scanf("%s", graphFileName);

        char seq[20], op[20];
        snprintf(seq, sizeof(seq), "%s", sequenceNumber);
        snprintf(op, sizeof(op), "%d", operationNumber);
        strcpy(sendMessage.mtext, seq);
        strcat(sendMessage.mtext, " ");
        strcat(sendMessage.mtext, op);
        strcat(sendMessage.mtext, " ");
        strcat(sendMessage.mtext, graphFileName);
        sendMessage.mtype = atoi(sequenceNumber);
        if (operationNumber == 1 || operationNumber == 2) {
            key_t newkey=ftok("client.c", 'B');
            int numNodes;
            printf("Enter number of nodes of the graph: ");
            scanf("%d", &numNodes);
            
            int shmid = shmget(newkey, buf, IPC_CREAT | 0666);
            if(shmid == -1) {
                perror("Error creating the shared memory");
                exit(-2);
            }
            int *shmptr = (int *)shmat(shmid,NULL,0);
            *shmptr = numNodes;
            shmptr++;
            int adjMatrix[numNodes][numNodes];
            printf("Enter the adjacency matrix, each row on a separate line, and elements of a single row separated by whitespace characters:\n");
            for (int i = 0; i < numNodes; i++) {
                for (int j = 0; j < numNodes; j++) {
                    scanf("%d", &adjMatrix[i][j]);
                    *shmptr=adjMatrix[i][j];
                    shmptr++;
                }
            }
            
            if (msgsnd(msqid, &sendMessage, sizeof(sendMessage.mtext), 0) == -1) {
            printf("Error in sending message to load balancer\n");
            exit(-1);
            }
            
            message receiveSuccessMessage;
          
            if (msgrcv(msqid, &receiveSuccessMessage, sizeof(receiveSuccessMessage.mtext), atoi(sequenceNumber)+1000, 0) == -1) {
                perror("Error receiving message from the primary server");
                exit(EXIT_FAILURE);
            }
            printf("%s",receiveSuccessMessage.mtext);
            shmdt(shmptr);
            if (shmctl(shmid, IPC_RMID, NULL) == -1) 
            {
                perror("Error removing shared memory segment");
                exit(EXIT_FAILURE);
            }
        } 
        else 
        {
            int startVertex;
            printf("Enter the starting vertex: ");
            scanf("%d", &startVertex);
            key_t newkeyS=ftok("client.c", 'S');
            int shmid = shmget(newkeyS, buf, IPC_CREAT | 0666);
            if(shmid == -1) {
                perror("Error creating the shared memory");
                exit(-2);
            }
            int *shmptr = (int *)shmat(shmid,NULL,0);
            *shmptr = startVertex;

            // shared memory is put, now sending message in msgqueue to lb

            if (msgsnd(msqid, &sendMessage, sizeof(sendMessage.mtext), 0) == -1) {
            printf("Error in sending message to load balancer\n");
            exit(-1);
            }
            
            // now we wait for answer from ss

            message receiveSuccessMessage;
          
            if (msgrcv(msqid, &receiveSuccessMessage, sizeof(receiveSuccessMessage.mtext), atoi(sequenceNumber)+1000, 0) == -1) {
                perror("Error receiving message from the primary server");
                exit(EXIT_FAILURE);
            }
            printf("%s",receiveSuccessMessage.mtext);
            shmdt(shmptr);
            if (shmctl(shmid, IPC_RMID, NULL) == -1) 
            {
                perror("Error removing shared memory segment");
                exit(EXIT_FAILURE);
            }
        }
    }
}