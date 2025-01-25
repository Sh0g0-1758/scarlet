void lexer::read_file(const std::string &file_path) {
  std::ifstream file(file_path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      file_contents += line;
      file_contents += '\n';
    }
    file.close();
  }
}