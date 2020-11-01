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

    int efecto; // 0 -> 10 | 0 = normal | 1 -> 10 = efecto
    int turnos; // 0 -> 1 | 0 = normal | 1 = al reves
    int sentido; // 0 -> 1 | 0 = normal | 1 = al reves
    int pos[4]; // 0 -> 29 | 0 = inicio | 29 = final
    int pregunta[27]; // 0 -> 1 | 0 = no ? | 1 = ?
    int preguntadoble[27]; // 0 -> 1 | 0 = no ?? | 1 = ??
	int buffer;
	int buffer2;

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
	table -> buffer = 0;
	table -> buffer2 = 0;

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
Nombre: todosRetroceden
Parametros: tablero* table (tabla), int jugadorActivador (jugador1 = 0)
Retorno:  void
Descripcion: Hace retroceder a todos menos al 
jugador activador. Para (?)
*/
void todosRetroceden(tablero* table, int jugadorActivador){
	int i;
	for(i = 0;i < 4;i++){
		if(i != jugadorActivador){
			retrocederJugador(table, i);
		}
	}
}

/*
Nombre: todosRetrocedenDoble
Parametros: tablero* table (tabla), int jugadorActivador (jugador1 = 0)
Retorno:  void
Descripcion: Hace retroceder dos veces
a todos menos al jugador activador. Para (?).
*/
void todosRetrocedenDoble(tablero* table, int jugadorActivador){
	todosRetroceden(table, jugadorActivador);
	todosRetroceden(table, jugadorActivador);
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
Nombre: random10
Parametros: void
Retorno:  void
Descripcion: Entrega un numero random entre 0 y 9.
*/
int random10(){
    int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
    return r%10;	// 0 -> 9
}

/*
Nombre: randomPregunta
Parametros: void
Retorno:  void
Descripcion: Entrega un numero random 
correspondiente a una casilla ?.
*/
int randomPregunta(){
	int rand = random10();
	rand = rand % 5;
	rand++;
	return rand;
}

/*
Nombre: randomPreguntaDoble
Parametros: void
Retorno:  void
Descripcion: Entrega un numero random 
correspondiente a una casilla ??.
*/
int randomPreguntaDoble(){
	int rand = randomPregunta();
	return(rand + 5);
}

/*
Nombre: getPrimero
Parametros: tablero* table
Retorno: int[2] | int[0] = jugador primero 
int[1] = pos de jug primero
Descripcion: Obtiene el jugador en primer
lugar, y su posicion.
*/
int* getPrimero(tablero* table){
	int maximo = 0;
	int jugMax = 0;
	int i;
	for(i = 0; i < 4; i++){
		if((table -> pos[i]) > maximo){
			maximo = table -> pos[i];
			jugMax = i;
		}
	}

	static int retorno[2];
	retorno[0] = jugMax;
	retorno[1] = maximo;
	return retorno;
}

int* getUltimo(tablero* table){
	int minimo = 29;
	int jugMin = 0;
	int i;
	for(i = 0; i < 4; i++){
		if((table -> pos[i]) < minimo){
			minimo = table -> pos[i];
			jugMin = i;
		}
	}

	static int retorno[2];
	retorno[0] = jugMin;
	retorno[1] = minimo;
	return retorno;
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

/*
Nombre: cambioSentido
Parametros: tablero* table (tabla)
Retorno:  void
Descripcion: Cambia el sentido del tablero.
*/
void cambioSentido(tablero* table){
	int s = table -> sentido;
	if(s == 0){
		table -> sentido = 1;
	}
	else if(s == 1){
		table -> sentido = 0;
	}
	else{
		printf("table -> sentido tiene algo raro \n");
		exit(1);
	}
}

/*
Nombre: jugadorSiguente
Parametros: tablero* table (tabla), turno(pasado)
Retorno:  int (turno siguente)
Descripcion: Entrega el turno siguente.
*/
int jugadorSiguente(tablero* table, int turno){
	int turnoSig;
	if((table -> turnos) == 0){
		if (turno == 4){
			turnoSig = 1;
		}
		else{
			turnoSig = turno + 1;
		}
	}
	else if((table -> turnos) == 1){
		if(turno == 1){
			turnoSig = 4;
		}
		else{
			turnoSig = turno - 1;
		}
	}
	else{
		printf("table -> turnos tiene algo extraño\n");
		exit(1);
	}
	return turnoSig;
}

/*
Nombre: checkEfecto
Parametros: tablero* table (tabla)
Retorno:  int (bool)
Descripcion: Revisa si hay que jugar
o hacer algun efecto en particular.
*/
int checkEfecto(tablero* table){
	if((table -> efecto) == 0){
		return 1;
	}
	else{
		return 0;
	}
}

/*
Nombre: getEfecto
Parametros: tablero* table (tabla)
Retorno:  int (efecto)
Descripcion: Retorna el codigo del efecto.
*/
int getEfecto(tablero* table){
	if((table -> efecto) < 0){
		printf("table -> efecto tiene algo extraño: %d\n", table -> efecto);
		exit(1);
	}
	else if((table -> efecto) > 11){
		printf("table -> efecto tiene algo extraño: %d\n", table -> efecto);
		exit(1);
	}
	else{
		return table -> efecto;
	}
}

/*
Nombre: isPregunta
Parametros: tablero* table (tablero), int jugador
Retorno:  int (bool)
Descripcion: Entrega 1 si es que la casilla 
actual del jugador corresponde a ?.
*/
int isPregunta(tablero* table, int jugador){
	int pos = table -> pos[jugador];
	return (table -> pregunta[pos-1]);
}

/*
Nombre: isPreguntaDoble
Parametros: tablero* table (tablero), int jugador
Retorno:  int (bool)
Descripcion: Entrega 1 si es que la casilla 
actual del jugador corresponde a ??.
*/
int isPreguntaDoble(tablero* table, int jugador){
	int pos = table -> pos[jugador];
	return (table -> preguntadoble[pos-1]);
}

void cambioTurno(tablero* table){
	int t = table -> turnos;
	if (t == 1){
		table -> turnos = 0;
	}
	else if(t == 0){
		table -> turnos = 1;
	}
	else{
		printf("table -> turnos tiene algo extraño: %d\n", table -> turnos);
		exit(1);
	}
}

int soyPrimero(tablero* table, int jugador){
	int max = 0;
	int jugMax;
	int i;
	for(i = 0; i < 4; i++){
		if(max < (table -> pos[i])){
			max = table -> pos[i];
			jugMax = i;
		}
	}
	return jugMax == jugador;
}

int soyUltimo(tablero* table, int jugador){
	int min = 0;
	int jugMin;
	int i;
	for(i = 0; i < 4; i++){
		if(min > (table -> pos[i])){
			min = table -> pos[i];
			jugMin = i;
		}
	}
	return jugMin == jugador;
}

int getPosPrimero(tablero *table){
	int pos = 0;
	int i;
	for(i = 0; i < 4; i++){
		if(pos < (table -> pos[i])){
			pos = table -> pos[i];
		}
	}
	return pos;
}

int getPosUltimo(tablero *table){
	int pos = 29;
	int i;
	for(i = 0; i < 4; i++){
		if(pos > (table -> pos[i])){
			pos = table -> pos[i];
		}
	}
	return pos;
}

void moveToBuffer(tablero* table, int jugador){
	table -> pos[jugador] = table -> buffer;
}

//getPrimero
//Retorno: int[2] | int[0] = jugador primero 
//int[1] = pos de jug primero
void cambiarConUltimo(tablero* table, int jugador){
	if (!(soyPrimero(table, jugador))){
		table -> efecto = 0;
	}
	else{
		int *p;
		p = getUltimo(table);
		int jugUlt = p[0];
		int posJugUlt = p[1];
		table -> buffer = table -> pos[jugador];
		table -> buffer2 = jugUlt;
		table -> pos[jugador] = posJugUlt;
	}
}

void cambiarConUltimo2(tablero* table, int jugador){
	int buffer2 = table -> buffer2; // jug
	if (jugador == buffer2){
		table -> pos[jugador] = table -> buffer;
	}
}

void cambiarConPrimero(tablero* table, int jugador){
	if(!(soyUltimo(table, jugador))){
		table -> efecto = 0;
	}
	else{
		int *p;
		p = getPrimero(table);
		int jugPri = p[0];
		int posJugPri = p[1];
		table -> buffer = table -> pos[jugador];
		table -> buffer2 = jugPri;
		table -> pos[jugador] = posJugPri;
	}
}

void cambiarConPrimero2(tablero* table, int jugador){
	int buffer2 = table -> buffer2; // jug
	if (jugador == buffer2){
		table -> pos[jugador] = table -> buffer;
	}
}


/*
Nombre: hacerEfecto
Parametros: tablero* table, int efecto, int jugador
Retorno:  void
Descripcion: Hace el efecto correspondiente.
*/
void hacerEfecto(tablero* table, int efecto, int jugador){
	if(efecto == 1){
		printf("El jugador retrocede uno.\n");
		retrocederJugador(table, jugador);
	}
	else if(efecto == 2){
		printf("Los demas jugadores retroceden uno.\n");
	}
	else if(efecto == 3){
		printf("El Jugador Avanza Uno\n");
		avanzarJugador(table, jugador);
	}
	else if(efecto == 4){
		printf("El Siguente Jugador no Puede Jugar su Turno \n");
	}
	else if(efecto == 5){
		printf("Cambio en el Sentido de Turnos\n");
		cambioTurno(table);
	}
	else if(efecto == 6){
		printf("Todos los Jugadores Retroceden Dos\n");
		retrocederJugador(table, jugador);
		retrocederJugador(table, jugador);
	}
	else if(efecto == 7){
		printf("Todos los Jugadores menos Tu, avanzan hasta la siguente casilla blanca.\n");
	}
	else if(efecto == 8){
		printf("El jugador cambia con el Ultimo.\n");
		cambiarConUltimo(table, jugador);
	}
	else if(efecto == 9){
		printf("El jugador cambia con el Primero.\n");
		cambiarConPrimero(table, jugador);
	}
	else if(efecto == 10){
		printf("Cambiar el sentido del Tablero\n");
		cambioSentido(table);
	}
	else{
		printf("hacer efecto recibio un efecto extraño: %d\n", efecto);
	}
}

/*
Nombre: activarPregunta
Parametros: tablero* table, int jugador
Retorno:  void
Descripcion: Hace los llamados para activar
una casilla ?
*/
void activarPregunta(tablero* table, int jugador){
	int pos = table -> pos[jugador];
	pos--;
	table -> pregunta[pos] = 0;
	hacerEfecto(table, randomPregunta(), jugador);
}

/*
Nombre: activarPregunta
Parametros: tablero* table, int jugador
Retorno:  void
Descripcion: Hace los llamados para activar
una casilla ??
*/
void activarPreguntaDoble(tablero* table, int jugador){
	int pos = table -> pos[jugador];
	pos--;
	table -> preguntadoble[pos] = 0;
	hacerEfecto(table, randomPreguntaDoble(), jugador);
}

void avanzarHastaBlanca(tablero* table, int jugador){
	int pos = table -> pos[jugador];
	int i = pos - 1;
	int move = 0;
	int sentido = table -> sentido;
	if(sentido == 0){
		while(i <= 27){
			if( (! (table -> pregunta[i])) &&  (! (table -> preguntadoble[i])) ){
				move = i + 1;
				break;
			}
			i++;
		}
	}
	else if(sentido == 1){
		while(i >= 0){
			if( (! (table -> pregunta[i])) &&  (! (table -> preguntadoble[i])) ){
				move = i + 1;
				break;
			}
			i--;
		}
	}
	else{
		printf("Avanzar hasta blanca recibio un table -> sentido extraño: %d\n", table -> sentido);
	}
	if(move != 0){
		table -> pos[jugador] = move;
	}
}

/*
Nombre: pipeEfecto
Parametros: int jugadorActivador, int* pipes
Retorno:  void
Descripcion: Hace los llamados a las pipes de los otros
jugadores para hacer uso de los efectos.
*/
void pipeEfecto(int jugadorActivador, int* pipe01, int* pipe10, int* pipe02, int* pipe20, int* pipe03, int* pipe30, int* pipe04, int* pipe40){
	char mensaje = '0';
	if(jugadorActivador != 0){
		write(pipe01[1], &mensaje, 1);
    	while(read(pipe10[0], &mensaje, 1) < 0){}
	}
	if(jugadorActivador != 1){
		write(pipe02[1], &mensaje, 1);
    	while(read(pipe20[0], &mensaje, 1) < 0){}
	}
	if(jugadorActivador != 2){
		write(pipe03[1], &mensaje, 1);
    	while(read(pipe30[0], &mensaje, 1) < 0){}
	}
	if(jugadorActivador != 3){
		write(pipe04[1], &mensaje, 1);
    	while(read(pipe40[0], &mensaje, 1) < 0){}
	}
	if((jugadorActivador > 3) | (jugadorActivador < 0)){
		printf("jugador activador tiene algo raro: %d\n", jugadorActivador);
		exit(1);
	}
}

void efectoSecundario(tablero* table, int jugador){
	int efecto = getEfecto(table);
	if(efecto == 2){
		retrocederJugador(table, jugador);
	}
	else if(efecto == 6){
		retrocederJugador(table, jugador);
		retrocederJugador(table, jugador);
	}
	else if(efecto == 7){
		avanzarHastaBlanca(table, jugador);
	}
	else if(efecto == 8){
		cambiarConUltimo2(table, jugador);
	}
	else if(efecto == 9){
		cambiarConPrimero2(table, jugador);
	}
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
			if(!checkEfecto(table)){
				pipeEfecto(turno-1, pipe01, pipe10, pipe02, pipe20, pipe03, pipe30, pipe04, pipe40);
				if(getEfecto(table) == 4){
					turno++;
				}
				table -> efecto = 0;
			}
			turno = jugadorSiguente(table, turno);
            sleep(1);
            printLugares(table);
			

            if(turno == 1){
                //dile a jugador[0] que juegue
                // write(pipe12[1],&mensaje,1);  // pongo el mensaje en la pipe        
                // while((read(pipe21[0],&mensaje,1))<0){}; //espero por la respuesta 
                char mensaje = '0';
                write(pipe01[1], &mensaje, 1);
                while(read(pipe10[0], &mensaje, 1) < 0){}
            }
            else if(turno == 2){
                char mensaje = '0';
                write(pipe02[1], &mensaje, 1);
                while(read(pipe20[0], &mensaje, 1) < 0){}

            }
            else if(turno == 3){
                char mensaje = '0';
                write(pipe03[1], &mensaje, 1);
                while(read(pipe30[0], &mensaje, 1) < 0){}
            }
            else if(turno == 4){
                char mensaje = '0';
                write(pipe04[1], &mensaje, 1);
                while(read(pipe40[0], &mensaje, 1) < 0){}
            }

        }
        
        else if (jugador[0] == 0){
            char mensaje;
            while(read(pipe01[0], &mensaje, 1) < 0){}
			if(checkEfecto(table)){
				printf("Jugador 1 (q) para salir (n) para jugar: \n");
				scanf("%s", buffer);
				if (!strcmp(buffer, "q")){
					exit(1);
					return 0;
				}
				dice = dado();
				printf("Dado: %d\n", dice);
				for(i = 0; i < dice; i++){
					avanzarJugador(table, 0);
				}
				if(isPregunta(table, 0)){
					printf("Caiste en (?).\n");
					printf("Activar? s/n: \n");
					char activate[20];
					scanf("%s", activate);
					if(strcmp(activate, "s")){
						activarPregunta(table, 0);
					}
				}
				else if(isPreguntaDoble(table, 0)){
					printf("Caiste en ( ? ? ).\n");
					printf("Activar? s/n: \n");
					char activate[20];
					scanf("%s", activate);
					if(strcmp(activate, "s")){
						activarPreguntaDoble(table, 0);
					}
				}
			}
			else{
				efectoSecundario(table, 0);
			}
			write(pipe10[1], &mensaje, 1);
        }
        else if (jugador[1] == 0){
            char mensaje;
            while(read(pipe02[0], &mensaje, 1) < 0){}
			if(checkEfecto(table)){
				dice = dado();
				printf("Dado: %d\n", dice);
				for(i = 0; i < dice; i++){
					avanzarJugador(table, 1);
				}
				if(isPregunta(table, 1)){
					activarPregunta(table, 1);
				}
				else if(isPreguntaDoble(table, 1)){
					activarPreguntaDoble(table, 1);
				}
			}
			else{
				efectoSecundario(table, 1);
			}
            write(pipe20[1], &mensaje, 1);
        }
        else if (jugador[2] == 0){
            char mensaje;
            while(read(pipe03[0], &mensaje, 1) < 0){}
            if(checkEfecto(table)){
				dice = dado();
				printf("Dado: %d\n", dice);
				for(i = 0; i < dice; i++){
					avanzarJugador(table, 2);
				}
				if(isPregunta(table, 2)){
					activarPregunta(table, 2);
				}
				else if(isPreguntaDoble(table, 2)){
					activarPreguntaDoble(table, 2);
				}
			}
			else{
				efectoSecundario(table, 2);
			}
            write(pipe30[1], &mensaje, 1);
        }
        else if (jugador[3] == 0){
            char mensaje;
            while(read(pipe04[0], &mensaje, 1) < 0){}
			if(checkEfecto(table)){
				dice = dado();
				printf("Dado: %d\n", dice);
				for(i = 0; i < dice; i++){
					avanzarJugador(table, 3);
				}
				if(isPregunta(table, 3)){
					activarPregunta(table, 3);
				}
				else if(isPreguntaDoble(table, 3)){
					activarPreguntaDoble(table, 3);
				}
			}
			else{
				efectoSecundario(table, 3);
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



