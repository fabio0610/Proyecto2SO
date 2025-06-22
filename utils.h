// utils.h
#ifndef UTILS_H
#define UTILS_H

void run_fifo_server();
void process_http_request(int client_socket, const char *request);
void run_threaded_server();

#endif
