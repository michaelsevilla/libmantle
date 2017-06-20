#include <iostream>
#include <Mantle.h>
#include <mpi.h>
#include <string>

int main() {

  /* Mantle garbage */
  Mantle mantle;
  mantle.start();

  /* MPI garbage */
  MPI_Init(NULL, NULL);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  vector < map<string, double> > metrics (world_size);
  map<rank_t,double> my_targets;
  mantle.balance("BAL_LOG(0, \"Hello world from balance!\")\n return {}", world_rank, metrics, my_targets);
}
