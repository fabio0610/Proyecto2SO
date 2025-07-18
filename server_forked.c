// server_forked.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "utils.h"

#define PORT DEFAULT_PORT

void run_forked_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Ignorar hijos muertos para evitar zombies
    signal(SIGCHLD, SIG_IGN);

    // Crear socket
    server_socket = create_server_socket(PORT);
    if (server_socket < 0) {
    exit(EXIT_FAILURE);
    }

    printf("🧬 Servidor Forked escuchando en el puerto %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            // Proceso hijo
            close(server_socket);

            char buffer[BUFFER_SIZE];
            int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (received > 0) {
                buffer[received] = '\0';
                printf("📥 [Fork] Solicitud:\n%s\n", buffer);
                process_http_request(client_socket, buffer);
            }

            close(client_socket);
            exit(0);
        } else if (pid > 0) {
            // Proceso padre
            close(client_socket);
        } else {
            perror("Error en fork");
            close(client_socket);
        }
    }

    close(server_socket);
}
