#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

int main() {
    key_t key;
    int msqid, shmid, numNodes;
    int *nodesPtr;

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
    message readRecMessage;

    if (msgrcv(msqid, &readRecMessage, sizeof(readRecMessage.mtext), 2, 0) == -1) {
        perror("Error receiving message from the load balancer");
        exit(EXIT_FAILURE);
    }
    int operation;
    char graphFileName[50];

    if (sscanf(readRecMessage.mtext, "%d %49s", &operation, graphFileName) != 2) {
        fprintf(stderr, "Error parsing the received message\n");
        exit(EXIT_FAILURE);
    }
//    shmid = shmget(key, sizeof(int), 0);
//    if (shmid == -1) {
//        perror("Error getting shared memory for numNodes");
//        exit(EXIT_FAILURE);
//    }
//    nodesPtr = (int *)shmat(shmid, NULL, 0);
//    if ((intptr_t)nodesPtr == -1) {
//        perror("Error attaching shared memory for numNodes");
//        exit(EXIT_FAILURE);
//    }
//    numNodes = *nodesPtr;
//    // Attach to the shared memory for adjMatrix
//    shmid = shmget(key, (numNodes * numNodes) * sizeof(int), 0);
//    if (shmid == -1) {
//        perror("Error getting shared memory for adjMatrix");
//        exit(EXIT_FAILURE);
//    }
//    int *adjMatrixPtr = (int *)shmat(shmid, NULL, 0);
//    if ((intptr_t)adjMatrixPtr == -1) {
//        perror("Error attaching shared memory for adjMatrix");
//        exit(EXIT_FAILURE);
//    }
    printf("Received operation from load balancer: %d\n", operation);
    printf("Received graph file name: %s\n", graphFileName);
//    printf("Shared Adjacency Matrix:\n");
//
//    for (int i = 0; i < numNodes; i++) {
//        for (int j = 0; j < numNodes; j++) {
//            printf("%d ", adjMatrixPtr[i * numNodes + j]);
//        }
//        printf("\n");
//    }
//    shmdt(nodesPtr);
//    shmdt(adjMatrixPtr);
//

    return 0;
}
