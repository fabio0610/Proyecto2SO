// server_preforked.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "utils.h"

#define PORT 8080
#define BUFFER_SIZE 4096

void child_process_loop(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Error en accept (hijo)");
            continue;
        }

        int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            printf("📥 [Pre-Fork] Solicitud:\n%s\n", buffer);
            process_http_request(client_socket, buffer);
        }

        close(client_socket);
    }
}

void run_preforked_server(int k) {
    int server_socket;
    struct sockaddr_in server_addr;

    // Ignorar hijos muertos
    signal(SIGCHLD, SIG_IGN);

    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creando socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 50) < 0) {
        perror("Error en listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("👨‍👧‍👦 Servidor Pre-Forked escuchando en el puerto %d con %d procesos...\n", PORT, k);

    for (int i = 0; i < k; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Proceso hijo
            child_process_loop(server_socket);
            exit(0); // Por seguridad
        } else if (pid < 0) {
            perror("Error en fork");
        }
    }

    // El padre solo espera (o podrías usar pause())
    while (1) {
        sleep(60);
    }

    close(server_socket);
}
