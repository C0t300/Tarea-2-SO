#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>

#define SMOBJ_NAME  "/tablero"

typedef struct Tablero{

    int efecto; // 0->10
    int turnos; // 0 -> 1 | 0 = normal | 1 = al reves
    int sentido; // 0 -> 1 | 0 = normal | 1 = al reves
    int pos[4]; // 0 -> 29 | 0 = inicio | 29 = final
    int pregunta[27]; // 0 -> 1 | 0 = no ? | 1 = ?
    int preguntadoble[27]; // 0 -> 1 | 0 = no ?? | 1 = ??

}tablero;

/*
Nombre: initTablero
Parametros: tablero* table (tabla)
Retorno: void
Descripcion: Inicia el tablero
y sus casillas (?) (??).
*/
void initTablero(tablero* table){

    int i;

    table -> efecto = 0;
    table -> turnos = 0;
    table -> sentido = 0;

    for(i = 0; i < 4; i++){
        table -> pos[i] = 0;
    }

    
    for(i = 0; i < 26; i++){
        table -> pregunta[i] = 0;
        table -> preguntadoble[i] = 0;
    }
    table -> pregunta[1] = 1;
    table -> pregunta[3] = 1;
    table -> pregunta[5] = 1;
    table -> pregunta[11] = 1;
    table -> pregunta[13] = 1;
    table -> pregunta[20] = 1;
    table -> pregunta[22] = 1;
    table -> pregunta[24] = 1;
    table -> pregunta[26] = 1;

    table -> preguntadoble[15] = 1;
    table -> preguntadoble[21] = 1;
    table -> preguntadoble[23] = 1;
    table -> preguntadoble[25] = 1;

}

/*
Nombre: avanzarJugador
Parametros: tablero* table (tabla), int jugador (jugador 1 == 0)
Retorno: void
Descripcion: Hace avanzar al jugador
previo revisar el sentido del tablero.
*/
void avanzarJugador(tablero* table, int jugador){
    if(table -> sentido == 0){
        if(table -> pos[jugador] < 29){
            table -> pos[jugador]++;
        }
    }
    else if(table -> sentido == 1){
        if(table -> pos[jugador] > 0){
            table -> pos[jugador]--;
        }
    }
    else{
        printf("table -> sentido tiene algo raro. \n");
        exit(1);
    }
}

/*
Nombre: retrocederJugador
Parametros: tablero* table (tabla), int jugador (jugador 1 == 0)
Retorno: void
Descripcion: Hace retroceder al jugador
previo revisar el sentido del tablero.
*/
void retrocederJugador(tablero* table, int jugador){
    if(table -> sentido == 0){
        if(table -> pos[jugador] > 0){
            table -> pos[jugador]--;
        }
    }
    else if(table -> sentido == 1){
        if(table -> pos[jugador] < 29){
            table -> pos[jugador]++;
        }
    }
    else{
        printf("table -> sentido tiene algo raro. \n");
        exit(1);
    }
}


/*
Nombre: vueltaPreguntas
Parametros: tablero* table (tabla)
Retorno: void
Descripcion: Cambias las casillas 
? por las ?? y viceversa.
*/
void vueltaPreguntas(tablero* table){
    int i;
    for(i = 0; i < 26; i++){
        if(table -> pregunta[i] == 1){
            table -> preguntadoble[i] = 1;
            table -> pregunta[i] = 0;
        }
        else if(table -> preguntadoble[i] == 1){
            table -> preguntadoble[i] = 0;
            table -> pregunta[i] = 1;
        }
    }
}

/*
Nombre: juegoValido
Parametros: tablero* table (tabla)
Retorno: int (bool)
Descripcion: Revisa si es que algun 
jugador llego a la meta.
*/
int juegoValido(tablero* table){
    int i;
    if(table -> sentido == 0){
        for(i = 0; i < 4; i++){
            if(table -> pos[i] == 29){
                return 0;
            }
        }
    }
    else if(table -> sentido == 1){
        for(i = 0; i < 4; i++){
            if(table -> pos[i] == 0){
                return 0;
            }
        }
    }
    else{
        printf("table -> sentido tiene algo extraño: %d\n", table -> sentido);
        exit(1);
    }

    return 1;
}


/*
Nombre: shMemoryCreate
Parametros: ninguno
Retorno: void
Descripcion: Crea la shared memory con el 
nombre definido SMOBJ_NAME.
Extra: The POSIX shared memory object implementation on Linux 2.4 makes 
use of a dedicated file system, which is normally mounted under /dev/shm.
*/
void shMemoryCreate(void){
   int fd;
   fd = shm_open (SMOBJ_NAME, O_CREAT | O_RDWR  , 00700); /* create s.m object*/
   if(fd == -1){
       printf("Error file descriptor \n");
	   exit(1);
   }
   if(-1 == ftruncate(fd, sizeof(tablero))){
       printf("Error shared memory cannot be resized \n");
	   exit(1);
   }
   
   close(fd);
}

/*
Nombre: shMemoryOpen
Parametros: ninguno
Retorno: int (fd)
Descripcion: Abre la Shared Memory y retorna el fd
*/
int shMemoryOpen(void){
    int fd;
    fd = shm_open (SMOBJ_NAME,  O_RDWR  , 00700); /* open s.m object*/
    if(fd == -1){
       printf("Error file descriptor %s\n", strerror(errno));
	   exit(1);
   }
    return fd;
}

/*
Nombre: shMemoryGet
Parametros: fd (int)
Retorno: tablero* 
Descripcion: Recibe el fd de la memoria compartida y 
devuelve un puntero al tablero.
Extra: QUE ESTOY HACIENDO CON LOS VOID POINTER AAA
*/
tablero* shMemoryGet(int fd){
    tablero* table;
    void *ptr;
    ptr = mmap(NULL, sizeof(tablero), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED){
        printf("Map failed in read process: %s\n", strerror(errno));
        exit(1);
    }
    table = (tablero*)ptr;
    return table;
}

/*
Nombre: shMemoryClose
Parametros: ninguno
Retorno: void
Descripcion: Cierra y elimina(?) la
shared memory creada.
*/
void shMemoryClose(void){
    int ret;
    ret = shm_unlink(SMOBJ_NAME);
    if(ret == -1){
        printf("Error shmUnlink\n");
        exit(1);
    }
}

/*
Nombre: printWinner
Parametros: tablero* table (tabla)
Retorno: void
Descripcion: Imprime el ganador.
*/
void printWinner(tablero* table){
    int winner;
    int i;
    if(table -> sentido == 0){
        for(i = 0; i < 4; i++){
            if(table -> pos[i] == 29){
                winner = i+1;
            }
        }
    }
    else if(table -> sentido == 1){
        for(i = 0; i < 4; i++){
            if(table -> pos[i] == 0){
                winner = i+1;
            }
        }
    }
    printf("Gano el jugador %d!\n", winner);
}


/*
Nombre: showTable
Parametros: void
Retorno:  void
Descripcion: Imprime una tabla en Consola.
Extra: Asumo que no se usara o se modificara ampliamente.
*/
void showTable(){

    printf("|   20 |?  21  |??  22 |?  23  |??  24 |    25 |?   26 |    27 |    Fin|\n");
    printf("|   19 |\n");
    printf("|   18 |   17  |?   16 |   15  |?   14 |    13 |?   12 |    11 |    10 |\n");
    printf("                                                               |    9  |\n");
    printf("|Inicio|    1  |?   2  |    3  |?   4  |    5  |?   6  |    7  |    8  |\n");

}

/*
Nombre: dado
Parametros: void
Retorno: int
Descripcion: Entrega un numero pseudo-random
entre 1 y 6 para simular el dado.
Extra: Necesita haber hecho srand antes.
*/
int dado(){
    int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
    return r%5 + 1;
}

/*
Nombre: soyPadre
Parametros: Array de 4 int con los process id de los fork
Retorno: int (basicamente bool)
Descripcion: Entrega 1 si es el proceso padre, y 0 
si es que no.
Extra: que padre xd
*/
int soyPadre(int ids[4]){
    if((ids[0] != 0) & (ids[1] != 0) & (ids[2] != 0) & (ids[3] != 0)){
        return 1;
    }
    return 0;
}

/*
Nombre: printLugares
Parametros: tablero* table (tabla)
Retorno: void
Descripcion: Imprime las posiciones.
Extra: Debug only
*/
void printLugares(tablero* table){
    printf("J1: %d | J2: %d | J3: %d | J4: %d\n", table -> pos[0], table -> pos[1], table -> pos[2], table -> pos[3]);
}

int main(){
	//showTable();
    

    int pipe01[2];  // se crean los arreglos que guardan los mensajes 
    int pipe02[2];  // IDA
    int pipe03[2];  
    int pipe04[2];

    int pipe10[2];  // VUELTA
    int pipe20[2];
    int pipe30[2];  
    int pipe40[2];

    pipe(pipe01);   // Se inician los pipes
    pipe(pipe02); 
    pipe(pipe03); 
    pipe(pipe04); 

    pipe(pipe10); 
    pipe(pipe20); 
    pipe(pipe30); 
    pipe(pipe40); 

    char buffer[256];

    int jugador[4];
    int i;
    int fd;     // fd para sh memory
    tablero* table;

    shMemoryCreate();
    fd = shMemoryOpen();
    table = shMemoryGet(fd);
    initTablero(table);
    

    for(i = 0; i < 4; i++){     // Se crean los 4 procesos hijos que representan a los jugadores
        jugador[i] = fork();
        if(jugador[i] == 0){
            break;
        }
    }
    
    // Linea de arriba mensaje de hijo hacia padre
    // pipe 10 | pipe10[1] escritura del hijo 
    // pipe 01 | pipe01[0] lectura del padre
    if(jugador[0] == 0){
        close(pipe10[0]);
        close(pipe01[1]);
    }
    else if(jugador[1] == 0){
        close(pipe02[1]);
        close(pipe20[0]);
    }
    else if(jugador[2] == 0){
        close(pipe03[1]);
        close(pipe30[0]);
    }
    else if(jugador[3] == 0){
        close(pipe04[1]);
        close(pipe40[0]);
    }
    else if(soyPadre(jugador)){
        // Linea de abajo mensaje padre hacia hijo
        // pipe 01 | pipe01[1] escritura del padre
        // pipe 10 | pipe10[0] recepcion del hijo
        close(pipe01[0]);
        close(pipe10[1]);

        close(pipe02[0]);
        close(pipe20[1]);

        close(pipe03[0]);
        close(pipe30[1]);

        close(pipe04[0]);
        close(pipe40[1]);
        
        

    }


    srand(time(NULL) ^ (getpid()<<16)); // Init, para dado()
    int turno = 0;
    int dice;

    while(juegoValido(table)){

        if(soyPadre(jugador)){
            if (turno == 4){
                turno = 1;
            }
            else{
                turno++;
            }
            sleep(1);
            printLugares(table);

            if(turno == 1){
                //dile a jugador[0] que juegue
                // write(pipe12[1],&mensaje,1);  // pongo el mensaje en la pipe        
                // while((read(pipe21[0],&mensaje,1))<0){}; //espero por la respuesta 
                char mensaje = '1';
                write(pipe01[1], &mensaje, 1);
                while(read(pipe10[0], &mensaje, 1) < 0){}
            }
            else if(turno == 2){
                char mensaje = '1';
                write(pipe02[1], &mensaje, 1);
                while(read(pipe20[0], &mensaje, 1) < 0){}

            }
            else if(turno == 3){
                char mensaje = '1';
                write(pipe03[1], &mensaje, 1);
                while(read(pipe30[0], &mensaje, 1) < 0){}
            }
            else if(turno == 4){
                char mensaje = '1';
                write(pipe04[1], &mensaje, 1);
                while(read(pipe40[0], &mensaje, 1) < 0){}
            }

        }
        
        else if (jugador[0] == 0){
            char mensaje;
            while(read(pipe01[0], &mensaje, 1) < 0){}
            printf("Jugador 1 (q) para salir (n) para jugar: \n");
            scanf("%s", buffer);
            if (!strcmp(buffer, "q")){
                return 0;
            }
            dice = dado();
            printf("Dado: %d\n", dice);
            for(i = 0; i < dice; i++){
                avanzarJugador(table, 0);
            }
            write(pipe10[1], &mensaje, 1);
        }
        else if (jugador[1] == 0){
            char mensaje;
            while(read(pipe02[0], &mensaje, 1) < 0){}
            dice = dado();
            printf("Dado: %d\n", dice);
            for(i = 0; i < dice; i++){
                avanzarJugador(table, 1);
            }
            write(pipe20[1], &mensaje, 1);
        }
        else if (jugador[2] == 0){
            char mensaje;
            while(read(pipe03[0], &mensaje, 1) < 0){}
            dice = dado();
            printf("Dado: %d\n", dice);
            for(i = 0; i < dice; i++){
                avanzarJugador(table, 2);
            }
            write(pipe30[1], &mensaje, 1);
        }
        else if (jugador[3] == 0){
            char mensaje;
            while(read(pipe04[0], &mensaje, 1) < 0){}
            dice = dado();
            printf("Dado: %d\n", dice);
            for(i = 0; i < dice; i++){
                avanzarJugador(table, 3);
            }
            write(pipe40[1], &mensaje, 1);
        }

        
    }

    if(soyPadre(jugador)){
        printLugares(table);
        printWinner(table);
        shMemoryClose();

    }

    

}



