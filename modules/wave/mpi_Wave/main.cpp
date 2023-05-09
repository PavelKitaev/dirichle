// Copyright 2023 Kitaev Pavel

#include "mpi.h"
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

void ParallelAlgMPI(double* matrix, int size, double eps)
{
  MPI_Barrier(MPI_COMM_WORLD);
  int procRank, procNum;
  MPI_Comm_size(MPI_COMM_WORLD, &procNum);
  MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

  double* dm;
  MPI_Win win_dm;
  if (procRank == 0) {
    MPI_Win_allocate_shared(size * sizeof(double), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &dm, &win_dm);
  }
  else {
    int disp;
    MPI_Aint ssize;
    MPI_Win_allocate_shared(0, sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &dm, &win_dm);
    MPI_Win_shared_query(win_dm, 0, &ssize, &disp, &dm);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  double dmax;

  int start_inc;
  int end_inc;
  int start_dec;
  int end_dec;

  int q = 0;

  if (procRank == 0)
  {
    q++;
    const int delta = size / procNum;
    int residue = size % procNum;

    int* i_start_inc = new int[procNum] {0};
    int* i_end_inc = new int[procNum] {0};
    int* i_start_dec = new int[procNum] {0};
    int* i_end_dec = new int[procNum] {0};

    i_start_inc[0] = 1;
    i_end_inc[0] = 0;

    i_start_dec[0] = (size - 2) - 1;
    i_end_dec[0] = (size - 2) - 1;

    for (int k = 0; k < procNum; k++) {
      i_end_inc[k] += (delta);
      i_end_dec[k] -= (delta);

      if (residue > 0) {
        i_end_dec[k]--;

        i_end_inc[k]++;
        residue--;
      }

      if (k == procNum - 1) {
        i_end_inc[k] = size - 1;
        if (procNum == 2) {
          i_start_dec[k]++;
          i_end_dec[k] = i_end_dec[k - 1] - delta + 1;
          i_end_dec[k - 1]++;
        }
        else {
          i_end_dec[k] = size - 1;
        }
      }
      else {
        i_start_inc[k + 1] = i_end_inc[k];
        i_start_dec[k + 1] = i_end_dec[k];

        i_end_inc[k + 1] = i_end_inc[k];
        i_end_dec[k + 1] = i_end_dec[k];
      }
    }

    start_inc = i_start_inc[0];
    end_inc = i_end_inc[0];
    start_dec = i_start_dec[0];
    end_dec = i_end_dec[0];

    char buff[255];
    for (int i = 1; i < procNum; i++) {
      int pos = 0;
      MPI_Pack(&i_start_inc[i], 1, MPI_INT, buff, 255, &pos, MPI_COMM_WORLD);
      MPI_Pack(&i_start_dec[i], 1, MPI_INT, buff, 255, &pos, MPI_COMM_WORLD);
      MPI_Pack(&i_end_inc[i], 1, MPI_INT, buff, 255, &pos, MPI_COMM_WORLD);
      MPI_Pack(&i_end_dec[i], 1, MPI_INT, buff, 255, &pos, MPI_COMM_WORLD);

      MPI_Send(buff, pos, MPI_PACKED, i, 0, MPI_COMM_WORLD);
    }

    delete[] i_start_inc;
    delete[] i_end_inc;
    delete[] i_start_dec;
    delete[] i_end_dec;
  }

  if (procRank != 0) {
    MPI_Status status;
    int temp[4];
    MPI_Recv(&temp, 4, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    start_inc = temp[0];
    start_dec = temp[1];
    end_inc = temp[2];
    end_dec = temp[3];
  }

  MPI_Barrier(MPI_COMM_WORLD);
  do
  {
    for (int wave = start_inc; wave < end_inc; wave++) {
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

    for (int wave = start_dec; wave > end_dec; wave--) {
      dm[wave - 1] = 0;
      for (int i = (size - 2) - wave + 1; i < (size - 2) + 1; i++) {
        int j = 2 * (size - 2) - wave - i + 1;

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

    MPI_Barrier(MPI_COMM_WORLD);

    if (procRank == 0)
    {
      dmax = 0;
      for (int i = 0; i < size; i++) {
        if (dmax < dm[i]) {
          dmax = dm[i];
        }
      }
    }

    MPI_Bcast(&dmax, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
  } while (dmax > eps);

  if (procRank == 0)
    std::cout << "MPI Iter: " << q;

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Win_free(&win_dm);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int procRank, procNum;
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    double eps = 0.01;
    int size;
    sscanf(argv[1], "%d", &size);
  
    double start_mpi, end_mpi;
    double* matrix_mpi;
    MPI_Win win_matrix;      //Создаем окно доступа к матрице

    if (procRank == 0)
    {
        MPI_Win_allocate_shared(size*size * sizeof(double), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &matrix_mpi, &win_matrix);
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

    if (procRank == 0){
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