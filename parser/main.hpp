#pragma once

#include<chrono>

#include "log.hpp"


/**
 * @brief Version
 */
constexpr const unsigned int VERSION_MAJOR    = 0;
constexpr const unsigned int VERSION_MINOR    = 0;
constexpr const unsigned int VERSION_REVISION = 0;

/**
 * @brief File that is used for the database.
 */
constexpr const char* const DB_FILE = "database.db";

/**
 * @brief File that is used for logging all SQL queries.
 */
constexpr const char* const SQL_LOG_FILE = "";

/**
 * @brief Maximal length of strings to put into the database.
 */
constexpr const unsigned int MAX_STR_LENGTH = 1024u;

/**
 * @brief Log level for output.
 *
 * Possible values: log::all
 *                  log::verbose
 *                  log::debug
 *                  log::info
 *                  log::warning
 *                  log::error
 *                  log::fatal
 *                  log::none
 */
constexpr const log::level LOG_LEVEL = log::level::debug;

/**
 * @brief Unit for duration output according to different tasks.
 *
 */
using duration_unit = std::chrono::milliseconds;
constexpr const char* const duration_unit_string = "ms";

