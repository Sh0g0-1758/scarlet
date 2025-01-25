#pragma once

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

class scarcmd {
private:
  po::options_description desc;
  po::options_description hidden;
  po::positional_options_description pos_desc;
  po::variables_map vm;
  po::options_description all_options;
  std::string input_file;

public:
  scarcmd()
      : desc("Usage: scarlet [options] file...\nOptions"),
        hidden("Hidden options"), all_options("All options") {

    desc.add_options()("help,h", "produce this help message")(
        "version,v", "print version string")("lex",
                                             "run only till the lexer stage")(
        "parse", "run only till the parser stage and print the AST")(
        "codegen",
        "run only till the codegen stage and print the IR(scar) AST")(
        "asm,S", "generate .s file containing X86-64 assembly")(
        "output-file,o", po::value<std::string>(), "output file");

    hidden.add_options()(
        "tacky", "run only till the codegen stage and print the IR(scar) AST")(
        "input-file", po::value<std::string>(), "input file");

    pos_desc.add("input-file", -1);
    all_options.add(desc).add(hidden);
  }

  void parse(int ac, char *av[]) {
    po::store(po::command_line_parser(ac, av)
                  .options(all_options)
                  .positional(pos_desc)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count("input-file")) {
      input_file = vm["input-file"].as<std::string>();
    }
  }

  bool has_option(const std::string &option) const {
    return vm.count(option) > 0;
  }

  void help() const { std::cout << desc; }

  const std::string get_input_file() const { return input_file; }

  template <typename T> T get_option(const std::string &option) const {
    return vm[option].as<T>();
  }
};
