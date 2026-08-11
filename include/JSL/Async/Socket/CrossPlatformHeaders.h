#pragma once
#if defined(_WIN32) || defined(_WIN64)
#define WINMODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#pragma comment(lib, "Ws2_32.lib")
// clang-format on

// Windows uses SOCKET (unsigned long long) instead of int for descriptors
using socket_t = SOCKET;
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define CONNECTION_REFUSED (WSAGetLastError() == WSAECONNREFUSED)
#define CLOSE closesocket
#else
#define POSIXMODE
#include <sys/socket.h>
#include <sys/un.h>
using socket_t = int;
#define INVALID_SOCKET_VAL (-1)
#define CLOSE close
#define CONNECTION_REFUSED (errno == ECONNREFUSED)
#endif
