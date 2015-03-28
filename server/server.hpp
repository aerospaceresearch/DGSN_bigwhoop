#pragma once

#include "PracticalSocket.hpp"


constexpr const unsigned int RCVBUFSIZE = 32;

void HandleTCPClient(TCPSocket *sock);

