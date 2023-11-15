#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message {
    long mtype;
    char tuple[100];
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

    msqid = msgget(key, 0);
    if (msqid == -1) {
        perror("Error opening message queue");
        exit(EXIT_FAILURE);
    }
    printf("Message queue opened successfully by the client.\n");

    message sendMessage;
    int sequenceNumber, operationNumber;
    char *graphFileName = (char *)malloc(50 * sizeof(char));

    printf("1. Add a new graph to the database\n2. Modify an existing graph of the database\n3. Perform DFS on an existing graph of the database\n4. Perform BFS on an existing graph of the database\n");
    printf("Enter the sequence number: ");
    scanf("%d", &sequenceNumber);
    printf("Enter the operation number: ");
    scanf("%d", &operationNumber);
    printf("Enter the graph file name: ");
    scanf("%s", graphFileName);

    char seq[20], op[20];
    snprintf(seq, sizeof(seq), "%d", sequenceNumber);
    snprintf(op, sizeof(op), "%d", operationNumber);
    strcpy(sendMessage.tuple, seq);
    strcat(sendMessage.tuple, " ");
    strcat(sendMessage.tuple, op);
    strcat(sendMessage.tuple, " ");
    strcat(sendMessage.tuple, graphFileName);
    sendMessage.mtype = 1;
    if (msgsnd(msqid, &message, sizeof(message.mtext), 0) == -1) {
        printf("Error in sending message to load balancer\n");
        exit(-1);
    }

    if (operationNumber == 1 || operationNumber == 2) {
        int numNodes;
        printf("Enter number of nodes of the graph: ");
        scanf("%d", &numNodes);
        int **adjMatrix = (int **)malloc(numNodes * sizeof(int *));
        for (int i = 0; i < numNodes; i++) {
            adjMatrix[i] = (int *)malloc(numNodes * sizeof(int));
        }
        printf("Enter the adjacency matrix, each row on a separate line, and elements of a single row separated by whitespace characters:\n");
        for (int i = 0; i < numNodes; i++) {
            for (int j = 0; j < numNodes; j++) {
                scanf("%d", &adjMatrix[i][j]);
            }
        }
    }
    else {
        int startVertex;
        printf("Enter the starting vertex: ");
        scanf("%d",&startVertex);
    }
    return 0;
}
