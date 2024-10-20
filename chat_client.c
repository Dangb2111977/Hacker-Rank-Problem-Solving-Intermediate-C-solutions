#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define PORT 8080
#define BUFFER_SIZE 1024

int client_socket;

DWORD WINAPI receive_messages(LPVOID arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            printf("Connection closed by server.\n");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
    }
    return 0;
}

int main()
{
    WSADATA wsaData;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        perror("Socket creation failed");
        WSACleanup();
        return 1;
    }

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Start thread to receive messages
    DWORD threadID;
    CreateThread(NULL, 0, receive_messages, NULL, 0, &threadID);

    char message[BUFFER_SIZE];
    while (1)
    {
        printf("You: ");
        fgets(message, sizeof(message), stdin);
        send(client_socket, message, strlen(message), 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}