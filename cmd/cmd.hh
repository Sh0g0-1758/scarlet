#pragma once

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;

class scarcmd {
private:
  po::options_description desc;
  po::options_description hidden;
  po::variables_map vm;
  po::options_description all_options;
  std::string input_file;
  std::vector<std::string> extra_args;

public:
  scarcmd()
      : desc("Usage: scarlet [options] file...\nOptions"),
        hidden("Hidden options"), all_options("All options") {
    // clang-format off
    desc.add_options()
      ("help,h", "produce this help message")
      ("version,v", "print version string")
      ("lex", "run only till the lexer stage")
      ("parse", "run only till the parser stage and print the AST")
      ("validate", "run semantic analysis and print the AST")
      ("scar", "print scar(IR for scarlet)")
      ("fold-constants", "Enable constant folding")
      ("propagate-copies","Enable copy propagation")
      ("eliminate-unreachable-code", "Enable unreachable code elimination")
      ("eliminate-dead-stores", "Enable dead store elimination")
      ("optimize", "Enable all optimizations")
      ("asm,S", "generate .s file containing X86-64 assembly")
      (",c", "Complie and assemble, but do not link")
      ("output-file,o", po::value<std::string>(), "output file");

    hidden.add_options()
      ("tacky", "run only till the codegen stage and print the IR(scar) AST")
      ("codegen", "run only till the codegen stage and generate the .s file containing X86-64 assembly")
      ("input-file", po::value<std::string>(), "input file");
    // clang-format on

    all_options.add(desc).add(hidden);
  }

  void parse(int ac, char *av[]) {
    try {
      po::parsed_options parsed = po::command_line_parser(ac, av)
                                      .options(all_options)
                                      .allow_unregistered()
                                      .run();
      po::store(parsed, vm);
      po::notify(vm);

      extra_args =
          po::collect_unrecognized(parsed.options, po::include_positional);
      for (auto it = extra_args.begin(); it != extra_args.end(); ++it) {
        if ((*it)[0] != '-') {
          input_file = *it;
          extra_args.erase(it);
          break;
        }
      }

      if (vm.count("input-file")) {
        input_file = vm["input-file"].as<std::string>();
      }
    } catch (po::error &e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      std::cerr << desc << std::endl;
      exit(1);
    }
  }

  bool has_option(const std::string &option) const {
    return vm.count(option) > 0;
  }

  void help() const { std::cout << desc; }

  void ver() const {
    std::cout << "\033[38;5;207m" // Neon pink color
              << R"(
┌─────────────────────────────────────┐
│       ░▒▓█ S C A R L E T █▓▒░       │
└─────────────────────────────────────┘
)" << "\033[38;5;159m"
              << "                  Compiler v0.1_  \033[0m\n\n";
  }

  const std::string get_input_file() const { return input_file; }

  std::vector<std::string> get_extra_args() const { return extra_args; }

  template <typename T> T get_option(const std::string &option) const {
    return vm[option].as<T>();
  }
};
