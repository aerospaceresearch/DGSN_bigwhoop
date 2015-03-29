#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <sstream>

#include "log.hpp"
#include "parser.hpp"


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
  std::stringstream sstr;
  sstr << root_;
  //log::write(log::verbose, "parsing JSON-Object: %s---\n",
  //           sstr.str().c_str());
}

