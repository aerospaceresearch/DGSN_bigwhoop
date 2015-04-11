#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

#include <boost/regex.hpp>
#include <chrono>
#include <exception>
#include <string>
#include <cstring>

#include "log.hpp"
#include "main.hpp"
#include "parser.hpp"

/**
 * @brief Overload operator << for a nicer syntax.
 */
soci::session& operator<<(soci::session& sql, const Parser& p)
{
  p.query(sql);
  return sql;
}

/**
 * @brief Construct the Parser object.
 */
Parser::Parser(std::istream& input)
{
  parse(input);
}

/**
 * @brief Parse input file as JSON.
 */
void Parser::parse(std::istream& input)
{
  log::write(log::level::debug, "  parsing JSON object …");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  input >> root_;
  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::level::debug, " done [%d%s]\n", duration,
      duration_unit_string);
}

/**
 * @brief Fill the database with the entries from the JSON file.
 */
void Parser::query(soci::session& sql) const
{
  log::write(log::level::debug, "  updating database …\n");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  {
    constexpr const char* const scan_mode = "analyze_full_spectrum_basic";
    const Json::Value& datasets = root_["data"]["dataset"][scan_mode];
    size_t count_datasets = datasets.size();

    static_assert(std::strlen(scan_mode) <= MAX_STR_LENGTH,
        "Scan mode description is too long");

    log::write(log::level::verbose, "    update %u datasets …",
        count_datasets);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    const std::string wu_id
      = root_["data"].get("workunitid", "<no entry>").asString();
    const Json::Value& metadata = root_["meta"];

    const std::string client_id_hash
      = metadata["client"].get("id", "<no entry>").asString();
    const std::string client_name
      = metadata["client"].get("name", "<no entry>").asString();
    const int sensor_id
      = metadata["client"]["sensor"].get("id", 0).asInt();
    const std::string sensor_name
      = metadata["client"]["sensor"].get("name", "<no entry>").asString();
    const int sensor_ppm
      = metadata["client"]["sensor"].get("ppm", 0).asInt();
    const std::string sensor_antenna
      = metadata["client"]["sensor"]
      .get("antenna", "<no entry>").asString();
    const std::string url
      = metadata["client"].get("url", "<no entry>").asString();

    const std::string sw_bit
      = metadata["sw"].get("bit", "<no entry>").asString();
    const std::string sw_os
      = metadata["sw"].get("os", "<no entry>").asString();
    const unsigned int sw_version
      = metadata["sw"].get("version", 0u).asUInt();

    if(sw_version > VERSION_MAJOR) {
      throw std::runtime_error("Version mismatch");
    }

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

    if(client_id_hash.length() != 56u) {
      throw std::runtime_error("Id hash has wrong size");
    }
    if(client_name.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Client name is too long");
    }
    if(sensor_name.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Sensor name is too long");
    }
    if(sensor_antenna.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Sensor antenna description is too long");
    }
    if(wu_id.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Workunit ID is too long");
    }
    if(sw_bit.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Bit description is too long");
    }
    if(sw_os.length() > MAX_STR_LENGTH) {
      throw std::runtime_error("Operating System description is too long");
    }

    for(Json::Value::ArrayIndex data_index = 0;
        data_index < count_datasets; ++data_index) {
      const float location_alt
        = datasets[data_index].get("alt", 0.0f).asFloat();
      const float location_lat
        = datasets[data_index].get("lat", 0.0f).asFloat();
      const float location_lon
        = datasets[data_index].get("lon", 0.0f).asFloat();
      const unsigned int freq
        = datasets[data_index].get("frequency", 0u).asUInt();
      const double time
        = datasets[data_index].get("timestamp", 0.0f).asDouble();
      const double amp_max
        = datasets[data_index].get("max_amplitude", 0.0).asDouble();
      const double amp_mean
        = datasets[data_index].get("mean_amplitude", 0.0).asDouble();

      // TODO: Sanity checks:
      //  • client_id_hash is hex
      if(time < 0.0f) {
        throw std::runtime_error("Timestamp is negative");
      }
      // TODO: Update reference timestamp at first release
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
      if(amp_max < 0) {
        throw std::runtime_error("Maximum amplitude is negative");
      }
      if(amp_mean < 0) {
        throw std::runtime_error("Mean amplitude is negative");
      }
      if(amp_mean > amp_max) {
        throw std::runtime_error(
            "Mean amplitude is greater than maximum");
      }

      sql << "INSERT INTO data ("
             "time, "
             "freq, "
             "amp_max, "
             "amp_mean, "
             "scan_mode, "
             "location_alt, "
             "location_lat, "
             "location_lon, "
             "client_id_hash, "
             "client_name, "
             "sensor_id, "
             "sensor_name, "
             "sensor_antenna, "
             "sensor_ppm, "
             "wu_id, "
             "url, "
             "sw_bit, "
             "sw_os, "
             "sw_v_major, "
             "sw_v_minor, "
             "sw_v_revision) "
             "VALUES ("
             << std::fixed
             << time << ", "
             << freq << ", "
             << amp_max << ", "
             << amp_mean << ", "
             << '\'' << scan_mode << '\'' << ", "
             << location_alt << ", "
             << location_lat << ", "
             << location_lon << ", "
             << '\'' << client_id_hash << '\'' << ", "
             << '\'' << client_name << '\'' << ", "
             << sensor_id << ", "
             << '\'' << sensor_name << '\'' << ", "
             << '\'' << sensor_antenna << '\'' << ", "
             << sensor_ppm << ", "
             << '\'' << wu_id << '\'' << ", "
             << '\'' << url << '\'' << ", "
             << '\'' << sw_bit << '\'' << ", "
             << '\'' << sw_os << '\'' << ", "
             << '\'' << sw_version << '\'' << ", "
             << '\'' << sw_version << '\'' << ", "
             << '\'' << sw_version << '\'' << ");";
    }
    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::verbose, " done [%d%s]\n", duration,
        duration_unit_string);
  }
  {
    // TODO: Insert entries only once
    constexpr const char* const scan_mode = "analyze_adsb";
    const Json::Value& datasets = root_["data"]["dataset"][scan_mode];
    size_t count_datasets = datasets.size();
    log::write(log::level::verbose, "    update %u adsb datasets …",
        count_datasets);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    for(Json::Value::ArrayIndex data_index = 0;
        data_index < count_datasets; ++data_index) {
      const float location_alt
        = datasets[data_index].get("alt", 0.0f).asFloat();
      const float location_lat
        = datasets[data_index].get("lat", 0.0f).asFloat();
      const float location_lon
        = datasets[data_index].get("lon", 0.0f).asFloat();
      const double time
        = datasets[data_index].get("timestamp", 0.0f).asDouble();

      // TODO: Sanity checks:
      //       • amp_mean <= amp_max
      //       • timestamp > 0

      sql << "INSERT INTO adsb (time, "
             "location_alt, location_lat, location_lon) "
             "VALUES ("
             << time << ", "
             << location_alt << ", "
             << location_lat << ", "
             << location_lon << ");";
    }
    end = std::chrono::system_clock::now();
    unsigned long duration
      = std::chrono::duration_cast<duration_unit>(end-start).count();
    log::write(log::level::verbose, " done [%d%s]\n", duration,
        duration_unit_string);
  }
  end = std::chrono::system_clock::now();
  unsigned long duration
    = std::chrono::duration_cast<duration_unit>(end-start).count();
  log::write(log::level::debug, "  database updated [%d%s]\n", duration,
      duration_unit_string);
}

void Parser::info() const
{
  log::write(log::level::info, "===== input data =====\n");
  if(!root_.empty()) {
    {
      constexpr const char* const scan_mode
        = "analyze_full_spectrum_basic";
      const Json::Value& datasets = root_["data"]["dataset"][scan_mode];
      size_t count_datasets = datasets.size();
      log::write(log::level::info, "  basic datasets:\t%u\n",
          count_datasets);
    }
    {
      constexpr const char* const scan_mode = "analyze_adsb";
      const Json::Value& datasets = root_["data"]["dataset"][scan_mode];
      size_t count_datasets = datasets.size();
      log::write(log::level::info, "  ADSB datasets:\t%u\n",
          count_datasets);
    }
  } else {
    log::write(log::level::info, "(empty)");
  }
}
