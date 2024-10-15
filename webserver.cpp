#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <winsock2.h>
#include "GlobalValues.h"
#include "GlobalMutex.h"
#include "Connection.cpp"

//this did not help link the required packages
//instead i needed to add arguments while compiling for proper linking
#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int PORT = 6789;

std::vector<User*> allclients {};
std::vector<Group*> allgroups {};
std::mutex gmutex;

void handleClient(SOCKET clientSocket){
    Connection clientConnection = Connection(clientSocket);
    clientConnection.run();
}


int main(){
    //initialize groups
    for(int i=0; i<6; i++){
        allgroups.push_back(new Group(i));
    }
    WSADATA wsa;
    SOCKET serverSocket;
    sockaddr_in serveraddr;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    if (bind(serverSocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while(true){
        SOCKET clientSocket; 
        if ((clientSocket = accept(serverSocket, nullptr, nullptr)) == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

