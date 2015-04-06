#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE

#define BOOST_TEST_MODULE basic
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "config.hpp"


BOOST_AUTO_TEST_SUITE( files_exists )

BOOST_AUTO_TEST_CASE( test_prog_run )
{
#ifdef __linux__
  BOOST_CHECK(std::system((test_prog + " > /dev/null").c_str())
      == EXIT_SUCCESS);
#else
  BOOST_CHECK(std::system(test_prog) == EXIT_SUCCESS);
#endif
}

BOOST_AUTO_TEST_CASE( exists_json )
{
  BOOST_REQUIRE( boost::filesystem::exists(test_json));
  BOOST_REQUIRE(!boost::filesystem::is_directory(test_json));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( basic_db_interaction )

BOOST_AUTO_TEST_CASE( add_datasets )
{
#ifdef __linux__
  BOOST_CHECK(std::system((test_prog + " " + test_json + " "
      + " > /dev/null").c_str()) == EXIT_SUCCESS);
#else
  BOOST_CHECK(std::system((test_prog + " " + test_json).c_str()) ==
      EXIT_SUCCESS);
#endif
}

BOOST_AUTO_TEST_CASE( exists_db )
{
  BOOST_REQUIRE( boost::filesystem::exists(test_db));
  BOOST_REQUIRE(!boost::filesystem::is_directory(test_db));
}

BOOST_AUTO_TEST_SUITE_END()

