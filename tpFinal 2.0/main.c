#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EQ 32 // cant equipos
#define MAX_G 8 // cant grupos
#define MAX_EQxG 4 // cant equipos por grupo

/* STRUCT EQUIPO */
typedef struct
{
    char nomEquipo[40];
    int mp; // partidos jugados
    int win; // partidos ganados
    int loss; // partidos perdidos
    int gf; // goles a favor
    int ga; // goles en contra
    int pts; // puntos totales
    int prb;  //del 0 al 99

} Equipo;

/* NODO EQUIPO : lo usamos una vez para crear la lista de equipos */
typedef struct nodoEquipo
{
    Equipo equipo;
    struct nodoEquipo* siguiente;
} nodoEquipo;

/* NODO GRUPO EQUIPO: trabaja con el equipo por referencia */
typedef struct nodoGrupoEquipo
{
    Equipo* equipo;
    struct nodoGrupoEquipo* siguiente;
} nodoGrupoEquipo;

/* ARREGLO GRUPO : contiene sublista de equipos */
typedef struct
{
    char letra; // 'A', 'B', ...
    struct nodoGrupoEquipo* equipos; /// sublista equipos (siempre hay 4)
} Grupo;

/* STRUCT PARTIDO */
typedef struct
{
    int id; // opcional
    char fecha[40]; // elegir criterio y tipo
    Equipo* equipo1;
    Equipo* equipo2;
    int golesEq1;
    int golesEq2;
    int penales1; // en la fase de grupos no hay penales (puede setearse en 1 y 0 como boolean)
    int penales2;
} Partido;

/* NODO PARTIDO */
typedef struct nodoPartido
{
    Partido partido;
    struct nodoPartido* siguiente;
} nodoPartido;

/* ARREGLO GRUPO PARTIDO (fase de grupos): contiene sublista de partidos */
typedef struct
{
    char letra;
    nodoPartido* partidos; /// sublista partidos
} GrupoPartido;

/* ARREGLO FASES : contiene sublista de partidos | [0]8vos - [1]4tos - [2]semifinal - [3]3er puesto - [4]final */
typedef struct
{
    int idFase; // de 0 a 4
    nodoPartido* partidos;
} Fase;

/**------------------------------------------------ FUNCIONES ---------------------------------------------------------------------**/

/** CARGAR LISTA DE EQUIPOS */

nodoEquipo* crearNodoEquipo(Equipo equipo)
{
    nodoEquipo* nuevo = (nodoEquipo*)malloc(sizeof(nodoEquipo));

    strcpy(nuevo->equipo.nomEquipo,equipo.nomEquipo);
    nuevo->equipo.ga = 0;
    nuevo->equipo.gf = 0;
    nuevo->equipo.loss = 0;
    nuevo->equipo.mp = 0;
    nuevo->equipo.pts = 0;
    nuevo->equipo.win = 0;
    nuevo->equipo.prb = 0;

    nuevo->siguiente = NULL;

    return nuevo;
}

void insertarAlFinalEquipo(nodoEquipo** listaEquipos, Equipo equipo)
{
    nodoEquipo* seg;
    nodoEquipo* nuevo = crearNodoEquipo(equipo);

    if(*listaEquipos == NULL)
    {
        *listaEquipos = nuevo;
    }
    else
    {
        seg = *listaEquipos;
        while(seg->siguiente != NULL)
        {
            seg = seg->siguiente;
        }
        seg->siguiente = nuevo;
    }
}

void cargaEquipos(nodoEquipo** listaEquipos)
{

    FILE* fp = fopen("ArchEquipos.bin","rb");;
    Equipo datoEquipo;

    if(fp)
    {
        while(fread(&datoEquipo,sizeof(Equipo),1,fp)>0)
        {
            insertarAlFinalEquipo(listaEquipos,datoEquipo);
        }
        fclose(fp);
    }

}

/** MUESTRA EQUIPOS */

void muestraEquipos(nodoEquipo* ListaEquipos)
{
    while(ListaEquipos != NULL)
    {
        printf(" %s\n",ListaEquipos->equipo.nomEquipo);
        printf("\t\t\t%i%%\n",ListaEquipos->equipo.prb);
        printf("--------------------------------\n");

        ListaEquipos = ListaEquipos->siguiente;
    }
}

/** EXISTE EQUIPO / GANA O PIERDE */

int existeEquipo(nodoEquipo* listaEquipos, char equipoABuscar[])
{
    while(listaEquipos != NULL && strcmp(listaEquipos->equipo.nomEquipo,equipoABuscar) != 0)
    {
        listaEquipos = listaEquipos->siguiente;
    }
    if (listaEquipos != NULL && strcmp(listaEquipos->equipo.nomEquipo,equipoABuscar) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void inicEmulador(nodoEquipo* listaEquipos, char equipoABuscar[40], char* GoP)
{
int enc = 0;
    printf("\nBienvenido al fixture del mundial 2022!\n\nA continuacion debes ingresar la seleccion por la que vas a apostar y si clasifica o no de fase de grupos.\n\nPor que seleccion queres apostar?: ");
    fflush(stdin);
    gets(equipoABuscar);
    enc = existeEquipo(listaEquipos,equipoABuscar);
    while(enc == 0)
    {
        printf("\nPor favor, ingrese un equipo valido.\n\nPor que seleccion queres apostar?: ");
        fflush(stdin);
        gets(equipoABuscar);
        enc = existeEquipo(listaEquipos,equipoABuscar);
    }
    printf("\nQueres que clasifique o no clasifique? (ingrese c/n): ");
    fflush(stdin);
    scanf("%c",GoP);
}

/** CARGA PROBABILIDADES */

void cargaProbabilidadesAleatorias(nodoEquipo** listaEquipos)
{
    if(*listaEquipos != NULL)
    {
        (*listaEquipos)->equipo.prb = rand()%100;
        cargaProbabilidadesAleatorias(&(*listaEquipos)->siguiente);
    }
}

void cargaProbabilidades(nodoEquipo** listaEquipos, char nombre[40], char GoP)
{
    if(*listaEquipos == NULL || strcmp((*listaEquipos)->equipo.nomEquipo,nombre) == 0)
    {
        if(GoP == 'c')
        {
            int N=100;
            int M=75;
            (*listaEquipos)->equipo.prb = rand()%(N-M+1)+M;
        }
        else
        {
            (*listaEquipos)->equipo.prb = rand()%26;
        }
    }
    else
    {
        cargaProbabilidades(&(*listaEquipos)->siguiente,nombre,GoP);
    }
}

/** CARGAR GRUPOS */

nodoGrupoEquipo* crearNodoGrupoEquipo(Equipo* equipo)
{
    nodoGrupoEquipo* nuevo = (nodoGrupoEquipo*)malloc(sizeof(nodoGrupoEquipo));
    nuevo->equipo = equipo;
    nuevo->siguiente = NULL;

    return nuevo;
}

void insertarAlFinalGrupoEquipo(nodoGrupoEquipo** ListaGrupoEquipo, nodoGrupoEquipo* nuevo)
{
    if(*ListaGrupoEquipo == NULL)
    {
        *ListaGrupoEquipo = nuevo;
    }
    else
    {
        insertarAlFinalGrupoEquipo(&((*ListaGrupoEquipo)->siguiente),nuevo);
    }
}

void cargarArregloGrupos(Grupo grupos[], nodoEquipo* listaEquipos)
{
    int i=0;
    int indice = 0;
    while(listaEquipos != NULL)
    {
        i=0;
        while(i < MAX_EQxG)
        {
            nodoGrupoEquipo* nuevo = crearNodoGrupoEquipo(&(listaEquipos->equipo));
            insertarAlFinalGrupoEquipo(&(grupos[indice].equipos),nuevo);
            listaEquipos = listaEquipos->siguiente;
            i++;
        }
        indice++;
    }
}

void crearGrupos(Grupo grupos[], int* validos)
{
    char letras[] = {"ABCDEFGH"};
    int validosLetras = 8;
    int i=0;
    while(i < validosLetras)
    {
        grupos[i].letra = letras[i];
        grupos[i].equipos = NULL;
        i++;
        (*validos)++;
    }
}

void mostarEquiposXgrupo(nodoGrupoEquipo* equipos)
{
    int i=0;
    while(equipos != NULL)
    {
        printf(" %i: %s\n",i+1,(equipos->equipo->nomEquipo));
        i++;
        equipos = equipos->siguiente;
    }
}

void mostrarGrupos(Grupo arreglo[], int validos)
{
    for(int i=0; i<validos; i++)
    {
        printf("\n\n--------- GRUPO %c ---------\n\n",arreglo[i].letra);
        mostarEquiposXgrupo((arreglo[i].equipos));
    }
}

/** MOSTRAR TABLA */

void mostarEquiposTabla(nodoGrupoEquipo* equipos)
{
    while(equipos != NULL)
    {
        printf(" %s\n",(equipos->equipo->nomEquipo));
        printf("\t\t%i\t%i\t%i\t%i\t%i\t%i\n",equipos->equipo->mp,equipos->equipo->win,equipos->equipo->loss,equipos->equipo->gf,equipos->equipo->ga,equipos->equipo->pts);
        printf("\n------------------------------------------------------------\n\n");
        equipos = equipos->siguiente;
    }
}

void mostrarTablaGrupos(Grupo arreglo[], int validos)
{
    for(int i=0; i<validos; i++)
    {
        printf("\n------------------------- GRUPO %c --------------------------",arreglo[i].letra);
        printf("\n\n TEAM\t\tMP\tW\tL\tGF\tGA\tPts\n");
        printf("\n------------------------------------------------------------\n\n");
        mostarEquiposTabla((arreglo[i].equipos));
    }
}

/** CARGA PARTIDOS FASE DE GRUPOS */

nodoPartido* crearNodoPartido(Partido partido)
{
    nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo->partido = partido;
    nuevo->siguiente = NULL;

    return nuevo;
}

void crearPartidos(GrupoPartido partidos[], int* validos)
{
    char letras[] = {"ABCDEFGH"};
    int validosLetras = 8;
    int i=0;
    while(i<validosLetras)
    {
        partidos[i].letra = letras[i];
        partidos[i].partidos = NULL;
        i++;
        (*validos)++;
    }
}

/** CARGA FECHAS FASE GRUPOS */

void cargarFechasPartidos(GrupoPartido partidos[], int validos)
{
    FILE* fp = fopen("ArchFechasFaseGrupos.bin","rb");
    Partido datoPartido;
    if(fp)
    {
        for(int i=0; i<validos; i++)
        {
            nodoPartido* seg = partidos[i].partidos;
            while(seg != NULL)
            {
                fread(&datoPartido,sizeof(Partido),1,fp);
                strcpy(seg->partido.fecha,datoPartido.fecha);
                seg = seg->siguiente;
            }
        }
        fclose(fp);
    }
}

void insertarAlFinalPartido(nodoPartido** partidos, nodoPartido* nuevo)
{
    if((*partidos) == NULL)
    {
        *partidos = nuevo;
    }
    else
    {
        insertarAlFinalPartido(&(*partidos)->siguiente,nuevo);
    }
}

void cargarArregloPartidosFaseGrupos(GrupoPartido partidosFG[], Grupo grupos[])
{
    int j=0;
    for(int i=0; i<MAX_G; i++)
    {
        nodoGrupoEquipo* seg = grupos[i].equipos;
        while(seg->siguiente != NULL)
        {
            nodoGrupoEquipo* aux = seg->siguiente;
            while(aux != NULL)
            {
                nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
                nuevo->partido.equipo1 = seg->equipo;
                nuevo->partido.equipo2 = aux->equipo;
                /// asignacion de goles por probabilidad (Partido)
                if(nuevo->partido.equipo1->prb > nuevo->partido.equipo2->prb)
                {
                    if(nuevo->partido.equipo2->prb < 95) // gana equipo 1
                    {
                        int N=5;
                        int M=3;
                        nuevo->partido.golesEq1 = rand()%(N-M+1)+M;
                        nuevo->partido.golesEq2 = rand()%(M-1);
                    }
                    else // empatan
                    {
                        int goles = rand()%3;
                        nuevo->partido.golesEq1 = goles;
                        nuevo->partido.golesEq2 = goles;
                    }
                }
                else
                {
                    if(nuevo->partido.equipo1->prb < nuevo->partido.equipo2->prb)
                    {
                        if(nuevo->partido.equipo1->prb < 95) // gana equipo 2
                        {
                            int N=5;
                            int M=3;
                            nuevo->partido.golesEq2 = rand()%(N-M+1)+M;
                            nuevo->partido.golesEq1 = rand()%(M-1);
                        }
                        else // empatan
                        {
                            int goles = rand()%3;
                            nuevo->partido.golesEq1 = goles;
                            nuevo->partido.golesEq2 = goles;
                        }
                    }
                    else // tienen la misma prob -> empatan
                    {
                        int goles = rand()%4;
                        nuevo->partido.golesEq1 = goles;
                        nuevo->partido.golesEq2 = goles;
                    }
                }
                /// registro de goles (Equipo)
                // equipo 1
                seg->equipo->gf += nuevo->partido.golesEq1;
                seg->equipo->ga += nuevo->partido.golesEq2;
                seg->equipo->mp++;
                // equipo 2
                aux->equipo->gf += nuevo->partido.golesEq2;
                aux->equipo->ga += nuevo->partido.golesEq1;
                aux->equipo->mp++;
                /// registro del partido
                if(nuevo->partido.golesEq1 > nuevo->partido.golesEq2) // gana equipo 1
                {
                    seg->equipo->win++;
                    seg->equipo->pts += 3;
                    aux->equipo->loss++;
                }
                else
                {
                    if(nuevo->partido.golesEq1 < nuevo->partido.golesEq2) // gana equipo 2
                    {
                        seg->equipo->loss++;
                        aux->equipo->win++;
                        aux->equipo->pts += 3;
                    }
                    else // empatan
                    {
                        seg->equipo->pts += 1;
                        aux->equipo->pts += 1;
                    }
                }
                insertarAlFinalPartido(&(partidosFG[j].partidos),nuevo);
                aux = aux->siguiente;
            }
            seg =seg->siguiente;
        }
        j++;
    }
}

void mostrarUnPartido(nodoPartido* partido)
{
    int id = 1;
    while(partido != NULL)
    {
        while(partido != NULL)
        {
            printf(" MATCH\t\tDATE\t\t\tTEAMS\n");
            printf("   %i\t",id);
            id++;
            printf("  %s\t",partido->partido.fecha);
            printf("%s",partido->partido.equipo1->nomEquipo);
            printf(" VS ");
            printf("%s\n\n",partido->partido.equipo2->nomEquipo);

            partido = partido->siguiente;
        }
        id = 0;
    }
}

void mostrarPartidos(GrupoPartido partidos[], int validos)
{
    for(int i=0; i<MAX_G; i++)
    {
        printf("\n------------------------ GRUPO %c ------------------------\n\n",partidos[i].letra);
        mostrarUnPartido((partidos[i].partidos));
    }
}

/** PASE 1ro Y 2do DE CADA GRUPO */

Equipo* buscarPrimeroXgrupo(Grupo grupos[], char grupo)
{
    Equipo* primero;
    int maxPts = 0;
    int maxDif = 0;
    int maxGoles = 0;
    int i=0;
    while(grupos[i].letra != grupo)
    {
        i++;
    }
    nodoGrupoEquipo* seg = grupos[i].equipos;
    while (seg!=NULL)
    {
        if(seg->equipo->pts > maxPts)
        {
            maxPts = seg->equipo->pts;
            maxDif = seg->equipo->gf - seg->equipo->ga;
            maxGoles = seg->equipo->gf;
            primero = seg->equipo;
        }
        else if(seg->equipo->pts == maxPts)
        {
            if((seg->equipo->gf - seg->equipo->ga) > maxDif)
            {
                maxPts = seg->equipo->pts;
                maxDif = seg->equipo->gf - seg->equipo->ga;
                maxGoles = seg->equipo->gf;
                primero = seg->equipo;
            }
            else if((seg->equipo->gf - seg->equipo->ga) == maxDif)
            {
                if(seg->equipo->gf > maxGoles)
                {
                    maxPts = seg->equipo->pts;
                    maxDif = seg->equipo->gf - seg->equipo->ga;
                    maxGoles = seg->equipo->gf;
                    primero = seg->equipo;
                }
            }
        }
        seg = seg->siguiente;
    }

    return primero;
}

Equipo* buscarSegundoXgrupo(Grupo grupos[], char grupo)
{
    Equipo* primero = buscarPrimeroXgrupo(grupos,grupo);
    int maxPts = 0;
    int maxDif = 0;
    int maxGoles = 0;
    Equipo* segundo;
    int i=0;
    while(grupos[i].letra != grupo)
    {
        i++;
    }
    nodoGrupoEquipo* seg = grupos[i].equipos;
    while(seg!=NULL)
    {
        if(strcmpi(primero->nomEquipo,seg->equipo->nomEquipo) == 0)
        {
            seg = seg->siguiente;
        }
        else
        {
            if(seg->equipo->pts > maxPts)
            {
                maxPts = seg->equipo->pts;
                maxDif = seg->equipo->gf - seg->equipo->ga;
                maxGoles = seg->equipo->gf;
                segundo = seg->equipo;
            }
            else if(seg->equipo->pts == maxPts)
            {
                if((seg->equipo->gf - seg->equipo->ga) > maxDif)
                {
                    maxPts = seg->equipo->pts;
                    maxDif = seg->equipo->gf - seg->equipo->ga;
                    maxGoles = seg->equipo->gf;
                    segundo = seg->equipo;
                }
                else if((seg->equipo->gf - seg->equipo->ga) == maxDif)
                {
                    if(seg->equipo->gf > maxGoles)
                    {
                        maxPts = seg->equipo->pts;
                        maxDif = seg->equipo->gf - seg->equipo->ga;
                        maxGoles = seg->equipo->gf;
                        segundo = seg->equipo;
                    }
                }
            }
            seg = seg->siguiente;
        }
    }

    return segundo;
}

void mostrarEquiposClasificadosOctavos(Grupo grupos[], int validos)
{
    for(int i=0; i<validos; i++)
    {
        Equipo* primero = buscarPrimeroXgrupo(grupos,grupos[i].letra);
        Equipo* segundo = buscarSegundoXgrupo(grupos,grupos[i].letra);

        printf("\n---------- GRUPO %c ----------\n",grupos[i].letra);
        printf("\n Primer lugar: %s",primero->nomEquipo);
        printf("\n Segundo lugar: %s\n",segundo->nomEquipo);
    }
}

void jugarPartidosOctavos(nodoPartido* partidosEnOctavos)
{
    while(partidosEnOctavos != NULL)
    {
        partidosEnOctavos->partido.golesEq1 = rand()%3;
        partidosEnOctavos->partido.golesEq2 = rand()%3;

        partidosEnOctavos->partido.equipo1->gf += partidosEnOctavos->partido.equipo1->gf;
        partidosEnOctavos->partido.equipo1->ga += partidosEnOctavos->partido.equipo2->ga;

        partidosEnOctavos->partido.equipo2->gf += partidosEnOctavos->partido.equipo2->gf;
        partidosEnOctavos->partido.equipo2->ga += partidosEnOctavos->partido.equipo1->ga;

        if(partidosEnOctavos->partido.golesEq1 == partidosEnOctavos->partido.golesEq2)
        {
            if(partidosEnOctavos->partido.equipo1->prb > partidosEnOctavos->partido.equipo2->prb)
            {
                partidosEnOctavos->partido.penales1 = 1;
                partidosEnOctavos->partido.penales2 = 0;
            }
            else
            {
                partidosEnOctavos->partido.penales1 = 0;
                partidosEnOctavos->partido.penales2 = 1;
            }
        }
        partidosEnOctavos = partidosEnOctavos->siguiente;
    }
}

void mostrarPartidosOctavos(nodoPartido* partidosEnOctavos)
{
    int i=1;
    while(partidosEnOctavos != NULL)
    {
        printf(" LLAVE %i:\n\n",i);
        printf("\t%s",partidosEnOctavos->partido.equipo1->nomEquipo);
        printf(" VS ");
        printf("%s\n\n",partidosEnOctavos->partido.equipo2->nomEquipo);

            printf("\t\t%i -",partidosEnOctavos->partido.golesEq1);
            printf(" %i ",partidosEnOctavos->partido.golesEq2);

        if(partidosEnOctavos->partido.golesEq1 > partidosEnOctavos->partido.golesEq2)
        {
            printf("\n\n -> %s pasa a cuartos de final.\n\n",partidosEnOctavos->partido.equipo1->nomEquipo);
        }
        else
        {
            if(partidosEnOctavos->partido.golesEq1 < partidosEnOctavos->partido.golesEq2)
            {
                printf("\n\n -> %s pasa a cuartos de final.\n\n",partidosEnOctavos->partido.equipo2->nomEquipo);
            }
            else
            {
                printf("\n\n Resultado penales: \n\n");
                printf("\t\t%i -",partidosEnOctavos->partido.penales1);
                printf(" %i",partidosEnOctavos->partido.penales2);

                if(partidosEnOctavos->partido.penales1 > partidosEnOctavos->partido.penales2)
                {
                    printf("\n\n -> %s pasa a cuartos de final.\n\n",partidosEnOctavos->partido.equipo1->nomEquipo);
                }
                else
                {
                    printf("\n\n -> %s pasa a cuartos de final.\n\n",partidosEnOctavos->partido.equipo2->nomEquipo);
                }
            }
        }
        printf("-------------------------------------------\n\n");
        partidosEnOctavos = partidosEnOctavos->siguiente;
        i++;
    }
}

/** PARTIDOS DE OCTAVOS */

void inicFases(Fase fases[])
{
    for(int i=0; i<5; i++)
    {
        fases[i].idFase=i;
        fases[i].partidos=NULL;
    }
}

void partidosOctavosIZQ(Fase fases[],Grupo ArregloGrupos[])
{
    Equipo* primero = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[0].letra);
    Equipo* segundo = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[1].letra);
    nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo->partido.equipo1 = primero;
    nuevo->partido.equipo2 = segundo;

    insertarAlFinalPartido(&(fases[0].partidos),nuevo);

    Equipo* primero1 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[2].letra);
    Equipo* segundo1 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[3].letra);
    nodoPartido* nuevo1 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo1->partido.equipo1 = primero1;
    nuevo1->partido.equipo2 = segundo1;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo1);

    Equipo* primero2 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[4].letra);
    Equipo* segundo2 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[5].letra);
    nodoPartido* nuevo2 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo2->partido.equipo1 = primero2;
    nuevo2->partido.equipo2 = segundo2;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo2);

    Equipo* primero3 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[6].letra);
    Equipo* segundo3 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[7].letra);
    nodoPartido* nuevo3 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo3->partido.equipo1 = primero3;
    nuevo3->partido.equipo2 = segundo3;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo3);

}

void partidosOctavosDER(Fase fases[],Grupo ArregloGrupos[])
{
    Equipo* primero = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[1].letra);
    Equipo* segundo = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[0].letra);
    nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo->partido.equipo1 = primero;
    nuevo->partido.equipo2 = segundo;

    insertarAlFinalPartido(&(fases[0].partidos),nuevo);

    Equipo* primero1 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[3].letra);
    Equipo* segundo1 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[2].letra);
    nodoPartido* nuevo1 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo1->partido.equipo1 = primero1;
    nuevo1->partido.equipo2 = segundo1;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo1);

    Equipo* primero2 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[5].letra);
    Equipo* segundo2 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[4].letra);
    nodoPartido* nuevo2 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo2->partido.equipo1 = primero2;
    nuevo2->partido.equipo2 = segundo2;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo2);

    Equipo* primero3 = buscarPrimeroXgrupo(ArregloGrupos,ArregloGrupos[7].letra);
    Equipo* segundo3 = buscarSegundoXgrupo(ArregloGrupos,ArregloGrupos[6].letra);
    nodoPartido* nuevo3 = (nodoPartido*)malloc(sizeof(nodoPartido));
    nuevo3->partido.equipo1 = primero3;
    nuevo3->partido.equipo2 = segundo3;
    insertarAlFinalPartido(&(fases[0].partidos),nuevo3);
}

/** PARTIDOS EN CUARTOS */

Equipo* buscarGanador(Partido partidos)
{
    if(partidos.golesEq1 == partidos.golesEq2)
    {
        if(partidos.penales1 > partidos.penales2)
        {
            return partidos.equipo1;
        }
        else
        {
            return partidos.equipo2;
        }
    }
    else
    {
        if(partidos.golesEq1 > partidos.golesEq2)
        {
            return partidos.equipo1;
        }
        else
        {
            return partidos.equipo2;
        }
    }
}

void partidosCuartos(Fase fases[])
{
    nodoPartido* seg;
    nodoPartido* aux;
    seg = fases[0].partidos;
    aux = seg->siguiente;

    while(seg != NULL)
    {
        nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
        nuevo->partido.equipo1 = buscarGanador(seg->partido);
        nuevo->partido.equipo2 = buscarGanador(aux->partido);
        nuevo->siguiente = NULL;

        nuevo->partido.golesEq1 = rand()%3;
        nuevo->partido.golesEq2 = rand()%3;

        nuevo->partido.equipo1->gf += nuevo->partido.equipo1->gf;
        nuevo->partido.equipo1->ga += nuevo->partido.equipo2->ga;

        nuevo->partido.equipo2->gf += nuevo->partido.equipo2->gf;
        nuevo->partido.equipo2->ga += nuevo->partido.equipo1->ga;

        if(nuevo->partido.golesEq1 == nuevo->partido.golesEq2)
        {
            if(nuevo->partido.equipo1->prb > nuevo->partido.equipo2->prb)
            {
                nuevo->partido.penales1 = 1;
                nuevo->partido.penales2 = 0;
            }
            else
            {
                nuevo->partido.penales1 = 0;
                nuevo->partido.penales2 = 1;
            }
        }

        insertarAlFinalPartido(&(fases[1].partidos),nuevo);

        seg = aux->siguiente;

        if(seg != NULL)
        {
            aux = seg->siguiente;
        }
    }
}

/** MOSTAR PARTIDOS EN CUARTOS */

void mostrarPartidosCuartos(nodoPartido* partidosCuartosF)
{
    int i=1;
    while(partidosCuartosF != NULL)
    {
        printf(" LLAVE %i:\n\n",i);
        printf("\t%s",partidosCuartosF->partido.equipo1->nomEquipo);
        printf(" VS ");
        printf("%s\n\n",partidosCuartosF->partido.equipo2->nomEquipo);

            printf("\t\t%i -",partidosCuartosF->partido.golesEq1);
            printf(" %i ",partidosCuartosF->partido.golesEq2);

        if(partidosCuartosF->partido.golesEq1 > partidosCuartosF->partido.golesEq2)
        {
            printf("\n\n -> %s pasa a semifinales.\n\n",partidosCuartosF->partido.equipo1->nomEquipo);
        }
        else
        {
            if(partidosCuartosF->partido.golesEq1 < partidosCuartosF->partido.golesEq2)
            {
                printf("\n\n -> %s pasa a semifinales.\n\n",partidosCuartosF->partido.equipo2->nomEquipo);
            }
            else
            {
                printf("\n\n Resultado penales: \n\n");
                printf("\t\t%i -",partidosCuartosF->partido.penales1);
                printf(" %i",partidosCuartosF->partido.penales2);

                if(partidosCuartosF->partido.penales1 > partidosCuartosF->partido.penales2)
                {
                    printf("\n\n -> %s pasa a semifinales.\n\n",partidosCuartosF->partido.equipo1->nomEquipo);
                }
                else
                {
                    printf("\n\n -> %s pasa a semifinales.\n\n",partidosCuartosF->partido.equipo2->nomEquipo);
                }
            }
        }
        printf("-------------------------------------------\n\n");
        partidosCuartosF = partidosCuartosF->siguiente;
        i++;
    }
}

/** PARTIDOS SEMIFINALES */

void partidosSemiFinales(Fase fases[])
{
    nodoPartido* seg;
    nodoPartido* aux;

    seg = fases[1].partidos;
    aux = seg->siguiente;

    while(seg != NULL)
    {
        nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
        nuevo->partido.equipo1 = buscarGanador(seg->partido);
        nuevo->partido.equipo2 = buscarGanador(aux->partido);
        nuevo->siguiente = NULL;

        nuevo->partido.golesEq1 = rand()%3;
        nuevo->partido.golesEq2 = rand()%3;

        nuevo->partido.equipo1->gf += nuevo->partido.equipo1->gf;
        nuevo->partido.equipo1->ga += nuevo->partido.equipo2->ga;

        nuevo->partido.equipo2->gf += nuevo->partido.equipo2->gf;
        nuevo->partido.equipo2->ga += nuevo->partido.equipo1->ga;

        if(nuevo->partido.golesEq1 == nuevo->partido.golesEq2)
        {
            if(nuevo->partido.equipo1->prb > nuevo->partido.equipo2->prb)
            {
                nuevo->partido.penales1 = 1;
                nuevo->partido.penales2 = 0;
            }
            else
            {
                nuevo->partido.penales1 = 0;
                nuevo->partido.penales2 = 1;
            }
        }

        insertarAlFinalPartido(&(fases[2].partidos),nuevo);

        seg = aux->siguiente;
        if(seg != NULL)
        {
            aux = seg->siguiente;
        }
    }
}

/** MOSTAR PARTIDOS SEMIFINALES */

void mostrarPartidosSemi(nodoPartido* partidosSemi)
{
    int i=1;
    while(partidosSemi != NULL)
    {
        printf(" LLAVE %i:\n\n",i);
        printf("\t%s",partidosSemi->partido.equipo1->nomEquipo);
        printf(" VS ");
        printf("%s\n\n",partidosSemi->partido.equipo2->nomEquipo);

            printf("\t\t%i -",partidosSemi->partido.golesEq1);
            printf(" %i ",partidosSemi->partido.golesEq2);

        if(partidosSemi->partido.golesEq1 > partidosSemi->partido.golesEq2)
        {
            printf("\n\n -> %s pasa a la final.\n\n",partidosSemi->partido.equipo1->nomEquipo);
            printf("\n -> %s disputa el tercer lugar.\n\n",partidosSemi->partido.equipo2->nomEquipo);
        }
        else
        {
            if(partidosSemi->partido.golesEq1 < partidosSemi->partido.golesEq2)
            {
                printf("\n\n -> %s pasa a la final.\n\n",partidosSemi->partido.equipo2->nomEquipo);
                printf("\n -> %s disputa el tercer lugar.\n\n",partidosSemi->partido.equipo1->nomEquipo);
            }
            else
            {
                printf("\n\n Resultado penales: \n\n");
                printf("\t\t%i -",partidosSemi->partido.penales1);
                printf(" %i",partidosSemi->partido.penales2);

                if(partidosSemi->partido.penales1 > partidosSemi->partido.penales2)
                {
                    printf("\n\n -> %s pasa a la final.\n\n",partidosSemi->partido.equipo1->nomEquipo);
                    printf("\n -> %s disputa el tercer lugar.\n\n",partidosSemi->partido.equipo2->nomEquipo);
                }
                else
                {
                    printf("\n\n -> %s pasa a la final.\n\n",partidosSemi->partido.equipo2->nomEquipo);
                    printf("\n -> %s disputa el tercer lugar.\n\n",partidosSemi->partido.equipo1->nomEquipo);
                }
            }
        }
        printf("-------------------------------------------\n\n");
        partidosSemi = partidosSemi->siguiente;
        i++;
    }

}

/** TERCER Y CUERTO PUESTO */

Equipo* buscarPerdedor(Partido partidos)
{
    if(partidos.golesEq1 == partidos.golesEq2)
    {
        if(partidos.penales1 > partidos.penales2)
        {
            return partidos.equipo2;
        }
        else
        {
            return partidos.equipo1;
        }
    }
    else
    {
        if(partidos.golesEq1 > partidos.golesEq2)
        {
            return partidos.equipo2;
        }
        else
        {
            return partidos.equipo1;
        }
    }
}

void partidos3erY4to(Fase fases[])
{
    nodoPartido* seg;
    nodoPartido* aux;

    seg = fases[2].partidos;
    aux = seg->siguiente;

    while(seg != NULL)
    {
        nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
        nuevo->partido.equipo1 = buscarPerdedor(seg->partido);
        nuevo->partido.equipo2 = buscarPerdedor(aux->partido);
        nuevo->siguiente = NULL;

        nuevo->partido.golesEq1 = rand()%3;
        nuevo->partido.golesEq2 = rand()%3;

        nuevo->partido.equipo1->gf += nuevo->partido.equipo1->gf;
        nuevo->partido.equipo1->ga += nuevo->partido.equipo2->ga;

        nuevo->partido.equipo2->gf += nuevo->partido.equipo2->gf;
        nuevo->partido.equipo2->ga += nuevo->partido.equipo1->ga;

        if(nuevo->partido.golesEq1 == nuevo->partido.golesEq2)
        {
            if(nuevo->partido.equipo1->prb > nuevo->partido.equipo2->prb)
            {
                nuevo->partido.penales1 = 1;
                nuevo->partido.penales2 = 0;
            }
            else
            {
                nuevo->partido.penales1 = 0;
                nuevo->partido.penales2 = 1;
            }
        }

        insertarAlFinalPartido(&(fases[3].partidos),nuevo);

        seg = aux->siguiente;
        if(seg != NULL)
        {
            aux = seg->siguiente;
        }
    }
}

/** MOSTAR PARTIDO 3er Y 4to PUESTO */

void mostrarPartidosTC(nodoPartido* partidosTC)
{
    while(partidosTC != NULL)
    {
        printf("\t%s",partidosTC->partido.equipo1->nomEquipo);
        printf(" VS ");
        printf("%s\n\n",partidosTC->partido.equipo2->nomEquipo);

            printf("\t\t%i -",partidosTC->partido.golesEq1);
            printf(" %i ",partidosTC->partido.golesEq2);

        if(partidosTC->partido.golesEq1 > partidosTC->partido.golesEq2)
        {
            printf("\n\n -> %s queda en tercer puesto.\n\n",partidosTC->partido.equipo1->nomEquipo);
            printf("\n -> %s queda en cuarto puesto.\n\n",partidosTC->partido.equipo2->nomEquipo);
        }
        else
        {
            if(partidosTC->partido.golesEq1 < partidosTC->partido.golesEq2)
            {
                printf("\n\n -> %s queda en tercer puesto.\n\n",partidosTC->partido.equipo2->nomEquipo);
                printf("\n -> %s queda en cuarto puesto.\n\n",partidosTC->partido.equipo1->nomEquipo);
            }
            else
            {
                printf("\n\n Resultado penales: \n\n");
                printf("\t\t%i -",partidosTC->partido.penales1);
                printf(" %i",partidosTC->partido.penales2);

                if(partidosTC->partido.penales1 > partidosTC->partido.penales2)
                {
                    printf("\n\n -> %s queda en tercer puesto.\n\n",partidosTC->partido.equipo1->nomEquipo);
                    printf("\n -> %s queda en cuarto puesto.\n\n",partidosTC->partido.equipo2->nomEquipo);
                }
                else
                {
                    printf("\n\n -> %s queda en tercer puesto.\n\n",partidosTC->partido.equipo2->nomEquipo);
                    printf("\n -> %s queda en cuarto puesto.\n\n",partidosTC->partido.equipo1->nomEquipo);
                }
            }
        }
        printf("-------------------------------------------\n\n");
        partidosTC = partidosTC->siguiente;
    }
}

/** FINAL FINALISIMA */

void finalMundial(Fase fases[])
{
    nodoPartido* seg;
    nodoPartido* aux;

    seg = fases[2].partidos;
    aux = seg->siguiente;

    while(seg != NULL)
    {
        nodoPartido* nuevo = (nodoPartido*)malloc(sizeof(nodoPartido));
        nuevo->partido.equipo1 = buscarGanador(seg->partido);
        nuevo->partido.equipo2 = buscarGanador(aux->partido);
        nuevo->siguiente = NULL;

        nuevo->partido.golesEq1 = rand()%3;
        nuevo->partido.golesEq2 = rand()%3;

        nuevo->partido.equipo1->gf += nuevo->partido.equipo1->gf;
        nuevo->partido.equipo1->ga += nuevo->partido.equipo2->ga;

        nuevo->partido.equipo2->gf += nuevo->partido.equipo2->gf;
        nuevo->partido.equipo2->ga += nuevo->partido.equipo1->ga;

        if(nuevo->partido.golesEq1 == nuevo->partido.golesEq2)
        {
            if(nuevo->partido.equipo1->prb > nuevo->partido.equipo2->prb)
            {
                nuevo->partido.penales1 = 1;
                nuevo->partido.penales2 = 0;
            }
            else
            {
                nuevo->partido.penales1 = 0;
                nuevo->partido.penales2 = 1;
            }
        }

        insertarAlFinalPartido(&(fases[4].partidos),nuevo);

        seg = aux->siguiente;
        if(seg != NULL)
        {
            aux = seg->siguiente;
        }
    }
}

/** MOSTAR PARTIDO FINAL */

void mostrarPartidoFinal(nodoPartido* partidoFinal)
{
    while(partidoFinal != NULL)
    {
        printf("\t%s",partidoFinal->partido.equipo1->nomEquipo);
        printf(" VS ");
        printf("%s\n\n",partidoFinal->partido.equipo2->nomEquipo);

            printf("\t\t%i -",partidoFinal->partido.golesEq1);
            printf(" %i ",partidoFinal->partido.golesEq2);

        if(partidoFinal->partido.golesEq1 > partidoFinal->partido.golesEq2)
        {
            printf("\n\n -> %s gana la copa del mundo!\n\n",partidoFinal->partido.equipo1->nomEquipo);
            printf("\n -> %s queda en segundo lugar.\n\n",partidoFinal->partido.equipo2->nomEquipo);
        }
        else
        {
            if(partidoFinal->partido.golesEq1 < partidoFinal->partido.golesEq2)
            {
                printf("\n\n -> %s gana la copa del mundo!\n\n",partidoFinal->partido.equipo2->nomEquipo);
                printf("\n -> %s queda en segundo lugar.\n\n",partidoFinal->partido.equipo1->nomEquipo);
            }
            else
            {
                printf("\n\n Resultado penales: \n\n");
                printf("\t\t%i -",partidoFinal->partido.penales1);
                printf(" %i",partidoFinal->partido.penales2);

                if(partidoFinal->partido.penales1 > partidoFinal->partido.penales2)
                {
                    printf("\n\n -> %s gana la copa del mundo!\n\n",partidoFinal->partido.equipo1->nomEquipo);
                    printf("\n -> %s queda en segundo lugar.\n\n",partidoFinal->partido.equipo2->nomEquipo);
                }
                else
                {
                    printf("\n\n -> %s gana la copa del mundo!n\n",partidoFinal->partido.equipo2->nomEquipo);
                    printf("\n -> %s queda en segundo lugar.\n\n",partidoFinal->partido.equipo1->nomEquipo);
                }
            }
        }
        printf("-------------------------------------------\n\n");
        partidoFinal = partidoFinal->siguiente;
    }
}

/** ARBOL */

void mostrarArbol(Fase fases[])
{
    nodoPartido* octavos = fases[0].partidos;
    nodoPartido* cuartos = fases[1].partidos;
    nodoPartido* semi = fases[2].partidos;
    nodoPartido* Final = fases[4].partidos;

    printf("\t  OCTAVOS\t\t  CUARTOS\t\t  SEMIFINALES\t\t  FINAL\t\t      CHAMPION\n\n");
    printf("%14s %i(%i) _\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)  |\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t     |_   %14s %i(%i) _\n",cuartos->partido.equipo1->nomEquipo,cuartos->partido.golesEq1,cuartos->partido.penales1);
    printf("\t\t     |    %14s %i(%i)  |\n",cuartos->partido.equipo2->nomEquipo,cuartos->partido.golesEq2,cuartos->partido.penales2);
    cuartos = cuartos->siguiente;

    printf("%14s %i(%i) _|\t\t\t       |\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i) \t\t\t       |\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t\t\t\t       |_  %14s %i(%i) _\n",semi->partido.equipo1->nomEquipo,semi->partido.golesEq1,semi->partido.penales1);
    printf("\t\t\t\t\t       |   %14s %i(%i)  |\n",semi->partido.equipo2->nomEquipo,semi->partido.golesEq2,semi->partido.penales2);
    semi = semi->siguiente;

    printf("%14s %i(%i) _\t\t\t       |\t\t        |\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)  |\t\t\t       |\t\t        |\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t     |_   %14s %i(%i) _|\t\t        |\n",cuartos->partido.equipo1->nomEquipo,cuartos->partido.golesEq1,cuartos->partido.penales1);
    printf("\t\t     |    %14s %i(%i)  \t\t\t\t|\n",cuartos->partido.equipo2->nomEquipo,cuartos->partido.golesEq2,cuartos->partido.penales2);
    cuartos = cuartos->siguiente;

    printf("%14s %i(%i) _|\t\t\t  \t\t\t\t|\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)  \t\t\t  \t\t\t\t|\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t\t\t\t\t\t\t\t|_ %14s %i(%i)",Final->partido.equipo1->nomEquipo,Final->partido.golesEq1,Final->partido.penales1);
    printf("   %14s\n",buscarGanador(Final->partido)->nomEquipo);
    printf("\t\t\t\t\t\t\t\t\t|  %14s %i(%i)\n",Final->partido.equipo2->nomEquipo,Final->partido.golesEq2,Final->partido.penales2);

    printf("%14s %i(%i) _\t\t\t\t\t\t\t|\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)  |\t\t\t\t\t\t\t|\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t     |_   %14s %i(%i) _\t\t\t\t|\n",cuartos->partido.equipo1->nomEquipo,cuartos->partido.golesEq1,cuartos->partido.penales1);
    printf("\t\t     |    %14s %i(%i)  |\t\t\t|\n",cuartos->partido.equipo2->nomEquipo,cuartos->partido.golesEq2,cuartos->partido.penales2);
    cuartos = cuartos->siguiente;

    printf("%14s %i(%i) _|\t\t\t       |\t\t\t|\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)   \t\t\t       |\t\t\t|\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t\t\t\t       |_  %14s %i(%i) _|\n",semi->partido.equipo1->nomEquipo,semi->partido.golesEq1,semi->partido.penales1);
    printf("\t\t\t\t\t       |   %14s %i(%i)  \n",semi->partido.equipo2->nomEquipo,semi->partido.golesEq2,semi->partido.penales2);

    printf("%14s %i(%i) _\t\t\t       |\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i)  |\t\t\t       |\n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    octavos = octavos->siguiente;

    printf("\t\t     |_   %14s %i(%i) _|\n",cuartos->partido.equipo1->nomEquipo,cuartos->partido.golesEq1,cuartos->partido.penales1);
    printf("\t\t     |    %14s %i(%i)   \n",cuartos->partido.equipo2->nomEquipo,cuartos->partido.golesEq2,cuartos->partido.penales2);
    cuartos = cuartos->siguiente;

    printf("%14s %i(%i) _|\n",octavos->partido.equipo1->nomEquipo,octavos->partido.golesEq1,octavos->partido.penales1);
    printf("%14s %i(%i) \n",octavos->partido.equipo2->nomEquipo,octavos->partido.golesEq2,octavos->partido.penales2);
    printf("\n");
}

/**------------------------------------------------ MAIN ---------------------------------------------------------------------*/

int main()
{
    srand(time(NULL));
    char nombre[40];
    char GoP;

    nodoEquipo* listaEquipos = NULL;
    cargaEquipos(&listaEquipos);

    printf("------------------------------------ FIXTURE MUNDIAL '22 V:0.1 --------------------------------------\n");
    inicEmulador(listaEquipos,nombre,&GoP);

    cargaProbabilidadesAleatorias(&listaEquipos);
    cargaProbabilidades(&listaEquipos,nombre,GoP);

    Grupo arregloGrupos[MAX_G];
    int validosGrupos=0;
    crearGrupos(arregloGrupos,&validosGrupos);
    cargarArregloGrupos(arregloGrupos,listaEquipos);

    GrupoPartido partidosFaseGrupos[MAX_G];
    int validosPFG=0;
    crearPartidos(partidosFaseGrupos,&validosPFG);
    cargarArregloPartidosFaseGrupos(partidosFaseGrupos,arregloGrupos);
    cargarFechasPartidos(partidosFaseGrupos,validosPFG);

    Fase fases[5];
    inicFases(fases);

    partidosOctavosIZQ(fases,arregloGrupos);
    partidosOctavosDER(fases,arregloGrupos);
    jugarPartidosOctavos(fases[0].partidos);
    partidosCuartos(fases);
    partidosSemiFinales(fases);
    partidos3erY4to(fases);

    finalMundial(fases);

    int op = -1;
    do
    {
        do
        {
            system("cls");
            printf(" ------------------------ MENU ------------------------\n");
            printf(" 1: Ver los equipos y sus probabilidades de ganar.\n");
            printf(" 2: Ver la formacion de grupos.\n");
            printf(" 3: Ver las fechas de los partidos de la fase de grupos.\n");
            printf(" 4: Ver la tabla de estadisticas.\n");
            printf(" 5: Ver los equipos que clasifican a 8vos.\n");
            printf(" 6: Ver los partidos de 8vos.\n");
            printf(" 7: Ver los partidos de 4tos.\n");
            printf(" 8: Ver los partidos de semifinales.\n");
            printf(" 9: Ver 3er y 4to puesto.\n");
            printf(" 10: Ver la final finalisima del mundo.\n");
            printf(" 11: Ver arbol de los partidos.\n");
            printf(" 0: EXIT.\n");
            printf("\n GO TO -> ");
            scanf("%i",&op);
        }
        while(op<0||op>12);


        switch(op)
        {
        case 1:
            system("cls");
            printf(" --- PROBABILIDAD DE EQUIPOS ---\n");
            printf("\n TEAM\t\t    PROBABILITY\n");
            printf("--------------------------------\n");
            muestraEquipos(listaEquipos);
            system("pause");
            break;
        case 2:
            system("cls");
            printf("--- FORMACION DE GRUPOS ---");
            mostrarGrupos(arregloGrupos,validosGrupos);
            printf("\n");
            system("pause");
            break;
        case 3:
            system("cls");
            printf("\n-------------------- FASE DE GRUPOS ---------------------\n");
            mostrarPartidos(partidosFaseGrupos,validosPFG);
            system("pause");
            break;
        case 4:
            system("cls");
            printf("\n\t\t   TABLA DE ESTADISTICAS \n");
            mostrarTablaGrupos(arregloGrupos,validosGrupos);
            system("pause");;
            break;
        case 5:
            system("cls");
            printf("\n -- CLASIFICADOS A OCTAVOS --\n");
            mostrarEquiposClasificadosOctavos(arregloGrupos,validosGrupos);
            printf("\n");
            system("pause");
            break;
        case 6:
            system("cls");
            printf("\n----------- PARTIDOS -> OCTAVOS -----------\n\n");
            mostrarPartidosOctavos(fases[0].partidos);
            system("pause");
            break;
        case 7:
            system("cls");
            printf("\n----------- PARTIDOS -> CUARTOS -----------\n\n");
            mostrarPartidosCuartos(fases[1].partidos);
            system("pause");
            break;
        case 8:
            system("cls");
            printf("\n--------- PARTIDOS -> SEMIFINALES ---------\n\n");
            mostrarPartidosSemi(fases[2].partidos);
            system("pause");
            break;
        case 9:
            system("cls");
            printf("\n--- PARTIDO -> 3ER Y CUARTO 4TO PUESTO ---\n\n");
            mostrarPartidosTC(fases[3].partidos);
            system("pause");
            break;
        case 10:
            system("cls");
            printf("\n------------ PARTIDO -> FINAL ------------\n\n");
            mostrarPartidoFinal(fases[4].partidos);
            system("pause");
            break;
        case 11:
            system("cls");
            printf("\n------------------------------------------------------- ARBOL -------------------------------------------------------\n\n\n");
            mostrarArbol(fases);
            system("pause");
            break;
        }
    }
    while(op != 0);


    return 0;
}


