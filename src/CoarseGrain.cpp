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

#include "CoarseGrain.hpp"
#include <utility>
#include <vector>

double CoarseGrain::compute_number_of_cores_from_deadline(
    const Application& app, const TimeInstant& deadline) {
  // Get ML model parameters
  const auto& mlm = app.get_machine_learning_model();
  const double chi_0 = mlm.get_chi_0();
  const double chi_c = mlm.get_chi_c();

  // Compute number of cores with formula:
  //   n_cores = (deadline - chi_0) / chi_c
  return (static_cast<double>(deadline) - chi_0) / chi_c;
}

double CoarseGrain::objective_function(const AppNCore& app1,
                                       const AppNCore& app2) {
  const Application& app1_ref = *app1.first;
  const Application& app2_ref = *app2.first;
  const unsigned& num_cores_app1 = app1.second;
  const unsigned& num_cores_app2 = app2.second;

  double cost_app1 = app1_ref.get_weight() * num_cores_app1;
  double cost_app2 = app2_ref.get_weight() * num_cores_app2;

  return cost_app1 + cost_app2;
}

double CoarseGrain::initialize_delta_deadline(const Process& process) {
  // TOT_Deadline / number_app_in_process
  return process.get_total_deadline() / process.get_number_applications();
}

bool CoarseGrain::shift_deadline(Application* app_reduce,
                                 Application* app_increment,
                                 const double delta_deadline,
                                 PossibleDeadlineShift* out_solution) {
  // Set some preliminary information in the output
  out_solution->m_delta_deadline = delta_deadline;
  out_solution->m_app_reduce = app_reduce;
  out_solution->m_app_increment = app_increment;

  // Get the current deadline of two applications
  const auto deadline_appI = app_reduce->get_deadline();
  const auto deadline_appJ = app_increment->get_deadline();

  // Get the number of cores in accordance with those deadlines
  const unsigned ncoresI =
      compute_number_of_cores_from_deadline(*app_reduce, deadline_appI);
  const unsigned ncoresJ =
      compute_number_of_cores_from_deadline(*app_increment, deadline_appJ);

  // Check feasible solution
  if (ncoresI == 0 || ncoresJ == 0) {
    return false;
  }

  // Compute the new deadine: subtract from appI and increment appJ
  const double deadline_appI_new = deadline_appI - delta_deadline;
  const double deadline_appJ_new = deadline_appJ + delta_deadline;

  // Compute the new number of cores with new deadlines
  const unsigned ncoresI_new =
      compute_number_of_cores_from_deadline(*app_reduce, deadline_appI_new);
  const unsigned ncoresJ_new =
      compute_number_of_cores_from_deadline(*app_increment, deadline_appJ_new);

  // Compute difference in number of cores
  const int ncores_delta_I = ncoresI_new - ncoresI;
  const int ncores_delta_J = ncoresJ_new - ncoresJ;

  // Check feasible solution
  if (ncores_delta_I < 0 || ncores_delta_J < 0) {
    return false;  // infeasible solution
  }

  // Compute the evaluation after move deadlines
  const double evaluation = objective_function({app_reduce, ncoresI_new},
                                               {app_increment, ncoresJ_new});
  out_solution->m_evaluation_cost = evaluation;
  out_solution->m_new_deadline_app_reduce = deadline_appI_new;
  out_solution->m_new_deadline_app_increment = deadline_appJ_new;
  out_solution->m_new_num_cores_app_reduce = ncoresI_new;
  out_solution->m_new_num_cores_app_increment = ncoresJ_new;

  return true;
}

void CoarseGrain::process(Process* process, std::ostream* log) {
  *log << "CourseGrain::process > Starting process\n";

  // Get number of applications
  const auto num_of_apps = process->get_number_applications();

  if (num_of_apps == 0) {
    THROW_RUNTIME_ERROR("CoarseGrain: no applications to process");
  }

  // Initialize deadline
  double delta_deadline = initialize_delta_deadline(*process);

  // Local vector of possible solution
  std::vector<PossibleDeadlineShift> possible_solutions;

  // Local solution
  PossibleDeadlineShift possible_solution;

  // almeno una attuo e reitero sempre con quel delta lì
  // se è vuoto prendo delta mezza
  // max number iterazioni complessive
  // debug
  // magari deltamin     ???? 10 secondi

  unsigned iteration_index = 0;
  while (stop_criteria(iteration_index) == false) {
    *log << "\t> Iteration number: " << iteration_index << "\n";
    *log << "\t> DeltaDeadline: " << delta_deadline << "\n";

    // Clean the possible solutions
    possible_solutions.clear();

    // For each pair of Apps   ---  O(N^2)
    for (unsigned i = 0; i < num_of_apps; ++i) {
      for (unsigned j = 0; j < num_of_apps; ++j) {
        // Not reflexive!
        if (i != j) {
          *log << "\t\t> Considering Application Pair (" << i << ", " << j
               << ")\n";

          // Get references to applications
          Application& appI = process->get_application_from_index_mod(i);
          Application& appJ = process->get_application_from_index_mod(j);

          // Get current number of cores for appI and appJ
          const unsigned num_coresI = appI.get_number_of_core();
          const unsigned num_coresJ = appJ.get_number_of_core();

          // Evaluation cost before the shifting deadline
          const double evaluation_before =
              objective_function({&appI, num_coresI}, {&appJ, num_coresJ});

          *log << "\t\t\t> Evaluation before deadline movements: "
               << evaluation_before << "\n";

          // Shift deadline (reduce appI and increment appJ)
          // The function return true if solution is feasible (no negative
          // delta)
          if (shift_deadline(&appI, &appJ, delta_deadline,
                             &possible_solution)) {
            *log << "\t\t\t> Evaluation after deadline movements: "
                 << possible_solution.m_evaluation_cost << "\n";

            // If solution is better then save it in the vector of possible
            // solutions. Minimi. problem
            if (possible_solution.m_evaluation_cost < evaluation_before) {
              possible_solutions.push_back(std::move(possible_solution));
            }
          } else {
            // Negative dealta so discard this pair of solution
            *log << "\t\t\t> Shift Deadline has produced a negative number of "
                    "cores. Solution discarded\n";
          }
        }  // if i != j
      }    // for all app j
    }      // for all app i

    // If no better solution found then split deadline
    if (possible_solutions.empty()) {
      *log << "\t\t> No better solution. Decreasing DeltaDeadline\n";
      delta_deadline /= 2.0;
    } else {
      // If some solutions found, then apply the best one

      // Get the solution with minor cost
      const auto min_it = std::min_element(
          possible_solutions.cbegin(), possible_solutions.cend(),
          [](const PossibleDeadlineShift& s1, const PossibleDeadlineShift& s2) {
            return s1.m_evaluation_cost < s2.m_evaluation_cost;
          });

      // Apply the solution (increase and decrease deadlines and num_cores)
      auto* app_to_reduce = min_it->m_app_reduce;
      auto* app_to_increase = min_it->m_app_increment;
      const auto& deadline_reduce = min_it->m_new_deadline_app_reduce;
      const auto& deadline_increase = min_it->m_new_deadline_app_increment;
      const auto& cores_reduce = min_it->m_new_num_cores_app_reduce;
      const auto& cores_increase = min_it->m_new_num_cores_app_increment;

      *log << "\t\t> Solution Found. Applying...\n";
      *log << "\t\t\t> Application '" << app_to_increase->get_application_id()
           << "' increase new deadline: " << deadline_increase
           << " (before was: " << app_to_increase->get_deadline() << ")\n";
      *log << "\t\t\t> Application '" << app_to_reduce->get_application_id()
           << "' decrease new deadline: " << deadline_reduce
           << " (before was: " << app_to_reduce->get_deadline() << ")\n";
      app_to_reduce->set_deadline(deadline_reduce);
      app_to_increase->set_deadline(deadline_increase);
      app_to_reduce->set_number_of_core(cores_reduce);
      app_to_increase->set_number_of_core(cores_increase);

      *log << "\t> [Current Result] Iteration Index: " << iteration_index
           << "; Global Objective Function: "
           << process->compute_global_objective_function() << "; CoarseGrain\n";
    }

    ++iteration_index;
  }

  *log << "CourseGrain::process > End process\n";
}
