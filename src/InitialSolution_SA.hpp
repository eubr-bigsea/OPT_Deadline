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

#ifndef __OPT_DEADLINE__INITIAL_SOLUTION_SA__HPP
#define __OPT_DEADLINE__INITIAL_SOLUTION_SA__HPP

#include <ostream>
#include <vector>
#include "Process.hpp"

class InitialSolution_SA {
 public:
  using Application = opt_common::Application;
  using TimeInstant = opt_common::TimeInstant;
  void process(Process* process_to_init, std::ostream* log);

 private:
  double compute_alpha_app(const Application& app,
                           const Application& reference) const;

  double compute_n1(const std::vector<double>& alpha_apps,
                    const Process& process) const;

  double compute_n_app(const unsigned index_app,
                       const std::vector<double>& alpha_apps,
                       const double n1) const;

  TimeInstant compute_deadline_app(const Application& app, double n) const;
};

#endif  // __OPT_DEADLINE__INITIAL_SOLUTION_SA__HPP
