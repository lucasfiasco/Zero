#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include "ast.hpp"
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

std::vector<Token> tokenizer(const std::string &inputedFileStringContents) {
  std::vector<Token> tokens;
  size_t i = 0;
  static const std::unordered_set<std::string> keywords = {"return", "int"};

  while (i < inputedFileStringContents.size()) {
    char x = inputedFileStringContents.at(i);

    if (std::isspace(x)) {
      ++i;
      continue;
    }

    if (std::isalpha(x) || x == '_') {
      size_t start = i;
      while (i < inputedFileStringContents.size() &&
             (std::isalnum(inputedFileStringContents.at(i)) ||
              (inputedFileStringContents.at(i) == '_'))) {
        ++i;
      }
      std::string word = inputedFileStringContents.substr(start, i - start);
      if (keywords.count(word))
        tokens.push_back({TokenType::Keyword, word});
      else
        tokens.push_back({TokenType::Identifier, word});
      continue;
    }

    if (std::isdigit(static_cast<unsigned char>(x))) {
      size_t start = i;
      while (i < inputedFileStringContents.size() &&
             std::isdigit(
                 static_cast<unsigned char>(inputedFileStringContents.at(i)))) {
        ++i;
      }
      tokens.push_back({TokenType::Integer,
                        inputedFileStringContents.substr(start, i - start)});
      continue;
    }

    tokens.push_back({TokenType::Symbol, std::string(1, x)});
    ++i;
  }

  return tokens;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "error" << std::endl;
    return 2;
  }
  auto toks = tokenizer(fileReader(argv[1]));
  auto asmText = miniCompiler::compileTokensToAsm(toks);
  std::ofstream("out.asm") << asmText;

  return 0;
}
