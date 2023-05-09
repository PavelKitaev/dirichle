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

void SequentialAlg(double* matrix, int size, double eps) {
  double dm = 0;
  double dmax, temp, d;
  const int N = size - 2;
  int j = 0;
  int q = 0;
  do {
    dmax = 0;
    q++;

    for (int wave = 1; wave < N + 1; wave++) {
      dm = 0;
      for (int i = 1; i < wave + 1; i++) {
        j = wave + 1 - i;
        temp = matrix[size * i + j];

        matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
          matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
          matrix[size * (i - 1) + j]);

        d = fabs(temp - matrix[size * i + j]);
        if (dm < d) {
          dm = d;
        }
      }
    }

    for (int wave = N - 1; wave > 0; wave--) {
      for (int i = N - wave + 1; i < N + 1; i++) {
        j = 2 * N - wave - i + 1;
        temp = matrix[size * i + j];

        matrix[size * i + j] = 0.25 * (matrix[size * i + j + 1] +
          matrix[size * i + j - 1] + matrix[size * (i + 1) + j] +
          matrix[size * (i - 1) + j]);

        d = fabs(temp - matrix[size * i + j]);
        if (dm < d) {
          dm = d;
        }
      }
    }

    if (dmax < dm) {
      dmax = dm;
    }

  } while (dmax > eps);

  std::cout << "Iter seq: " << q;
}

int main(int argc, char **argv)
{
    double eps = 0.01;
    int size;
    sscanf(argv[1], "%d", &size);

    std::cout << "Start seq... " << std::endl;
    double* matrix_seq;
    matrix_seq = new double[size * size];
    FillingTheMatrix(matrix_seq, size);

    double start_seq = omp_get_wtime();
    SequentialAlg(matrix_seq, size, eps);
    double end_seq = omp_get_wtime();

    double time_seq = end_seq - start_seq;
    std::cout << ", SEQ Time: " << time_seq << std::endl;

    if (size < 15)
        PrintMatrix(matrix_seq, size);

    delete[] matrix_seq;
    return 0;
}
