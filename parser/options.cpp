#include <iostream>

#include "main.hpp"
#include "options.hpp"


/*
 * @brief Get file extension.
 */
std::string get_ext(const std::string& path) noexcept
{
   size_t period = path.find_last_of(".");
   std::string ext = path.substr(period + 1);
   return ext;
}

Options::Options()
  : desc_("Options")
{
}

void Options::init(int argc, const char * const * const argv)
{
  namespace bpo = boost::program_options;

  desc_.add_options()
    ("help,h", "print help message")
    ("version", "print version")
    ("no-db-checks,c", "skip database checks")
    ("no-remove-duplicates,d", "do not remove duplicate entries")
    ("info,i", "print information about JSON file and current database")
    ("quiet,q", "quiet output")
    ("verbose,v", "verbose output")
    ("input-file", bpo::value<std::vector<std::string>>(),
        "JSON input file")
    ;

  pos_desc_.add("input-file", -1);

  bpo::store(bpo::command_line_parser(argc, argv).
                options(desc_).positional(pos_desc_).run(), vm_);
  bpo::notify(vm_);
}

bool Options::process()
{
  if(vm_.count("help")) {
    std::cout << desc_ << "\n";
    return false;
  }
  if(vm_.count("version")) {
    log::write(log::level::info, "DGSN BigWhoop parser v%d.%d.%d\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
    return false;
  }
  if(vm_.count("no-db-checks")) {
    db_checks_ = false;
  }
  if(vm_.count("no-remove-duplicates")) {
    remove_duplicates_ = false;
  }
  if(vm_.count("quiet")) {
    loglevel_ = log::level::error;
  }
  if(vm_.count("verbose")) {
    loglevel_ = log::level::verbose;
  }
  if(vm_.count("info")) {
    info_ = true;
    if(static_cast<unsigned int>(loglevel_) >=
       static_cast<unsigned int>(log::level::error)) {
      std::cout << "[Note] --quiet and --info provided at the same time"
        << std::endl;
      return false;
    }
  }
  if(!info_ && vm_.count("input-file")) {
    json_files_ = vm_["input-file"].as<std::vector<std::string>>();
    for(std::vector<std::string>::iterator it = json_files_.begin();
        it < json_files_.end(); ++it) {
      if(get_ext(*it) != "json") {
        log::write(log::level::info, "  [Note] '%s' ignored\n",
            (*it).c_str());
        it = json_files_.erase(it);
      }
    }
  }
  if(json_files_.size() < 1 && !info_) {
    log::write(log::level::info, "Usage: ./parser <file.json>\n");
    return false;
  }

  return true;
}

