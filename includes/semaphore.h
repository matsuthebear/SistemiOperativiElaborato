/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

#include <sys/stat.h>
#include <sys/sem.h>
#include "defines.h"

union semun {
  int val;
  struct semid_ds * buf;
  unsigned short * array;
};

void semOp(int semid, unsigned short sem_num, short sem_op);
void printSemaphoresValue(int semDev, int processes);
int createSemaphoreSet(key_t semkey, int num, unsigned short *values);
void waitDeviceSemaphore(int semid, int device);
void signalFreeDeviceSemaphore(int semaphoreid, int device);

void deleteSemaphores(int semaphoreid);