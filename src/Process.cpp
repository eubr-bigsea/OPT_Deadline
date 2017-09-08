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
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
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
  // TODO(biagio) why change cores?!?
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

Process Process::create_process(const std::string& data_input_namefile,
                                const std::string& config_namefile) {
  std::ifstream ifs{data_input_namefile};
  if (ifs.fail()) {
    THROW_RUNTIME_ERROR("Impossible open the file '" + data_input_namefile +
                        "'");
  }

  Process process;
  process.m_config_namefile = config_namefile;

  std::string line;
  while (std::getline(ifs, line)) {
    // Skip empty line and if starts with dash
    if (line.empty() == false && line.at(0) != '#') {
      std::istringstream iss{line};
      Application::FileResources resources_filename;
      std::string weight_str;
      iss >> resources_filename.m_Application_File;
      iss >> resources_filename.m_Jobs_File;
      iss >> resources_filename.m_Stages_File;
      iss >> resources_filename.m_Tasks_File;
      iss >> resources_filename.m_Lua_File;
      iss >> resources_filename.m_Infrastructure_File;
      iss >> weight_str;

      auto application = Application::create_application(resources_filename,
                                                         config_namefile, "0");
      application.set_weight(std::stod(weight_str));

      process.push_application(std::move(application));
    }
  }

  return process;
}
