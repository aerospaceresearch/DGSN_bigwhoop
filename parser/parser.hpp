#pragma once

#include <iostream>
#include <ostream>

#include <json/json.h>

/**
 * @class Parser
 *
 * @brief Parse a JSON file and store the incoming data in a database.
 */
class Parser
{
  public:
    Parser(std::istream& input);
    void query(soci::session& sql) const;
    void info() const;

  private:
    void parse(std::istream& input);

    Json::Value root_;
};

/**
 * @brief Put json structure which is stored in root_ into the database.
 *
 * @param sql Reference to a soci::session
 * @param p Constant reference to a parser object
 *
 * @return Reference to a soci::session
 */
soci::session& operator<<(soci::session& sql, const Parser& p);

