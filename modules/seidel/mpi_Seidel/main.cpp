#include <iostream>
#include <cmath>
#include "mpi.h"

void PrintMatrix(double* matrix, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      std::cout << matrix[i * size + j] << " ";
    }

    std::cout << std::endl;
  }
}

void FillingTheMatrix(double* matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ((i == 0) || (i == size - 1) || (j == 0) || (j == size - 1)) {
                matrix[i * size + j] = 100;
            } else {
                matrix[i * size + j] = 0;
            }
        }
    }
}

void ParallelAlgMPI(double* matrix, int size, double eps)
{
    int procRank, procNum;
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    
    const int delta = (size-2) / procNum;
    int residue = (size-2) % procNum;
    
    int begin, end;
    
    int* begin_temp;
    int* end_temp;
    
    if (procRank == 0) {
        begin_temp = new int[procNum];
        end_temp = new int[procNum];
        
        begin_temp[0] = 1;
        end_temp[0] = begin_temp[0] + delta;
        
        for (int i = 1; i < procNum; i++) {
            begin_temp[i] = end_temp[i-1];
            end_temp[i] = begin_temp[i] + delta;
            
            if (residue != 0) {
                end_temp[i-1] += residue;
            }
        }
        
        end_temp[procNum-1] += residue;
    }
    
    MPI_Scatter(&begin_temp[0], 1, MPI_INT, &begin, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&end_temp[0], 1, MPI_INT, &end, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int q = 0;
    double dmax = 0;
    double temp_dmax = 0;
    double dm = 0;
    
    do {
        q++;
        dmax = 0;
        dm = 0;
        
        for (int i = begin; i < end; i++ ) {
            for (int j = 1; j < size - 1; j++ ) {
                double temp = matrix[size * i + j];
                matrix[size * i + j] = 0.25 * (matrix[size * (i - 1) + j] +
                                               matrix[size * (i + 1) + j] +
                                               matrix[size * i + (j - 1)] +
                                               matrix[size * i + (j + 1)] );
                
                double d = fabs(temp-matrix[size * i + j]);
                if (dm < d)
                    dm = d;
            }
        }
        
        MPI_Reduce(&dm, &temp_dmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        
        if (procRank == 0) {
            dmax = (temp_dmax > dmax) ? temp_dmax : dmax;
        }
        
        MPI_Bcast(&dmax, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } while ( dmax > eps );
    
    if (procRank == 0) {
        delete[] begin_temp;
        delete[] end_temp;
        std::cout << "\nMPI iter: " << q;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int procRank, procNum;
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    int size = 10;
    double eps = 0.01;

    double start_mpi, end_mpi;
    double* matrix_mpi;
    MPI_Win win_matrix;      //Создаем окно доступа к матрице

    if (procRank == 0) {
        MPI_Win_allocate_shared(size*size * sizeof(double), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &matrix_mpi, &win
        FillingTheMatrix(matrix_mpi, size);
        std::cout << "ProcNum: " << procNum << std::endl;
        std::cout << "Start mpi... " << std::endl;
        start_mpi = MPI_Wtime();
    } else {
        int disp;
        MPI_Aint ssize;
        MPI_Win_allocate_shared(1, sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &matrix_mpi, &win_matrix);
        MPI_Win_shared_query(win_matrix, 0, &ssize, &disp, &matrix_mpi);
    }

    ParallelAlgMPI(matrix_mpi, size, eps);

    if (procRank == 0) {
        end_mpi = MPI_Wtime();
        double time_mpi = end_mpi - start_mpi;
        std::cout << ", MPI Time: " << time_mpi << std::endl;

        if (size < 15)
            PrintMatrix(matrix_mpi, size);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Win_free(&win_matrix);
    
    MPI_Finalize();
    return 0;
}

