/*
Copyright 2017 Valeria Callioni
Copyright 2017 Giulia Landriani
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

#ifndef CoarseGrain_hpp
#define CoarseGrain_hpp

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <opt_common/Application.hpp>
#include <opt_common/Job.hpp>
#include <opt_common/Stage.hpp>
#include <opt_common/helper.hpp>
#include <sstream>
#include <string>
#include <vector>
#include "Process.hpp"

typedef std::chrono::milliseconds milli;
typedef std::chrono::time_point<std::chrono::system_clock> time_point;

class CoarseGrain {
 public:
  using TimeInstant = opt_common::TimeInstant;

  CoarseGrain(TimeInstant deadline);
  void min(std::ofstream &ofs, Process &p, std::string argv);

 private:
  TimeInstant d;  // deadline
  TimeInstant D;  // delta
  TimeInstant tol = 1000;
  unsigned int diff1 = 2, diff2 = 2;
  unsigned int it = 0;
  unsigned int max_it = 5;
  // FineGrain *fine_grain;  // TODO
};

#endif
