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

#include <opt_common/Application.hpp>
#include <opt_common/helper.hpp>
#include <ostream>

class Process {
 public:
  using Application = opt_common::Application;
  using TimeInstant = opt_common::TimeInstant;

  Process() = default;

  static Process create_process(const std::string& data_input_namefile,
                                const std::string& config_namefile,
                                TimeInstant total_deadline_process);

  void dump_process(std::ostream* out, const std::string& additional_message) const;

  const Application& get_application_from_index(unsigned index) const;
  Application& get_application_from_index_mod(unsigned index);

  unsigned get_number_applications() const noexcept {
    return m_applications.size();
  }

  TimeInstant get_total_deadline() const noexcept {
    assert(m_total_deadline != 0);
    return m_total_deadline;
  }

  void set_total_deadline(const TimeInstant& total_deadline) noexcept {
    m_total_deadline = total_deadline;
  }

  void push_application(Application app);

  TimeInstant compute_min_deadline();

  const std::string& get_config_filename() const noexcept {
    return m_config_namefile;
  }

  double compute_global_objective_function() const noexcept {
    double of = 0.0;
    for (const auto& app : m_applications) {
      of += app.get_weight() * app.get_number_of_core();
    }
    return of;
  }

 private:
  std::vector<Application> m_applications;
  TimeInstant m_total_deadline = 0;
  std::string m_config_namefile;

  TimeInstant compute_total_real_time() const;

  void set_cores_applications();
};

#endif
