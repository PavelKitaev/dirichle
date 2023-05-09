#include <iostream>
#include <cmath>
#include <omp.h>
#include <thread>
#include <future>

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

void ParallelAlgSTD(double* matrix, int size, double eps, int th_num) {
    try {
        if (size-2 < th_num) {
            throw "size-2 < th_num";
        }
    } catch (const char* error) {
        std::cout << error << std::endl;
        th_num = size-2;
    }
    
    std::thread* threads = new std::thread[th_num];
    double dmax, temp, d, dm;
    
    const int delta = (size-2) / th_num;
    int residue = (size-2) % th_num;
    
    int* begin = new int[th_num];
    int* end = new int[th_num];
    
    begin[0] = 1;
    end[0] = begin[0] + delta;
    
    for (int i = 1; i < th_num; i++) {
        begin[i] = end[i-1];
        end[i] = begin[i] + delta;
        
        if (residue != 0) {
            end[i-1] += residue;
        }
    }
    
    end[th_num - 1] += residue;
    
    std::mutex mutex;
    int q = 0;
    
    do {
        q++;
        dmax = 0;
        
        for (int i = 0; i < th_num; i++) {
            threads[i] = std::thread([&](int begin, int end) {
                for (int row = begin; row < end; row++) {
                    double dm = 0;
                    for (int col = 1; col < size-1; col++) {
                        double temp = matrix[size * row + col];
                        
                        matrix[size * row + col] = 0.25 * (matrix[size * (row - 1) + col] +
                                                           matrix[size * (row + 1) + col] +
                                                           matrix[size * row + (col - 1)] +
                                                           matrix[size * row + (col + 1)] );
                        double d = fabs(temp-matrix[size * row + col]);
                        if ( dm < d )
                            dm = d;
                    }
                    
                    mutex.lock();
                    if ( dmax < dm ) {
                        dmax = dm;
                    }
                    mutex.unlock();
                }
            }, begin[i], end[i]);
        }
        
        for (int i = 0; i < th_num; i++) {
            threads[i].join();
        }
    } while (dmax > eps);
    
    delete[] threads;
    
    delete[] begin;
    delete[] end;
    
    std::cout << "\nSTD iter: " << q;
}

int main(int argc, char **argv)
{
    int size = 10;
    double eps = 0.01;
    int procNum = 2;

    double* matrix_std = new double[size * size];
    FillingTheMatrix(matrix_std, size);

    oneapi::tbb::tick_count start_std = omp_get_wtime();
    ParallelAlgSTD(matrix_std, size, eps, procNum);
    oneapi::tbb::tick_count end_std = omp_get_wtime();

    double time_std = end_std - start_std;
    std::cout << ", STD Time: " << time_std << std::endl;

    if (size < 15)
        PrintMatrix(matrix_std, size);

    delete[] matrix_std;
    return 0;
}