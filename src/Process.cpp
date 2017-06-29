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

#include "Process.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

const Process::Application& Process::get_application_from_index(
    unsigned index) const {
  return m_applications.at(index);
}

Process::Application& Process::get_application_from_index_mod(unsigned index) {
  return m_applications.at(index);
}

void Process::push_application(opt_common::Application app) {
  app.set_alpha_beta(15, 10);
  m_applications.push_back(std::move(app));
}

Process::TimeInstant Process::compute_min_deadline() {
  set_cores_applications();
  return compute_total_real_time();
}

Process::TimeInstant Process::compute_total_real_time() const {
  TimeInstant sum = 0;
  for (const auto& app : m_applications) {
    sum += app.compute_avg_execution_time();
  }
  return sum;
}

void Process::set_cores_applications() {
  for (auto& app : m_applications) {
    app.set_number_of_core(app.compute_max_number_of_task());
  }
}
