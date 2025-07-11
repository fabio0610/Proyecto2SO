// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "utils.h"

typedef struct {
    int T;
    char recurso[256];
    char ip[100];
    int puerto;
    double *tiempos_total;
    double *esperas;
    long *bytes;
    int id;
} HiloArgs;

void *cliente_thread(void *arg) {
    HiloArgs *args = (HiloArgs *)arg;
    struct timeval start, end, request_start;

    for (int i = 0; i < args->T; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(args->puerto);
        inet_pton(AF_INET, args->ip, &server_addr.sin_addr);

        gettimeofday(&start, NULL);
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando");
            close(sock);
            continue;
        }
        gettimeofday(&end, NULL);

        double espera = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
        args->esperas[args->id * args->T + i] = espera;

        char request[512];
        snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", args->recurso, args->ip);

        gettimeofday(&request_start, NULL);
        send(sock, request, strlen(request), 0);

        char buffer[BUFFER_SIZE];
        long total_bytes = 0;
        int bytes_recibidos;

        // Contar bytes reales recibidos
        while ((bytes_recibidos = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
            total_bytes += bytes_recibidos;
        }
        
        gettimeofday(&end, NULL);

        double tiempo = (end.tv_sec - request_start.tv_sec) * 1000.0 + (end.tv_usec - request_start.tv_usec) / 1000.0;
        args->tiempos_total[args->id * args->T + i] = tiempo;
        args->bytes[args->id * args->T + i] = total_bytes;

        close(sock);
    }

    // Liberar memoria del thread
    free(args);
    pthread_exit(NULL);
}

double calcular_promedio(double *valores, int n) {
    double suma = 0;
    for (int i = 0; i < n; i++) suma += valores[i];
    return suma / n;
}

double calcular_varianza(double *valores, int n, double promedio) {
    double var = 0;
    for (int i = 0; i < n; i++) var += (valores[i] - promedio) * (valores[i] - promedio);
    return var / n;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Uso: %s T H recurso IP puerto\n", argv[0]);
        return 1;
    }

    int T = atoi(argv[1]);
    int H = atoi(argv[2]);
    int total = T * H;

    // ⏱️ TIEMPO TOTAL - Inicio
    struct timeval tiempo_inicio, tiempo_fin;
    gettimeofday(&tiempo_inicio, NULL);

    pthread_t hilos[H];
    double *tiempos = malloc(sizeof(double) * total);
    double *esperas = malloc(sizeof(double) * total);
    long *bytes = malloc(sizeof(long) * total);

    printf("🚀 Iniciando %d hilos, cada uno con %d requests (%d total)...\n", H, T, total);

    for (int i = 0; i < H; i++) {
        HiloArgs *args = malloc(sizeof(HiloArgs));
        args->T = T;
        strcpy(args->recurso, argv[3]);
        strcpy(args->ip, argv[4]);
        args->puerto = atoi(argv[5]);
        args->tiempos_total = tiempos;
        args->esperas = esperas;
        args->bytes = bytes;
        args->id = i;

        pthread_create(&hilos[i], NULL, cliente_thread, args);
    }

    for (int i = 0; i < H; i++) pthread_join(hilos[i], NULL);

    // ⏱️ TIEMPO TOTAL - Fin
    gettimeofday(&tiempo_fin, NULL);
    double tiempo_total_ms = (tiempo_fin.tv_sec - tiempo_inicio.tv_sec) * 1000.0 + 
                             (tiempo_fin.tv_usec - tiempo_inicio.tv_usec) / 1000.0;

    // Cálculos estadísticos
    double prom_tiempo = calcular_promedio(tiempos, total);
    double var_tiempo = calcular_varianza(tiempos, total, prom_tiempo);
    double prom_espera = calcular_promedio(esperas, total);
    double var_espera = calcular_varianza(esperas, total, prom_espera);
    long total_bytes = 0;
    for (int i = 0; i < total; i++) total_bytes += bytes[i];

    // Métricas adicionales
    double throughput = total / (tiempo_total_ms / 1000.0);  // requests por segundo

    printf("\n📊 ===== RESULTADOS FINALES =====\n");
    printf("Total de solicitudes: %d\n", total);
    printf("Tiempo total de ejecución: %.2f ms (%.2f segundos)\n", tiempo_total_ms, tiempo_total_ms / 1000.0);
    printf("Throughput: %.2f requests/segundo\n", throughput);
    printf("Tiempo atención promedio: %.2f ms\n", prom_tiempo);
    printf("Varianza tiempo atención: %.2f\n", var_tiempo);
    printf("Tiempo espera promedio: %.2f ms\n", prom_espera);
    printf("Varianza tiempo espera: %.2f\n", var_espera);
    printf("Bytes totales recibidos: %ld bytes (%.2f KB)\n", total_bytes, total_bytes / 1024.0);
    printf("Bytes promedio por request: %.2f bytes\n", (double)total_bytes / total);
    printf("=====================================\n");

    free(tiempos); 
    free(esperas); 
    free(bytes);
    return 0;
}