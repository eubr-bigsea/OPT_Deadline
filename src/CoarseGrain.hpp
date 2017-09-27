/*
Copyright 2017 Biagio Festa <info@biagiofesta.it>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef __OPT_DEADLINE__COARSE_GRAIN__HPP
#define __OPT_DEADLINE__COARSE_GRAIN__HPP

#include <ostream>
#include "Process.hpp"

class CoarseGrain {
 public:
  using TimeInstant = opt_common::TimeInstant;
  using Application = opt_common::Application;
  using AppNCore = std::pair<const Application*, unsigned>;

  void process(Process* process, std::ostream* log);

 private:
  static constexpr unsigned MAX_NUMBER_OF_ITERATION = 1000;

  //! A possible solution in shifting the deadline among a couple of application
  struct PossibleDeadlineShift {
    double m_delta_deadline;       // The delta deadline
    Application* m_app_reduce;     // Application to reduce deadline
    Application* m_app_increment;  // Application to increase deadline
    double m_evaluation_cost;
    double m_new_deadline_app_reduce;
    double m_new_deadline_app_increment;
    int m_new_num_cores_app_reduce;
    int m_new_num_cores_app_increment;
  };

  //! Initialize the delta deadline
  static double initialize_delta_deadline(const Process& process);

  //! \return the cost associated with those application and number of cores for
  //! they
  static double objective_function(const AppNCore& app1, const AppNCore& app2);

  //! Applying formula, return the number of cores (in double) for an
  //! application,
  //! given the deadline
  static double compute_number_of_cores_from_deadline(
      const Application& app, const TimeInstant& deadline);

  //! \return 'true' if the iterative algorithm should be stopped
  inline bool stop_criteria(unsigned num_tot_iteration) const;

  //! Reduce deadline for app_reduce and increment deadline for app_increment
  //! The amount of deadline reduces is in delta_deadline
  //! \param [in] app_reduce     The app to reduce deadline
  //! \param [in] app_increment  The app to increase deadline
  //! \param [in] delta_deadline The amount of increment/reduction
  //! \param [out] out_solution  The output of solution in shifting
  static bool shift_deadline(Application* app_reduce,
                             Application* app_increment,
                             const double delta_deadline,
                             PossibleDeadlineShift* out_solution);
};

inline bool CoarseGrain::stop_criteria(unsigned num_tot_iteration) const {
  // TODO(biagio): maybe you want to specify a better stop criteria
  return num_tot_iteration > MAX_NUMBER_OF_ITERATION;
}

#endif  // __OPT_DEADLINE__COARSE_GRAIN__HPP
