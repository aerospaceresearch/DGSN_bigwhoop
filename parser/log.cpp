#include "main.hpp"

#include "log.hpp"

bool log::initialised_;
int  log::loglevel_;

void log::write(int loglevel, const char *format, ...) noexcept
{
    checkInit();
    if (loglevel >= loglevel_){
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void log::setLogLevel(int loglevel) noexcept
{
    loglevel_ = loglevel;
    initialised_ = true;
}

void log::checkInit() noexcept
{
    if(!initialised_){
        init();
    }
}

void log::init() noexcept
{
    setLogLevel(LOG_LEVEL);
}

