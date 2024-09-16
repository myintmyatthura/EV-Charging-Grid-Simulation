#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MSG_EXIT 1
#define MSG_PRINT_ORDERED 2
#define MSG_PRINT_UNORDERED 3
#define REQUEST_TAG 100
#define RESPONSE_TAG 200
int messages_count = 0;

/*
Name - Myint Myat Thura
ID - 31861067

How to run code:

Option 1 - Use makefile
Option 2 - mpicc -Wall -o main.c Output ; mpirun -oversubscribe -np x main.c


*/

int master_io(MPI_Comm world_comm, MPI_Comm comm);
int slave_io(MPI_Comm world_comm, MPI_Comm comm);

int main(int argc, char **argv)
{
    int rank, size;
    int loop_count = 100;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
	// 1 if its the root process else, slave
    MPI_Comm_split( MPI_COMM_WORLD,rank == size-1, 0, &new_comm); // color will either be 0 or 1 
    // initialize master and slave io
    if (rank == size-1) 
	master_io( MPI_COMM_WORLD, new_comm );
    else
	slave_io( MPI_COMM_WORLD, new_comm );

    double start_time = MPI_Wtime();
    // iteration counter
    while (loop_count > 0) {
        if (rank == size-1) 
            master_io(MPI_COMM_WORLD, new_comm);
        else
            slave_io(MPI_COMM_WORLD, new_comm);
        
        loop_count--;
    }
    // finalizes the program
    
    MPI_Finalize();
    double end_time = MPI_Wtime();
    double time_taken = end_time - start_time;
    if (rank == size-1){
        printf("Total Executed time [ %f s]\n",time_taken);
        printf("Average Communication time from Slave to Master [ %f s]\n",time_taken/size-1 > 0 ? time_taken/size-1 :(time_taken/size-1)*(-1));
        char buffer[8045];  // Assuming 100 chars is enough for the conversion, adjust as needed
sprintf(buffer, "Total Messages Exchanged: %d\n", messages_count);

fputs(buffer, stdout);
    }
    
    return 0;
}

#define RECENT_TRACK_COUNT 88 // or any desired number

int master_io(MPI_Comm world_comm, MPI_Comm comm)
{
    int        size, nslaves;
    char       buf[8024];
    MPI_Status status;
    MPI_Comm_size(world_comm, &size);
    nslaves = size - 1; // account for master and get number of slaves
    
    int recently_communicated_ranks[(size-1)*2];
    // stores recently communicated ranks
    for (int i = 0; i < (size-1)*2; i++) {
        recently_communicated_ranks[i] = -1;
    }
    int pointer = 0;
    FILE *initialClear = fopen("log.txt", "w");  // Open in write mode to clear the contents
    if (!initialClear) {
        perror("Error opening log file");
        // Handle the error if necessary
    } else {
        fclose(initialClear);  // Close the file immediately
    }
        FILE *logfile = fopen("log.txt", "a");  // Open the log file in append mode
    if (!logfile) {
        perror("Error opening log file");
    }   


    while (nslaves > 0) {
        // receives a buffer from any source
        MPI_Recv(buf, 8024, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, world_comm, &status);
        
        // Update the recently_communicated_ranks array
        recently_communicated_ranks[pointer] = status.MPI_SOURCE;
        pointer = (pointer + 1) % RECENT_TRACK_COUNT; // loop around to start if exceeded
        
        // Broadcast recently_communicated_ranks to all slaves
        MPI_Bcast(recently_communicated_ranks, RECENT_TRACK_COUNT, MPI_INT, size-1, world_comm);  // Assuming master rank is 0
        // based on the tag, choose what to do
        switch (status.MPI_TAG) {
            case MSG_EXIT:
                nslaves--; 
                break;
            case MSG_PRINT_UNORDERED:
                fputs(buf, stdout);
                break;
            // this is our main tag
            case MSG_PRINT_ORDERED:
                if (strlen(buf) > 0) {
                    messages_count++;
                    fputs(buf, logfile);
                    
                }
                break;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return 0;
}



/* This is the slave */
int slave_io(MPI_Comm world_comm, MPI_Comm comm) {
     MPI_Request send_request[4], recv_request;
    double start_time = MPI_Wtime(); // start time
    int ndims=2, size, my_rank, reorder, my_cart_rank, ierr, worldSize, waittime;
    MPI_Comm comm2D;
    int dims[ndims], coord[ndims];
    int wrap_around[ndims];
    char buf[8024];
    int chargers = 20;

    MPI_Comm_size(world_comm, &worldSize);
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &my_rank);
    dims[0]=dims[1]=0;
    waittime = 0;

    MPI_Dims_create(size, ndims, dims);

    wrap_around[0] = 1;
    wrap_around[1] = 1;
    reorder = 0;
    ierr = 0;
    ierr = MPI_Cart_create(comm, ndims, dims, wrap_around, reorder, &comm2D);
    if(ierr != 0) printf("ERROR[%d] creating CART\n",ierr);

    MPI_Cart_coords(comm2D, my_rank, ndims, coord);
    // find neighbours for current slave
    int left, right, up, down;
    int coord_left[2], coord_right[2], coord_up[2], coord_down[2];

    MPI_Cart_shift(comm2D, 0, 1, &up, &down);
    MPI_Cart_shift(comm2D, 1, 1, &left, &right);

    MPI_Cart_coords(comm2D, left, ndims, coord_left);
    MPI_Cart_coords(comm2D, right, ndims, coord_right);
    MPI_Cart_coords(comm2D, up, ndims, coord_up);
    MPI_Cart_coords(comm2D, down, ndims, coord_down);

    int array[chargers];
    int count_zeros = 0;
    srand(time(NULL) + my_rank); 
    sleep(waittime);

    // count empty charging ports
    for(int i = 0; i < chargers; i++) {
        array[i] = rand() % 2;
        if(array[i] == 0)
            count_zeros++;
    }
    // prepares the status buffer
    char full_status[6]; 
    if ((chargers - count_zeros) > 3)
        strcpy(full_status, "Full");
    else
        strcpy(full_status, "Empty");

    char neighbour_status[4][6];
    MPI_Status status;
    char buf_neighbour[6];

    int neighbours[4] = {up, down, left, right};
    int my_free_ports = count_zeros;
    int neighbor_ports[4];
    
    // sends full or empty status to neighbours
    for (int i = 0; i < 4; i++) {
        MPI_Send(full_status, 10, MPI_CHAR, neighbours[i], 0, world_comm);
    }

    // receives the status from neighbour and pass the number of free ports
    for (int i = 0; i < 4; i++) {
        MPI_Recv(buf_neighbour, 10, MPI_CHAR, neighbours[i], 0, world_comm, &status);
        strcpy(neighbour_status[i], buf_neighbour);
        MPI_Send(&my_free_ports, 1, MPI_INT, neighbours[i], 0, world_comm);
    }

    // reveives the number of free ports
    for (int i = 0; i < 4; i++) {
        MPI_Recv(&neighbor_ports[i], 1, MPI_INT, neighbours[i], 0, world_comm, &status);
    }

    // sends a null request tag to prepare for neighbours of neighbours
    for (int i = 0; i < 4; i++) {
    MPI_Send(NULL, 0, MPI_INT, neighbours[i], REQUEST_TAG, comm2D);
}

    // checks the neighbours of neighbours
    int received_messages = 0;
    while (received_messages < 4) {
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, comm2D, &status);
        
        int my_neighbours[4];
        MPI_Cart_shift(comm2D, 0, 1, &my_neighbours[0], &my_neighbours[1]);
        MPI_Cart_shift(comm2D, 1, 1, &my_neighbours[2], &my_neighbours[3]);
        
        MPI_Send(&my_neighbours, 4, MPI_INT, status.MPI_SOURCE, RESPONSE_TAG, comm2D);
        received_messages++;
    }

    // receives others' neighbours of neighbours
    int neighbours_of_my_neighbours[4][4];
    for (int i = 0; i < 4; i++) {
        MPI_Recv(&neighbours_of_my_neighbours[i], 4, MPI_INT, neighbours[i], RESPONSE_TAG, comm2D, &status);
    }



    // stores coordinates
    int coord_neighbours_of_my_neighbours[4][4][2];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int rank_of_neighbour_of_neighbour = neighbours_of_my_neighbours[i][j];
            MPI_Cart_coords(comm2D, rank_of_neighbour_of_neighbour, ndims, coord_neighbours_of_my_neighbours[i][j]);
        }
    }

    int unique_neighbours_of_my_neighbours[16];
    int unique_coord_neighbours_of_my_neighbours[16][2];

    // flattens array for better iteration
    int count = 0;
    int added[worldSize];
    for (int i = 0; i < worldSize; i++) {
        added[i] = 0;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int rank = neighbours_of_my_neighbours[i][j];
            if (!added[rank]) {
                unique_neighbours_of_my_neighbours[count] = rank;
                unique_coord_neighbours_of_my_neighbours[count][0] = coord_neighbours_of_my_neighbours[i][j][0];
                unique_coord_neighbours_of_my_neighbours[count][1] = coord_neighbours_of_my_neighbours[i][j][1];
                added[rank] = 1;
                count++;
            }
        }
    }


    // This will hold all the ranks that communicated with the master in the previous 3 iterations
    int recently_communicated_ranks[(worldSize-1)*2]; 
    MPI_Bcast(recently_communicated_ranks, (worldSize-1)*2, MPI_INT, 0, world_comm); 
    // This will hold the neighbours of neighbours' ranks that didn't recently communicate
    int not_recently_communicated_neighbours[16];
    int not_recently_count = 0;

    // A boolean array to track which ranks have already been added
    memset(added, 0, sizeof(added));

    // Check each neighbour of neighbour, simple duplicate removal function
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            bool recently_communicated = false;
            for (int k = 0; k < 2*(worldSize-1); k++) {
                if (neighbours_of_my_neighbours[i][j] == recently_communicated_ranks[k]) {
                    recently_communicated = true;
                    break;
                }
            }
            // If this neighbour of neighbour hasn't communicated recently and hasn't been added yet, add it to our list
            if (!recently_communicated && added[neighbours_of_my_neighbours[i][j]] == 0) {
                not_recently_communicated_neighbours[not_recently_count++] = neighbours_of_my_neighbours[i][j];
                added[neighbours_of_my_neighbours[i][j]] = 1;
            }
        }
    }

    int final_neighbours[16]; // This array will hold the final values
    int final_count = 0;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            bool is_present = false;
            
            for (int k = 0; k < not_recently_count; k++) {
                if (neighbours_of_my_neighbours[i][j] == not_recently_communicated_neighbours[k]) {
                    is_present = true;
                    break;
                }
            }
            
            // If the value is not present in the not_recently_communicated_neighbours array and not added to final_neighbours
            if (!is_present && added[neighbours_of_my_neighbours[i][j]] == 0) {
                final_neighbours[final_count++] = neighbours_of_my_neighbours[i][j];
                added[neighbours_of_my_neighbours[i][j]] = 1;
            }
        }
    }





    // if current node is full, we prepare to send data to master that request
    // for neighbour nodes
    if (strcmp(full_status, "Full") == 0) {
        char message[8024];
        char temp[8024];
        int portSize = chargers;
        int size_arr = 0;
        // find the number of elements that are less than 3 in neighbor_ports array
        for (int i = 0; i < 4; i++) {
            if (neighbor_ports[i] < 3) {
                size_arr++;
            }
        }


        sprintf(message, "Grid Dims [%d , %d] \n\n-Number of Adjacent Nodes: %d \n-Availability to be considered full: 2\n\nReporting Node:\n\n-Rank %d , Coordinates (%d,%d), Port Size %d, Free Ports %d\n\nNeighbours:\n\n", 
        dims[0],dims[1],size_arr, my_rank, coord[0], coord[1], portSize, count_zeros);

        // checks and sets coordinates
        for (int i = 0; i < 4; i++) {
        if (neighbor_ports[i] < 3) {
            int x, y;
            switch(i) {
                case 0: // up
                    x = coord_up[0];
                    y = coord_up[1];
                    break;
                case 1: // down
                    x = coord_down[0];
                    y = coord_down[1];
                    break;
                case 2: // left
                    x = coord_left[0];
                    y = coord_left[1];
                    break;
                case 3: // right
                    x = coord_right[0];
                    y = coord_right[1];
                    break;
                default:
                    x = 0;
                    y = 0;
                    break;
            }
            sprintf(temp, "-Rank %d , Coordinates (%d,%d), Free Ports %d\n", neighbours[i], x, y, neighbor_ports[i]);
            strcat(message, temp);
        }
}

        

    
        sprintf(temp, "\n~~~~~~~~~Recommendations~~~~~~~~~\n\nFor node %d, we recommend:\n\n", my_rank);
        if (strlen(message) + strlen(temp) < 500) {  // Just to be safe
            strcat(message, temp);
        }
        // checks the coordinates of neighbours of neighbours
        for (int i = 0; i < count; i++) {
            sprintf(temp, "-Node %d, Coordinates (%d,%d)\n", 
            unique_neighbours_of_my_neighbours[i], 
            unique_coord_neighbours_of_my_neighbours[i][0], 
            unique_coord_neighbours_of_my_neighbours[i][1]);

            if (strlen(message) + strlen(temp) < 800) {  // Just to be safe and assuming your buffer can hold up to 8024 characters
                strcat(message, temp);
            }

            
}

        // Now, add the message about the neighbours not communicating in the last 3 iterations.
        sprintf(temp, "\nThese Neighbours [");

        for (int i = 0; i < not_recently_count; i++) {
            char rank_str[16];
            sprintf(rank_str, "%d", not_recently_communicated_neighbours[i]);
            
            // If adding the next rank doesn't overflow the temp buffer, add it.
            if (strlen(temp) + strlen(rank_str) + 3 < sizeof(temp)) { // +3 for possible comma, bracket and null terminator
                strcat(temp, rank_str);
                
                // If it's not the last rank, add a comma for separation.
                if (i != not_recently_count - 1) {
                    strcat(temp, ",");
                }
            }
        }
        strcat(temp, "] have not sent reports in the last 3 iterations.\n");



        if (strlen(message) + strlen(temp) < 800) {  // Same precaution as before
            strcat(message, temp);
        }

        // Message about neighbours that are most likely full.
        sprintf(temp, "These neighbours [");
        for (int i = 0; i < final_count; i++) {
            char rank_str[16];
            sprintf(rank_str, "%d", final_neighbours[i]);
            
            // If adding the next rank doesn't overflow the temp buffer, add it.
            if (strlen(temp) + strlen(rank_str) + 3 < sizeof(temp)) { // +3 for possible comma, bracket, and null terminator
                strcat(temp, rank_str);
                
                // If it's not the last rank, add a comma for separation.
                if (i != final_count - 1) {
                    strcat(temp, ",");
                }
            }
        }
        strcat(temp, "] are most likely full.\n");

        // If adding this message doesn't overflow the main message buffer, add it.
        if (strlen(message) + strlen(temp) < 800) {
            strcat(message, temp);
        }
        // find the total communication time
        double end_time = MPI_Wtime();
        double time_taken = end_time - start_time;

        sprintf(temp, "\nCommunication Time with Adjacent Nodes: [ %fs ] (Not including Sleep)\n", time_taken-2 < 0 ? time_taken : time_taken-2);
            if (strlen(message) + strlen(temp) < 1000) {  // Just to be safe
                strcat(message, temp);
            }
        // send the message to master
        strcat(message,"\n************************************************************\n************************************************************\n\n");
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, worldSize-1, MSG_PRINT_ORDERED, world_comm);
    }
    else{
        // if current node isn't full send a signal to base station either way
        MPI_Send(NULL, 0, MPI_CHAR, worldSize-1, MSG_PRINT_ORDERED, world_comm);
    }

    // send exit message after processes finish
    MPI_Send(NULL, 0, MPI_CHAR, worldSize-1, MSG_EXIT, world_comm);
    // barrior to prevent deadlock
    MPI_Barrier(MPI_COMM_WORLD);
    // frees the Comms
    MPI_Comm_free(&comm2D);
    return 0;
}