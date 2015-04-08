#pragma once

#include <boost/program_options.hpp>
#include <string>
#include <vector>

/**
 * @class Options
 *
 * @brief Process command line options.
 */
class Options
{
  public:
    static Options& get_instance()
    {
      static Options instance;
      return instance;
    }

    void init(int argc, const char * const * const argv);
    bool process();

    bool db_checks() const { return db_checks_; }
    bool remove_duplicates() const { return remove_duplicates_; }
    bool info_only() const { return info_only_; }
    log::level loglevel() const { return loglevel_; }
    const std::vector<std::string>& json_files() const
      { return json_files_; }

  private:
    Options();
    ~Options(){};
    Options(const Options& options) = delete;
    Options(const Options&& options) = delete;
    Options& operator=(const Options& options) = delete;
    Options& operator=(const Options&& options) = delete;

    boost::program_options::positional_options_description pos_desc_;
    boost::program_options::options_description desc_;
    boost::program_options::variables_map vm_;

    log::level loglevel_ = LOG_LEVEL;
    bool db_checks_ = true;
    bool remove_duplicates_ = true;
    bool info_only_ = false;
    std::vector<std::string> json_files_;
};

