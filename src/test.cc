#include <iostream>
#include <fstream>
#include <sstream>
#include <Mantle.h>
#include <mpi.h>
#include <string>
#include <unistd.h>

std::string read_policy(std::string fname) {
  std::ifstream f(fname);
  std::stringstream buffer;
  buffer << f.rdbuf();
  return buffer.str();
}

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

  if(world_rank > 0) {
    MPI_Finalize();
    exit(EXIT_SUCCESS);
  }

  /* Mantle garbage */
  std::string d   = "../src/policies/";
  Policy load     = read_policy(d+"load.lua");
  Policy when     = read_policy(d+"when.lua");
  Policy where    = read_policy(d+"where.lua");
  Policy howmuch  = read_policy(d+"howmuch.lua");
  Policy debugenv = read_policy(d+"debugenv.lua");
  Mantle mantle(world_rank);
  mantle.configure(load, when, where, howmuch);

  ClusterMetrics metrics (world_size);
  for(int i = 0; i < world_size; i++)
    metrics[i] = {{"metric0", 0.00}, {"metric1", 1.11}, {"metric2", 2.22}};
  mantle.update(metrics);
  mantle.debugenv(debugenv);

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
