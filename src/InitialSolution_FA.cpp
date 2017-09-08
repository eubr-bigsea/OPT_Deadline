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

#include "InitialSolution_FA.hpp"
#include <vector>

void InitialSolution_FA::process(Process* process_to_init, std::ostream* log) {
  *log << "InitialSolution_FA::process > Starting initialization\n";

  // Get the number of application in the process
  const auto number_of_applications =
      process_to_init->get_number_applications();

  TimeInstant total_avg_all_apps = 0;

  // Initialize zeros total avg per application
  std::vector<TimeInstant> total_avg_per_app;
  total_avg_per_app.resize(number_of_applications, 0);

  // For all applications in the process
  for (unsigned index_app = 0; index_app < number_of_applications;
       ++index_app) {
    // Get index-app-th application
    const auto& application =
        process_to_init->get_application_from_index(index_app);

    // Get all stages in the application
    const auto& stages = application.get_all_stages();

    TimeInstant local_total_avg_this_app = 0;
    // For all stages
    for (const auto& pair_stage : stages) {
      const auto& stage = pair_stage.second;
      local_total_avg_this_app += stage.get_avg_time();
      total_avg_all_apps += stage.get_avg_time();
    }  // for all stages

    total_avg_per_app[index_app] = local_total_avg_this_app;
  }  // for all apps

  // Get D process
  const auto process_deadline = process_to_init->get_total_deadline();

  // For each app set its initial deadline
  for (unsigned index_app = 0; index_app < number_of_applications;
       ++index_app) {
    // Get index-app-th application
    auto& application =
        process_to_init->get_application_from_index_mod(index_app);

    // Formula is = D_tot * total_avg_ith / total_avg
    double new_deadline_fp = process_deadline * total_avg_per_app[index_app] /
                             static_cast<double>(total_avg_all_apps);

    // Convert into unsigned long (timestamp)
    TimeInstant new_deadline = static_cast<TimeInstant>(new_deadline_fp);

    *log << "\tApp index (" << index_app
         << ") setting initial deadline: " << new_deadline << "\n";

    application.set_deadline(new_deadline);
  }  // for all apps

  *log << "InitialSolution_FA::process > Initialization completed\n";
}
