#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "log.hpp"
#include "main.hpp"
#include "options.hpp"
#include "parser.hpp"


/**
 * @brief Print version
 */
static void print_version() noexcept
{
  log::write(log::level::info, "DGSN BigWhoop parser v%d.%d.%d\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
}

/**
 * @brief Greeting message.
 */
static void greeting() noexcept
{
  print_version();
  log::write(log::level::info, "Here we go!\n");
}

/**
 * @brief Message to say good bye.
 */
static void valediction() noexcept
{
  log::write(log::level::info, "Done and done. Good bye!\n");
}

/**
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

/**
 * @brief Check and setup the database.
 *
 * Note: If using other database library than sqlite3, additional
 *       exceptions might be thrown.
 */
static void db_init(soci::session& sql) throw (std::exception)
{
  log::write(log::level::debug, "  database initialisation …\n");
  size_t count_entries_total = 0u;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  if(table_exists(sql, "db_version")) {
    int count_entries;
    int v_major, v_minor;
    sql << "SELECT Count(*) FROM db_version;", soci::into(count_entries);
    if(count_entries != 1) {
      log::write(log::level::error,
          "    [Error] Database is ill-formed! (%d version entries)\n",
          count_entries);
    }
    sql << "SELECT v_major FROM db_version", soci::into(v_major);
    sql << "SELECT v_minor FROM db_version", soci::into(v_minor);
    log::write(log::level::verbose, "    found database: v%d.%d\n",
      v_major, v_minor);
    if(VERSION_MAJOR != v_major ||
       VERSION_MINOR != v_minor) {
      log::write(log::level::warning,
          "    [Warning] Version mismatch between parser and database\n");
    }
  } else {
    sql << "CREATE TABLE IF NOT EXISTS db_version("
      "v_major INTEGER UNIQUE, "
      "v_minor INTEGER UNIQUE);";
    sql << "INSERT INTO db_version VALUES(:v_major,:v_minor)",
      soci::use(VERSION_MAJOR), soci::use(VERSION_MINOR);
    log::write(log::level::debug, "    created database: v%u.%u\n",
      VERSION_MAJOR, VERSION_MINOR);
  }

  if(table_exists(sql, "data")) {
    int count_entries;
    sql << "SELECT Count(*) FROM data;", soci::into(count_entries);
    log::write(log::level::verbose, "    found data (%d entries)\n",
        count_entries);
    count_entries_total += count_entries;
  } else {
    sql << "CREATE TABLE IF NOT EXISTS data("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "time REAL, "
        "freq INTEGER, "
        "amp_max REAL, "
        "amp_mean REAL, "
        "scan_mode TEXT, "
        "location_alt REAL, "
        "location_lat REAL, "
        "location_lon REAL, "
        "client_id_hash TEXT, "
        "client_name TEXT, "
        "sensor_id INTEGER, "
        "sensor_name TEXT, "
        "sensor_antenna TEXT, "
        "sensor_ppm INTEGER, "
        "wu_id TEXT, "
        "url TEXT, "
        "sw_bit INTEGER, "
        "sw_os TEXT, "
        "sw_v_major INTEGER, "
        "sw_v_minor INTEGER, "
        "sw_v_revision INTEGER);";
    log::write(log::level::verbose, "    data table created\n");
  }

  if(table_exists(sql, "adsb")) {
    int count_entries;
    sql << "SELECT Count(*) FROM adsb;", soci::into(count_entries);
    log::write(log::level::verbose, "    found adsb data (%d entries)\n",
        count_entries);
    count_entries_total += count_entries;
  } else {
    sql << "CREATE TABLE IF NOT EXISTS adsb("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "time REAL, "
        "location_alt REAL, location_lat REAL, location_lon REAL);";
    log::write(log::level::verbose, "    adsb table created\n");
  }

  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::level::debug,
      "  database initialised (%u entries) [%d%s]\n",
      count_entries_total, duration, duration_unit_string);
}

/**
 * @brief Get file extension.
 */
std::string get_ext(std::string path) noexcept
{
   size_t period = path.find_last_of(".");
   std::string ext = path.substr(period + 1);
   return ext;
}

/**
 * @brief Remove duplicate entries in the database tables.
 */
static void remove_duplicates(soci::session& sql)
{
  log::write(log::level::debug, "  remove duplicate entries …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  sql << "DELETE   FROM data "
         "WHERE    rowid NOT IN "
         "( "
         "SELECT MIN(rowid) "
         "FROM    data "
         "GROUP BY "
         "        time"
         ",       freq"
         ",       amp_max"
         ",       amp_mean"
         ",       scan_mode"
         ",       location_alt"
         ",       location_lat"
         ",       location_lon"
         ",       client_id_hash"
         ",       sensor_id"
         ",       sensor_name"
         ",       sensor_antenna"
         ",       sensor_ppm"
         ",       wu_id"
         ");";

  sql << "DELETE   FROM adsb "
         "WHERE    rowid NOT IN "
         "( "
         "SELECT MIN(rowid) "
         "FROM    adsb "
         "GROUP BY "
         "        time"
         ",       location_alt"
         ",       location_lat"
         ",       location_lon"
         ");";

  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::level::debug, " skipped [%d%s]\n", duration,
      duration_unit_string);
}

/**
 * @brief Performs various checks to validate the database.
 */
static void check_database(soci::session& sql)
{
  log::write(log::level::debug, "  check database …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  if(Options::get_instance().db_checks()) {
    // TODO:
    // • Check the uniqueness of entries
    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::debug, " done [%d%s]\n", duration,
        duration_unit_string);
  } else {
    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::debug, " skipped [%d%s]\n", duration,
        duration_unit_string);
  }
}

/**
 * @brief Main function
 */
int main(int argc, char** argv)
{
  int result = EXIT_SUCCESS;
  try {
    Options::get_instance().init(argc, argv);
    if(Options::get_instance().process()) {
      greeting();
      soci::session sql(soci::sqlite3, DB_FILE);
      std::ofstream file_log(SQL_LOG_FILE);
      sql.set_log_stream(&file_log);

      db_init(sql);

      std::filebuf fb;
      std::string json_file;
      if(Options::get_instance().json_files().size()) {
        json_file = Options::get_instance().json_files()[0];
        if(fb.open(json_file, std::ios::in)) {
          std::istream is(&fb);
          Parser parser(is);
          soci::transaction tr(sql);
          sql << parser;
          tr.commit();
          fb.close();
        }
      }
      remove_duplicates(sql);
      check_database(sql);
      valediction();
    }
  } catch (const std::exception& exception) {
    log::write(log::level::fatal, "\n[Error] %s\n\n", exception.what());
    result = EXIT_FAILURE;
  }

  return result;
}

