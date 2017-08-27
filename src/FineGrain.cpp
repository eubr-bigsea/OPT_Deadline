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

FineGrain::FineGrain(const Configuration& configuration)
    : m_optIC_command(configuration.get_opt_command()),
      m_dagSim_command(configuration.get_dagsim_path() + "/" + DAGSIM_SH),
      m_tmp_directory((configuration.get_tmp_directory().empty()
                           ? DEFAULT_TMP
                           : configuration.get_tmp_directory())) {}

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
  std::string temp_filename = m_tmp_directory + "/app_" +
                              application.get_application_id() + "_" +
                              generate_random_string();

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
  std::string dagSim_result;

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
    dagSim_result = invoke_dagSim(application, num_cores);

#ifndef NDEBUG
    // Print output of execution OPT_IC
    *log << "########################################\n"
         << dagSim_result << "########################################\n";
#endif

    // TODO(biagio) to continue
    assert(false);  // to implement...
  }                 // for all applications
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

std::string FineGrain::invoke_dagSim(const Application& application,
                                     int num_cores_to_evaluate) const {
  static constexpr const std::size_t SIZE_BUFFER = 1024;

  // Lua template filename absolute
  const std::string& lua_template_filename = application.get_lua_name();

  // Generate temporary LUA from template inserting num_cores
  std::string lua_mod_filename =
      create_temporary_lua_file(lua_template_filename, num_cores_to_evaluate);

  // Create the complete command to invoke
  /*
  const std::string cmd = m_dagSim_command + " " + lua_mod_filename +
                          " 2>&1 | sed -n 1,1p | awk '{print $3}'";
  */
  const std::string cmd = m_dagSim_command + " " + lua_mod_filename + " 2>&1";

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

std::string FineGrain::create_temporary_lua_file(
    const std::string& abs_lua_filename, const int num_cores_to_write) const {
  static constexpr const char* TO_FIND = "Nodes = @@nodes@@;";
  // Get a unique temporary filename
  std::string temp_filename =
      m_tmp_directory + "/lua_" + generate_random_string() + ".lua";

  // Open the input LUA template file
  std::ifstream input_file;
  input_file.open(abs_lua_filename);
  if (input_file.fail()) {
    THROW_RUNTIME_ERROR("Cannot open the input LUA file '" + abs_lua_filename +
                        "'");
  }

  std::string input_file_str;

  // Reserve buffer into string (memory optimization)
  input_file.seekg(0, std::ios::end);
  input_file_str.reserve(input_file.tellg());
  input_file.seekg(0, std::ios::beg);

  // Read the entire input file into string
  input_file_str.assign(std::istreambuf_iterator<char>(input_file),
                        std::istreambuf_iterator<char>());

  // Close input file
  input_file.close();

  // Mod the template and substitute the nodes string
  const auto finder = input_file_str.find(TO_FIND);
  if (finder == std::string::npos) {
    THROW_RUNTIME_ERROR("Cannot find the template line '" +
                        std::string(TO_FIND) +
                        "' into the LUA template file '" + abs_lua_filename +
                        "'. Are you sure this is a LUA template?");
  }

  input_file_str = input_file_str.replace(
      finder, std::strlen(TO_FIND),
      "Nodes = " + std::to_string(num_cores_to_write) + ";");

  // Open the output LUA file
  std::ofstream output_file;
  output_file.open(temp_filename);
  if (output_file.fail()) {
    THROW_RUNTIME_ERROR("Cannot open the input LUA file '" + temp_filename +
                        "'");
  }

  output_file << input_file_str;

  return temp_filename;
}
