
#include "wave_equation.h"

#include <xtensor/xpad.hpp>

#include <mpi.h>

void fill_ghosts(xt::xtensor<double, 1>& f_g)
{
  const int G = 1;
  int n = f_g.shape(0) - 2 * G;

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const int tag_left_to_right = 123;
  const int tag_right_to_left = 456;

  int rank_right = (rank < size - 1) ? rank + 1 : 0;
  int rank_left = (rank > 0) ? rank - 1 : size - 1;

  MPI_Send(&f_g(G + n - 1), 1, MPI_DOUBLE, rank_right, tag_left_to_right,
           MPI_COMM_WORLD);
  MPI_Recv(&f_g(G - 1), 1, MPI_DOUBLE, rank_left, tag_left_to_right,
           MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  MPI_Send(&f_g(G + 0), 1, MPI_DOUBLE, rank_left, tag_right_to_left,
           MPI_COMM_WORLD);
  MPI_Recv(&f_g(G + n), 1, MPI_DOUBLE, rank_right, tag_right_to_left,
           MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

xt::xtensor<double, 1> derivative(const mpi_domain& domain,
                                  const xt::xtensor<double, 1>& f)
{
  const int G = 1;
  int n = domain.n();
  assert(f.shape(0) == n);

  auto f_g = xt::pad(f, G);
  fill_ghosts(f_g);

  auto fprime = xt::zeros_like(f);
  for (int i = 0; i < n; i++) {
    fprime(i) = (f_g(G + i + 1) - f_g(G + i + -1)) / (2. * domain.dx());
  }

  return fprime;
}
