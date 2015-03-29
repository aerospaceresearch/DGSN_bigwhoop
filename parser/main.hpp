#pragma once

#include "log.hpp"


/*
 * @brief Version
 */
constexpr const unsigned int VERSION_MAJOR    = 0;
constexpr const unsigned int VERSION_MINOR    = 0;
constexpr const unsigned int VERSION_REVISION = 0;

/*
 * @brief File that is used for the database.
 */
constexpr const char* const DB_FILE = "database.db";

/*
 * @brief Log level for log output.
 */
constexpr const unsigned int LOG_LEVEL = log::all;

