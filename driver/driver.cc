#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <vector>

#include <cmd/cmd.hh>
#include <codegen/codegen.hh>
#include <lexer/lexer.hh>
#include <parser/parser.hh>

// extract out the name of the file
// possibilities : ./<path>/<filename>.<ext> or <filename>.<ext>
#include <filesystem>

int main(int argc, char *argv[]) {
  scarcmd cmd;
  cmd.parse(argc, argv);
  if (cmd.has_option("help")) {
    cmd.help();
    return 0;
  }
  if (cmd.has_option("version")) {
    cmd.ver();
    return 0;
  }
  if (!cmd.has_option("input-file")) {
    std::cerr << "[ERROR]: No input file provided\npass --help to know how to "
                 "use scarlet"
              << std::endl;
    return 1;
  }
  std::filesystem::path path(cmd.get_input_file());
  std::string file_name = path.stem().string();

  if (path.extension().string() != ".sc" and
      path.extension().string() != ".c" and
      path.extension().string() != ".cc") {
    std::cerr << "[ERROR]: file format not recognized; It should have an sc | "
                 "c | cc extension"
              << std::endl;
    return 1;
  }

  int result = system(std::format("gcc -x c++ -E -w -P {} -o {}.scp",
                                  cmd.get_input_file(), file_name)
                          .c_str());

  if (result != 0) {
    std::cerr << "[ERROR]: Preprocessing failed" << std::endl;
    return 1;
  }

  // LEXER
  scarlet::lexer::lexer lex;
  lex.set_file_path(std::format("{}.scp", file_name));
  lex.tokenize();

  // delete the intermediate preprocessed file
  result = system(std::format("rm {}.scp", file_name).c_str());

  if (result != 0) {
    std::cerr << "[ERROR]: Unable to delete the intermediate preprocessed file"
              << std::endl;
    return 1;
  }

  if (!lex.is_success()) {
    lex.print_error_recovery();
    return 1;
  }

  if (cmd.has_option("lex")) {
    std::cout << "[LOG]: Lexing done successfully" << std::endl;
    lex.print_symbol_table();
    return 0;
  }

  // PARSER
  scarlet::parser::parser gnu;
  gnu.parse_program(lex.get_tokens());
  if (!gnu.is_success()) {
    gnu.display_errors();
    std::cerr << "[ERROR]: Parsing failed" << std::endl;
    return 1;
  }

  if (cmd.has_option("parse")) {
    std::cout << "[LOG]: Parsing done successfully" << std::endl;
    gnu.pretty_print();
    return 0;
  }

  // SEMANTIC ANALYSIS
  gnu.semantic_analysis();
  if (!gnu.is_success()) {
    gnu.display_errors();
    std::cerr << "[ERROR]: Semantic analysis failed" << std::endl;
    return 1;
  }

  if (cmd.has_option("validate")) {
    std::cout << "[LOG]: Semantic analysis done successfully" << std::endl;
    gnu.pretty_print();
    return 0;
  }

  // CODEGEN
  scarlet::codegen::Codegen codegen(gnu.get_program(),
                                    gnu.get_symbol_counter());

  codegen.gen_scar();

  if (cmd.has_option("tacky") or cmd.has_option("scar")) {
    std::cout << "[LOG]: Successfully generated scar" << std::endl;
    codegen.pretty_print();
    return 0;
  }
  codegen.set_file_name(std::format("{}.s", file_name));
  codegen.codegen();

  if (!codegen.is_success()) {
    std::cerr << "[ERROR]: Code generation failed" << std::endl;
    return 1;
  }

  if (cmd.has_option("asm") or cmd.has_option("codegen")) {
    std::cout << "[LOG]: Successfully generated asm" << std::endl;
    return 0;
  }

  // convert the assembly file to object file
  std::string output_file_name = file_name;
  if (cmd.has_option("output-file")) {
    output_file_name = cmd.get_option<std::string>("output-file");
  }

  if (cmd.has_option("-c")) {
    result =
        system(std::format("gcc -c {}.s -o {}.o", file_name, output_file_name)
                   .c_str());
    if (result != 0) {
      std::cerr << "[ERROR]: Failed to generate the object file" << std::endl;
      return 1;
    }
  } else {
    result = system(
        std::format("gcc {}.s -o {}", file_name, output_file_name).c_str());
    if (result != 0) {
      std::cerr << "[ERROR]: Failed to generate the executable" << std::endl;
      return 1;
    }
  }

  // delete the intermediate assembly file
  result = system(std::format("rm {}.s", file_name).c_str());
  if (result != 0) {
    std::cerr << "[ERROR]: Unable to delete the intermediate assembly file"
              << std::endl;
    return 1;
  }

  std::cout << "[LOG]: Scarlet ran Successfully" << std::endl;

  return 0;
}
