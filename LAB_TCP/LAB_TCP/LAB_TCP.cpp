#include <iostream>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[ERROR] Winsock init failed\n";
        return -1;
    }

    SOCKET srvSock = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSock == INVALID_SOCKET) {
        std::cerr << "[ERROR] Socket creation failed\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10111);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(srvSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Bind failed\n";
        closesocket(srvSock);
        WSACleanup();
        return -1;
    }

    if (listen(srvSock, 5) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Listen failed\n";
        closesocket(srvSock);
        WSACleanup();
        return -1;
    }

    std::cout << "Server active on port 10111. Waiting for clients...\n";

    bool keepRunning = true;
    while (keepRunning) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET client = accept(srvSock, (sockaddr*)&clientAddr, &addrLen);
        if (client == INVALID_SOCKET) continue;

        char ipBuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, INET_ADDRSTRLEN);
        std::cout << ">>> New connection from " << ipBuf << "\n";

        char recvBuf[1024];
        bool clientActive = true;
        while (clientActive) {
            int n = recv(client, recvBuf, sizeof(recvBuf) - 1, 0);
            if (n <= 0) {
                if (n == 0) std::cout << ">>> Client disconnected\n";
                break;
            }

            recvBuf[n] = '\0';
            std::string cmd(recvBuf);
            std::cout << ">>> CMD: " << cmd << "\n";

            if (cmd == "STOP") {
                send(client, "Server stopping...", 17, 0);
                keepRunning = false;
                clientActive = false;
            }
            else {
                send(client, recvBuf, n, 0); // echo
            }
        }
        closesocket(client);
    }

    closesocket(srvSock);
    WSACleanup();
    std::cout << "Server shutdown complete.\n";
    return 0;
}