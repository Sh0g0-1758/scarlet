#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <vector>

#include <codegen/codegen.cc>
#include <lexer/lexer.cc>
#include <parser/parser.cc>

// extract out the name of the file
// possibilities : ./<path>/<filename>.<ext> or <filename>.<ext>
#include <filesystem>

bool isFlagValid(const std::string &flag) {
  return flag == "lex" || flag == "parse" || flag == "codegen" ||
         flag == "tacky" || flag == "scasm";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[USAGE]: " << argv[0] << " <filepath>" << " <flags>"
              << std::endl;
    return 1;
  }

  std::vector<std::string> flags;
  flags.reserve(argc - 2);

  for (int i = 1; i < argc - 1; i++) {
    std::string flagArg = std::string(argv[i]);
    std::string flag = flagArg.substr(2);
    if (flagArg.substr(0, 2) != "--" or !isFlagValid(flag)) {
      std::cerr << std::format("[ERROR]: Invalid flag {}", argv[i])
                << std::endl;
      return 1;
    } else {
      flags.emplace_back(flag);
    }
  }

  std::filesystem::path path(argv[argc - 1]);
  std::string file_name = path.stem().string();

  if (path.extension().string() != ".sc" and
      path.extension().string() != ".c" and
      path.extension().string() != ".cc") {
    std::cerr << "[ERROR]: file format not recognized; It should have an sc | "
                 "c | cc extension"
              << std::endl;
    return 1;
  }

  int result = system(
      std::format("gcc -x c++ -E -P {} -o {}.scp", argv[argc - 1], file_name)
          .c_str());

  if (result != 0) {
    std::cerr << "[ERROR]: Preprocessing failed" << std::endl;
    return 1;
  }

  // LEXER
  lexer lex;
  lex.read_file(std::format("{}.scp", file_name));
  lex.tokenize();
  if (!lex.is_success()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    std::cerr << "[ERROR]: Lexical analysis failed" << std::endl;
    return 1;
  }

  if (std::find(flags.begin(), flags.end(), "lex") != flags.end()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    return 0;
  }

  // PARSER
  parser gnu;
  gnu.parse_program(lex.get_tokens());
  if (!gnu.is_success()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    gnu.display_errors();
    std::cerr << "[ERROR]: Parsing failed" << std::endl;
    return 1;
  }

  if (std::find(flags.begin(), flags.end(), "parse") != flags.end()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    gnu.pretty_print();
    return 0;
  }

  // CODEGEN
  Codegen codegen(gnu.get_program());

  codegen.gen_scar();

  if (std::find(flags.begin(), flags.end(), "tacky") != flags.end()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    codegen.pretty_print();
    return 0;
  }
  codegen.pretty_print();
  codegen.set_file_name(std::format("{}.s", file_name));
  codegen.codegen();

  if (!codegen.is_success()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    std::cerr << "[ERROR]: Code generation failed" << std::endl;
    return 1;
  }

  if (std::find(flags.begin(), flags.end(), "codegen") != flags.end()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }

    result = system(std::format("rm {}.s", file_name).c_str());
    if (result != 0) {
      std::cerr << "[ERROR]: Unable to delete the intermediate assembly file"
                << std::endl;
      return 1;
    }
    return 0;
  }

  if (std::find(flags.begin(), flags.end(), "scasm") != flags.end()) {
    result = system(std::format("rm {}.scp", file_name).c_str());

    if (result != 0) {
      std::cerr
          << "[ERROR]: Unable to delete the intermediate preprocessed file"
          << std::endl;
      return 1;
    }
    return 0;
  }

  // delete the intermediate preprocessed file
  result = system(std::format("rm {}.scp", file_name).c_str());

  if (result != 0) {
    std::cerr << "[ERROR]: Unable to delete the intermediate preprocessed file"
              << std::endl;
    return 1;
  }

  // convert the assembly file to object file
  result = system(std::format("gcc {}.s -o {}", file_name, file_name).c_str());

  if (result != 0) {
    std::cerr << "[ERROR]: Failed to generate the executable" << std::endl;
    return 1;
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
