#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>

#define CONTROL_FILE "queue_control_file"
#define PERMISSIONS 0666

struct message {
    long mtype;
    char mtext[100];
};
typedef struct message message;

int main() {
    key_t key;
    int msgid;

    if (access(CONTROL_FILE, F_OK) == -1) {
        key = ftok("load_balancer.c", 'A');
        if (key == -1) {
            perror("Error generating key");
            exit(EXIT_FAILURE);
        }
        msgid = msgget(key, IPC_CREAT | PERMISSIONS);
        if (msgid == -1) {
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
}
