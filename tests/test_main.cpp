/**
 * @file    test_main.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

//External includes
#include <catch2/catch.hpp>

#include "Kokkos_Core.hpp"

class MyTestEventListener : public Catch::TestEventListenerBase {
public:
    using Catch::TestEventListenerBase::TestEventListenerBase;

    void testRunStarting( Catch::TestRunInfo const& ) override {
      /*
      // For debugging
      Kokkos::InitializationSettings args;
      args.set_num_threads( 1 );
      Kokkos::initialize( args );
      */
      Kokkos::initialize();
    }

    void testRunEnded( Catch::TestRunStats const& testRunStats ) override {
      Kokkos::finalize();
    }
};

CATCH_REGISTER_LISTENER(MyTestEventListener)

TEST_CASE( "1: All test cases reside in other .cpp files (empty)") {
}
