#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <cstdlib>
#pragma comment(lib, "ws2_32.lib")

int main()
{
    const char* HOST = "127.0.0.1";
    const int PORT = 10111;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[CLIENT ERROR] Winsock init failed\n";
        std::system("pause");
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[CLIENT ERROR] Socket creation failed\n";
        WSACleanup();
        std::system("pause");
        return -1;
    }

    // Тайм-аут 20 секунд
    DWORD timeoutMs = 20000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeoutMs, sizeof(timeoutMs));

    sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &srvAddr.sin_addr);

    if (connect(sock, (sockaddr*)&srvAddr, sizeof(srvAddr)) != 0) {
        std::cerr << "[CLIENT ERROR] Connection to server failed\n";
        closesocket(sock);
        WSACleanup();
        std::system("pause");
        return -1;
    }

    std::cout << "Connected to server. Type 'BYE' to exit.\n";

    std::string input;
    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        send(sock, input.c_str(), (int)input.size(), 0);

        if (input == "BYE") break;

        char buf[1024] = { 0 };
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            std::cout << "[SERVER] " << std::string(buf, n) << "\n";
        }
        else if (n == 0) {
            std::cout << "[SERVER] Closed connection\n";
            break;
        }
        else if (WSAGetLastError() == WSAETIMEDOUT) {
            std::cout << "[TIMEOUT] No reply in 20 sec. Exiting.\n";
            break;
        }
        else {
            std::cerr << "[RECV ERROR] Code: " << WSAGetLastError() << "\n";
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    std::cout << "Client closed.\n";
    std::system("pause");
    return 0;
}