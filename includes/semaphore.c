/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include "err_exit.h"
#include "semaphore.h"


void semOp(int semid, unsigned short sem_num, short sem_op){
  // Struttura del semaforo, spiegato in semaphore.h
  struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

  if (semop(semid, &sop, 1) == -1)
    ErrExit("<Server> Errore: SEMOP fallito");
}

void printSemaphoresValue(int semDev, int processes){
  unsigned short semVal[processes];
  union semun arg;
  arg.array = semVal;

  if(semctl(semDev,0,GETALL,arg) == -1)
    ErrExit("<Server> Errore! SEMCTL GETALL fallito");

  printf("<Server Test> Semaphore set state : \n");
  for(int i = 0; i < processes; i++)
    printf("id : %d --> %d \n",i,semVal[i]);
}

int createSemaphoreSet(key_t semkey, int number_of_elements, unsigned short *initialValues){
  int semaphoreid = semget(semkey, number_of_elements, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (semaphoreid == -1)
    ErrExit("<Server> Errore! SEMGET fallito");
  union semun arg;
  arg.array = initialValues;
  if (semctl(semaphoreid, 0, SETALL, arg) == -1)
    ErrExit("<Server> Errore! SEMCTL SETALL fallito");
  return semaphoreid;
}

void waitDeviceSemaphore(int semaphoreid, int device){
  //Aspetta il proprio turno
  semOp(semaphoreid, (unsigned short)device, -1); 
  //Aspetta che la board si liberi
  semOp(semaphoreid, (unsigned short)NUMBER_OF_DEVICES, 0);
}

void signalFreeDeviceSemaphore(int semaphoreid, int device){
  //Da 0 a Numero dei device - 1
  if (device < NUMBER_OF_DEVICES - 1){
    //Sblocca il device successivo
    semOp(semaphoreid, (unsigned short)(device + 1), 1);
  }
  //da Numero dei device a numero dei device + 1
  else{
    //Blocca la board
    semOp(semaphoreid, (unsigned short)NUMBER_OF_DEVICES, 1);
    //Sblocca il primo semaforo, per ripartire in cascata
    semOp(semaphoreid, (unsigned short)0, 1); 
    //In questo modo non raggiungera' mai il semaforo della ack,
    //ma ripartira' dal primo device senza problemi
  }
}

void deleteSemaphores(int semaphoreid){
    if (semctl(semaphoreid, 0 /*ignored*/, IPC_RMID, 0) == -1)
        ErrExit("semctl IPC_RMID failed");
}