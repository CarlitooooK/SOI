#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


void escribir_archivo(char* cadena){
    FILE *file=fopen("cadena.txt","a+");
    if(file==NULL){
        perror("\nERROR ABRIENDO EL ARCHIVO PARA ESCRITURA");
        exit(1);
    }
    fputs(cadena,file);
    fclose(file);
}

char* leer_archivo(char* cad){
    FILE *file= fopen("cadena.txt", "r+");
    if(file==NULL){
        perror("\nERROR ABRIENDO EL ARCHIVO PARA LECTURA");
        exit(1);
    }
    fgets(cad, sizeof(cad), file);
    fclose(file);
    return cad;
}

int main(){

    char cadena[1024];
    while(1){
        printf("\nDIGITE UNA CADENA:");
        fgets(cadena,sizeof(cadena),stdin);
        escribir_archivo(cadena);
    }
    return 0;
}