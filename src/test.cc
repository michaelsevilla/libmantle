#include <iostream>
#include <Mantle.h>
#include <mpi.h>
#include <string>
#include <unistd.h>

int main() {

  /* MPI garbage */
  MPI_Init(NULL, NULL);
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if(world_size < 2) {
    std::cerr<<"Please run with mpirun -np > 1"<<std::endl;
    exit(EXIT_FAILURE);
  }

  /* Mantle garbage */
  string testlog = "BAL_LOG(0, \"received \", server[1][\"metric2\"])\n";
  Policy when    = testlog + "return true";
  Policy where   = testlog + "return {1.111, 2.222, 3.333}";
  Policy howmuch = testlog + "return 3.145";
  Mantle mantle(world_rank);
  mantle.configure(when, where, howmuch);

  ClusterMetrics metrics (world_size);
  for(int i = 0; i < world_size; i++)
    metrics[i] = {{"metric0", 0.00}, {"metric1", 1.11}, {"metric2", 2.22}};
  mantle.update(metrics);

  /* Call each Mantle function with empty metrics */
  bool when_decision;
  mantle.when(when_decision);

  Load howmuch_decision;
  mantle.howmuch(howmuch_decision);

  Targets where_decision;
  mantle.where(where_decision);

  usleep(1000000);   
  if (world_rank == 0) {
    std::cout<<"where:(";
    for(Targets::iterator it = where_decision.begin();
        it != where_decision.end();
        it++)
      std::cout<<"mds"<<it->first<<"="<<it->second<<" ";
    std::cout<<")"<<std::endl;
    std::cout<<"when:"<<when_decision<<std::endl;
    std::cout<<"howmuch:"<<howmuch_decision<<std::endl;
  }
  MPI_Finalize();
}
