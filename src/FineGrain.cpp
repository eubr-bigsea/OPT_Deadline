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
#include <list>
#include <memory>
#include <queue>
#include <set>
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
                                    const std::string& config_filename,
                                    std::ostream* log) const {
  static constexpr std::size_t SIZE_BUFFER = 1024;

  // Generate the input file for OPT_IC for this application
  const auto input_file_application = gen_temporary_input_file(application);

  // Create the complete command to invoke
  const std::string cmd = m_optIC_command + " " + input_file_application +
                          " -f -c " + config_filename + " 2>&1";

  *log << "\tOptIC Invoke cmd: " << cmd << '\n';

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

void FineGrain::process(Process* process, std::ostream* log,
                        std::ostream* result_log) {
  *log << "FineGrain::process > Starting process\n";

  // Get number of application in the process
  const auto number_of_applications = process->get_number_applications();

  // Useful data structure
  using IndexApplication = std::size_t;
  using CloseList = std::set<IndexApplication>;

  std::string opt_IC_result;
  std::string dagSim_result;

  std::vector<int> coresFromOptIC_perApp;
  std::vector<int> residualTime_perApp;
  TimeInstant total_residual_time = 0;

  // For all applications in the process
  for (IndexApplication i = 0; i < number_of_applications; ++i) {
    *log << "\t> Analysis application n. " << i << '\n';
    // Get i-th application
    Application& application = process->get_application_from_index_mod(i);

    // Invoke OPT_IC with the deadline in application object and same
    // configuration file of OPT_Deadline
    opt_IC_result =
        invoke_optIC(application, process->get_config_filename(), log);

#ifndef NDEBUG
    // Print output of execution OPT_IC
    *log << "########### OUTPUT_OPT_IC ##############\n"
         << opt_IC_result << "########################################\n";
#endif

    // Get the number of cores stimed by OPT_IC
    const int num_cores =
        get_number_of_cores_from_optIC_output(opt_IC_result, application);
    *log << "\t> Number of cores: " << num_cores << '\n';

    // Store the number of cores in the vector (i-th position)
    coresFromOptIC_perApp.push_back(num_cores);
    application.set_number_of_core(num_cores);

    // now you have to call dagsim with 'num_cores' information
    // and get the execution time
    dagSim_result = invoke_dagSim(application, num_cores, log);

#ifndef NDEBUG
    // Print output of execution dagSim
    *log << "########### OUTPUT_DAGSIM ##############\n"
         << dagSim_result << "########################################\n";
#endif

    // Get execution time parsing output dagsim
    const TimeInstant execution_time =
        get_execution_time_from_dagSim_output(dagSim_result);
    *log << "\t> Execution time: " << execution_time << '\n';

    // Get residual time
    const auto residual_time = application.get_deadline() - execution_time;
    *log << "\t> Current Deadline Application: " << application.get_deadline()
         << '\n';
    *log << "\t> Residual Time: " << residual_time << '\n';
    residualTime_perApp.push_back(residual_time);

    // Add to the total residual time
    total_residual_time += residual_time;
    *log << "\t> Updated Total residual Time: " << total_residual_time << '\n';
  }  // for all applications
  process->dump_process(result_log, "Initial Solution SA");

  // Apps to not cosider any more
  CloseList apps_to_remove;

  // Iteration in the while loop
  unsigned iteration_index = 0;

  // Until no all applications have been removed
  while (apps_to_remove.size() < number_of_applications) {
    *log << "\t> Iteration Index: " << iteration_index << '\n';

    double best = 0;
    int best_new_n_cores;
    IndexApplication best_index;

    // For all applications
    for (IndexApplication i = 0; i < number_of_applications; ++i) {
      // Check if the application has not been removed
      if (apps_to_remove.find(i) == apps_to_remove.cend()) {
        *log << "\t> Considering Application Index: " << i << '\n';

        // Get application reference
        Application& application = process->get_application_from_index_mod(i);

        // Invoke OPT_IC with the deadline in application object and same
        // configuration file of OPT_Deadline
        // We need to temporary update deadline because invoke_opt method will
        // evaluate the internal deadline of application
        const auto saved_deadline = application.get_deadline();
        *log << "\t> Deadline input for OPT_IC (deadline + total_residual): "
             << saved_deadline + total_residual_time << '\n';
        application.set_deadline(saved_deadline + total_residual_time);
        opt_IC_result =
            invoke_optIC(application, process->get_config_filename(), log);

#ifndef NDEBUG
        // Print output of execution OPT_IC
        *log << "########### OUTPUT_OPT_IC ##############\n"
             << opt_IC_result << "########################################\n";
#endif

        // Restore previous deadline
        application.set_deadline(saved_deadline);

        // Get the number of cores stimed by OPT_IC
        const int new_num_cores =
            get_number_of_cores_from_optIC_output(opt_IC_result, application);

        if (new_num_cores < coresFromOptIC_perApp.at(i)) {
          const double evaluation =
              application.get_weight() *
              (new_num_cores - coresFromOptIC_perApp.at(i));

          if (evaluation < best) {
            *log << "\t\t> Found new best " << evaluation << '\n';
            best = evaluation;
            best_index = i;
            best_new_n_cores = new_num_cores;
          }
        } else {
          *log << "\t\t> Application removed\n";
          // Insert i-th app in the close list
          apps_to_remove.insert(i);
        }
      }  // If app is not in the close list
    }    // For all apps

    // If there is a best
    if (best < 0) {
      *log << "\t> New improvement found for application index: " << best_index
           << "\n";
      // Get the candidate application
      Application& application =
          process->get_application_from_index_mod(best_index);

      // Set the new best number of cores
      application.set_number_of_core(best_new_n_cores);

      // Call dagsim with new no. cores
      dagSim_result = invoke_dagSim(application, best_new_n_cores, log);

      // Get execution time parsing output dagsim
      const TimeInstant execution_time =
          get_execution_time_from_dagSim_output(dagSim_result);

      // Update total residual time
      *log << "\t> Total Residual (Before): " << total_residual_time << "\n";
      *log << "\t> Residual Time best prev iteration: "
           << residualTime_perApp.at(best_index) << "\n";
      *log << "\t> New Execution time DagSim: " << execution_time << "\n";
      *log << "\t> Deadline (before): " << application.get_deadline() << "\n";
      *log << "\t> New Residual Extimation: "
           << execution_time - application.get_deadline() << "\n";

      if (application.get_deadline() > execution_time) {
        THROW_RUNTIME_ERROR("New execution time is smaller than current deadline: error in residual time computation");
      }

      // Update total residual time
      total_residual_time -=
          std::abs(execution_time - application.get_deadline());

      // total = total - (new deadline - prev deadline)
      // new deadline deve essere pià grande

      // Update deadline application
      application.set_deadline(execution_time);

      *log << "\t> New deadline for application: " << execution_time << '\n';
      *log << "\t> New total residual time: " << total_residual_time << '\n';

      // Add application to the close set
      apps_to_remove.insert(best_index);

      *log << "\t> [Current Result] Iteration Index: " << iteration_index
           << "; Global Objective Function: "
           << process->compute_global_objective_function() << "; FineGrain\n";

      process->dump_process(result_log, "FineGrain");
    }

    ++iteration_index;
  }  // while all applications removed
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
                                     int num_cores_to_evaluate,
                                     std::ostream* log) const {
  static constexpr const std::size_t SIZE_BUFFER = 1024;

  // Lua template filename absolute
  const std::string& lua_template_filename = application.get_lua_name();

  // Generate temporary LUA from template inserting num_cores
  std::string lua_mod_filename =
      create_temporary_lua_file(lua_template_filename, num_cores_to_evaluate);

  // Create the complete command to invoke
  const std::string cmd =
      m_dagSim_command + " " + lua_mod_filename +
      " 2>&1 | sed -n 1,1p | awk '{print $3}' | tee result.txt";

  *log << "\tDagSim invoke cmd: " << cmd << '\n';

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

auto FineGrain::get_execution_time_from_dagSim_output(
    const std::string& dagsim_result) const -> TimeInstant {
  // I expect dagsim result is a string with just the time
  try {
    return std::stold(dagsim_result);
  } catch (const std::exception& error) {
    throw std::runtime_error(
        "Dagsim result is empty or bad-formed. Check Dagsim configuration and "
        "all data paths are correct.");
  }
}
