// Copyright 2023 Kitaev Pavel

#include "oneapi/tbb.h"
#include <iostream>
#include <cmath>

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

void ParallelAlgTBB(double* matrix, int size, double eps, int th_num)
{
  tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, th_num);

  double* dm = new double[size] { 0 };
  double dmax;
  const int N = size - 2;

  int q = 0;

  do {
    q++;

    dmax = 0;
    tbb::parallel_for(tbb::blocked_range<int>(1, N + 1, size / th_num),
      [&](const tbb::blocked_range<int>& range) {
        for (int wave = range.begin(); wave < range.end(); wave++) {
          dm[wave - 1] = 0;
          for (int i = 1; i < wave + 1; i++) {
            int j = wave + 1 - i;
            double temp = matrix[size * i + j];

            matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
              matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
              matrix[size * (i - 1) + j]);

            double d = fabs(temp - matrix[size * i + j]);

            if (dm[wave - 1] < d) {
              dm[wave - 1] = d;
            }
          }
        }
      });

    tbb::parallel_for(tbb::blocked_range<int>(0, N - 1, size / th_num),
      [&](const tbb::blocked_range<int>& range) {
        for (int wave = range.end(); wave > range.begin(); wave--) {
          dm[wave - 1] = 0;
          for (int i = N - wave + 1; i < N + 1; i++) {
            int j = 2 * N - wave - i + 1;
            double temp = matrix[size * i + j];

            matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
              matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
              matrix[size * (i - 1) + j]);

            double d = fabs(temp - matrix[size * i + j]);

            if (dm[wave - 1] < d) {
              dm[wave - 1] = d;
            }
          }
        }
      });

    for (int i = 0; i < size; i++) {
      if (dmax < dm[i]) {
        dmax = dm[i];
      }
    }
  } while (dmax > eps);

  delete[] dm;

  std::cout << "Iter tbb: " << q;
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