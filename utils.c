// utils.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "utils.h"

#define RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 - Recurso no encontrado 🧨</h1>"
#define RESPONSE_200_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define RESOURCE_DIR "./resources/"

void process_http_request(int client_socket, const char *request) {
    char method[8], path[256];
    sscanf(request, "%s %s", method, path);

    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
        write(client_socket, RESPONSE_404, strlen(RESPONSE_404));
        return;
    }

    // Construir ruta al archivo
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", RESOURCE_DIR, path[1] ? path + 1 : "index.html");

    // Abrir archivo solicitado
    FILE *file = fopen(full_path, "r");
    if (!file) {
        write(client_socket, RESPONSE_404, strlen(RESPONSE_404));
        return;
    }

    write(client_socket, RESPONSE_200_HEADER, strlen(RESPONSE_200_HEADER));

    char c;
    while ((c = fgetc(file)) != EOF)
        write(client_socket, &c, 1);

    fclose(file);
}
