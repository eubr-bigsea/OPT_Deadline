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

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include "Algorithm1.hpp"
#include "Algorithm2.hpp"
#include "Process.hpp"

enum class AlgorithmSelection { ALGORITHM_1, ALGORITHM_2, ALGORITHM_12 };

AlgorithmSelection parse_algorithm_selection_from_cmd_line(
    const std::string& cmd_option) {
  if (cmd_option == "-1") {
    return AlgorithmSelection::ALGORITHM_1;
  }
  if (cmd_option == "-2") {
    return AlgorithmSelection::ALGORITHM_2;
  }
  if (cmd_option == "-12") {
    return AlgorithmSelection::ALGORITHM_12;
  }
  THROW_RUNTIME_ERROR("Option '" + cmd_option + "' not recognized");
}

std::string AlgorithmType2String(AlgorithmSelection algorithm_type) {
  switch (algorithm_type) {
    case AlgorithmSelection::ALGORITHM_1:
      return "Algorithm1";
    case AlgorithmSelection::ALGORITHM_2:
      return "Algorithm2";
    default:
      THROW_RUNTIME_ERROR("Algorithm type not recognized");
  }
}

opt_common::TimeInstant parse_total_deadline_process(
    const std::string& deadline_str) {
  try {
    return std::stoul(deadline_str);
  } catch (const std::invalid_argument&) {
    THROW_RUNTIME_ERROR("The deadline value '" + deadline_str +
                        "' cannot be matched into a number");
  }
}

std::string generate_rnd_string(unsigned rnd_seed, std::size_t len) {
  static constexpr char alphanum_table[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::default_random_engine rnd_engine(rnd_seed);
  std::uniform_int_distribution<> rnd_char_distr(0, sizeof(alphanum_table) - 1);

  std::string rnd_string;
  rnd_string.reserve(len);

  for (std::size_t i = 0; i < len; ++i) {
    char rnd_char = alphanum_table[rnd_char_distr(rnd_engine)];
    rnd_string.push_back(rnd_char);
  }

  return rnd_string;
}

std::ofstream generate_result_file(AlgorithmSelection algorithm_type,
                                   std::ostream* log, unsigned rnd_seed = 0) {
  static constexpr const char* STATIC_FILENAME = "output_result";
  static constexpr int LEN_RND_STRING = 6;
  const std::string filename_result =
      std::string(STATIC_FILENAME) + "_" +
      AlgorithmType2String(algorithm_type) + "__" +
      generate_rnd_string(rnd_seed, LEN_RND_STRING) + ".txt";

  std::ofstream file(filename_result);
  *log << "Generated solution file: `" << filename_result << '\n';
  return file;
}

int main(int argc, char* argv[]) {
  if (argc < 5) {
    std::cerr << "Usage:\n"
              << argv[0] << " DATAFILE CONFIGFILE DEADLINE (-1|-2|-12)\n";
    return -1;
  }

  // Create configuration
  opt_common::Configuration opt_deadline_conf;
  opt_deadline_conf.read_configuration_from_file(argv[2]);

  // Create process
  const auto total_deadline = parse_total_deadline_process(argv[3]);
  auto process = Process::create_process(argv[1], argv[2], total_deadline);

  // Parse algorithm type
  const auto algorithm_type = parse_algorithm_selection_from_cmd_line(argv[4]);

  std::cout << "Algorithm selected: " << AlgorithmType2String(algorithm_type)
            << "\n";

  auto result_log = generate_result_file(
      algorithm_type, &std::cout,
      std::chrono::system_clock::now().time_since_epoch().count());

  // Launch algorithm class in according to type
  bool status_algorithm = false;
  switch (algorithm_type) {
    case AlgorithmSelection::ALGORITHM_1:
      Algorithm1 algorithm1;
      status_algorithm = algorithm1.process(opt_deadline_conf, &process,
                                            &std::cout, &result_log);
      break;
    case AlgorithmSelection::ALGORITHM_2:
      Algorithm2 algorithm2;
      status_algorithm = algorithm2.process(opt_deadline_conf, &process,
                                            &std::cout, &result_log);
      break;
    case AlgorithmSelection::ALGORITHM_12:
      Algorithm1 algorithm1_2;
      Algorithm2 algorithm2_2;
      status_algorithm = algorithm1_2.process(opt_deadline_conf, &process,
                                              &std::cout, &result_log);
      if (status_algorithm == true) {
        status_algorithm = algorithm2_2.process(opt_deadline_conf, &process,
                                                &std::cout, &result_log);
      }
      break;
    default:
      std::cerr << "Algorithm type not recognized\n";
  }

  result_log.close();
  return status_algorithm == true ? 0 : -1;
}
