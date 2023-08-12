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
        fprintf(stderr, "Usage: %s <server IP> <server port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        error_exit("Socket creation failed");
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) {
        error_exit("Invalid IP address");
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        error_exit("Connection failed");
    }

    char command[BUFFER_SIZE];

    while (1) {
        printf("Enter command: ");
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            error_exit("Input error");
        }
        command[strcspn(command, "\n")] = '\0'; // Remove newline

        if (send(clientSocket, command, strlen(command), 0) == -1) {
            error_exit("Command send failed");
        }

        char result[BUFFER_SIZE];
        memset(result, 0, sizeof(result));

        if (recv(clientSocket, result, sizeof(result), 0) == -1) {
            error_exit("Result receive failed");
        }

        printf("Result:\n%s\n", result);
    }

    close(clientSocket);

    return 0;
}
