/**
 * @file    utility.hpp
 * @author  Chirag Jain <cjain7@gatech.edu>
 */

#ifndef PAIRG_UTILITY_HPP
#define PAIRG_UTILITY_HPP

#include <chrono>

namespace pairg
{
  /**
   * @brief         class for convenient time measurements of code using STL,
   *                borrowed from mxx library github.com/patflick/mxx
   */
  template <typename duration>
    class timer_impl
    {
      protected:
        //use the monotonic `steady_clock` for time measurements of code
        std::chrono::steady_clock::time_point start;

      public:
        // constructor (begin timing)
        timer_impl() 
        {
          start = std::chrono::steady_clock::now();
        }

        // returns time elapsed since creation
        typename duration::rep elapsed() const 
        {
          std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
          typename duration::rep elapsed_time = duration(stop-start).count();
          return elapsed_time;
        }
    };

  //pairg::timer, specialization for measuring milliseconds in double precision
  using timer = timer_impl<std::chrono::duration<double, std::milli> >;
}

#endif
