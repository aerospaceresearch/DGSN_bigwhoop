#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "log.hpp"
#include "main.hpp"
#include "parser.hpp"


static void greeting()
{
  std::cout << "Here we go!" << std::endl;
  std::cout << "DGSN BigWhoop parser v" << VERSION_MAJOR << "."
      << VERSION_MINOR << "." << VERSION_REVISION << std::endl;
}

static void valediction()
{
  std::cout << "Good bye!" << std::endl;
}

/*
 * @brief Check if a table exists in the database.
 */
static bool table_exists(soci::session& sql, const std::string& table)
{
  int table_found;
  std::string query = "select count(type) from sqlite_master where "
    "type='table' and name='" + table + "';";
  sql << query, soci::into(table_found);
  return table_found;
}

/*
 * @brief Check and setup the database.
 */
static void db_init(soci::session& sql)
{
  log::write(log::debug, "  database initialization ...\n");

  if(table_exists(sql, "version")) {
    int v_major, v_minor, v_revision;
    sql << "SELECT major FROM version", soci::into(v_major);
    sql << "SELECT minor FROM version", soci::into(v_minor);
    sql << "SELECT revision FROM version", soci::into(v_revision);
    log:: write(log::debug, "    database found: v%u.%u.%u\n",
      v_major, v_minor, v_revision);
    if(VERSION_MAJOR != v_major ||
       VERSION_MINOR != v_minor ||
       VERSION_REVISION != v_revision) {
      log::write(log::warning, "    [Warning] Version mismatch between parser and database\n");
    }
  } else {
    sql << "CREATE TABLE IF NOT EXISTS version(major INTEGER, "
      "minor INTEGER, revision INTEGER);";
    sql << "INSERT INTO version VALUES(:v_major,:v_minor,:v_revision)",
      soci::use(VERSION_MAJOR), soci::use(VERSION_MINOR),
      soci::use(VERSION_REVISION);
    log:: write(log::debug, "    created database: v%u.%u.%u\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
  }

  //sql << "CREATE TABLE IF NOT EXISTS data(Id INTEGER);";

  log::write(log::debug, "  database initialized\n");
}

/*
 * @brief Get file extension.
 */
std::string get_ext(std::string path)
{
   size_t period = path.find_last_of(".");
   std::string ext = path.substr(period + 1);
   return ext;
}

/*
 * @brief Main function
 */
int main(int argc, char** argv)
{
  int result = EXIT_SUCCESS;

  greeting();
  std::filebuf fb;
  if(argc < 2 ||
  !(get_ext(argv[1]) != "json" || get_ext(argv[1]) != "js")) {
    std::cout << "Usage: " << argv[0] << " <file.json>" << std::endl;
  } else {
    try {
      soci::session sql(soci::sqlite3, DB_FILE);
      db_init(sql);

      if(fb.open(argv[1], std::ios::in)) {
        std::istream is(&fb);
        Parser parser(is);
        fb.close();
      }
    } catch (const std::exception & exception) {
      log::write(log::fatal, "[Error] %s\n", exception.what());
      result = EXIT_FAILURE;
    }
  }
  valediction();

  return result;
}

//int main(int argc, char** argv)
//{
//
//  return EXIT_SUCCESS;
//}

