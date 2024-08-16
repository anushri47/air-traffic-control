//plane.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>

#define MAX_PLANE_ID 10
#define MAX_SEATS 10
#define MAX_CARGO_ITEMS 100
#define MAX_CARGO_WEIGHT 100
#define MAX_PASSENGER_WEIGHT 100
#define MIN_PASSENGER_WEIGHT 10
#define MAX_LUGGAGE_WEIGHT 25
#define NUM_CREW_MEMBERS 7
#define CREW_WEIGHT 75

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
    struct PassengerDetails passengers[MAX_SEATS];
};

struct Message {
    long msg_type;
    char msg_text[150]; // Increased buffer size
};

// Function to send message to message queue
void send_message(int msgid, struct Message msg) {
    if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

// Function to receive message from message queue
struct Message receive_message(int msgid) {
    struct Message msg;
    if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    return msg;
}

void create_passenger_process(int plane_pipe[2], int passenger_number) {
    pid_t pid;
    struct PassengerDetails passenger_details;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(plane_pipe[1]); // Close write end of pipe

        // Read passenger details from plane process
        read(plane_pipe[0], &passenger_details, sizeof(passenger_details));

        // Close read end of pipe
        close(plane_pipe[0]);
        exit(EXIT_SUCCESS);
    }
}

int main() {
    struct PlaneDetails plane;
    int plane_pipe[MAX_SEATS][2]; // One pipe for each passenger
    int airport_departure, airport_arrival, plane_type, num_cargo_items, avg_cargo_weight, num_occupied_seats;
    int msgid;
    struct Message msg;

    printf("Enter Plane ID: ");
    scanf("%d", &plane.plane_id);

    if (plane.plane_id < 1 || plane.plane_id > MAX_PLANE_ID) {
        printf("Invalid plane ID.\n");
        return 1;
    }

    printf("Enter Type of Plane (1 for passenger, 0 for cargo): ");
    scanf("%d", &plane.plane_type);

    if (plane.plane_type != 0 && plane.plane_type != 1) {
        printf("Invalid plane type.\n");
        return 1;
    }

    if (plane.plane_type == 1) {
        printf("Enter Number of Occupied Seats: ");
        scanf("%d", &plane.occupied_seats);

        if (plane.occupied_seats < 1 || plane.occupied_seats > MAX_SEATS) {
            printf("Invalid number of occupied seats.\n");
            return 1;
        }

        plane.num_passengers = plane.occupied_seats;

        // Input details for each passenger
        for (int i = 0; i < plane.num_passengers; ++i) {
            // Create pipe for the current passenger
            if (pipe(plane_pipe[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            // Create child process for the current passenger
            create_passenger_process(plane_pipe[i], i);

            // Close write end of pipe in parent process
            close(plane_pipe[i][0]);
        }
    }

    printf("Enter Airport Number for Departure: ");
    scanf("%d", &plane.airport_departure);

    if (plane.airport_departure < 1 || plane.airport_departure > 10) {
        printf("Invalid airport number for departure.\n");
        return 1;
    }

    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &plane.airport_arrival);

    if (plane.airport_arrival < 1 || plane.airport_arrival > 10 || plane.airport_arrival == plane.airport_departure) {
        printf("Invalid airport number for arrival.\n");
        return 1;
    }

    plane.total_weight = 0;
    if (plane.plane_type == 1) { // Passenger plane
        for (int i = 0; i < plane.num_passengers; ++i) {
            struct PassengerDetails passenger_details;

            // Prompt for luggage weight
            printf("Enter Weight of Luggage for Passenger %d: ", i);
            scanf("%d", &passenger_details.luggage_weight);
            while (passenger_details.luggage_weight < 0 || passenger_details.luggage_weight > MAX_LUGGAGE_WEIGHT) {
                printf("Invalid luggage weight. Please enter a value between 0 and %d: ", MAX_LUGGAGE_WEIGHT);
                scanf("%d", &passenger_details.luggage_weight);
            }

            // Prompt for passenger weight
            printf("Enter Body Weight of Passenger %d: ", i);
            scanf("%d", &passenger_details.passenger_weight);
            while (passenger_details.passenger_weight < MIN_PASSENGER_WEIGHT || passenger_details.passenger_weight > MAX_PASSENGER_WEIGHT) {
                printf("Invalid passenger weight. Please enter a value between %d and %d: ", MIN_PASSENGER_WEIGHT, MAX_PASSENGER_WEIGHT);
                scanf("%d", &passenger_details.passenger_weight);
            }

            // Send passenger details to child process through pipe
            write(plane_pipe[i][1], &passenger_details, sizeof(passenger_details));

            // Close write end of pipe in parent process
            close(plane_pipe[i][1]);

            // Calculate total weight
            plane.total_weight += passenger_details.luggage_weight + passenger_details.passenger_weight;
        }
        plane.total_weight += (NUM_CREW_MEMBERS * CREW_WEIGHT);
    } else { // Cargo plane
        printf("Enter Number of Cargo Items: ");
        scanf("%d", &num_cargo_items);

        if (num_cargo_items < 1 || num_cargo_items > MAX_CARGO_ITEMS) {
            printf("Invalid number of cargo items.\n");
            return 1;
        }

        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d", &avg_cargo_weight);

        if (avg_cargo_weight < 1 || avg_cargo_weight > MAX_CARGO_WEIGHT) {
            printf("Invalid average weight of cargo items.\n");
            return 1;
        }

        plane.total_weight += (num_cargo_items * avg_cargo_weight);
        plane.total_weight += (2 * CREW_WEIGHT); // 2 pilots
    }

    // Create message queue

    msgid = msgget(1234, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Wait for confirmation from air traffic controller
    //msg = receive_message(msgid);
    if (strcmp(msg.msg_text, "Ready for Departure") == 0) {
        printf("Received confirmation from air traffic controller. Plane is ready for departure.\n");
    }

    // Send plane details to air traffic controller
    sprintf(msg.msg_text, "Plane ID: %d, Departure from Airport: %d, Arrival at Airport: %d", plane.plane_id, plane.airport_departure, plane.airport_arrival);
    
    //msg.msg_type = 1; // Message type for plane details
    plane.msg_type = 1;
    plane.flag=1;
    //send_message(msgid, msg);
    //send_message(msgid, plane);
    
 
    
    msgsnd(msgid, &plane, sizeof(plane), 0) ;

    // Simulate boarding/loading process
    printf("Boarding/Loading...\n");
    sleep(3); // Simulate boarding/loading process duration

    // Simulate plane journey
    printf("Plane journey in progress...\n");
    sleep(30); // Simulate plane journey duration

    // Send arrival notification to air traffic controller
    sprintf(msg.msg_text, "Arrival of Plane %d at Airport %d", plane.plane_id, plane.airport_arrival);
    
    
    //plane = receive_message(msgid);
    msgrcv(msgid, &plane, sizeof(plane), 4, 0);
    //send_message(msgid, msg);

    // Wait for confirmation from air traffic controller for deboarding/unloading
    //msg = receive_message(msgid);
    printf("done\n");
    
    if (strcmp(msg.msg_text, "Ready for Deboarding") == 0) {
        printf("Received confirmation from air traffic controller. Plane is ready for deboarding/unloading.\n");
    }
    
    

    // Simulate deboarding/unloading process
    printf("Deboarding/Unloading...\n");
    sleep(3); // Simulate deboarding/unloading process duration

    // Display completion message
    printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", plane.plane_id, plane.airport_departure, plane.airport_arrival);

    // Remove message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}