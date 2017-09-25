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

#include "InitialSolution_SA.hpp"
#include <cmath>
#include <vector>

void InitialSolution_SA::process(Process* process_to_init, std::ostream* log) {
  *log << "InitialSolution_SA::process > Starting initialization\n";

  // The index of the app of comparisons
  static constexpr unsigned INDEX_APP_REF = 0;

  // Get the number of application in the process
  const auto number_of_applications =
      process_to_init->get_number_applications();

  // Are there enoght applications in the process?
  assert(number_of_applications > INDEX_APP_REF);

  std::vector<double> alpha_apps;
  alpha_apps.resize(number_of_applications);

  // Get reference application
  const auto& ref_application =
      process_to_init->get_application_from_index(INDEX_APP_REF);

  // For all applications (computation of alphas)
  for (unsigned index_app = 0; index_app < number_of_applications;
       ++index_app) {
    const auto& application =
        process_to_init->get_application_from_index(index_app);

    // Compute the alpha for that application
    alpha_apps[index_app] =
        index_app == INDEX_APP_REF
            ? 1.0
            : compute_alpha_app(application, ref_application);
  }  // for all apps

  // Computation n1
  const double n1 = compute_n1(alpha_apps, *process_to_init);

  // For all applications (computation of initial deadline)
  for (unsigned index_app = 0; index_app < number_of_applications;
       ++index_app) {
    auto& application =
        process_to_init->get_application_from_index_mod(index_app);

    // Get n for that app
    const double n_app = index_app == INDEX_APP_REF
                             ? n1
                             : compute_n_app(index_app, alpha_apps, n1);

    if (n_app <= 0.0) {
      THROW_RUNTIME_ERROR("The number of cores (n) per application index " +
                          std::to_string(index_app) +
                          " (id_app: " + application.get_application_id() +
                          ") has been exstimed with a value of " +
                          std::to_string(n_app) +
                          " <= 0.0. This error is probably due because the "
                          "input deadline provided as input for the process is "
                          "too small. The problem is, thus, unfeasible.");
    }

    // Compute initial deadline for application
    TimeInstant init_deadline = compute_deadline_app(application, n_app);

    *log << "\tAppliation index (" << index_app
         << ") init deadline: " << init_deadline << "\n";

    application.set_deadline(init_deadline);
  }  // for all apps

  *log << "InitialSolution_SA::process > Initialization completed\n";
}

double InitialSolution_SA::compute_alpha_app(
    const Application& app, const Application& reference) const {
  // Get weights applications
  const double w_r = reference.get_weight();
  const double w_a = app.get_weight();

  // Get MLM application
  const auto& mlm_r = reference.get_machine_learning_model();
  const auto& mlm_a = reference.get_machine_learning_model();

  // Get Chi C
  const double chiC_r = mlm_r.get_chi_c();
  const double chiC_a = mlm_a.get_chi_c();

  return std::sqrt((w_r * chiC_a) / (w_a * chiC_r));
}

double InitialSolution_SA::compute_n1(const std::vector<double>& alpha_apps,
                                      const Process& process) const {
  const auto number_of_applications = process.get_number_applications();

  double sum_chi_0 = 0.0;
  double num = 0.0;

  // For all apps in the process
  for (unsigned index_app = 0; index_app < number_of_applications;
       ++index_app) {
    // Get application and its machine learning model
    const auto& application = process.get_application_from_index(index_app);
    const auto& mlm_app = application.get_machine_learning_model();
    const double chiC_app = mlm_app.get_chi_c();
    const double chi0_app = mlm_app.get_chi_0();

    // Add to the sum of all chi_0
    sum_chi_0 += chi0_app;

    num += (chiC_app / alpha_apps.at(index_app));
  }  // for all apps

  // return formula
  return num / (process.get_total_deadline() - sum_chi_0);
}

double InitialSolution_SA::compute_n_app(const unsigned index_app,
                                         const std::vector<double>& alpha_apps,
                                         const double n1) const {
  return alpha_apps.at(index_app) * n1;
}

auto InitialSolution_SA::compute_deadline_app(const Application& app,
                                              double n) const -> TimeInstant {
  const auto& mlm = app.get_machine_learning_model();
  const double chi_0 = mlm.get_chi_0();
  const double chi_c = mlm.get_chi_c();
  const double deadline = chi_0 + chi_c / n;

  // Convert into integer
  return static_cast<TimeInstant>(deadline);
}
