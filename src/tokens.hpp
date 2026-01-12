#pragma once

#include <string>

enum class TokenType { Keyword, Identifier, Integer, Symbol };

struct Token {
  TokenType type;
  std::string value;
};
