#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "log.hpp"
#include "main.hpp"
#include "parser.hpp"


/*
 * @brief Greeting message.
 */
static void greeting() noexcept
{
  std::cout << "Here we go!" << std::endl;
  std::cout << "DGSN BigWhoop parser v" << VERSION_MAJOR << "."
      << VERSION_MINOR << "." << VERSION_REVISION << std::endl;
}

/*
 * @brief Message to say good bye.
 */
static void valediction() noexcept
{
  std::cout << "Good bye!" << std::endl;
}

/*
 * @brief Check if a table exists in the database.
 */
static bool table_exists(soci::session& sql, const std::string& table)
  throw (std::exception)
{
  int table_found;
  std::string query = "select count(type) from sqlite_master where "
    "type='table' and name='" + table + "';";
  sql << query, soci::into(table_found);
  return table_found;
}

/*
 * @brief Check and setup the database.
 *
 * Note: If using other database library than sqlite3, additional
 *       exceptions might be thrown.
 */
static void db_init(soci::session& sql) throw (std::exception)
{
  log::write(log::debug, "  database initialisation …\n");
  size_t count_entries_total = 0u;

  if(table_exists(sql, "version")) {
    // TODO: Check if there is only one entry in the version table
    int v_major, v_minor;
    sql << "SELECT major FROM version", soci::into(v_major);
    sql << "SELECT minor FROM version", soci::into(v_minor);
    log:: write(log::debug, "    found database: v%u.%u\n",
      v_major, v_minor);
    if(VERSION_MAJOR != v_major ||
       VERSION_MINOR != v_minor) {
      log::write(log::warning,
          "    [Warning] Version mismatch between parser and database\n");
    }
  } else {
    sql << "CREATE TABLE IF NOT EXISTS version(major INTEGER, "
      "minor INTEGER);";
    sql << "INSERT INTO version VALUES(:v_major,:v_minor)",
      soci::use(VERSION_MAJOR), soci::use(VERSION_MINOR);
    log:: write(log::debug, "    created database: v%u.%u\n",
      VERSION_MAJOR, VERSION_MINOR);
  }

  if(table_exists(sql, "data")) {
    log:: write(log::verbose, "    data found\n");
  } else {
    // TODO: Create proper data table
    sql << "CREATE TABLE IF NOT EXISTS data(Id INTEGER);";
    log:: write(log::verbose, "    data table created\n");
  }

  log::write(log::debug, "  database initialized\n");
}

/*
 * @brief Get file extension.
 */
std::string get_ext(std::string path) noexcept
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
  if(argc < 2 ||
  !(get_ext(argv[1]) != "json" || get_ext(argv[1]) != "js")) {
    std::cout << "Usage: " << argv[0] << " <file.json>" << std::endl;
  } else {
    try {
      soci::session sql(soci::sqlite3, DB_FILE);
      db_init(sql);

      std::filebuf fb;
      if(fb.open(argv[1], std::ios::in)) {
        std::istream is(&fb);
        Parser parser(is);
        fb.close();
      }
    } catch (const std::exception& exception) {
      log::write(log::fatal, "\n[Error] %s\n\n", exception.what());
      result = EXIT_FAILURE;
    }
  }
  valediction();

  return result;
}

