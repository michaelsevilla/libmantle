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

  if(world_size < 2) {
    std::cerr<<"Please run with mpirun -np > 1"<<std::endl;
    exit(EXIT_FAILURE);
  }

  vector < map<string, double> > metrics (world_size);
  for(int i = 0; i < world_size; i++) {
    metrics[i] = {{"metric0", 0.00},
                  {"metric1", 1.11},
                  {"metric2", 2.22}};
  }
  
  /* Call each Mantle function with empty metrics */
  bool when; float howmuch; map<rank_t,double> where;
  mantle.when(   "BAL_LOG(0, \"when() received \", server[1]['metric1'])\nreturn true",
                 metrics, world_rank, when);
  mantle.howmuch("BAL_LOG(0, \"howmuch() received \", server[1]['metric1'])\nreturn 3.145",
                 metrics, world_rank, howmuch);
  mantle.where(  "BAL_LOG(0, \"where() received \", server[1]['metric1'])\nreturn {1.111, 2.222, 3.333}",
                 metrics, world_rank, where);

  std::cout<<"where= ";
  for(map<rank_t, double>::iterator it = where.begin();
      it != where.end();
      it++)
    std::cout<<"mds"<<it->first<<"="<<it->second<<" ";
  std::cout<<std::endl;
  std::cout<<"when="<<when<<std::endl;
  std::cout<<"howmuch="<<howmuch<<std::endl;
  MPI_Finalize();
}
