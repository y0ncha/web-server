#include "server.h"

Server::Server() {
    WSAData wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "Web Server: Error at WSAStartup()\n";
        listenSocket = INVALID_SOCKET;
        return;
    }
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listenSocket) {
        cout << "Web Server: Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }
    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(SERVER_PORT);
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService))) {
        cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        listenSocket = INVALID_SOCKET;
        return;
    }
    if (SOCKET_ERROR == listen(listenSocket, 5)) {
        cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        listenSocket = INVALID_SOCKET;
        return;
    }
    addSocket(listenSocket, SocketType::Listen);
}

Server::~Server() {
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    WSACleanup();
    cout << "Web Server: Closing Connection.\n";
}

bool Server::addSocket(SOCKET id, SocketType what) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (sockets[i].recv == SocketType::Empty) {
            sockets[i].id = id;
            sockets[i].recv = what;
            sockets[i].send = SocketType::Idle;
            sockets[i].sendSubType = SendSubType::None;
            sockets[i].buffer.clear();
            socketsCount++;
            return true;
        }
    }
    return false;
}

void Server::removeSocket(int index) {
    sockets[index].recv = SocketType::Empty;
    sockets[index].send = SocketType::Empty;
    sockets[index].sendSubType = SendSubType::None;
    sockets[index].buffer.clear();
    socketsCount--;
}

void Server::acceptConnection(int index) {
    SOCKET id = sockets[index].id;
    sockaddr_in from;
    int fromLen = sizeof(from);
    SOCKET msgSocket = accept(id, (sockaddr*)&from, &fromLen);
    if (INVALID_SOCKET == msgSocket) {
        cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
        return;
    }
    cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;
    unsigned long flag = 1;
    if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
        cout << "Web Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
    }
    if (!addSocket(msgSocket, SocketType::Receive)) {
        cout << "\t\tToo many connections, dropped!\n";
        closesocket(id);
    }
}

void Server::receiveMessage(int index) {
    SOCKET msgSocket = sockets[index].id;
    char tempBuff[128] = {0};
    int bytesRecv = recv(msgSocket, tempBuff, sizeof(tempBuff) - 1, 0);
    if (SOCKET_ERROR == bytesRecv) {
        cout << "Web Server: Error at recv(): " << WSAGetLastError() << endl;
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    if (bytesRecv == 0) {
        closesocket(msgSocket);
        removeSocket(index);
        return;
    } else {
        // Placeholder: Handle incoming HTTP request here
        tempBuff[bytesRecv] = '\0';
        sockets[index].buffer += string(tempBuff, bytesRecv);
        cout << "Web Server: Received: " << bytesRecv << " bytes of \"" << tempBuff << "\" message.\n";
        // Example: Parse HTTP request, set up response, etc.
    }
}

void Server::sendMessage(int index) {
    SOCKET msgSocket = sockets[index].id;
    // Placeholder: Send HTTP response here
    string sendBuff;
    // Example: sendBuff = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
    int bytesSent = send(msgSocket, sendBuff.c_str(), (int)sendBuff.size(), 0);
    if (SOCKET_ERROR == bytesSent) {
        cout << "Web Server: Error at send(): " << WSAGetLastError() << endl;
        return;
    }
    cout << "Web Server: Sent: " << bytesSent << "\\" << sendBuff.size() << " bytes of response.\n";
    sockets[index].send = SocketType::Idle;
    sockets[index].sendSubType = SendSubType::None;
    sockets[index].buffer.clear();
}

void Server::run() {
    if (listenSocket == INVALID_SOCKET) {
        cout << "Web Server: Initialization failed.\n";
        return;
    }
	cout << "Server is running, waiting for connections...\n";
    while (true) {
        fd_set waitRecv;
        FD_ZERO(&waitRecv);
        for (int i = 0; i < MAX_SOCKETS; i++) {
            if (sockets[i].recv == SocketType::Listen || sockets[i].recv == SocketType::Receive)
                FD_SET(sockets[i].id, &waitRecv);
        }
        fd_set waitSend;
        FD_ZERO(&waitSend);
        for (int i = 0; i < MAX_SOCKETS; i++) {
            if (sockets[i].send == SocketType::Send)
                FD_SET(sockets[i].id, &waitSend);
        }
        int nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
        if (nfd == SOCKET_ERROR) {
            cout << "Web Server: Error at select(): " << WSAGetLastError() << endl;
            return;
        }
        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++) {
            if (FD_ISSET(sockets[i].id, &waitRecv)) {
                nfd--;
                switch (sockets[i].recv) {
                case SocketType::Listen:
                    acceptConnection(i);
                    break;
                case SocketType::Receive:
                    receiveMessage(i);
                    break;
                default:
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++) {
            if (FD_ISSET(sockets[i].id, &waitSend)) {
                nfd--;
                switch (sockets[i].send) {
                case SocketType::Send:
                    sendMessage(i);
                    break;
                default:
                    break;
                }
            }
        }
    }
}
