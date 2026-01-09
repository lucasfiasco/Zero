#include <fstream>
#include <iostream>
#include <string>
// Zero -> tokenize -> AST (Nodes -> return, conditional, recursion, variable)
// (return, string), (0, int)
std::string fileReader(const char *inputFilePath) {
  std::string fileContents = "";
  std::string fileLine = "";
  std::ifstream reader(inputFilePath);
  if (!reader.is_open()) {
    std::cout << "file didnt open" << std::endl;
    return "";
  }
  while (getline(reader, fileLine)) {
    fileContents += fileLine + '\n';
  }
  reader.close();
  return fileContents;
}

enum class TokenType { Keyword, Identifier, Integer, Symbol };

struct Token {
  TokenType type;
  std::string value;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "error" << std::endl;
    return 2;
  }
  std::cout << fileReader(argv[1]) << std::endl;
  return 0;
}
