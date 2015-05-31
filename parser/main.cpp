#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <boost/regex.hpp>

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

static void check_db_version(soci::session& sql)
{
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
    check_db_version(sql);
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
        "sw_bit TEXT, "
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

  if(Options::get_instance().remove_duplicates()) {

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
 * @brief Performs various checks to validate the database.
 */
static void check_database(soci::session& sql)
{
  log::write(log::level::debug, "  check database …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  if(Options::get_instance().db_checks()) {
    soci::row row;
    sql << "SELECT * FROM DATA;", into(row);

    for(unsigned int index = 0; index < row.size(); ++index) {
      const soci::column_properties& props = row.get_properties(index);
      const std::string name = props.get_name();
      if(name == "time") {
        const double time = row.get<double>(index);
        if(time < 0.0f) {
          throw std::runtime_error("Timestamp is negative");
        }
        if(time < 1428173820.0) {
          std::cout << std::endl << std::fixed << time << std::endl;
          throw std::runtime_error("Timestamp is too old");
        }
        auto unix_timestamp = std::chrono::seconds(std::time(NULL));
        int unix_timestamp_sec
          = std::chrono::seconds(unix_timestamp).count();
        if(time > unix_timestamp_sec) {
          throw std::runtime_error("Timestamp is in the future");
        }
      } else if(name == "freq") {
        //const unsigned int freq = row.get<unsigned int>(index);
      } else if(name == "amp_max") {
        const double amp_max = row.get<double>(index);
        if(amp_max < 0) {
          throw std::runtime_error("Maximum amplitude is negative");
        }
      } else if(name == "amp_mean") {
        const double amp_mean = row.get<double>(index);
        if(amp_mean < 0) {
          throw std::runtime_error("Mean amplitude is negative");
        }
      } else if(name == "scan_mode") {
        //const std::string amp_mean = row.get<std::string>(index);
      } else if(name == "location_alt") {
        //const double location_alt = row.get<double>(index);
      } else if(name == "location_lat") {
        //const double location_lat = row.get<double>(index);
      } else if(name == "location_lon") {
        //const double location_lon = row.get<double>(index);
      } else if(name == "client_id_hash") {
        const std::string client_id_hash = row.get<std::string>(index);
        if(client_id_hash.length() != 56u) {
          throw std::runtime_error("Id hash has wrong size");

          if(client_id_hash.find_first_not_of("0123456789abcdefABCDEF", 0)
              != std::string::npos) {
            throw std::runtime_error("Id hash is not a hex value");
          }
        }
      } else if(name == "client_name") {
        const std::string client_name = row.get<std::string>(index);
        if(client_name.length() > MAX_STR_LENGTH) {
          throw std::runtime_error("Client name is too long");
        }
      } else if(name == "sensor_id") {
        //const int sensor_id = row.get<int>(index);
      } else if(name == "sensor_name") {
        const std::string sensor_name = row.get<std::string>(index);
        if(sensor_name.length() > MAX_STR_LENGTH) {
          throw std::runtime_error("Sensor name is too long");
        }
      } else if(name == "sensor_antenna") {
        const std::string sensor_antenna = row.get<std::string>(index);
        if(sensor_antenna.length() > MAX_STR_LENGTH) {
          throw std::runtime_error(
              "Sensor antenna description is too long");
        }
      } else if(name == "sensor_ppm") {
        //const int sensor_ppm = row.get<int>(index);
      } else if(name == "wu_id") {
        const std::string wu_id = row.get<std::string>(index);
        if(wu_id.length() > MAX_STR_LENGTH) {
          throw std::runtime_error("Workunit ID is too long");
        }
      } else if(name == "url") {
        const std::string url = row.get<std::string>(index);
        if(url.length()) {
          if(url.length() > MAX_STR_LENGTH) {
            throw std::runtime_error("URL is too long");
          }

          boost::regex rgx_url(R"((www\.|(news|(ht|f)tp(s?))\://){1}\S+)");
          boost::regex rgx_email(R"([\w-]+@([\w-]+\.)+[\w-]+)");
          if (!boost::regex_match(url, rgx_url)) {
            if (!boost::regex_match(url, rgx_email)) {
              throw std::runtime_error("URL is not formatted correctly");
            }
          }
        }
      } else if(name == "sw_bit") {
        const std::string sw_bit = row.get<std::string>(index);
        if(sw_bit.length() > MAX_STR_LENGTH) {
          throw std::runtime_error("Bit description is too long");
        }
      } else if(name == "sw_os") {
        const std::string sw_os = row.get<std::string>(index);
        if(sw_os.length() > MAX_STR_LENGTH) {
          throw std::runtime_error(
              "Operating System description is too long");
        }
      } else if(name == "sw_v_major") {
        const int sw_v_major = row.get<int>(index);
        if(sw_v_major > VERSION_MAJOR) {
          std::stringstream error("Version mismatch: ");
          error << sw_v_major << " > " << VERSION_MAJOR;
          throw std::runtime_error(error.str());
        }
      } else if(name == "sw_v_minor") {
        //const unsigned int sw_v_minor = row.get<unsigned int>(index);
      } else if(name == "sw_v_revision") {
        //const unsigned int sw_v_revision = row.get<unsigned int>(index);
      }
    }

    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::debug, " done [%d%s]\n", duration,
        duration_unit_string);
    std::cout << duration << std::endl;
  } else {
    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::debug, " skipped [%d%s]\n", duration,
        duration_unit_string);
  }
}

static void info(soci::session& sql)
{
  if(table_exists(sql, "db_version")) {
    check_db_version(sql);
    unsigned int count_data = 0;
    unsigned int count_adsb = 0;
    log::write(log::level::info, "====== database ======\n");
    sql << "SELECT Count(*) FROM data;", soci::into(count_data);
    sql << "SELECT Count(*) FROM adsb;", soci::into(count_adsb);
    log::write(log::level::info, "  data entries:\t\t%u\n", count_data);
    log::write(log::level::info, "  ADSB entries:\t\t%u\n", count_adsb);
  } else {
    throw std::runtime_error("No version information in database");;
  }
}

static void info(soci::session& sql, const Parser& parser)
{
  parser.info();
  info(sql);
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
      soci::session sql(soci::sqlite3, DB_FILE);
      std::ofstream file_log(SQL_LOG_FILE);
      sql.set_log_stream(&file_log);
      std::filebuf fb;
      std::string json_file;

      if(Options::get_instance().info()) {
        if(Options::get_instance().json_files().size()) {
          json_file = Options::get_instance().json_files().at(0);
          if(fb.open(json_file, std::ios::in)) {
            std::istream is(&fb);
            info(sql, Parser(is));
            fb.close();
          }
        } else {
          info(sql);
        }
      } else {
        greeting();

        db_init(sql);

        if(Options::get_instance().json_files().size()) {
          json_file = Options::get_instance().json_files().at(0);
          if(fb.open(json_file, std::ios::in)) {
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
        }
      }
    }
  } catch (const std::exception& exception) {
    log::write(log::level::fatal, "\n[Error] %s\n\n", exception.what());
    result = EXIT_FAILURE;
  }

  return result;
}

