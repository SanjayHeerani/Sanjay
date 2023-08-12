#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void error_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <bind IP> <bind port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Create a server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        error_exit("Socket creation failed");
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) {
        error_exit("Invalid IP address");
    }

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        error_exit("Binding failed");
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == -1) {
        error_exit("Listening failed");
    }

    printf("Server is listening on %s:%s\n", argv[1], argv[2]);

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    // Accept a connection from a client
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        error_exit("Accept failed");
    }

    char command[BUFFER_SIZE];
    char result[BUFFER_SIZE];

    while (1) {
        // Clear the command and result buffers
        memset(command, 0, sizeof(command));
        memset(result, 0, sizeof(result));

        // Receive the command from the client
        ssize_t bytesReceived = recv(clientSocket, command, sizeof(command), 0);
        if (bytesReceived == -1) {
            error_exit("Command receive failed");
        } else if (bytesReceived == 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Execute the command using popen
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            strcpy(result, "Command execution failed");
        } else {
            // Read the output of the command
            while (fgets(result, sizeof(result), fp) != NULL) {
                // Send the result back to the client
                if (send(clientSocket, result, strlen(result), 0) == -1) {
                    error_exit("Result send failed");
                }
                memset(result, 0, sizeof(result));
            }
            pclose(fp);
        }

        // Send an empty result to signal the end of the command output
        if (send(clientSocket, "", 1, 0) == -1) {
            error_exit("Result send failed");
        }
    }

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
