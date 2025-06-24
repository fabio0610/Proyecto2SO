// utils.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "utils.h"

#define RESOURCE_DIR "./resources/"

// Función para detectar Content-Type dinámicamente
const char* get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    
    // Archivos de texto
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    
    // Imágenes
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".webp") == 0) return "image/webp";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    
    // Audio/Video
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".mp4") == 0) return "video/mp4";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    
    // Documentos
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";
    if (strcmp(ext, ".zip") == 0) return "application/zip";
    
    return "application/octet-stream";
}

// Nueva función para enviar respuesta 404 agradable
void send_404_response(int client_socket) {
    char error_path[512];
    snprintf(error_path, sizeof(error_path), "%s404.html", RESOURCE_DIR);
    
    // Intentar servir 404.html personalizado
    struct stat file_stat;
    if (stat(error_path, &file_stat) == 0) {
        FILE *file = fopen(error_path, "r");
        if (file) {
            char header[512];
            snprintf(header, sizeof(header), 
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: %ld\r\n\r\n", 
                     file_stat.st_size);
            write(client_socket, header, strlen(header));
            
            char buffer[8192];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(client_socket, buffer, bytes_read);
            }
            fclose(file);
            printf("📄 Sirviendo página 404 personalizada\n");
            return;
        }
    }
    
    // Fallback si no existe 404.html
    const char *fallback_404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                               "<h1>404 - Recurso no encontrado 🧨</h1>"
                               "<p>El archivo solicitado no existe en el servidor.</p>";
    write(client_socket, fallback_404, strlen(fallback_404));
    printf("📄 Sirviendo página 404 por defecto\n");
}

void process_http_request(int client_socket, const char *request) {
    char method[8], path[256];
    sscanf(request, "%s %s", method, path);

    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
        send_404_response(client_socket);
        return;
    }

    // Construir ruta al archivo
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", RESOURCE_DIR, 
             path[1] ? path + 1 : "index.html");

    // Obtener tamaño del archivo
    struct stat file_stat;
    if (stat(full_path, &file_stat) != 0) {
        printf("❌ Archivo no encontrado: %s\n", full_path);
        send_404_response(client_socket);
        return;
    }

    // Abrir archivo en modo binario (funciona para texto e imágenes)
    FILE *file = fopen(full_path, "rb");
    if (!file) {
        printf("❌ No se pudo abrir archivo: %s\n", full_path);
        send_404_response(client_socket);
        return;
    }

    // Detectar Content-Type dinámicamente
    const char *content_type = get_content_type(full_path);

    // Enviar headers con Content-Type dinámico
    char header[512];
    snprintf(header, sizeof(header), 
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n\r\n", 
             content_type, file_stat.st_size);
    write(client_socket, header, strlen(header));
    
    // Leer y enviar en bloques (funciona para cualquier tipo de archivo)
    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(client_socket, buffer, bytes_read);
    }
    
    printf("✅ Archivo servido: %s (%ld bytes) - %s\n", 
           full_path, file_stat.st_size, content_type);
    fclose(file);
}

// Función para crear socket del servidor (para reutilizar en todos los server_*.c)
int create_server_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return -1;
    }
    
    // Permitir reutilizar la dirección (importante para desarrollo)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server_socket);
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Crucial para conexiones externas
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return -1;
    }
    
    if (listen(server_socket, 50) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        return -1;
    }
    
    return server_socket;
}

// Función para logging simple (útil para debugging)
void log_request(const char *method, const char *path, int status_code) {
    printf("[REQUEST] %s %s - Status: %d\n", method, path, status_code);
    fflush(stdout);
}

// Función simple para validar si archivo existe
int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

// Función para obtener tamaño de archivo (ya usan stat en process_http_request)
long get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size;
}