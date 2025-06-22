#include <stdio.h>
#include <stdlib.h>  // para atoi(), exit()
#include "utils.h"   // para las funciones run_*

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <modo> [k]\n", argv[0]);
        exit(1);
    }

    int modo = atoi(argv[1]);
    int k = argc >= 3 ? atoi(argv[2]) : 0;

    switch (modo) {
        case 1: run_fifo_server(); break;
        case 2: run_threaded_server(); break;
        case 3: run_prethreaded_server(k); break;
        case 4: run_forked_server(); break;
        case 5: run_preforked_server(k); break;
        default:
            fprintf(stderr, "Modo inválido. Usa 1-5\n");
            exit(1);
    }

    return 0;
}
