#pragma once

#include <iostream>

#include <jsoncpp/json/json.h>


class Parser
{
  public:
    Parser(std::istream& input);

  private:
    void parse(std::istream& input);

    Json::Value root_;
};

