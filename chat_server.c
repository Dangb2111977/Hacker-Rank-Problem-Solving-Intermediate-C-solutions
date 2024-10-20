#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int client_sockets[MAX_CLIENTS];
char client_names[MAX_CLIENTS][50];
int num_clients = 0;

DWORD WINAPI handle_client(LPVOID param)
{
    int sock = *(int *)param;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Yêu cầu tên người dùng
    char name[50];
    send(sock, "Enter your name: ", 17, 0);
    recv(sock, name, sizeof(name), 0);
    name[strcspn(name, "\n")] = 0;
    strcpy(client_names[sock], name);
    printf("%s connected\n", name);

    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytes_received] = '\0'; // Null-terminate the string
        printf("%s: %s\n", name, buffer);

        // Broadcast message to all clients
        for (int i = 0; i < num_clients; i++)
        {
            if (client_sockets[i] != sock)
            {
                send(client_sockets[i], buffer, bytes_received, 0);
            }
        }
    }

    // Thông báo khi client ngắt kết nối
    printf("%s disconnected\n", name);

    // Remove client from list and close socket
    for (int i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] == sock)
        {
            client_sockets[i] = client_sockets[num_clients - 1];
            num_clients--;
            break;
        }
    }
    closesocket(sock);
    return 0;
}

int main()
{
    WSADATA wsaData;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        perror("Socket creation failed");
        WSACleanup();
        return 1;
    }

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    listen(server_socket, 3);
    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            perror("Accept failed");
            continue;
        }

        // Add client socket to array
        client_sockets[num_clients++] = client_socket;

        // Create a new thread to handle the client
        DWORD threadID;
        CreateThread(NULL, 0, handle_client, (LPVOID)&client_socket, 0, &threadID);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}