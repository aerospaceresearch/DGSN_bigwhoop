#include "main.hpp"

#include "log.hpp"

bool log::m_bInitialised;
int  log::m_nLevel;

void log::write(int nLevel, const char *szFormat, ...) noexcept
{
    checkInit();
    if (nLevel >= m_nLevel){
        va_list args;
        va_start(args, szFormat);
        vprintf(szFormat, args);
        va_end(args);
    }
}

void log::setLevel(int nLevel) noexcept
{
    m_nLevel = nLevel;
    m_bInitialised = true;
}

void log::checkInit() noexcept
{
    if(!m_bInitialised){
        init();
    }
}

void log::init() noexcept
{
    setLevel(LOG_LEVEL);
}

