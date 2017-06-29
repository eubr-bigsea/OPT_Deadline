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

#ifndef Process_hpp
#define Process_hpp

#include <memory>
#include <opt_common/Application.hpp>
#include <utility>

class Process {
 public:
  using Application = opt_common::Application;
  using TimeInstant = opt_common::TimeInstant;

  Process() = default;

  Process(opt_common::TimeInstant d) : d_line_tot(d) {}

  const Application& operator[](unsigned index) const {
    return applications.at(index);
  }

  Application& get_application_from_index(unsigned index);
  unsigned get_number_applications() const noexcept {
    return applications.size();
  }
  TimeInstant get_total_deadline() const noexcept { return d_line_tot; }

  TimeInstant total_real_time() const;
  TimeInstant min_d_line();

  void push_application(Application app);

  // divide the deadline among the applications using the given order
  bool distribute_d_line();

  // find the correct applications to add or remove cores
  std::pair<unsigned int, unsigned int> delta(TimeInstant D);

  // evaluate the old cost and the new one if the number of cores of the
  // applications
  // found by the delta method is set
  std::pair<double, double> evaluate_costs(unsigned int max,
                                           unsigned int min) const;

  // set the number of cores
  void reset_core_dline(unsigned int max, unsigned int min, TimeInstant D);

  double total_cost() const;

 private:
  std::vector<Application> applications;
  TimeInstant d_line_tot;
};

#endif
