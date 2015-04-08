#include "log.hpp"
#include "main.hpp"

#include "log.hpp"
#include "options.hpp"


bool log::initialised_;
log::level log::loglevel_;

void log::write(log::level loglevel, const char *format, ...) noexcept
{
  checkInit();
  if (static_cast<unsigned int>(loglevel) >=
      static_cast<unsigned int>(loglevel_)){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
}

void log::setLogLevel(log::level loglevel) noexcept
{
  loglevel_ = loglevel;
}

void log::checkInit() noexcept
{
  if(!initialised_){
    init();
    initialised_ = true;
  }
}

void log::init() noexcept
{
  setLogLevel(Options::get_instance().loglevel());
}

