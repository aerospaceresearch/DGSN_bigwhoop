#pragma once

#include <iostream>
#include <ostream>

#include <jsoncpp/json/json.h>

class Parser
{
  public:
    Parser(std::istream& input);
    void query(soci::session& sql) const;

  private:
    void parse(std::istream& input);

    Json::Value root_;
};

/*
 * @brief Put json structure which is stored in root_ into the database.
 */
soci::session& operator<<(soci::session& sql, const Parser& p);

