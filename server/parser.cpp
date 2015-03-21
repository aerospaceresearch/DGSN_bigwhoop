#include "parser.hpp"

#include <iostream> // TODO: debug


Parser::Parser(std::istream& input)
{
  parse(input);
}

void Parser::parse(std::istream& input)
{
  input >> root_;
  debugPrintJson();
}

void Parser::debugPrintJson() const
{
  std::cout << root_;
}
