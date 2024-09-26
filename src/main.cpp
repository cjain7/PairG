/**
 * @file    main.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

#include <pairg/spgemm_utility.hpp>
#include <pairg/utility.hpp>
#include <pairg/reachability.hpp>
#include <pairg/heuristics.hpp>

//External includes
#include "clipp/include/clipp.h"
#include "prettyprint/prettyprint.hpp"
#include "PaSGAL/graphLoad.hpp"

#include "parseCmdArgs.hpp"

/**
 * @brief   get a random pair of integers, each value lies in [0, MAX)
 */
std::pair<int,int> getRandomPair(int MAX)
{
  int first = rand() % MAX;
  int second = rand() % MAX;

  return std::make_pair(first, second);
}

/**
 * @brief     main function
 */
int main(int argc, char* argv[]) 
{
  //parse command line arguments   
  pairg::Parameters parameters;        
  pairg::parseandSave(argc, argv, parameters);   

  //initialize kokkos
  Kokkos::initialize();

  {
    pairg::timer T1;

    //load input graph
    psgl::graphLoader g;
    {
      if (parameters.gmode.compare("vg") == 0)
        g.loadFromVG(parameters.graphfile);
      else if(parameters.gmode.compare("txt") == 0)
        g.loadFromTxt(parameters.graphfile);
      else
      {
        std::cerr << "Invalid graph format " << parameters.gmode << std::endl;
        exit(1);
      }
    }

    //build adjacency matrix from input graph
    //Use g.diCharGraph to build adjacency matrix
    pairg::matrixOps<>::crsMat_t adj_mat = pairg::getAdjacencyMatrix(g.diCharGraph);
    std::cout << "INFO, pairg::main, Time to build adjacency matrix (ms): " << T1.elapsed() << "\n";
    pairg::matrixOps<>::printMatrix(adj_mat, 1);

    pairg::timer T2;

    //build index matrix
    pairg::matrixOps<>::crsMat_t valid_pairs_mat = pairg::buildValidPairsMatrix(adj_mat, parameters.d_low, parameters.d_up);
    std::cout << "INFO, pairg::main, Time to build result matrix (ms): " << T2.elapsed() << "\n";
    pairg::matrixOps<>::printMatrix(valid_pairs_mat, 1);

    //build a set of distance queries
    std::vector< std::pair<int,int> > random_pairs;

    for(int i = 0; i < parameters.querycount; i++)
    {
      auto p = getRandomPair (valid_pairs_mat.numRows());
      random_pairs.push_back(p);
    }

    //answer queries using index
    std::vector<bool> results_spgemm(parameters.querycount);
    pairg::timer T3;
    for(int i = 0; i < parameters.querycount; i++)
    {
      results_spgemm[i] = pairg::matrixOps<>::queryValue (valid_pairs_mat, random_pairs[i].first, random_pairs[i].second);
    }
    std::cout << "INFO, pairg::main, Time to execute " << parameters.querycount << " queries (ms): " << T3.elapsed() << "\n";
  }

  std::cout << std::flush;

  Kokkos::finalize();
}
