#include <iostream>
using namespace std;

#define _WINDSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

int main() {

	unsigned long ulAddress;

	inet_pton(AF_INET, "127.0.0.1", &ulAddress);

	cout << "Straight up:" << ulAddress << endl;
	cout << "Hex version:" << hex << ulAddress << endl;

	return 0;
}