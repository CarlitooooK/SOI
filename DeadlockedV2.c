#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h> //Funciones para la manipulación de archivos y control de acceso.
#include <signal.h>


#define KEY 1234
#define SEM_KEY 5678

pid_t pid1, pid2;  
char *archivo = "var_compartida.txt";

#define SEMAFORO 0

//Funcion UP
void up(int semid) {
    struct sembuf sops;
    sops.sem_num = SEMAFORO;
    sops.sem_op = 1;  // Incrementar semáforo
    sops.sem_flg = IPC_NOWAIT;
    if (semop(semid, &sops, 1) == -1) {
        perror("ERROR SEMOP\n");
        exit(1);
    }
}

//Funcion DOWN
void down(int semid) {
    struct sembuf sops;
    sops.sem_num = SEMAFORO;
    sops.sem_op = -1;  // Decrementar semáforo
    sops.sem_flg = IPC_NOWAIT;
    if (semop(semid, &sops, 1) == -1) {
        perror("ERROR SEMOP\n");
        exit(1);
    }
}

//Escribir en memoria compartida
void escribir_var(int *var,int valor){
    *var = valor;
}

//Leer en memoria compartida
int leer_valor(int *var){
    return *var;
}

void inicializar_sem(int semid){
    if (semctl(semid, SEMAFORO, SETVAL, 10000) == -1) {
        perror("ERROR SEMCTL\n");
        exit(1);
    }
}
 //Liberar recursos semaforo y memoria compartida
void liberar(int semid, int *shm_ptr){
    shmdt(shm_ptr);
    up(semid);
    exit(0);
}

// Handler para SIGALRM, usado por el padre para matar a los hijos
void manejador_alarma(int sig) {
    kill(SIGTERM, pid1);
    kill(SIGTERM,pid2);
    printf("Mate a mis hijos");
}

int main() {
   //me deja cual sola ya estas en tu reunion hasta cantando canciones del sam o algo asi
    signal(SIGALRM,manejador_alarma);
    int shmid,semid;
    int *shm_ptr;

    //Memoria compartida
    
    shmid=shmget(KEY,sizeof(int),IPC_CREAT | 0666);
    if(shmid<0){
        perror("Error al crear memoria compartida");
        exit(1);
    }
    shm_ptr = (int *)shmat(shmid,0,0);
    if(shm_ptr == (int *)-1){
        perror("Error al atar");
        exit(1);
    }

    // Semaforos
    if ((semid = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1) {
        perror("semget");
        exit(1);
    }
    
    inicializar_sem(semid);


    //inicializar la variable
    escribir_var(shm_ptr,1);
    

    // Crear primer hijo
    if ((pid1 = fork()) == 0) {
        //hijo1
        while(1){
            down(semid);
            int valor= leer_valor(shm_ptr);
            valor+=1;
            printf("Hijo 1 Variable Modificada %d\n",valor);
            escribir_var(shm_ptr,valor);
            up(semid);
            sleep(2); 
        }
    }

    // Crear segundo hijo
    if ((pid2 = fork()) == 0) {
        //hijo2
        while(1){
            down(semid);
            int valor= leer_valor(shm_ptr);
            valor+=2;
            printf("Hijo 2 Variable Modificada %d\n", valor);
            escribir_var(shm_ptr,valor);
            up(semid);
            sleep(2);
        }
        
    }


    // Configurar la alarma para que suene después de 10 segundos
    alarm(10);

    // El padre espera hasta que la alarma envíe la señal SIGALRM
    pause();

    // Esperar a que los hijos terminen
    wait(NULL);
    wait(NULL);


    printf("Padre: Hijos terminados.\n");

    liberar(semid,shm_ptr);

    semctl(semid,0,IPC_RMID); //libera el semaforo
    shmctl(shmid,IPC_RMID,0); //libera la memoria

    return 0;
}