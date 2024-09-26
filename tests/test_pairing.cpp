/**
 * @file    test_pairing.hpp
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

TEST_CASE("building valid-pair matrix for a chain graph") 
{
  //get file name
  std::string file = FOLDER;
  file = file + "/chain.txt";

  std::vector<char> RFILE(file.c_str(), file.c_str() + file.size() + 1u);

  SECTION( "both distance limits are zero" )
  {
    char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "0", "-u", "0", "-t", "4", "-c", "0", nullptr};
    int argc = 13;

    pairg::Parameters parameters;        
    pairg::parseandSave(argc, argv, parameters);

    int V = 81189;
    int E = 81188;

    psgl::graphLoader g;
    g.loadFromTxt(parameters.graphfile);

    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);
    pairg::matrixOps<>::crsMat_t B = pairg::buildValidPairsMatrix(A, parameters.d_low, parameters.d_up);

    //B should be VxV identity matrix if distance contraints are 0,0

    SECTION( "evaluating matrix size" ) {
      REQUIRE(B.numRows() == V);  
      REQUIRE(B.numCols() == V); 
      REQUIRE(B.graph.row_map.extent(0) == V + 1); 
      REQUIRE(B.graph.entries.extent(0) == V); 
      REQUIRE(B.values.extent(0) == V); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 0) == V);
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 1, std::multiplies<int>()) == 1);
    }
  }

  SECTION( "both distance limits are one" )
  {
    char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "1", "-u", "1", "-t", "4", "-c", "0", nullptr};
    int argc = 13;

    pairg::Parameters parameters;        
    pairg::parseandSave(argc, argv, parameters);

    int V = 81189;
    int E = 81188;

    psgl::graphLoader g;
    g.loadFromTxt(parameters.graphfile);

    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);
    pairg::matrixOps<>::crsMat_t B = pairg::buildValidPairsMatrix(A, parameters.d_low, parameters.d_up);

    //B should be same as A if distance contraints are 1,1

    SECTION( "evaluating matrix size" ) {
      REQUIRE(B.numRows() == V);  
      REQUIRE(B.numCols() == V); 
      REQUIRE(B.graph.row_map.extent(0) == V + 1); 
      REQUIRE(B.graph.entries.extent(0) == E); 
      REQUIRE(B.values.extent(0) == E); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 0) == E);
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 1, std::multiplies<int>()) == 1);
    }
  }

  SECTION( "both distance limits are > |V|" )
  {
    char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "100000", "-u", "100000", "-t", "4", "-c", "0", nullptr};
    int argc = 13;

    pairg::Parameters parameters;        
    pairg::parseandSave(argc, argv, parameters);

    int V = 81189;
    int E = 81188;

    psgl::graphLoader g;
    g.loadFromTxt(parameters.graphfile);

    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);
    pairg::matrixOps<>::crsMat_t B = pairg::buildValidPairsMatrix(A, parameters.d_low, parameters.d_up);

    //B should be a zero matrix 

    SECTION( "evaluating matrix size" ) {
      REQUIRE(B.numRows() == V);  
      REQUIRE(B.numCols() == V); 
      REQUIRE(B.graph.row_map.extent(0) == V + 1); 
      REQUIRE(B.graph.entries.extent(0) == 0); 
      REQUIRE(B.values.extent(0) == 0); 
    }
  }

  SECTION( "distance limits are 0, 50" )
  {
    char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "0", "-u", "50", "-t", "4", "-c", "0", nullptr};
    int argc = 13;

    pairg::Parameters parameters;        
    pairg::parseandSave(argc, argv, parameters);

    int V = 81189;
    int E = 81188;

    psgl::graphLoader g;
    g.loadFromTxt(parameters.graphfile);

    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);
    pairg::matrixOps<>::crsMat_t B = pairg::buildValidPairsMatrix(A, parameters.d_low, parameters.d_up);

    //B should have 4,139,313 nnz values (= 51*81139 + {50,49...1})
    int NNZ = 4139364;

    SECTION( "evaluating matrix size" ) {
      REQUIRE(B.numRows() == V);  
      REQUIRE(B.numCols() == V); 
      REQUIRE(B.graph.row_map.extent(0) == V + 1); 
      REQUIRE(B.graph.entries.extent(0) == NNZ); 
      REQUIRE(B.values.extent(0) == NNZ); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 0) == NNZ);
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 1, std::multiplies<int>()) == 1);
    }

    SECTION( "checking whether queries are answered correctly" ) {
      //Remember that vertex ids are 0-based while querying

      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 0) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 1) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 1, 0) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 50) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 51) == false);

      REQUIRE(pairg::matrixOps<>::queryValue (B, 81137, 81188) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 81138, 81188) == true);
    }
  }

  SECTION( "distance limits are 100, 110" )
  {
    char *argv[] = {"pairmap2graph", "-m", "txt", "-r", RFILE.data(), "-l", "100", "-u", "110", "-t", "4", "-c", "0", nullptr};
    int argc = 13;

    pairg::Parameters parameters;        
    pairg::parseandSave(argc, argv, parameters);

    int V = 81189;
    int E = 81188;

    psgl::graphLoader g;
    g.loadFromTxt(parameters.graphfile);

    pairg::matrixOps<>::crsMat_t A = pairg::getAdjacencyMatrix(g.diCharGraph);
    pairg::matrixOps<>::crsMat_t B = pairg::buildValidPairsMatrix(A, parameters.d_low, parameters.d_up);

    //B should have 891,924 nnz values (= 11*81079 + {10,9...1})
    int NNZ = 891924;

    SECTION( "evaluating matrix size" ) {
      REQUIRE(B.numRows() == V);  
      REQUIRE(B.numCols() == V); 
      REQUIRE(B.graph.row_map.extent(0) == V + 1); 
      REQUIRE(B.graph.entries.extent(0) == NNZ); 
      REQUIRE(B.values.extent(0) == NNZ); 
    }

    SECTION( "evaluating matrix content" ) {
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 0) == NNZ);
      REQUIRE(std::accumulate(B.values.data(), B.values.data() + B.values.extent(0), 1, std::multiplies<int>()) == 1);
    }

    SECTION( "checking whether queries are answered correctly" ) {
      //Remember that vertex ids are 0-based while querying

      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 0) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 1) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 99) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 100) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 110) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 0, 111) == false);

      REQUIRE(pairg::matrixOps<>::queryValue (B, 81088, 81188) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 81089, 81188) == false);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 81078, 81188) == true);
      REQUIRE(pairg::matrixOps<>::queryValue (B, 81077, 81188) == false);
    }
  }
}
