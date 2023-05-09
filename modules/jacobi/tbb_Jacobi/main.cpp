#include <iostream>
#include <cmath>
#include "oneapi/tbb.h"

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

void ParallelAlgTBB(double* matrix, int size, double eps, int th_num) {
    oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism, th_num);
    
    double* matrix_temp = new double[size*size];
    for (int i = 1; i < size - 1; i++ ) {
        for (int j = 1; j < size - 1; j++ ) {
            matrix_temp[size * i + j] = matrix  [size * i + j];
        }
    }
    
    int q = 0;
    double dmax, temp, dm, d;
    static tbb::spin_mutex mtx;
    std::mutex mutex;

    do {
        q++;
        dmax = 0;
        tbb::parallel_for(tbb::blocked_range<int>(1, size - 1, 1), [&](const tbb::blocked_range<int>& range) {
            for (int i = range.begin(); i < range.end(); i++ ) {
                double dm = 0;
                for (int j = 1; j < size - 1; j++ ) {
                    double temp = matrix[size * i + j];
                    matrix_temp[size * i + j] = 0.25 * (matrix[size * (i - 1) + j] +
                                                   matrix[size * (i + 1) + j] +
                                                   matrix[size * i + (j - 1)] +
                                                   matrix[size * i + (j + 1)] );
                    double d = fabs(temp-matrix_temp[size * i + j]);
                    if ( dm < d )
                        dm = d;
                }
                
                mtx.lock();
                if ( dmax < dm ) {
                    dmax = dm;
                }
                mtx.unlock();
            }
        });
        
        for (int i = 1; i < size - 1; i++ ) {
            for (int j = 1; j < size - 1; j++ ) {
                matrix[size * i + j] = matrix_temp[size * i + j];
            }
        }
    } while ( dmax > eps );
    
    std::cout << "\nTBB iter: " << q;
}

int main(int argc, char **argv)
{
    double eps = 0.01;
    int size;
    sscanf(argv[1], "%d", &size);
    int procNum;
    sscanf(argv[2], "%d", &procNum);

    double* matrix_tbb = new double[size * size];
    FillingTheMatrix(matrix_tbb, size);

    oneapi::tbb::tick_count start_tbb = oneapi::tbb::tick_count::now();
    ParallelAlgTBB(matrix_tbb, size, eps, procNum);
    oneapi::tbb::tick_count end_tbb = oneapi::tbb::tick_count::now();

    double time_tbb = (end_tbb - start_tbb).seconds();
    std::cout << ", TBB Time: " << time_tbb << std::endl;

    if (size < 15)
        PrintMatrix(matrix_tbb, size);

    delete[] matrix_tbb;
    return 0;
}
