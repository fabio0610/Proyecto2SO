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

#define PORT DEFAULT_PORT

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
    server_socket = create_server_socket(PORT);
    if (server_socket < 0) {
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
        pause();
    }

    close(server_socket);
}
