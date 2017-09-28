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

#ifndef __OPT_DEADLINE__ALGORITHM_2__HPP
#define __OPT_DEADLINE__ALGORITHM_2__HPP

#include <ostream>
#include "Process.hpp"

class Algorithm2 {
 public:
  using Configuration = opt_common::Configuration;

  bool process(const Configuration& configuration, Process* process,
               std::ostream* log, std::ostream* result_log);
};

#endif  // __OPT_DEADLINE__ALGORITHM_2__HPP
