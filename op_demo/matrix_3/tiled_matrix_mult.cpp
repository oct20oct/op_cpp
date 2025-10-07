#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <ctime>

void tiled_matrix_multiply(double* A, double* B, double* C, int N, int tile_size) {
   for (int i = 0; i < N * N; ++i)
        C[i] = 0.0;

    //Implement Tiled Matrix Multiplication 
    // Outer loops iterate over the blocks (tiles) of the matrices
    // The "b" prefix stands for "block"
    
    // 1. Iterate over C row blocks (i-dimension)
    for (int bi = 0; bi < N; bi += tile_size) {
        
        // 2. Iterate over C column blocks (j-dimension)
        for (int bj = 0; bj < N; bj += tile_size) {
            
            // 3. Iterate over the common dimension K blocks (k-dimension)
            // This is the accumulation loop across the tiles
            for (int bk = 0; bk < N; bk += tile_size) {
                
                // Inner loops perform the standard matrix multiplication (GEMM) 
                // on the current set of tiles (A[bi:bi+ts, bk:bk+ts] * B[bk:bk+ts, bj:bj+ts] -> C[bi:bi+ts, bj:bj+ts])

                // 4. Inner loop for C row within the block
                for (int i = bi; i < bi + tile_size && i < N; ++i) {
                    
                    // 5. Inner loop for C column within the block
                    for (int j = bj; j < bj + tile_size && j < N; ++j) {
                        
                        // 6. Innermost loop for the dot product (k-dimension) within the block
                        // The result accumulates in C[i][j]
                        double sum = 0.0;
                        for (int k = bk; k < bk + tile_size && k < N; ++k) {
                            
                            // C[i][j] += A[i][k] * B[k][j]
                            sum += A[i * N + k] * B[k * N + j];
                        }
                        
                        // Accumulate the partial result from the block
                        C[i * N + j] += sum;
                    }
                }
            }
        }
    }
}

void matrix_multiply(double *A, double *B, double *C, int N) {
    for (int i = 0; i < N * N; ++i)
        C[i] = 0.0;

    for (int i = 0; i < N; ++i) {       
        for (int j = 0; j < N; ++j) {   
            for (int k = 0; k < N; ++k) {
                C[i*N + j] += A[i*N + k] * B[k*N + j];
            }
        }
    }
}

void initialize_matrices(double* A, double* B, int M, int N) {
    srand(time(NULL));

    // Initialize matrix A with random values
    for (int i = 0; i < M * N; i++) {
        A[i] = static_cast<double>(rand()) / RAND_MAX;
    }

    // Initialize matrix B with random values
    for (int i = 0; i < M * N; i++) {
        B[i] = static_cast<double>(rand()) / RAND_MAX;
    }
}

// 
int main() {
    const int M = 1024, N = 1024;
    double* A = new double[M * N];
    double* B = new double[M * N];
    double* C = new double[M * N];

    // Initialize matrices A and B with some values
    initialize_matrices(A, B, M, N);

    // Measure execution time for native implementation
    auto native_start = std::chrono::high_resolution_clock::now();
    matrix_multiply(A, B, C, N);
    auto native_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> native_elapsed = native_end - native_start;
    std::cout << "Non-tiled execution time: " << native_elapsed.count() << " seconds" << std::endl;
    std::cout << "Verification checksum: " << C[0] + C[M*N-1] << std::endl; 
    std::cout << std::endl;

    const int cache_sizes[] = {64 * 1024, 128 * 1024, 256 * 1024, 1024 * 1024}; // Sizes in bytes

    for (size_t i = 0; i < sizeof(cache_sizes) / sizeof(cache_sizes[0]); ++i) {
        int tile_size = static_cast<int>(std::sqrt(cache_sizes[i] / (8 * (2 * M + N))));
        std::cout << "Cache size: " << cache_sizes[i] / 1024 << " KB" << std::endl;
        std::cout << "Tile size: " << tile_size << std::endl;

        // Measure execution time for tiled implementation
        auto start = std::chrono::high_resolution_clock::now();
        tiled_matrix_multiply(A, B, C, N, tile_size);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Tiled execution time: " << elapsed.count() << " seconds" << std::endl;
        std::cout << "Verification checksum: " << C[0] + C[M*N-1] << std::endl; 
        std::cout << std::endl;
    }


    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
