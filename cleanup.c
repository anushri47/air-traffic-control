//cleanup.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message_queue_key.h"

#define MAX_MSG_SIZE 256

struct msg_buffer {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
};

int main() {
    // Access the existing message queue
    key_t key;
    int msgid;

    int msgqid = msgget(1234,  IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("msgget failed");
        return 1;
    }

    // Main loop for cleanup process
    while (1) {
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No)\n");

        // Read user input
        char input[2];
        fgets(input, sizeof(input), stdin);

        if (input[0] == 'Y' || input[0] == 'y') {
            // Send termination message to Air Traffic Controller
            struct msg_buffer msg;
            msg.msg_type = 1; // Set message type
            strcpy(msg.msg_text, "TERMINATE");
            if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            printf("Termination request sent to Air Traffic Controller. Exiting cleanup process.\n");
            break; // Exit loop and terminate cleanup process
        } else if (input[0] == 'N' || input[0] == 'n') {
            // Continue running cleanup process
            continue;
        } else {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    return 0;
}
