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
    ("help", "print help message")
    ("version", "print version")
    ("no-db-checks", "skip database checks")
    ("no-remove-duplicates", "do not remove duplicate entries")
    ("info-only", "print information about JSON file and current database")
    ("quiet", "quiet output")
    ("verbose", "verbose output")
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
  if(vm_.count("input-file")) {
    json_files_ = vm_["input-file"].as<std::vector<std::string>>();
    for(std::vector<std::string>::iterator it = json_files_.begin();
        it != json_files_.end(); ++it) {
      if(get_ext(*it) != "json") {
        json_files_.erase(it);
      }
    }
    if(json_files_.size() < 1) {
      log::write(log::level::info, "Usage: ./parser <file.json>\n");
      return false;
    }
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
  if(vm_.count("info-only")) {
    info_only_ = true;
    log::write(log::level::info,
        "[Note] --info-only currently unsupported\n");
  }
  return true;
}

