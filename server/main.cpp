//#include <istream>
//#include <fstream>
//#include <cstdlib>
#include <iostream>
#include <cstdlib>

#include "parser.hpp"
#include "server.hpp"

#include "PracticalSocket.hpp"


int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
    exit(1);
  }

  unsigned short echoServPort = atoi(argv[1]);

  try {
    TCPServerSocket servSock(echoServPort);

    for (;;) {
      HandleTCPClient(servSock.accept());
    }
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    exit(1);
  }
  // NOT REACHED

  return EXIT_SUCCESS;
}

//int main(int argc, char** argv)
//{
//  std::filebuf fb;
//  if(fb.open("test.json", std::ios::in)) {
//    std::istream is(&fb);
//    Parser parser(is);
//    fb.close();
//  }
//
//  return EXIT_SUCCESS;
//}

