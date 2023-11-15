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
                message writeMessage;
                writeMessage.mtype = 2;
                snprintf(writeMessage.mtext, sizeof(writeMessage.mtext), "%d", opNum);
                strcat(writeMessage.mtext," ");
                strcat(writeMessage.mtext,graphFileName);

                if (msgsnd(msqid, &writeMessage, sizeof(writeMessage.mtext), 0) == -1) {
                    perror("Error sending message to the primary server");
                    exit(EXIT_FAILURE);
                }
            }
            else if(opNum == 3 || opNum == 4){
                // logic for secondary server
                message readMessage;
                readMessage.mtype = 3;
                snprintf(readMessage.mtext, sizeof(readMessage.mtext), "%d", opNum);
                strcat(readMessage.mtext," ");
                strcat(readMessage.mtext,graphFileName);

                if (msgsnd(msqid, &readMessage, sizeof(readMessage.mtext), 0) == -1) {
                    perror("Error sending message to the secondary server");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr,"Invalid operation number");
            }

        }
    }

    return 0;
}
