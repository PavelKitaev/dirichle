// Copyright 2023 Kitaev Pavel

#include "omp.h"
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

void ParallelAlgOMP(double* matrix, int size, double eps, int th_num)
{
  double* dm = new double[size];
  double dmax, temp, d;
  const int N = size - 2;
  int j = 0;
  int q = 0;
  do {
    dmax = 0;
    q++;

#pragma omp parallel for private(temp, d, j) num_threads(th_num)
    for (int wave = 1; wave < N + 1; wave++) {
      dm[wave - 1] = 0;
      for (int i = 1; i < wave + 1; i++) {
        j = wave + 1 - i;
        temp = matrix[size * i + j];

        matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
          matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
          matrix[size * (i - 1) + j]);

        d = fabs(temp - matrix[size * i + j]);
        if (dm[wave - 1] < d) {
          dm[wave - 1] = d;
        }
      }
    }

#pragma omp parallel for private(temp, d, j) num_threads(th_num)
    for (int wave = N - 1; wave > 0; wave--) {
      dm[wave - 1] = 0;
      for (int i = N - wave + 1; i < N + 1; i++) {
        j = 2 * N - wave - i + 1;
        temp = matrix[size * i + j];

        matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
          matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
          matrix[size * (i - 1) + j]);

        d = fabs(temp - matrix[size * i + j]);
        if (dm[wave - 1] < d) {
          dm[wave - 1] = d;
        }
      }
    }

    for (int i = 0; i < size; i++)
    {
      if (dmax < dm[i]) {
        dmax = dm[i];
      }
    }

  } while (dmax > eps);

  std::cout << "Iter omp: " << q;
}

int main(int argc, char **argv)
{
    double eps = 0.01;
    int size;
    sscanf(argv[1], "%d", &size);

    int procNum;
    sscanf(argv[2], "%d", &procNum);
    
    double* matrix_omp = new double[size * size];
    FillingTheMatrix(matrix_omp, size);

    double start_omp = omp_get_wtime();
    ParallelAlgOMP(matrix_omp, size, eps, procNum);
    double end_omp = omp_get_wtime();

    double time_omp = end_omp - start_omp;
    std::cout << ", OMP Time: " << time_omp << std::endl;

    if (size < 15)
        PrintMatrix(matrix_omp, size);

    delete[] matrix_omp;
    return 0;
}
