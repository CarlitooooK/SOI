#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

pid_t pid1, pid2;  
char *archivo = "var_compartida.txt";

// Función para escribir la variable en el archivo
void escribir_var(int valor) {
    FILE *file = fopen(archivo, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo para escribir");
        exit(1);
    }
    fprintf(file, "%d", valor);
    fclose(file);
}

// Función para leer la variable del archivo
int leer_var() {
    FILE *file = fopen(archivo, "r");
    int valor;
    if (file == NULL) {
        perror("Error al abrir el archivo para leer");
        exit(1);
    }
    fscanf(file, "%d", &valor);
    fclose(file);
    return valor;
}

// Handler para SIGALRM, usado por el padre para matar a los hijos
void manejador_alarma(int sig) {
    printf("Padre: Tiempo agotado. Matando a los hijos...\n");
    kill(pid1, SIGTERM);  // Matar al hijo 1
    kill(pid2, SIGTERM);  // Matar al hijo 2
}

// Handler de señal para recibir la señal y modificar la variable
void manejador_signal(int sig) {
    int valor = leer_var(); // Leer el valor del archivo
    valor += 1; // Modificar la variable
    printf("Proceso %d: Variable compartida modificada a %d\n", getpid(), valor);
    escribir_var(valor); // Escribir la variable modificada en el archivo
}

// Handler de señal para terminar el proceso
void manejador_terminar(int sig) {
    printf("Proceso %d: Recibí la señal SIGTERM. Terminando...\n", getpid());
    exit(0);
}

int main() {
    // Asignar handler de la alarma al padre
    signal(SIGALRM, manejador_alarma);

    // Inicializar la variable compartida en el archivo
    escribir_var(0);

    // Crear primer hijo
    if ((pid1 = fork()) == 0) {
        // Hijo 1
        signal(SIGUSR1, manejador_signal);  // Asignar handler a SIGUSR1
        signal(SIGTERM, manejador_terminar);  // Para terminar el proceso

        // Bucle de procesamiento del hijo 1
        while (1) {
            pause();  // Esperar la señal SIGUSR1 o SIGTERM
            sleep(2); // Simular procesamiento
            kill(getpid() + 1, SIGUSR1);  // Enviar señal al segundo hijo
        }
    }

    // Crear segundo hijo
    if ((pid2 = fork()) == 0) {
        // Hijo 2
        signal(SIGUSR1, manejador_signal);  // Asignar handler a SIGUSR1
        signal(SIGTERM, manejador_terminar);  // Para terminar el proceso

        // Bucle de procesamiento del hijo 2
        while (1) {
            pause();  // Esperar la señal SIGUSR1 o SIGTERM
            sleep(2); // Simular procesamiento
            kill(getpid() - 1, SIGUSR1);  // Enviar señal al primer hijo
        }
    }

    // El padre comienza el ciclo enviando la primera señal al hijo 1
    kill(pid1, SIGUSR1);

    // Configurar la alarma para que suene después de 30 segundos
    alarm(30);

    // El padre espera hasta que la alarma envíe la señal SIGALRM
    pause();

    // Esperar a que los hijos terminen
    wait(NULL);
    wait(NULL);

    printf("Padre: Hijos terminados.\n");

    return 0;
}
