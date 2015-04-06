#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "log.hpp"
#include "main.hpp"
#include "parser.hpp"


/*
 * @brief Print version
 */
static void print_version() noexcept
{
  std::cout << "DGSN BigWhoop parser v" << VERSION_MAJOR
    << "." << VERSION_MINOR << "." << VERSION_REVISION << std::endl;
}

/*
 * @brief Print usage
 */
static void print_usage() noexcept
{
  std::cout << "Usage: ./parser <file.json>" << std::endl;
}

/*
 * @brief Greeting message.
 */
static void greeting() noexcept
{
  print_version();
  std::cout << "Here we go!" << std::endl;
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
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  if(table_exists(sql, "db_version")) {
    int count_entries;
    int v_major, v_minor;
    sql << "SELECT Count(*) FROM db_version;", soci::into(count_entries);
    if(count_entries != 1) {
      log::write(log::error,
          "    [Error] Database is ill-formed! (%d version entries)\n",
          count_entries);
    }
    sql << "SELECT v_major FROM db_version", soci::into(v_major);
    sql << "SELECT v_minor FROM db_version", soci::into(v_minor);
    log::write(log::verbose, "    found database: v%u.%u\n",
      v_major, v_minor);
    if(VERSION_MAJOR != v_major ||
       VERSION_MINOR != v_minor) {
      log::write(log::warning,
          "    [Warning] Version mismatch between parser and database\n");
    }
  } else {
    sql << "CREATE TABLE IF NOT EXISTS db_version(v_major INTEGER, "
      "v_minor INTEGER);";
    sql << "INSERT INTO db_version VALUES(:v_major,:v_minor)",
      soci::use(VERSION_MAJOR), soci::use(VERSION_MINOR);
    log::write(log::debug, "    created database: v%u.%u\n",
      VERSION_MAJOR, VERSION_MINOR);
  }

  if(table_exists(sql, "data")) {
    int count_entries;
    sql << "SELECT Count(*) FROM data;", soci::into(count_entries);
    log::write(log::verbose, "    found data (%d entries)\n",
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
    log::write(log::verbose, "    data table created\n");
  }

  if(table_exists(sql, "adsb")) {
    int count_entries;
    sql << "SELECT Count(*) FROM adsb;", soci::into(count_entries);
    log::write(log::verbose, "    found adsb data (%d entries)\n",
        count_entries);
    count_entries_total += count_entries;
  } else {
    sql << "CREATE TABLE IF NOT EXISTS adsb("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "time REAL, "
        "location_alt REAL, location_lat REAL, location_lon REAL);";
    log::write(log::verbose, "    adsb table created\n");
  }

  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::debug, "  database initialised (%u entries) [%d%s]\n",
      count_entries_total, duration, duration_unit_string);
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
 * @brief Remove duplicate entries in the database tables.
 */
static void remove_duplicates(soci::session& sql)
{
  log::write(log::debug, "  remove duplicate entries …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  // TODO: Verify!
  //sql << "DELETE FROM data WHERE rowid NOT IN (SELECT MIN(rowid) FROM "
  //  "data GROUP BY time);";
  //sql << "DELETE FROM geolocation WHERE rowid NOT IN "
  //  "(SELECT MAX(rowid) FROM data GROUP BY id);";
  //sql << "DELETE FROM client WHERE rowid NOT IN (SELECT MAX(rowid) FROM "
  //  "data GROUP BY id);";
  //sql << "DELETE FROM software WHERE rowid NOT IN "
  //  "(SELECT MAX(rowid) FROM data GROUP BY id);";
  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::debug, " skipped [%d%s]\n", duration,
      duration_unit_string);
}

/*
 * @brief Performs various checks to validate the database.
 */
static void check_database(soci::session& sql)
{
  log::write(log::debug, "  check database …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  // TODO:
  // • Only one entry in table version
  // • Check the uniqueness of entries
  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::debug, " skipped [%d%s]\n", duration,
      duration_unit_string);
}

/*
 * @brief Main function
 */
int main(int argc, char** argv)
{
  int result = EXIT_SUCCESS;

  if(argc < 2 ||
  !(get_ext(argv[1]) != "json" || get_ext(argv[1]) != "js")) {
    print_usage();
  } else {
    try {
      greeting();
      soci::session sql(soci::sqlite3, DB_FILE);
      std::ofstream file_log(SQL_LOG_FILE);
      sql.set_log_stream(&file_log);

      db_init(sql);

      std::filebuf fb;
      if(fb.open(argv[1], std::ios::in)) {
        std::istream is(&fb);
        Parser parser(is);
        soci::transaction tr(sql);
        sql << parser;
        tr.commit();
        fb.close();
      }
      remove_duplicates(sql);
      check_database(sql);
      valediction();
    } catch (const std::exception& exception) {
      log::write(log::fatal, "\n[Error] %s\n\n", exception.what());
      result = EXIT_FAILURE;
    }
  }

  return result;
}

