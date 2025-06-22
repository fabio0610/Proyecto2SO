// server_threaded.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "utils.h"

#define PORT DEFAULT_PORT

void *thread_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (received > 0) {
        buffer[received] = '\0';
        printf("📥 [Thread] Solicitud:\n%s\n", buffer);
        process_http_request(client_socket, buffer);
    }

    close(client_socket);
    pthread_exit(NULL);
}

void run_threaded_server() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_socket = create_server_socket(PORT);
    if (server_socket < 0) {
        exit(EXIT_FAILURE);
    }

    printf("🧵 Servidor Threaded escuchando en el puerto %d...\n", PORT);

    while (1) {
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (*client_socket < 0) {
            perror("Error en accept");
            free(client_socket);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_handler, client_socket) != 0) {
            perror("Error creando hilo");
            close(*client_socket);
            free(client_socket);
        }

        pthread_detach(thread); // Se libera automáticamente al terminar
    }

    close(server_socket);
}
