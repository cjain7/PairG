/**
 * @file    test_graphLoad.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

//External includes
#include <pairg/reachability.hpp>
#include <catch2/catch.hpp>

#include "PaSGAL/graphLoad.hpp"
#include "parseCmdArgs.hpp"

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define FOLDER STR(PROJECT_TEST_DATA_DIR)

TEST_CASE("loading .txt formatted graph") 
{
  //get file name
  std::string file = FOLDER;
  file = file + "/chain.txt";

  int V = 81189;
  int E = 81188;

  //load graph

  psgl::graphLoader g;
  g.loadFromTxt(file);
  auto &graph = g.diCharGraph;

  SECTION( "evaluating graph size" ) {
    REQUIRE(graph.numVertices == V); 
    REQUIRE(graph.numEdges == E); 
  }
}


TEST_CASE("loading .vg formatted graph") 
{
  //get file name
  std::string file = FOLDER;
  file = file + "/chain.vg";

  /**
   * PaSGAL ends up adding one extra dummy vertex
   * since vg uses a 1-based vertex ordering
   */
  int V = 81190;
  int E = 81188;

  //load graph

  psgl::graphLoader g;
  g.loadFromVG(file);
  auto &graph = g.diCharGraph;

  SECTION( "evaluating graph size" ) {
    REQUIRE(graph.numVertices == V);  
    REQUIRE(graph.numEdges == E); 
  }
}


TEST_CASE("converting .txt formatted graph to CSR adjacency matrix") 
{
  //get file name
  std::string file = FOLDER;
  file = file + "/chain.txt";

  std::vector<char> RFILE(file.c_str(), file.c_str() + file.size() + 1u);

  char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "1", "-u", "2", "-c", "0", "-t", "4", nullptr};
  int argc = 13;

  int V = 81189;
  int E = 81188;

  pairg::Parameters parameters;        
  pairg::parseandSave(argc, argv, parameters);

  psgl::graphLoader g;
  g.loadFromTxt(parameters.graphfile);

  {
    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);

    SECTION( "evaluating matrix size" ) {
      REQUIRE(A.numRows() == V);  
      REQUIRE(A.numCols() == V); 
      REQUIRE(A.graph.row_map.extent(0) == V + 1); 
      REQUIRE(A.graph.entries.extent(0) == E); 
      REQUIRE(A.values.extent(0) == E); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(A.values.data(), A.values.data() + A.values.extent(0), 0) == E);
      REQUIRE(std::accumulate(A.values.data(), A.values.data() + A.values.extent(0), 1, std::multiplies<int>()) == 1);
    }
  }
}


TEST_CASE("converting .vg formatted graph to CSR adjacency matrix") 
{
  //get file name
  std::string file = FOLDER;
  file = file + "/chain.vg";

  std::vector<char> RFILE(file.c_str(), file.c_str() + file.size() + 1u);

  char *argv[] = {"pairmap2graph", "-m", "vg", "-r", RFILE.data(), "-l", "1", "-u", "2", "-c", "0", "-t", "4", nullptr};
  int argc = 13;

  int V = 81190;
  int E = 81188;

  pairg::Parameters parameters;        
  pairg::parseandSave(argc, argv, parameters);

  psgl::graphLoader g;
  g.loadFromVG(parameters.graphfile);

  {
    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);

    SECTION( "evaluating matrix size" ) {
      REQUIRE(A.numRows() == V);  
      REQUIRE(A.numCols() == V); 
      REQUIRE(A.graph.row_map.extent(0) == V + 1); 
      REQUIRE(A.graph.entries.extent(0) == E); 
      REQUIRE(A.values.extent(0) == E); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(A.values.data(), A.values.data() + A.values.extent(0), 0) == E);
      REQUIRE(std::accumulate(A.values.data(), A.values.data() + A.values.extent(0), 1, std::multiplies<int>()) == 1);
    }
  }
}
