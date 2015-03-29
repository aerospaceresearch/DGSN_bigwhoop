#pragma once

#include <istream>

#include <jsoncpp/json/json.h>


class Parser
{
  public:
    Parser(std::istream& input);

  private:
    void parse(std::istream& input);
    void debugPrintJson() const;

    Json::Value root_;
};

