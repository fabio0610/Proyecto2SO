// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>

// Variables constantes
#define BUFFER_SIZE 8192
#define MAX_PATH 512
#define MAX_HEADER 1024
#define RESOURCE_DIR "./resources/"
#define DEFAULT_PORT 8080

// Códigos de respuesta HTTP
#define HTTP_200 "HTTP/1.1 200 OK"
#define HTTP_404 "HTTP/1.1 404 Not Found"
#define HTTP_500 "HTTP/1.1 500 Internal Server Error"

// Funciones principales de servidores 
void run_fifo_server();
void run_threaded_server();
void run_prethreaded_server(int k);
void run_forked_server();
void run_preforked_server(int k);

// Funciones utilitarias HTTP
void process_http_request(int client_socket, const char *request);
void send_404_response(int client_socket);  // Nueva función para 404 agradable
const char* get_content_type(const char *path);  // Nueva función para MIME types dinámicos
// Funciones de socket
int create_server_socket(int port);

// Funciones de logging y debugging
void log_request(const char *method, const char *path, int status_code);

// Funciones de utilidad general
int file_exists(const char *path);
long get_file_size(const char *path);
#endif