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

#include "FineGrain.hpp"
#include <stdio.h>
#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

std::string FineGrain::invoke_optIC(const Application& application,
                                    const std::string& config_filename) const {
  static constexpr std::size_t SIZE_BUFFER = 1024;

  // Generate the input file for OPT_IC for this application
  const auto input_file_application = gen_temporary_input_file(application);

  // Create the complete command to invoke
  const std::string cmd = m_optIC_command + " " + input_file_application +
                          " -f -c " + config_filename + " 2>&1";

  // Launch the process (wrapped in shared ptr for safe)
  std::shared_ptr<FILE> process_pipe(popen(cmd.c_str(), "r"), pclose);
  if (!process_pipe) {
    throw std::runtime_error(std::string("Error launch process '") + cmd + "'");
  }

  // Prepare buffer to read
  std::array<char, SIZE_BUFFER> buffer;
  std::string result_invoke;

  // Read the complete process
  while (!feof(process_pipe.get())) {
    if (fgets(buffer.data(), buffer.size(), process_pipe.get())) {
      result_invoke += buffer.data();
    }
  }

  return result_invoke;
}

std::string FineGrain::gen_temporary_input_file(
    const Application& application) const {
  static constexpr const char* TEMP_DIR = "/tmp";
  char temp_template[] = "XXXXXX";
  mkstemp(temp_template);

  std::string temp_filename = std::string(TEMP_DIR) + "/app_" +
                              application.get_application_id() + "_" +
                              temp_template;

  std::ofstream file;
  file.open(temp_filename.c_str());
  if (file.fail()) {
    THROW_RUNTIME_ERROR("Cannot open temporary file '" + temp_filename + "'");
  }

  const auto& files_app = application.get_files_resources();

  file << files_app.m_Application_File << ' ' << files_app.m_Jobs_File << ' '
       << files_app.m_Stages_File << ' ' << files_app.m_Tasks_File << ' '
       << files_app.m_Lua_File << ' ' << files_app.m_Infrastructure_File << ' '
       << std::to_string(application.get_deadline());

  file.close();

  return temp_filename;
}

void FineGrain::process(Process* process, std::ostream* log, char* argv[]) {
  *log << "FineGrain::process > Starting process\n";

  // Get number of application in the process
  const auto number_of_applications = process->get_number_applications();

  std::string opt_IC_result;

  std::vector<int> coresFromOptIC_perApp;

  for (unsigned i = 0; i < number_of_applications; ++i) {
    *log << "\t> Analysis application n. " << i << '\n';
    // Get i-th application
    const Application& application = process->get_application_from_index(i);

    // Invoke OPT_IC with the deadline in application object and same
    // configuration file of OPT_Deadline
    opt_IC_result = invoke_optIC(application,
                                 get_extented_path_config_file(*process, argv));

#ifndef NDEBUG
    // Print output of execution OPT_IC
    *log << "########################################\n"
         << opt_IC_result << "########################################\n";
#endif

    // Get the number of cores stimed by OPT_IC
    const int num_cores =
        get_number_of_cores_from_optIC_output(opt_IC_result, application);
    *log << "\t> Number of cores: " << num_cores << '\n';

    // Store the number of cores in the vector (i-th position)
    coresFromOptIC_perApp.push_back(num_cores);

    // TODO(biagio): now you have to call dagsim with 'num_cores' information
    // and get the execution time
    assert(false);  // to implement...
  }  // for all applications
}

int FineGrain::get_number_of_cores_from_optIC_output(
    const std::string& optIC_output, const Application& application) const {
  static constexpr const char* RELEVANT_ROW = "N YARN containers (VMs): ";
  const auto index = optIC_output.find(RELEVANT_ROW);
  if (index == std::string::npos) {
    THROW_RUNTIME_ERROR("Parsing error of OPT_IC output");
  }
  std::string num_vm_str =
      optIC_output.substr(index + std::strlen(RELEVANT_ROW));

  // trim space and \n
  num_vm_str = num_vm_str.substr(0, num_vm_str.find_first_of(" \n"));

  int num_vm = std::stoi(num_vm_str);

  // Get the infra. configuratio of app
  const auto& infr_conf = application.get_infrastructure_config();

  // TODO(biagio, danilo): verify the information is correct
  const int num_cores_per_vm = infr_conf.getContainter_cores();

  return num_vm * num_cores_per_vm;
}
