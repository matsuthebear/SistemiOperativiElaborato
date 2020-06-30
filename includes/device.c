#include "defines.h"
#include "fifo.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"

int semaphoreid;
Board * board_position;
Acknowledgment * acklist;
char pathfifo[20];
int filedescriptor, fifodescriptor;
char buffer[BUFFER_LENGTH] = {0};

void deviceSignalHandler(int signal){
  free_shared_memory(board_position);
  free_shared_memory(acklist);
  unlinkFIFO(pathfifo);
  if (close(fifodescriptor) == -1)
    ErrExit("<Device>Errore! UNLINK fallito");
  exit(0);
}

void setDeviceSignalMask(){
  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGTERM);
  sigprocmask(SIG_SETMASK, &set, NULL);
  signal(SIGTERM, deviceSignalHandler);
}

void startDevice(char * filepath, int semaphoreid, int device, int board_sharedmemory, int acklist_sharedmemory){
  printf("<Device %d> Creato Device con PID %d\n", device, getpid());
  //Setta la maschera
  setDeviceSignalMask();
  //Creazione e apertura fifo in lettura
  sprintf(pathfifo, "%s%d", PATHFIFO, getpid());
  createFIFO(pathfifo);
  int fifodescriptor = openFIFO(pathfifo, O_RDONLY | O_NONBLOCK);
  //Apertura file  
  filedescriptor = open(filepath, O_RDONLY | O_NONBLOCK, 0); 
  Position position = {.x = -1, .y = -1, .row = 1};
  board_position = (Board *)get_shared_memory(board_sharedmemory, 0);
  acklist = (Acknowledgment *)get_shared_memory(acklist_sharedmemory, 0);
  while(1){
    //Aspetta il proprio turno 
    //printf("<Device %i> Aspetto il mio turno\n", getpid());
    waitDeviceSemaphore(semaphoreid, device);
    //Si sposta all'interno della scacchiera
    position = getPositionAndMove(filedescriptor, position, device, buffer, board_position);
    //Finito lo spostamento,invia le fifo ricevute
    semOp(semaphoreid, NUMBER_OF_DEVICES + 1, -1);
    checkAndSendMessages(fifodescriptor,position, acklist ,board_position);
    semOp(semaphoreid, NUMBER_OF_DEVICES + 1, 1);
    //Libera le memorie condivise
    signalFreeDeviceSemaphore(semaphoreid, device);
  }
}
