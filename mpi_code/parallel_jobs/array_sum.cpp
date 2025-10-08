#include <iostream>
#include <vector>
#include <numeric>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int ARRAY_SIZE = 1000;
    std::vector<int> global_array(ARRAY_SIZE);

    if (rank == 0)
    {
        // Initialize the array with values 1 to 1000
        std::iota(global_array.begin(), global_array.end(), 1);
        std::cout << "Original array sum: " << std::accumulate(global_array.begin(), global_array.end(), 0) << std::endl;
    }

    // Calculate the chunk size for each process
    int chunk_size = ARRAY_SIZE / size;
    std::vector<int> local_array(chunk_size);

    // Implement MPI_Scatter to distribute the array
    // Scatter the array from rank 0 to all processes
    // partial_sums holds the collected partial sums (Root only)
    std::vector<int> partial_sums(size);
    
    MPI_Scatter(
        global_array.data(), // Send buffer pointer (Root only)
        chunk_size,          // Send count (chunk size)
        MPI_INT,             // Send Datatype
        local_array.data(),  // Receive buffer pointer (All processes)
        chunk_size,          // Receive count (chunk size)
        MPI_INT,             // Receive Datatype
        0,                   // Root Rank
        MPI_COMM_WORLD
    );    

    // Compute partial sum
    int partial_sum = std::accumulate(local_array.begin(), local_array.end(), 0);
    std::cout << "Process " << rank << " partial sum: " << partial_sum << std::endl;
   

    // Implement MPI_Gather to collect partial sums
    // Gather the partial sums from all processes to rank 0
    MPI_Gather(
        &partial_sum,          // Send buffer: pointer to the single integer sum
        1,                     // Send count: 1 (sending one integer)
        MPI_INT,               // Send Datatype
        partial_sums.data(),     // Receive buffer (Root only)
        1,                     // Receive count: 1 (receiving one integer FROM EACH process)
        MPI_INT,               // Receive Datatype
        0,                     // Root Rank
        MPI_COMM_WORLD
    );

    if (rank == 0)
    {
        int final_sum = std::accumulate(partial_sums.begin(), partial_sums.end(), 0);
        std::cout << "Final sum: " << final_sum << std::endl;
    }

    MPI_Finalize();
    return 0;
}
