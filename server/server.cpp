#include <iostream>
#include <cstdlib>

#include "server.hpp"


// TCP client handling function
void HandleTCPClient(TCPSocket *sock) {
  std::cout << "Handling client ";
  try {
    std::cout << sock->getForeignAddress() << ":";
  } catch (SocketException e) {
    cerr << "Unable to get foreign address" << std::endl;
  }
  try {
    std::cout << sock->getForeignPort();
  } catch (SocketException e) {
    cerr << "Unable to get foreign port" << std::endl;
  }
  std::cout << std::endl;

  // Send received string and receive again until the end of transmission
  char echoBuffer[RCVBUFSIZE];
  int recvMsgSize;
  // sock->recv() == 0 for end of transmission
  while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0) {
    sock->send(echoBuffer, recvMsgSize);
  }
  delete sock;
}

