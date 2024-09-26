/**
 * @file    reachability.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

#ifndef PAIRG_REACHABILITY_HPP
#define PAIRG_REACHABILITY_HPP

#include "spgemm_utility.hpp"

namespace pairg
{
  /**
   * @brief     build adjacency matrix from variaton graph
   */
  template< class TCharGraph, typename TMatrixOps=matrixOps<> >
  inline typename TMatrixOps::crsMat_t getAdjacencyMatrix(const TCharGraph &cg)
  {
    typename TMatrixOps::lno_t nrows = cg.numVertices;
    typename TMatrixOps::size_type nnz = cg.numEdges;

    typename TMatrixOps::lno_nnz_view_t entries("entries", nnz);
    typename TMatrixOps::scalar_view_t values("values", nnz);
    typename TMatrixOps::lno_view_t rowmap("rowmap", nrows + 1);

    for(typename TMatrixOps::size_type i = 0; i < nnz; i++) {
      values(i) = 1;  //boolean
    }

    for(typename TMatrixOps::size_type i = 0; i < nnz; i++) {
      entries(i) = cg.adjcny_out[i];
    }

    for(typename TMatrixOps::lno_t i = 0; i < nrows + 1; i++) {
      rowmap(i) = cg.offsets_out[i];
    }

    return typename TMatrixOps::crsMat_t("adjacency matrix", nrows, nrows, nnz, values, rowmap, entries);
  }

  /**
   * @brief            build matrix associated with valid vertices that satisfy
   *                   distance constraints
   * @param[in] A      graph adjacency matrix
   * @param[in] d_low  distance constraint: lower bound on path length
   * @param[in] d_up   distance constraint: upper bound on path length
   * @return           validity matrix
   *                   cell (i,j) = 1 iff there is a valid path from v_i to v_j
   */
  template< typename TMatrixOps=matrixOps<> >
  inline typename TMatrixOps::crsMat_t buildValidPairsMatrix(const typename TMatrixOps::crsMat_t &A, int d_low, int d_up)
  {
    pairg::timer T1;
    typename TMatrixOps::crsMat_t B = TMatrixOps::addMatrices(A, TMatrixOps::createIdentityMatrix(A.numRows()));
    std::cout << "INFO, pairg::buildValidPairsMatrix, time to add identity matrix (ms): " << T1.elapsed() << "\n";

    pairg::timer T2;
    typename TMatrixOps::crsMat_t C = TMatrixOps::power(A, d_low);
    std::cout << "INFO, pairg::buildValidPairsMatrix, time to raise adjacency matrix (ms): " << T2.elapsed() << "\n";

    pairg::timer T3;
    typename TMatrixOps::crsMat_t D = TMatrixOps::power (B, d_up - d_low);
    std::cout << "INFO, pairg::buildValidPairsMatrix, time to raise adjacency+identity matrix (ms): " << T3.elapsed() << "\n";

    pairg::timer T4;
    typename TMatrixOps::crsMat_t E = TMatrixOps::multiplyMatrices(C,D);
    std::cout << "INFO, pairg::buildValidPairsMatrix, time to execute final multiplication (ms): " << T4.elapsed() << "\n";

    //sort entries within each row
    pairg::timer T5;
    TMatrixOps::indexForQuery(E);
    std::cout << "INFO, pairg::buildValidPairsMatrix, time to index for querying (ms): " << T5.elapsed() << "\n";

    return E;
  }
}

#endif
