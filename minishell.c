#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

pid_t pid;  // Variable global para almacenar el PID del hijo

// Manejador de señal
void signal_handler(int sig) {
    if (pid == 0) {
        printf("\nSeñal recibida: %d\n", sig);
        if (sig == SIGINT || sig == SIGTERM) {
            printf("Saliendo del programa debido a la señal.\n");
            exit(0);
        }
    }
}

void prompt() {
    printf("[prompt]$ ");
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    int status;

    // Asociar el manejador de señal con SIGINT y SIGTERM
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while (1) {
        // Mostrar el prompt
        prompt();

        // Leer la línea de comando
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error al leer la entrada\n");
            continue;
        }

        // Eliminar el salto de línea final
        input[strcspn(input, "\n")] = 0;

        // Si el comando es "Salir", termina el programa
        if (strcmp(input, "Salir") == 0) {
            printf("Programa finalizado.\n");
            return 0;
            break;
        }

        // Separar los argumentos del comando
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            args[++i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Crear un nuevo proceso
        pid = fork();
        if (pid < 0) {
            printf("Error al crear el proceso\n");
            continue;
        } 
        if (pid == 0) {
            // Proceso hijo: ejecutar el comando
            if (execvp(args[0], args) == -1) {
                printf("Error al ejecutar el comando: %s\n", args[0]);
                return 1;  // Terminamos solo el proceso hijo con código de error
            }
        } else {
            
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                int status_salida = WEXITSTATUS(status);
                printf("El proceso hijo terminó normalmente con estado %d\n", status_salida);
            } else if (WIFSIGNALED(status)) {
                int id_señal = WTERMSIG(status);
                printf("El proceso hijo terminó anormalmente por señal %d\n", id_señal);
            }
        }
    }

    return 0;
}
