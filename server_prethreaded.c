// server_prethreaded.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <semaphore.h>
#include "utils.h"

#define PORT DEFAULT_PORT
#define MAX_QUEUE 100

// Cola circular de clientes
int client_queue[MAX_QUEUE];
int front = 0, rear = 0, count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_clients;

void enqueue_client(int client_socket) {
    pthread_mutex_lock(&mutex);
    client_queue[rear] = client_socket;
    rear = (rear + 1) % MAX_QUEUE;
    count++;
    pthread_mutex_unlock(&mutex);
    sem_post(&sem_clients);
}

int dequeue_client() {
    sem_wait(&sem_clients);
    pthread_mutex_lock(&mutex);
    int client_socket = client_queue[front];
    front = (front + 1) % MAX_QUEUE;
    count--;
    pthread_mutex_unlock(&mutex);
    return client_socket;
}

void *thread_worker(void *arg) {
    while (1) {
        int client_socket = dequeue_client();

        char buffer[BUFFER_SIZE];
        int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            printf("📥 [Worker] Solicitud:\n%s\n", buffer);
            process_http_request(client_socket, buffer);
        }

        close(client_socket);
    }
    return NULL;
}

void run_prethreaded_server(int k) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    sem_init(&sem_clients, 0, 0);

    // Crear socket
    server_socket = create_server_socket(PORT);
    if (server_socket < 0) {
        exit(EXIT_FAILURE);
    }

    printf("👷 Servidor Pre-Threaded escuchando en el puerto %d con %d hilos...\n", PORT, k);

    // Crear pool de hilos
    pthread_t threads[k];
    for (int i = 0; i < k; i++) {
        pthread_create(&threads[i], NULL, thread_worker, NULL);
    }

    // Bucle de aceptación
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }

        enqueue_client(client_socket);  // Se encola para ser procesado por los hilos
    }

    close(server_socket);
}
