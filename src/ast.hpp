#pragma once
#include "tokens.hpp"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace miniCompiler {

struct ParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct CodegenError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Expr {
  virtual ~Expr() = default;
};

struct IntExpr final : Expr {
  long long value;
  explicit IntExpr(long long value) : value(value) {}
};

struct BinaryExpr final : Expr {
  char op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(char op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : op(op), left(std::move(left)), right(std::move(right)) {}
};

struct Stmt {
  virtual ~Stmt() = default;
};

struct ReturnStmt final : Stmt {
  std::unique_ptr<Expr> value;
  explicit ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

class Parser {
public:
  explicit Parser(const std::vector<Token> &tokens) : tokens(tokens), pos(0) {}

  std::unique_ptr<Stmt> parseStatement() {
    if (!matchKeyword("return")) {
      throw ParseError("Expected 'return' at start of statement");
    }
    auto expr = parseExpression();
    expectSymbol(";");
    return std::make_unique<ReturnStmt>(std::move(expr));
  }

private:
  const std::vector<Token> &tokens;
  size_t pos;

  bool isAtEnd() const { return pos >= tokens.size(); }

  const Token &peek() const {
    if (isAtEnd()) {
      throw ParseError("Unexpected end of input");
    }
    return tokens.at(pos);
  }

  const Token &consume() {
    const Token &t = peek();
    pos += 1;
    return t;
  }

  bool matchKeyword(const std::string &kw) {
    if (!isAtEnd() && tokens.at(pos).type == TokenType::Keyword &&
        tokens.at(pos).value == kw) {
      pos += 1;
      return true;
    }
    return false;
  }

  bool matchSymbol(const std::string &sym) {
    if (!isAtEnd() && tokens.at(pos).type == TokenType::Symbol &&
        tokens.at(pos).value == sym) {
      pos += 1;
      return true;
    }
    return false;
  }

  void expectSymbol(const std::string &sym) {
    if (!matchSymbol(sym)) {
      throw ParseError("Expected symbol '" + sym + "'");
    }
  }

  std::unique_ptr<Expr> parseExpression() {
    auto left = parsePrimary();
    while (matchSymbol("+")) {
      auto right = parsePrimary();
      left =
          std::make_unique<BinaryExpr>('+', std::move(left), std::move(right));
    }
    return left;
  }

  std::unique_ptr<Expr> parsePrimary() {
    if (isAtEnd()) {
      throw ParseError("Expected primary expression, got end of input");
    }

    const Token &t = peek();
    if (t.type == TokenType::Integer) {
      consume();
      long long value = 0;
      try {
        value = std::stoll(t.value);
      } catch (...) {
        throw ParseError("Invalid integer literal: " + t.value);
      }
      return std::make_unique<IntExpr>(value);
    }

    throw ParseError("Expected integer literal");
  }
};

inline std::unique_ptr<Expr> foldConstants(std::unique_ptr<Expr> expr) {
  if (!expr)
    return nullptr;

  if (auto *bin = dynamic_cast<BinaryExpr *>(expr.get())) {
    bin->left = foldConstants(std::move(bin->left));
    bin->right = foldConstants(std::move(bin->right));

    auto *l = dynamic_cast<IntExpr *>(bin->left.get());
    auto *r = dynamic_cast<IntExpr *>(bin->right.get());
    if (l && r) {
      if (bin->op == '+') {
        long long v = l->value + r->value;
        return std::make_unique<IntExpr>(v);
      }
      throw CodegenError("Unsupported binary operator");
    }
  }

  return expr;
}

inline void genExprAsm(std::stringstream &out, const Expr &expr) {
  if (auto const *i = dynamic_cast<IntExpr const *>(&expr)) {
    out << "    mov rax, " << i->value << "\n";
    return;
  }

  if (auto const *b = dynamic_cast<BinaryExpr const *>(&expr)) {
    if (b->op != '+') {
      throw CodegenError("Only '+' is supported right now");
    }

    genExprAsm(out, *b->left); 
    out << "    push rax\n";
    genExprAsm(out, *b->right);
    out << "    pop rcx\n";
    out << "    add rax, rcx\n";
    return;
  }

  throw CodegenError("Unknown expression node");
}

inline std::string compileStmtToAsm(const Stmt &stmt) {
  std::stringstream out;

  out << ".intel_syntax noprefix\n";
  out << ".global _start\n";
  out << "_start:\n";

  auto const *ret = dynamic_cast<ReturnStmt const *>(&stmt);
  if (!ret) {
    throw CodegenError("Only return statements are supported");
  }

  genExprAsm(out, *ret->value); 
  out << "    mov rdi, rax\n";
  out << "    mov rax, 60\n";
  out << "    syscall\n";

  return out.str();
}

inline std::string compileTokensToAsm(const std::vector<Token> &tokens) {
  Parser parser(tokens);
  auto stmt = parser.parseStatement();

  if (auto *ret = dynamic_cast<ReturnStmt *>(stmt.get())) {
    ret->value = foldConstants(std::move(ret->value));
  }

  return compileStmtToAsm(*stmt);
}

}
