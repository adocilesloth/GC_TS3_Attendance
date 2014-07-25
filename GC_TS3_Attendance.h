#define WIN32_MEAN_AND_LEAN
#include <winsock2.h>
#include <iostream>

bool ConnectToHost(int, char*, SOCKET&);
void CloseConnection(SOCKET&);