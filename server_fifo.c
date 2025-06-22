// server_fifo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utils.h"

#define PORT DEFAULT_PORT 

void run_fifo_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_size = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    // Crear socket
    server_socket = create_server_socket(PORT);
    if (server_socket < 0) {
    exit(EXIT_FAILURE);
    }

    // Configurar dirección
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Escuchar
    if (listen(server_socket, 10) < 0) {
        perror("Error en listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("🟢 Servidor FIFO escuchando en puerto %d...\n", PORT);

    // Bucle principal: un cliente a la vez
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&server_addr, &addr_size);
        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }

        int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (received <= 0) {
            close(client_socket);
            continue;
        }

        buffer[received] = '\0';  // Asegura terminación
        printf("📥 Solicitud:\n%s\n", buffer);

        process_http_request(client_socket, buffer);

        close(client_socket);  // Solo uno a la vez
    }

    close(server_socket);
}
