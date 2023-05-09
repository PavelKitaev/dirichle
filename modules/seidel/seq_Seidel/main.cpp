#include <iostream>
#include <cmath>
#include "omp.h"

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

void SequentialAlg(double* matrix, int size, double eps) {
    int q = 0;
    
    double dmax, temp, dm, d;
    
    do {
        q++;
        dmax = 0;
        
        for (int i = 1; i < size - 1; i++ ) {
            dm = 0;
            for (int j = 1; j < size - 1; j++ ) {
                temp = matrix[size * i + j];
                matrix[size * i + j] = 0.25 * (matrix[size * (i - 1) + j] +
                                               matrix[size * (i + 1) + j] +
                                               matrix[size * i + (j - 1)] +
                                               matrix[size * i + (j + 1)] );
                d = fabs(temp-matrix[size * i + j]);
                if ( dm < d )
                    dm = d;
            }
            
            if ( dmax < dm ) {
                dmax = dm;
            }
        }

    } while ( dmax > eps );
    
    std::cout << "\nSEQ iter: " << q;
}

int main(int argc, char **argv)
{
    int size = 1000;
    double eps = 0.01;
    
    std::cout << "Start seq... " << std::endl;
    double* matrix_seq;
    matrix_seq = new double[size * size];
    FillingTheMatrix(matrix_seq, size);
    
    oneapi::tbb::tick_count start_seq = omp_get_wtime();
    SequentialAlg(matrix_seq, size, eps);
    oneapi::tbb::tick_count end_seq = omp_get_wtime();
    
    double time_seq = end_seq - start_seq;
    std::cout << ", SEQ Time: " << time_seq << std::endl;
    
    if (size < 15)
        PrintMatrix(matrix_seq, size);
    
    delete[] matrix_seq;
    return 0;
}