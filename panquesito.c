//Un programa que tenga dos hijos uno, ejecuta un exec con un while infinito que guarde un dato
//en un archivo y el otro despues de 10 segundos envia una señal a su hermano para terminar 
//y ese hijo muere, el padre debe mostrar atexit "finish teacher" de ambos(dentro de cada hijo) 
//y su forma de terminar(macros)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//Variables Globales
pid_t hijo1,hijo2;
int status1,status2;


void manejador_senial(){
    printf("\nCuchiplancheo\n");
}

void manejador_alarm(){
    printf("\nAlarma Activada\n");
}

void mensaje_terminacion(){
    printf("\nFINISH TEACHER\n");
}


//Funcion de macros para el padre
void manejador_macros(int hijo,int status){
    if(WIFEXITED(status)){
        printf("\nEl hijo %d termino normalmente con estado:%d\n",hijo,WEXITSTATUS(status));
    }else if(WIFSIGNALED(status)){
        printf("\nEl hijo %d termino anormalmente por la señal:%d\n",hijo,WTERMSIG(status));
    }
}

int main(){
    char* args[2];
    args[0]="./bucle";
    args[1]=NULL;
    

    if((hijo1=fork())==0){
        if(execvp(args[0],args)==-1){
            perror("\nTU COLA");
        }
    }

    if((hijo2=fork())==0){
        signal(SIGALRM,manejador_alarm);
        atexit(mensaje_terminacion);
        alarm(10);
        pause();
        kill(hijo1,SIGKILL);
        exit(0);
        
    }

    waitpid(hijo1,&status1,0);
    waitpid(hijo2,&status2,0);

    manejador_macros(hijo1,status1);
    manejador_macros(hijo2,status2);

    return 0;
}