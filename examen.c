#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define nombre_archivo "datos.txt"
#define archivo_salida1 "output1.txt"
#define archivo_salida2 "output2.txt"

// Función para manejar la alarma y matar a los hijos
void handle_alarm(int sig) {
    printf("Alarma: tiempo excedido, matando a los hijos...\n");
    kill(0, SIGKILL); // Matar a todos los procesos en el grupo
}

int calcular_edad(const char *fecha) {
    int dia, mes, anio;
    sscanf(fecha, "%d/%d/%d", &dia, &mes, &anio);

    time_t t = time(NULL);
    struct tm *current_time = localtime(&t); //Aqui solo obtengo la fecha del sistema para hacer mas facil las comparaciones
    
    int current_anio = current_time->tm_year + 1900;
    int current_mes = current_time->tm_mon + 1;
    int current_dia = current_time->tm_mday;

    int edad = current_anio - anio;
    if (current_mes < mes || (current_mes == mes && current_dia < dia)) {
        edad--;
    }
    return edad;
}

// Función para validar fechas en el formato dd/mm/yyyy
int validar_fecha(const char *fecha) {
    int dia, mes, anio;
    if (sscanf(fecha, "%d/%d/%d", &dia, &mes, &anio) != 3) return 0;
    if (dia < 1 || dia > 31 || mes < 1 || mes > 12 || anio < 1900 || anio > 2024) return 0;
    return 1;
}

// Función para validar el género
int validar_genero(const char genero) {
    return (genero == 'm' || genero == 'f');
}

// Hijo 1: Acomoda los datos en formato requerido
void hijo1() {
    FILE *file = fopen(nombre_archivo, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char linea[1024];
    if (fgets(linea, sizeof(linea), file) == NULL) {
        perror("Error al leer el archivo");
        exit(1);
    }
    fclose(file);

    FILE *output = fopen(archivo_salida1, "w");
    if (!output) {
        perror("Error al crear el archivo de salida");
        exit(1);
    }
    
    fprintf(output, "Datos acomodados:\n");
    
    char nombre[100], apellido[100], fecha[11], genero;
    char *ptr = linea;
    while (sscanf(ptr, "%s %s %s %c", nombre, apellido, fecha, &genero) == 4) {
        if (!validar_fecha(fecha) || !validar_genero(genero)) {
            printf("Error en el formato de los datos\n");
            exit(2);
        }
        fprintf(output, "%s %s %s %c\n", nombre, apellido, fecha, genero);
        ptr = strchr(ptr, genero) + 1;
    }
    
    fclose(output);
    exit(0);
}

// Hijo 2: Filtra personas con más de 60 años
void hijo2() {
    FILE *file = fopen(nombre_archivo, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char linea[1024];
    if (fgets(linea, sizeof(linea), file) == NULL) {
        perror("Error al leer el archivo");
        exit(1);
    }
    fclose(file);

    FILE *output = fopen(archivo_salida2, "w");
    if (!output) {
        perror("Error al crear el archivo de salida");
        exit(1);
    }

    char nombre[100], apellido[100], fecha[11], genero;
    char *ptr = linea;
    while (sscanf(ptr, "%s %s %s %c", nombre, apellido, fecha, &genero) == 4) {
        if (!validar_fecha(fecha) || !validar_genero(genero)) {
            printf("Error en el formato de los datos\n");
            exit(2);
        }
        if (calcular_edad(fecha) > 60) {
            fprintf(output, "%s %s %s %c\n", nombre, apellido, fecha, genero);
        }
        ptr = strchr(ptr, genero) + 1;
    }

    fclose(output);
    exit(0);
}

// Hijo 3: Imprime en pantalla los archivos generados por los hijos 1 y 2
void hijo3() {
    FILE *file1 = fopen(archivo_salida1, "r");
    if (!file1) {
        perror("Error al abrir el archivo 1");
        exit(1);
    }

    FILE *file2 = fopen(archivo_salida2, "r");
    if (!file2) {
        perror("Error al abrir el archivo 2");
        exit(1);
    }

    char buffer[1024];
    printf("Contenido de %s:\n", archivo_salida1);
    while (fgets(buffer, sizeof(buffer), file1)) {
        printf("%s", buffer);
    }
    fclose(file1);

    printf("\nContenido de %s:\n", archivo_salida2);
    while (fgets(buffer, sizeof(buffer), file2)) {
        printf("%s", buffer);
    }
    fclose(file2);

    exit(0);
}

int main() {
    signal(SIGALRM, handle_alarm);
    alarm(10); // Configura la alarma de 30 segundos

    pid_t pid1, pid2, pid3;
    int status;

    // Hijo 1
    if ((pid1 = fork()) == 0) {
        hijo1();
    }

    // Hijo 2
    if ((pid2 = fork()) == 0) {
        hijo2();
    }

    // Esperar que los hijos 1 y 2 terminen antes de crear al hijo 3
    for (int i = 0; i < 2; i++) {
        pid_t pid = wait(&status);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 2) {
                printf("Hijo %d terminó con error de formato\n", pid);
            } else {
                printf("Hijo %d terminó exitosamente\n", pid);
            }
        }
    }

    // Hijo 3
    if ((pid3 = fork()) == 0) {
        hijo3();
    }

    // Espera al hijo 3
    wait(&status);
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status == 2) {
            printf("Hijo %d terminó con error de formato\n", pid3);
        } else {
            printf("Hijo %d terminó exitosamente\n", pid3);
        }
    }

    return 0;
}
