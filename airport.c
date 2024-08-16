//airport.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include "message_queue_key.h"

#define MAX_RUNWAYS 10
#define MSG_SIZE 8192

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

// Runway structure
struct Runway {
    int number;
    int loadCapacity;
    pthread_mutex_t mutex;
    int available;
};

// Function prototypes
void *handleDeparture(void *planeDetails);
void *handleArrival(void *planeDetails);
void sendMessage(int msgqid, long mtype, char *msg);

// Global variables
struct Runway runways[MAX_RUNWAYS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int msgqid;

struct Plane {
    int planeID;
    int totalWeight;
    int airportNumber;
};

void cleanup() {
    // Destroy mutexes
    for (int i = 0; i < MAX_RUNWAYS; i++) {
        pthread_mutex_destroy(&runways[i].mutex);
    }
}

int main() {
    // Input variables
    int airportNumber, numRunways, i;

    // Create or get message queue
    if ((msgqid = msgget(1234, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Request user input for airport number
    printf("Enter Airport Number: ");
    scanf("%d", &airportNumber);

    // Request user input for number of runways
    printf("Enter number of Runways: ");
    scanf("%d", &numRunways);

    // Validate number of runways
    if (numRunways % 2 != 0 || numRunways < 1 || numRunways > MAX_RUNWAYS) {
        printf("Number of runways should be an even number between 2 and 10.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize runways
    for (i = 0; i < numRunways; i++) {
        printf("Enter loadCapacity of Runway %d (between 1000 and 12000 kgs): ", i + 1);
        scanf("%d", &runways[i].loadCapacity);
        if (runways[i].loadCapacity < 1000 || runways[i].loadCapacity > 12000) {
            printf("Load capacity should be between 1000 and 12000 kgs.\n");
            exit(EXIT_FAILURE);
        }
        runways[i].number = i + 1;
        runways[i].available = 1; // Runway is initially available
        pthread_mutex_init(&runways[i].mutex, NULL);
    }

    // Additional backup runway with loadCapacity of 15,000 kgs
    runways[numRunways].number = numRunways + 1;
    runways[numRunways].loadCapacity = 15000;
    runways[numRunways].available = 1;
    pthread_mutex_init(&runways[numRunways].mutex, NULL);

    // Create threads for arrival and departure handling
    pthread_t arrivalThread, departureThread;
		
   struct PlaneDetails plane;
    // Main loop for handling messages from air traffic controller
    while (1) {
        // Receive message from air traffic controller
        struct Message {
            long mtype;
            char mtext[MSG_SIZE];
        } message;

        //if (msgrcv(msgqid, &message, sizeof(message.mtext), airportNumber, 0) == -1) {
         //   perror("msgrcv");
          //  exit(EXIT_FAILURE);
        //}
        
        printf("%d\n",5*airportNumber);
        msgrcv(msgqid, &plane, sizeof(plane), 5*airportNumber, 0);
        
        printf("rcv\n");
        
        

        // Parse message and create new thread for departure or arrival
        //struct Plane plane;
        //sscanf(message.mtext, "%d %d %d", &plane.planeID, &plane.totalWeight, &plane.airportNumber);

        if (plane.total_weight == 0) {
            // Departure
            int i;
    pthread_mutex_lock(&mutex);

    // Find the best-fit runway
    int selectedRunway = -1;
    for (i = 0; i < MAX_RUNWAYS; i++) {
        if (runways[i].available && runways[i].loadCapacity >= plane.total_weight) {
            if (selectedRunway == -1 || runways[i].loadCapacity < runways[selectedRunway].loadCapacity) {
                selectedRunway = i;
            }
        }
    }

    // If no runway is available, use backup runway
    if (selectedRunway == -1 || runways[selectedRunway].loadCapacity < plane.total_weight) {
        selectedRunway = MAX_RUNWAYS;
    }

    // Mark the selected runway as unavailable
    runways[selectedRunway].available = 0;
    pthread_mutex_unlock(&mutex);

    // Boarding/loading process
    sleep(3);

    // Send message to air traffic controller
    char msg[MSG_SIZE];
    //sprintf(msg, "%d", plane->planeID);
    //sendMessage(msgqid, plane->airportNumber, msg);
    
    plane.msg_type = 1;
    plane.flag=0;
    plane.flag2=1;
    printf("fff\n");
    msgsnd(msgqid, &plane, sizeof(plane), 0);

    // Display message
    //printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", plane.planeID, runways[selectedRunway].number, plane->airportNumber);

    // Sleep for 2 seconds to simulate takeoff
    sleep(2);

    // Mark the runway as available after takeoff
    pthread_mutex_lock(&mutex);
    runways[selectedRunway].available = 1;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
            //pthread_create(&departureThread, NULL, handleDeparture, (void *)&plane);
        } else {
            // Arrival
            pthread_create(&arrivalThread, NULL, handleArrival, (void *)&plane);
        }
    }

    // Cleanup
    cleanup();

    // Close message queue
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void *handleDeparture(void *planeDetails) {
    struct PlaneDetails *plane = (struct PlaneDetails *)planeDetails;
    int i;
    pthread_mutex_lock(&mutex);

    // Find the best-fit runway
    int selectedRunway = -1;
    for (i = 0; i < MAX_RUNWAYS; i++) {
        if (runways[i].available && runways[i].loadCapacity >= plane->total_weight) {
            if (selectedRunway == -1 || runways[i].loadCapacity < runways[selectedRunway].loadCapacity) {
                selectedRunway = i;
            }
        }
    }

    // If no runway is available, use backup runway
    if (selectedRunway == -1 || runways[selectedRunway].loadCapacity < plane->total_weight) {
        selectedRunway = MAX_RUNWAYS;
    }

    // Mark the selected runway as unavailable
    runways[selectedRunway].available = 0;
    pthread_mutex_unlock(&mutex);

    // Boarding/loading process
    sleep(3);

    // Send message to air traffic controller
    char msg[MSG_SIZE];
    //sprintf(msg, "%d", plane->planeID);
    //sendMessage(msgqid, plane->airportNumber, msg);
    
    plane->msg_type = 1;
    plane->flag=0;
    plane->flag2=1;
    msgsnd(msgqid, &plane, sizeof(plane), 0);

    // Display message
    //printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", plane.planeID, runways[selectedRunway].number, plane->airportNumber);

    // Sleep for 2 seconds to simulate takeoff
    sleep(2);

    // Mark the runway as available after takeoff
    pthread_mutex_lock(&mutex);
    runways[selectedRunway].available = 1;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void *handleArrival(void *planeDetails) {
    struct Plane *plane = (struct Plane *)planeDetails;
    int i;
    pthread_mutex_lock(&mutex);

    // Find the best-fit runway
    int selectedRunway = -1;
    for (i = 0; i < MAX_RUNWAYS; i++) {
        if (runways[i].available && runways[i].loadCapacity >= plane->totalWeight) {
            if (selectedRunway == -1 || runways[i].loadCapacity < runways[selectedRunway].loadCapacity) {
                selectedRunway = i;
            }
        }
    }

    // If no runway is available, use backup runway
    if (selectedRunway == -1 || runways[selectedRunway].loadCapacity < plane->totalWeight) {
        selectedRunway = MAX_RUNWAYS;
    }

    // Mark the selected runway as unavailable
    runways[selectedRunway].available = 0;
    pthread_mutex_unlock(&mutex);

    // Simulate landing
    sleep(2);

    // Deboarding/unloading process
    sleep(3);

    // Send message to air traffic controller
    char msg[MSG_SIZE];
    sprintf(msg, "%d", plane->planeID);
    sendMessage(msgqid, plane->airportNumber, msg);

    // Display message
    printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", plane->planeID, runways[selectedRunway].number, plane->airportNumber);

    // Mark the runway as available after arrival
    pthread_mutex_lock(&mutex);
    runways[selectedRunway].available = 1;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void sendMessage(int msgqid, long mtype, char *msg) {
    struct Message {
        long mtype;
        char mtext[MSG_SIZE];
    } message;

    message.mtype = mtype;
    strcpy(message.mtext, msg);

    if (msgsnd(msgqid, &message, sizeof(message.mtext), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}