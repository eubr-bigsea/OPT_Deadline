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

Process::Application& Process::get_application_from_index(unsigned index) {
  return applications.at(index);
}

void Process::push_application(opt_common::Application app) {
  app.set_alpha_beta(15, 10);
  applications.push_back(std::move(app));
}

Process::TimeInstant Process::total_real_time() const {
  TimeInstant sum = 0;
  for (const auto& app : applications) {
    sum += app.compute_avg_execution_time();
  }
  return sum;
}

Process::TimeInstant Process::min_d_line() {
  for (auto& app : applications) {
    app.set_number_of_core(app.compute_max_number_of_task());
  }

  return total_real_time();
}

bool Process::distribute_d_line() {
  bool control = true;

  if (d_line_tot <= min_d_line()) {
    control = false;
  } else {
    TimeInstant sum = 0;
    std::vector<TimeInstant> temps;

    int i = 1;
    for (auto& app : applications) {
      TimeInstant temp = 0;
      for (const auto& stage : app.get_all_stages()) {
        temp +=
            stage.second.get_avg_time() * stage.second.get_number_of_tasks();
      }
      sum += temp;
      temps.push_back(temp);
      // std::cout << "Application " << i << " takes " << temp << "
      // milliseconds" << std::endl;
      i++;
    }

    for (unsigned i = 0; i < applications.size(); i++) {
      applications[i].set_d_line(d_line_tot * (static_cast<float>(temps[i]) /
                                               static_cast<float>(sum)));
      // std::cout << "Application " << i << " : " <<
      // applications[i]->get_d_line() << std::endl;
    }
  }
  return control;
}

std::pair<unsigned int, unsigned int> Process::delta(TimeInstant D) {
  std::vector<double> f1, f2;

  for (unsigned int i = 0; i < applications.size(); i++) {
    applications[i].set_n1(D);
    applications[i].set_n2(D);
    // std::cout << "Application " << i+1 << " n1: " <<
    // applications[i]->get_n1() << " n2: " << applications[i]->get_n2() <<
    // std::endl;
    f1.push_back(applications[i].get_weight() *
                 static_cast<double>(applications[i].get_number_of_core() -
                                     applications[i].get_n1()));
    f2.push_back(applications[i].get_weight() *
                 static_cast<double>(-applications[i].get_number_of_core() +
                                     applications[i].get_n2()));
  }

  // std::cout << "Saves with more time :\n";
  // for (auto x:f1)
  //   std::cout << x << std::endl;
  //
  // std::cout << "Costs with less time :\n";
  // for (auto x:f2)
  //   std::cout << x << std::endl;
  unsigned int max = opt_common::compute_max_get_index(f1);
  unsigned int min = opt_common::compute_min_get_index(f2);

  if (max == min) {
    std::vector<double> f1_copy(f1);
    unsigned int k = 0;
    for (auto it = f1_copy.begin(); it != f1_copy.end(); it++) {
      if (k == max) {
        it = f1_copy.erase(it);
        return std::make_pair(opt_common::compute_max_get_index(f1_copy), min);
      }
      k++;
    }
  }
  // std::cout << "Max: " << max << " Min: " << min << std::endl;

  return std::make_pair(max, min);
}

std::pair<double, double> Process::evaluate_costs(unsigned int max,
                                                  unsigned int min) const {
  double old_cost = 0, new_cost = 0;
  for (unsigned int i = 0; i < applications.size(); i++) {
    old_cost += applications[i].get_weight() *
                static_cast<double>(applications[i].get_number_of_core());
    if (i == max) {
      new_cost += applications[i].get_weight() *
                  static_cast<double>(applications[i].get_n1());
    } else {
      if (i == min)
        new_cost += applications[i].get_weight() *
                    static_cast<double>(applications[i].get_n2());
      else
        new_cost += applications[i].get_weight() *
                    static_cast<double>(applications[i].get_number_of_core());
    }
  }
  return std::make_pair(old_cost, new_cost);
}

void Process::reset_core_dline(unsigned int max, unsigned int min,
                               TimeInstant D) {
  if (applications[max].get_n1() > 0 && applications[min].get_n2()) {
    applications[max].set_number_of_core(applications[max].get_n1());
    applications[max].set_d_line(applications[max].get_d_line() + D);
    applications[min].set_number_of_core(applications[min].get_n2());
    applications[min].set_d_line(applications[min].get_d_line() - D);
  }
}

double Process::total_cost() const {
  double sum = 0;

  for (unsigned int i = 1; i < applications.size() + 1; i++) {
    sum += applications[i - 1].get_weight() *
           static_cast<double>(applications[i - 1].get_number_of_core());
  }

  return sum;
}
