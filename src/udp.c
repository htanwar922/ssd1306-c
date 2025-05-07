
#define PORT 12345

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "udp.h"
#include "layout.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close_socket closesocket
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define close_socket close
    #define SOCKET_ERROR -1
    #define INVALID_SOCKET -1
#endif

static SOCKET sockfd;
static struct sockaddr_in server_addr;

#ifdef _WIN32
static WSADATA wsa;
#endif

bool udp_init() {
    // Initialize the socket
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return false;
    }
#endif

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror("Socket creation failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close_socket(sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    return true;
}

bool udp_send(const uint8_t *data, size_t len) {
    printf("[UDP] Sending %s: ", data[1] == 0x00 ? "command" : "data");
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    if (sendto(sockfd, data, len, 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Send failed");
        return false;
    }
    return true;
}

bool udp_close() {
    close_socket(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return true;
}
