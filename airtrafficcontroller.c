//airtrafficcontroller.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define MAX_AIRPORTS 10
#define MAX_MESSAGE_SIZE 150 // Reduced buffer size
#define MESSAGE_QUEUE_KEY 1234 // Example key, change as needed

struct PassengerDetails {
    int luggage_weight;
    int passenger_weight;
};

struct PlaneDetails {
	int flag;
	int flag2;   //1 for dept and 0 for arr
    long msg_type;
    int airport_departure;
    int airport_arrival;
    int plane_id;
    int total_weight;
    int plane_type; // 1 for passenger, 0 for cargo
    int num_passengers; // Relevant only for passenger planes
    int occupied_seats; // Relevant only for passenger planes
    struct PassengerDetails passengers[10];
};

// Define message structure for communication
struct message {
    long mtype; // Message type
    char mtext[MAX_MESSAGE_SIZE]; // Message content
};

// Function to send message via message queue
void send_message(int msgqid, long mtype, char *mtext) {
    struct message msg;
    msg.mtype = mtype;
    strcpy(msg.mtext, mtext);
    if (msgsnd(msgqid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

// Function to receive message via message queue
void receive_message(int msgqid, long mtype, char *mtext) {
    struct message msg;
    if (msgrcv(msgqid, &msg, sizeof(msg.mtext), mtype, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }	printf("hi 3\n");
    strcpy(mtext, msg.mtext);
}

// Function to append plane journey to AirTrafficController.txt
void append_to_file(int plane_id, int departure_airport, int arrival_airport) {
    FILE *file = fopen("AirTrafficController.txt", "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }	printf("hi 2\n");
    fprintf(file, "Plane %d has departed from Airport %d and will land at Airport %d.\n", plane_id, departure_airport, arrival_airport);
    fclose(file);
}

// Function to communicate with airports and planes
void communicate(int msgqid, int num_airports) {
    // Initialize variables to track airport states
    int airport_states[MAX_AIRPORTS] = {0}; // 0 = available, 1 = busy
    int airport_id, plane_id, departure_airport, arrival_airport;
	struct PlaneDetails plane;
    // Communication loop
    //while (1) {
        // Receive message from a plane
        struct message msg;
        
        //receive_message(msgqid, 1, msg.mtext); // Assuming type 1 is for plane messages
        
        msgrcv(msgqid, &plane, sizeof(plane), 1, 0);
        
        printf("hugfh\n");
	plane.msg_type = 5*plane.airport_departure;
	printf("%d\n",5*plane.airport_departure);
	msgsnd(msgqid, &plane, sizeof(plane), 0);
	printf("hiiiii\n");
        printf("%d\n",plane.msg_type);
	//msgrcv(msgqid, &plane, sizeof(plane), 1, 0);
        
        
        	//if(plane.flag2==1){
        		//printf("de[t airport free\n");
        		//plane.msg_type = 5*plane.airport_arrival;
        	  //      msgsnd(msgqid, &plane, sizeof(plane), 0);
        	//}
        	
        	
        

        
        
        
/*
        // Parse message (format: "plane_id departure_airport arrival_airport")
        sscanf(msg.mtext, "%d %d %d", &plane_id, &departure_airport, &arrival_airport);
		printf("plane id is %d", plane_id);
        // Process request
        if (airport_states[departure_airport] == 0) {
            // Airport is available, grant landing permission
            plane.msg_type = 5 * plane.airport_departure;
            msgsnd(msgqid, &plane, sizeof(PlaneDetails), 0);
            //send_message(msgqid, 2, "Permission granted"); // Assuming type 2 is for airport responses

            // Send landing instructions to plane
            char landing_instructions[MAX_MESSAGE_SIZE];
            sprintf(landing_instructions, "Land at Airport %d", arrival_airport);
            send_message(msgqid, 3, landing_instructions); // Assuming type 3 is for landing instructions

            // Append plane journey to file
            append_to_file(plane_id, departure_airport, arrival_airport);
	printf("hi1 \n");
            // Update airport state to busy
            airport_states[departure_airport] = 1;
        //} else {
            // Airport is busy, deny landing permission
        //    send_message(msgqid, 2, "Permission denied"); // Assuming type 2 is for airport responses
        //}

        // Optionally, add delay or sleep to simulate processing time
        sleep(1);*/
    //}
}

int main() {
    // Create or get message queue
    int msgqid = msgget(1234, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("msgget failed");
        return 1;
    }

    // Ask user for the number of airports to manage
    int num_airports;
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d", &num_airports);
    if (num_airports < 2 || num_airports > MAX_AIRPORTS) {
        printf("Invalid number of airports. Please enter a number between 2 and %d.\n", MAX_AIRPORTS);
        exit(EXIT_FAILURE);
    }
    // Communicate with airports and planes
    communicate(msgqid, num_airports);

    // Cleanup and termination logic goes here

    return 0;
}