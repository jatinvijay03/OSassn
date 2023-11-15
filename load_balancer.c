#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CONTROL_FILE "queue_control_file"
#define PERMISSIONS 0666

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

int main() {
    key_t key;
    int msqid;
    message receiveMessage;

    if (access(CONTROL_FILE, F_OK) == -1) {
        key = ftok("load_balancer.c", 'A');
        if (key == -1) {
            perror("Error generating key");
            exit(EXIT_FAILURE);
        }
        msqid = msgget(key, IPC_CREAT | PERMISSIONS);
        if (msqid == -1) {
            perror("Error creating message queue");
            exit(EXIT_FAILURE);
        }
        int control_fd = open(CONTROL_FILE, O_CREAT | O_WRONLY, PERMISSIONS);
        if (control_fd == -1) {
            perror("Error creating control file");
            exit(EXIT_FAILURE);
        }
        printf("Message queue created successfully by the load balancer\n");
    } else {
        printf("Message queue already exists. No need to create by the load balancer\n");
    }

    while (1) {
        if (msgrcv(msqid, &receiveMessage, sizeof(receiveMessage.mtext), 1, 0) == -1) {
            perror("Error receiving message from the client");
            exit(EXIT_FAILURE);
        }
        // mtype 1 for client sending tuple to the load balancer
        if (receiveMessage.mtype == 1) {
            int seqNum, opNum;
            char graphFileName[50];

            if (sscanf(receiveMessage.mtext, "%d %d %49s", &seqNum, &opNum, graphFileName) != 3) {
                fprintf(stderr, "Error parsing the received message\n");
                exit(EXIT_FAILURE);
            }
            if(opNum == 1 || opNum == 2) {
                // logic for primary server
                // use mtype 2 here
            }
            else if(opNum == 3 || opNum == 4){
                // logic for secondary server
                // use mtype 3 here
            }
            else {
                fprintf(stderr,"Invalid operation number")
            }

        }
    }

    return 0;
}
