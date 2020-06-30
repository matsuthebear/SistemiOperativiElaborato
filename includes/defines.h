/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

//Librerie C
#include <stdbool.h> //Booleani
#include <stdio.h> //Standard Input - Output, necessaria per C
#include <time.h> // Serve per gestire il tempo (delay, timestamp)
#include <ctype.h> // Dichiara funzioni utilizzate per la classificazione dei caratteri
#include <stdlib.h> //Funzioni e costanti di utilita' generale
#include <unistd.h> // Permette l'accesso alle API dello standard POSIX
#include <string.h> //Per lavorare con le stringhe
#include <signal.h> //Segnali per chiudere il server
#include <math.h> // Funzioni matematiche pow() e sqrt()
#include <stddef.h> //Per definire NULL 
#include <sys/types.h> // pid_t, time_t, key_t e size_t
#include <sys/stat.h> // Flags S_
#include <sys/sem.h> // Creazione e gestione dei semafori
#include <sys/shm.h> // 
#include <sys/ipc.h> //IPC : messaggi, semafori e memoria condivisa
#include <fcntl.h> //Flags F_ 
#include <sys/wait.h>
#include <sys/msg.h> // Per creazione dei processi pid_t
//#include <errno.h> // Definizioni macro per la gestione delle situazioni di errore,
// presente in ERR_EXIT.H

#define NUMBER_OF_DEVICES 5 //Numero dei devices 
#define BOARD_SIZE 10 //Grandezza della scacchiera NxN
#define MAX_NUMBER_ACKS 100 //Numero massimo di ack che la lista acknowledgment puo' contenere
#define BUFFER_LENGTH 20

// Structs
/* 

*/
typedef struct{
  pid_t pid_sender;
  pid_t pid_receiver; 
  int message_id;
  char message[256]; 
  double max_distance; 
}Message;

typedef struct{
  int board[BOARD_SIZE][BOARD_SIZE];
}Board;

/*
*/
typedef struct{
  pid_t pid_sender;
  pid_t pid_receiver;
  int message_id;
  time_t timestamp;
}Acknowledgment;

typedef struct{
  long mtype;
  Acknowledgment acklist[NUMBER_OF_DEVICES];
  int check;
}AckClient;

typedef struct{
  int x;
  int y;
  int row;
}Position;
// Funzioni

Position getPositionAndMove(int filedescriptor, Position position, int device, char * buffer, Board *  board_position);

void checkAndSendMessages(int fifodescriptor, Position position, Acknowledgment * acklist, Board * board_position);


int getMessageQueue(key_t key ,int flags);

void readMessageQueue(int msqid, AckClient *ackclient, size_t msgsz, long msgtype, int msgflg);

void writeMessageQueue(int msqid, AckClient *ackclient, size_t msgsz,int msgflg);

int checkAckDevices(int message_id, Acknowledgment * acklist);

AckClient getAckClient(Acknowledgment * acklist);
