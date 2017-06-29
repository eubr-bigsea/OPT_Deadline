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

#include "CoarseGrain.hpp"
#include <algorithm>
#include <string>
#include <utility>

CoarseGrain::CoarseGrain(TimeInstant deadline) {
  d = deadline;
  D = deadline / TimeInstant(100);
}

void CoarseGrain::min(std::ofstream &ofs, Process &p, std::string argv) {
  while (D > tol && it <= max_it && diff1 >= 1 && diff2 >= 1) {
    ofs << "Now the coarse grain method begins. Given a delta, we want to "
           "select the application that \r\n provides the minimum cost if we "
           "add cores and the one that \r\n provides the maximum saving if we "
           "remove cores. \r\n \r\n";

    std::pair<unsigned int, unsigned int> maxmin = p.delta(D);

    diff1 = p[maxmin.first].get_number_of_core() - p[maxmin.first].get_n1();
    diff2 = p[maxmin.second].get_n2() - p[maxmin.second].get_number_of_core();

    std::pair<double, double> costs =
        p.evaluate_costs(maxmin.first, maxmin.second);
    ofs << "Using delta = " << D << " application " << maxmin.first
        << " implies a saving of "
        << p[maxmin.first].get_weight() *
               (p[maxmin.first].get_number_of_core() - p[maxmin.first].get_n1())
        << "; \r\n";
    ofs << "application " << maxmin.first << " implies a saving of "
        << p[maxmin.second].get_weight() *
               (p[maxmin.second].get_n1() -
                p[maxmin.second].get_number_of_core())
        << ".\r\n";

    // costs = (old_cost, new_cost)
    ofs << "Old total cost " << costs.first << "\r\n";
    ofs << "New total cost " << costs.second << "\r\n";

    // if we can save something, do that
    if (costs.first > costs.second) {
      ofs << "Since the new cost is less than the old one, we proceed with the "
             "delta method, considering every time half of the previous delta. "
             "\r\n \r\n";

      while (costs.first > costs.second && p.total_real_time() < d) {
        p.reset_core_dline(maxmin.first, maxmin.second, D);
        D = D / 2;
        maxmin = p.delta(D);
        costs = p.evaluate_costs(maxmin.first, maxmin.second);
        ofs << "Using delta = " << D << " application " << maxmin.first
            << " implies a saving of "
            << p[maxmin.first].get_weight() *
                   (p[maxmin.first].get_number_of_core() -
                    p[maxmin.first].get_n1())
            << "; \r\n";
        ofs << "while application " << maxmin.first << " implies a saving of "
            << p[maxmin.second].get_weight() *
                   (p[maxmin.second].get_n1() -
                    p[maxmin.second].get_number_of_core())
            << ".\r\n";
        ofs << "Old total cost " << costs.first << "\r\n";
        ofs << "New total cost " << costs.second << "\r\n \r\n";
      }

      ofs << "After coarse grain method the total cost is " << p.total_cost()
          << "\r\n \r\n";

      ofs << "Total deadline: " << d
          << " and total_real_time: " << p.total_real_time() << "\r\n";

      for (unsigned int k = 0; k < p.get_number_applications(); k++)
        ofs << "Application " << k + 1
            << " nr. of cores: " << p[k].get_number_of_core() << "\r\n";

      ofs << "\r\n";
      ofs << "Now we start the fine grain search with the " << argv
          << " method. \r\n \r\n";
      if (argv == "weights")
        fine_grain = new FineGrainWeights();
      else
        fine_grain = new FineGrainDerivative();

      fine_grain->set_weights(p, ofs);
      time_point start = std::chrono::system_clock::now();
      fine_grain->find_min(p, ofs);
      time_point finish = std::chrono::system_clock::now();
      help::time_consumption(start, finish, ofs);

      delete fine_grain;
      break;  // if we already have added ore removed just one core at a time,
              // do not use delta/2
    } else {
      D = D / 2;
    }

    it++;
  }  // while
}
