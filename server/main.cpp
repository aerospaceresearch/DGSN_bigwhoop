#include <istream>
#include <fstream>
#include <cstdlib>

#include "parser.hpp"


int main(int argc, char** argv)
{
  std::filebuf fb;
  if(fb.open("test.json", std::ios::in)) {
    std::istream is(&fb);
    Parser parser(is);
    fb.close();
  }

  return EXIT_SUCCESS;
}

