/**
 * @file    spgemm_utility.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

#ifndef SPGEMM_UTILITY_HPP
#define SPGEMM_UTILITY_HPP

#define KOKKOSKERNELS_IMPL_COMPILE_LIBRARY false

#include <algorithm>   
#include <cstdlib>     
#include <type_traits>
#include <cassert>
#include <typeinfo> 
#include <iostream>

//Own includes
#include "utility.hpp" 

//External includes
#include "Kokkos_Core.hpp"
#include "KokkosSparse_CrsMatrix.hpp"
#include "KokkosSparse_spgemm.hpp"
#include "KokkosSparse_spadd.hpp"

namespace pairg
{
  template<typename TScalar = char,
           typename TOrdinal = int,
           typename TDevice = Kokkos::DefaultHostExecutionSpace::device_type>
  class matrixOps
  {
    public:

      //value type in matrices
      typedef TScalar scalar_t;

      //locus type (or coordinate type)
      typedef TOrdinal lno_t;

      //parallelization support requested from kokkos
      typedef TDevice Device;

      //matrix format
      typedef typename KokkosSparse::CrsMatrix<scalar_t, lno_t, Device> crsMat_t;

      typedef typename crsMat_t::StaticCrsGraphType graph_t;
      typedef typename crsMat_t::values_type::non_const_type scalar_view_t;
      typedef typename graph_t::row_map_type::non_const_type lno_view_t;
      typedef typename graph_t::entries_type::non_const_type lno_nnz_view_t;
      typedef typename lno_view_t::value_type size_type;

      //Kokkos's KernelHandle
      typedef KokkosKernels::Experimental::KokkosKernelsHandle
        <size_type, lno_t, scalar_t,
        typename Device::execution_space, typename Device::memory_space,typename Device::memory_space > KernelHandle;

      //for kokkos::parallel_for
      typedef Kokkos::RangePolicy<typename Device::execution_space, lno_t> range_type;

      /**
       * @brief     boolean addition of two matrices 
       * @details   using API from kokkos-kernels/unit_test/sparse/Test_Sparse_spadd.hpp
       * @return    C = A '+' B (nnz values in C are set as 1)
       */
      static crsMat_t addMatrices(const crsMat_t &A, const crsMat_t &B)
      {
        KernelHandle kh; 

        //Assume rows are not sorted
        kh.create_spadd_handle(false);

        const lno_t num_rows_A = A.numRows();
        [[maybe_unused]] const lno_t num_cols_A = A.numCols();
        [[maybe_unused]] const lno_t num_rows_B = B.numRows();
        [[maybe_unused]] const lno_t num_cols_B = B.numCols();

        assert(num_rows_A == num_rows_B);
        assert(num_cols_A == num_cols_B);

        lno_view_t row_map_C("C row map", num_rows_A + 1);
        lno_nnz_view_t  entries_C;
        scalar_view_t values_C;

        //Compute no. of nnz elements in C
        KokkosSparse::Experimental::spadd_symbolic (&kh,
             A.graph.row_map, A.graph.entries,
             B.graph.row_map, B.graph.entries, row_map_C);

        size_type max_result_nnz = kh.get_spadd_handle()->get_c_nnz();

        if (max_result_nnz) {
          entries_C = lno_nnz_view_t ("C entries", max_result_nnz);
          values_C = scalar_view_t ("C values", max_result_nnz);
        }
        else {
          std::cout << "WARNING, pairg::matrixOps::addMatrices, max_result_nnz == 0" << std::endl;
        }

        //Compute values in C
        KokkosSparse::Experimental::spadd_numeric (&kh, 
            A.graph.row_map, A.graph.entries, A.values, (int8_t)1,
            B.graph.row_map, B.graph.entries, B.values, (int8_t)1,
            row_map_C, entries_C, values_C);

        //reset nnz values in C to 1
        for(size_type i = 0; i < max_result_nnz; i++) {
          values_C(i) = 1;
        }

        kh.destroy_spadd_handle();

        return crsMat_t ("C", num_rows_A, num_rows_A, max_result_nnz, values_C, row_map_C, entries_C);
      }

      /**
       * @brief     boolean multiplication of two matrices 
       * @details   - modified from API specied in kokkos-kernels
       *              github.com/kokkos/kokkos-kernels/wiki/SPARSE-3::spgemm 
       *
       *            - non-zero values are reset to 1 in the returned matrix
       */
      static crsMat_t multiplyMatrices(const crsMat_t &A, const crsMat_t &B)
      {
        KernelHandle kh; 
        kh.set_team_work_size(16);
        kh.set_dynamic_scheduling(true);

        // Select an spgemm algorithm, limited by configuration at compile-time and set via the handle
        // Some options: {SPGEMM_KK_MEMORY, SPGEMM_KK_SPEED, SPGEMM_KK_MEMSPEED, */ SPGEMM_MKL}
        KokkosSparse::SPGEMMAlgorithm spgemm_algorithm = KokkosSparse::SPGEMM_KK_MEMORY;
        kh.create_spgemm_handle(spgemm_algorithm);

        const lno_t num_rows_A = A.numRows();
        [[maybe_unused]] const lno_t num_cols_A = A.numCols();
        const lno_t num_rows_B = B.numRows();
        const lno_t num_cols_B = B.numCols();

        assert(num_cols_A == num_rows_B);

        // Prepare resultant matrix
        lno_view_t row_map_C ("non_const_lnow_row", num_rows_A + 1);
        lno_nnz_view_t  entries_C;
        scalar_view_t values_C;

        pairg::timer T1;

        // Get count of nnz in matrix C
        KokkosSparse::Experimental::spgemm_symbolic (&kh, 
            num_rows_A, num_rows_B, num_cols_B,
            A.graph.row_map, A.graph.entries, false,
            B.graph.row_map, B.graph.entries, false,
            row_map_C
            );

        //std::cout << "INFO, pairg::matrixOps::multiplyMatrices, time to execute symbolic phase (ms): " << T1.elapsed() << "\n";


        size_type c_nnz_size = kh.get_spgemm_handle()->get_c_nnz();
        if (c_nnz_size) {
          entries_C = lno_nnz_view_t (Kokkos::ViewAllocateWithoutInitializing("entries_C"), c_nnz_size);
          values_C = scalar_view_t (Kokkos::ViewAllocateWithoutInitializing("values_C"), c_nnz_size);
        }
        else {
          std::cout << "WARNING, pairg::matrixOps::multiplyMatrices, c_nnz_size == 0" << std::endl;
        }

        pairg::timer T2;

        // Fill matrix multiplication values 
        KokkosSparse::Experimental::spgemm_numeric (&kh, 
            num_rows_A, num_rows_B, num_cols_B,
            A.graph.row_map, A.graph.entries, A.values, false,
            B.graph.row_map, B.graph.entries, B.values, false,
            row_map_C, entries_C, values_C
            );

        //std::cout << "INFO, pairg::matrixOps::multiplyMatrices, time to execute numeric phase (ms): " << T2.elapsed() << "\n";

        //reset nnz values in C to 1
        for(size_type i = 0; i < c_nnz_size; i++) {
          values_C(i) = 1;
        }

        kh.destroy_spgemm_handle();

        graph_t static_graph (entries_C, row_map_C);
        return crsMat_t("C", num_cols_B, values_C, static_graph);
      }

      /**
       * @brief   raise a square matrix to a power
       * @return  C = A raised by n
       */
      static crsMat_t power(const crsMat_t &A, unsigned int n)
      {
        [[maybe_unused]] const lno_t num_rows_A = A.numRows();
        [[maybe_unused]] const lno_t num_cols_A = A.numCols();

        assert(num_rows_A == num_cols_A);

        crsMat_t C = createIdentityMatrix(num_rows_A); // Initialize result

        crsMat_t A_copy = A;

        while (true) {
          // If n is odd, multiply x with result
          if (n & 1) 
            C = multiplyMatrices(C, A_copy);

          n = n >> 1;
          if (n <= 0) break;
          A_copy = multiplyMatrices(A_copy, A_copy);
        }

        return C;
      }

      /**
       * @brief                       query value at given coordinates in a given matrix
       * @note                        row and column indices should be 0-based
       *                              the matrix entries in each row are assumed sorted
       */
      static bool queryValue(const crsMat_t &A, lno_t i, lno_t j)
      {
        if (i >= A.numRows () || j >= A.numCols ()) {
          std::cout << "WARNING, pairg::matrixOps::queryValue, query index out of range" << std::endl;
          return false;
        }

        //find start and end range within entries array 
        size_type begin = A.graph.row_map(i);
        size_type end = A.graph.row_map(i + 1);

        return std::binary_search(A.graph.entries.data() + begin, A.graph.entries.data() + end, j);

        //linear search
        //auto it = std::find(A.graph.entries.data() + begin, A.graph.entries.data() + end, j);
        //return (it != (A.graph.entries.data() + end));
      }

      /**
       * @brief                       sort indices within each row, required for fast querying
       */
      static void indexForQuery(crsMat_t &A)
      {
        lno_t num_rows = A.numRows();
        [[maybe_unused]] size_type nnz = A.graph.entries.extent(0);

        //Functor to sort entries within each row
        auto sortEntries = [&](const lno_t i)
        {
          size_type begin = A.graph.row_map(i);
          size_type end = A.graph.row_map(i+1);
          std::sort(A.graph.entries.data() + begin, A.graph.entries.data() + end);
        };

        //Sort all rows in parallel
        Kokkos::parallel_for("pairg::matrixOps::indexForQuery", range_type(0, num_rows), sortEntries);
      }

      /**
       * @brief                       print matrix to stdout
       * @param[in]  verbose          1 - just print matrix properties
       *                              2 - print matrix properties and limited set of values
       *                              3 - print matrix properties and all values
       */
      static void printMatrix(const crsMat_t &A, int verbose)
      {
        std::cout << "INFO, pairg::matrixOps::printMatrix, row map size:" << A.graph.row_map.extent(0) << "\n";
        std::cout << "INFO, pairg::matrixOps::printMatrix, entries (nnz):" << A.graph.entries.extent(0) << "\n";
        std::cout << "INFO, pairg::matrixOps::printMatrix, values (nnz):" << A.values.extent(0) << "\n";

        if(verbose > 1) 
        {
          KokkosKernels::Impl::print_1Dview(A.graph.row_map, verbose > 2);
          KokkosKernels::Impl::print_1Dview(A.graph.entries, verbose > 2);
          KokkosKernels::Impl::print_1Dview(A.values, verbose > 2);
        }

        std::cout << "\n";
      }

      /**
       * @brief   a small utility function to print size (in bytes) of commonly 
       *          used types during matrix operations
       */
      static void printTypeSizes()
      {
        std::cout << "Printing out type sizes:" << "\n";
        std::cout << "scalar_t is: " << sizeof(scalar_t)  << "\n"; 
        std::cout << "size_type is: " << sizeof(size_type)  << "\n"; 
        std::cout << "lno_t is: " << sizeof(lno_t)  << "\n"; 
      }

      /**
       * @brief                       create a square identity matrix
       * @param[in] nrows             count of rows
       * @return                      the generated matrix
       */
      static crsMat_t createIdentityMatrix(lno_t nrows)
      {
        //first, populate rowmap
        lno_view_t rowmap("rowmap", nrows + 1);
        typename lno_view_t::HostMirror h_rowmap = Kokkos::create_mirror_view(rowmap);
        size_type nnz = 0;
        for(lno_t i = 0; i < nrows; i++)
        {
          size_type rowEntries = 1;   //one entry per row
          h_rowmap(i) = nnz;
          nnz += rowEntries;
        }
        h_rowmap(nrows) = nnz;
        Kokkos::deep_copy(rowmap, h_rowmap);

        //populate values
        scalar_view_t values("values", nnz);
        typename scalar_view_t::HostMirror h_values = Kokkos::create_mirror_view(values);
        for(size_type i = 0; i < nnz; i++) {
          h_values(i) = 1;
        }
        Kokkos::deep_copy(values, h_values);

        //populate entries 
        lno_nnz_view_t entries("entries", nnz);
        typename lno_nnz_view_t::HostMirror h_entries = Kokkos::create_mirror_view(entries);
        for(lno_t i = 0; i < nrows; i++) {
          h_entries(i) = i;
        }
        Kokkos::deep_copy(entries, h_entries);

        return crsMat_t("identity matrix", nrows, nrows, nnz, values, rowmap, entries);
      }

      /**
       * @brief                       create a random square matrix for testing, using kokkos-kernels
       * @param[in] nrows             count of rows
       * @param[in] minNNZ            minimum non-zero elements * per row *
       * @param[in] maxNNZ            maximum non-zero elements * per row *
       * @param[in] sortRows          sort indices within each row 
       * @return                      the generated matrix
       * @details                     - value of each non-zero element is set to 1
       *                              - not suitable for very large matrices as the randomization procedure is expensive
       *                              - modified from kokkos-kernels repo: unit_test/sparse/Test_Sparse_spadd.hpp
       */
      template<class URBG=std::mt19937>
      static crsMat_t createRandomMatrix(lno_t nrows, lno_t minNNZ, lno_t maxNNZ, bool sortRows,
                                         URBG&& g={std::random_device()})
      {
        //first, populate rowmap
        lno_view_t rowmap("rowmap", nrows + 1);
        typename lno_view_t::HostMirror h_rowmap = Kokkos::create_mirror_view(rowmap);
        size_type nnz = 0;
        for(lno_t i = 0; i < nrows; i++)
        {
          size_type rowEntries = rand() % (maxNNZ - minNNZ + 1) + minNNZ;
          h_rowmap(i) = nnz;
          nnz += rowEntries;
        }
        h_rowmap(nrows) = nnz;
        Kokkos::deep_copy(rowmap, h_rowmap);

        //allocate values and entries
        scalar_view_t values("values", nnz);
        //populate values
        typename scalar_view_t::HostMirror h_values = Kokkos::create_mirror_view(values);
        for(size_type i = 0; i < nnz; i++)
        {
          h_values(i) = 1;
        }
        Kokkos::deep_copy(values, h_values);

        //populate entries (make sure no entry is repeated within a row)
        lno_nnz_view_t entries("entries", nnz);
        typename lno_nnz_view_t::HostMirror h_entries = Kokkos::create_mirror_view(entries);
        std::vector<lno_t> indices(nrows);
        for(lno_t i = 0; i < nrows; i++)
        {
          for(lno_t j = 0; j < nrows; j++)
          {
            indices[j] = j;
          }
          std::shuffle(indices.begin(), indices.end(), g);
          size_type rowStart = h_rowmap(i);
          size_type rowCount = h_rowmap(i + 1) - rowStart;
          if(sortRows)
          {
            std::sort(indices.begin(), indices.begin() + rowCount);
          }
          for(size_type j = 0; j < rowCount; j++)
          {
            h_entries(rowStart + j) = indices[j];
          }
        }
        Kokkos::deep_copy(entries, h_entries);

        return crsMat_t("test matrix", nrows, nrows, nnz, values, rowmap, entries);
      }
  };
}

#endif
