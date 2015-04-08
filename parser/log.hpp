#pragma once

#include <cstdarg>
#include <cstdio>


/**
 * @class log
 *
 * @brief Class for basic logging to standard output.
 */
class log
{
  public:
    enum class level
        { all=0, verbose, debug, info, warning, error, fatal, none };

    static void write(log::level loglevel,
        const char *format, ...) noexcept;
    static void setLogLevel(log::level loglevel) noexcept;

  protected:
    static void checkInit() noexcept;
    static void init() noexcept;

  private:
    log() = delete;
    ~log() = delete;
    log& operator= (const log&) = delete;
    log& operator= (const log&&) = delete;

    static bool initialised_;
    static log::level loglevel_;
};

