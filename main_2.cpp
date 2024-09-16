#include <iostream>
#include <vector>
#include <string>
#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <unistd.h>

#define MSG_EXIT 1
#define MSG_PRINT_ORDERED 2
#define MSG_PRINT_UNORDERED 3
#define REQUEST_TAG 100
#define RESPONSE_TAG 200
int messages_count = 0;

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

    MPI_Comm_split(MPI_COMM_WORLD, rank == size - 1, 0, &new_comm); // color will either be 0 or 1 

    // initialize master and slave io
    if (rank == size - 1)
        master_io(MPI_COMM_WORLD, new_comm);
    else
        slave_io(MPI_COMM_WORLD, new_comm);

    double start_time = MPI_Wtime();
    // iteration counter
    while (loop_count > 0) {
        if (rank == size - 1)
            master_io(MPI_COMM_WORLD, new_comm);
        else
            slave_io(MPI_COMM_WORLD, new_comm);

        loop_count--;
    }
    // finalizes the program
    MPI_Finalize();
    double end_time = MPI_Wtime();
    double time_taken = end_time - start_time;

    if (rank == size - 1) {
        std::cout << "Total Executed time [ " << time_taken << " s]" << std::endl;
        std::cout << "Average Communication time from Slave to Master [ " << (size - 1 > 0 ? time_taken / (size - 1) : -time_taken / (size - 1)) << " s]" << std::endl;
        std::cout << "Total Messages Exchanged: " << messages_count << std::endl;
    }

    return 0;
}

#define RECENT_TRACK_COUNT 88 // or any desired number

int master_io(MPI_Comm world_comm, MPI_Comm comm)
{
    int size, nslaves;
    char buf[8024];
    MPI_Status status;
    MPI_Comm_size(world_comm, &size);
    nslaves = size - 1; // account for master and get number of slaves
    
    std::vector<int> recently_communicated_ranks((size - 1) * 2, -1);
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
        MPI_Recv(buf, sizeof(buf), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, world_comm, &status);
        
        // Update the recently_communicated_ranks array
        recently_communicated_ranks[pointer] = status.MPI_SOURCE;
        pointer = (pointer + 1) % RECENT_TRACK_COUNT; // loop around to start if exceeded
        
        // Broadcast recently_communicated_ranks to all slaves
        MPI_Bcast(recently_communicated_ranks.data(), RECENT_TRACK_COUNT, MPI_INT, size - 1, world_comm);  // Assuming master rank is 0
        
        switch (status.MPI_TAG) {
            case MSG_EXIT:
                nslaves--;
                break;
            case MSG_PRINT_UNORDERED:
                std::cout << buf;
                break;
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
    int ndims = 2, size, my_rank, reorder, ierr, worldSize, waittime;
    MPI_Comm comm2D;
    int dims[ndims], coord[ndims];
    int wrap_around[ndims];
    char buf[8024];
    int chargers = 20;

    MPI_Comm_size(world_comm, &worldSize);
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &my_rank);
    dims[0] = dims[1] = 0;
    waittime = 0;

    MPI_Dims_create(size, ndims, dims);

    wrap_around[0] = 1;
    wrap_around[1] = 1;
    reorder = 0;
    ierr = MPI_Cart_create(comm, ndims, dims, wrap_around, reorder, &comm2D);
    if (ierr != 0) std::cerr << "ERROR[" << ierr << "] creating CART\n";

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

    std::vector<int> array(chargers);
    int count_zeros = 0;
    std::srand(std::time(nullptr) + my_rank); 
    sleep(waittime);

    // count empty charging ports
    for (int i = 0; i < chargers; i++) {
        array[i] = std::rand() % 2;
        if (array[i] == 0)
            count_zeros++;
    }
    // prepares the status buffer
    std::string full_status = (chargers - count_zeros) > 3 ? "Full" : "Empty";

    std::vector<std::string> neighbour_status(4);
    MPI_Status status;
    char buf_neighbour[6];

    std::vector<int> neighbours = {up, down, left, right};
    int my_free_ports = count_zeros;
    std::vector<int> neighbor_ports(4);
    
    // sends full or empty status to neighbours
    for (int i = 0; i < 4; i++) {
        MPI_Send(full_status.c_str(), 10, MPI_CHAR, neighbours[i], 0, world_comm);
    }

    // receives the status from neighbour and pass the number of free ports
    for (int i = 0; i < 4; i++) {
        MPI_Recv(buf_neighbour, 10, MPI_CHAR, neighbours[i], 0, world_comm, &status);
        neighbour_status[i] = buf_neighbour;
        MPI_Send(&my_free_ports, 1, MPI_INT, neighbours[i], 0, world_comm);
    }

    // receives the number of free ports
    for (int i = 0; i < 4; i++) {
        MPI_Recv(&neighbor_ports[i], 1, MPI_INT, neighbours[i], 0, world_comm, &status);
    }

    // sends a null request tag to prepare for neighbours of neighbours
    for (int i = 0; i < 4; i++) {
        MPI_Send(nullptr, 0, MPI_INT, neighbours[i], REQUEST_TAG, comm2D);
    }

    // checks the neighbours of neighbours
    int received_messages = 0;
    while (received_messages < 4) {
        MPI_Recv(nullptr, 0, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, comm2D, &status);
        
        std::vector<int> my_neighbours(4);
        MPI_Cart_shift(comm2D, 0, 1, &my_neighbours[0], &my_neighbours[1]);
        MPI_Cart_shift(comm2D, 1, 1, &my_neighbours[2], &my_neighbours[3]);
        
        MPI_Send(my_neighbours.data(), 4, MPI_INT, status.MPI_SOURCE, RESPONSE_TAG, comm2D);
        received_messages++;
    }

    // receives others' neighbours of neighbours
    std::vector<std::vector<int>> neighbours_of_my_neighbours(4, std::vector<int>(4));
    for (int i = 0; i < 4; i++) {
        MPI_Recv(neighbours_of_my_neighbours[i].data(), 4, MPI_INT, neighbours[i], RESPONSE_TAG, comm2D, &status);
    }

    // sends the status to master
    std::string message = full_status + ": " + std::to_string(my_free_ports) + '\n';
    MPI_Send(message.c_str(), message.size(), MPI_CHAR, 0, MSG_PRINT_ORDERED, world_comm);

    double end_time = MPI_Wtime();
    std::cout << "Slave[" << my_rank << "] time taken: " << end_time - start_time << std::endl;

    return 0;
}
