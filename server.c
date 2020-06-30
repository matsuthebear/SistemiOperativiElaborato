/// @file server.c
/// @brief Contiene l'implementazione del SERVER.
#include "server.h"
//Variabili Globali
int semaphoreid;
int board_sharedmemory;
int acklist_sharedmemory;
Board * boardlist;
Acknowledgment * acklist;

void serverSignalHandler(int signal){
  kill(-getpid(), SIGTERM);
  printf("<Server> Terminazione Server...\n");
  //free_shared_memory(boardlist);
  //free_shared_memory(acklist);
  remove_shared_memory(acklist_sharedmemory);
  remove_shared_memory(board_sharedmemory);
  deleteSemaphores(semaphoreid);
  exit(0);
}

void serverSignalMask(){
  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGINT);
  sigdelset(&set, SIGTERM);
  sigprocmask(SIG_SETMASK, &set, NULL);
  signal(SIGINT, serverSignalHandler);
  signal(SIGTERM, serverSignalHandler);
}

void createDevices(char * filePath, int semaphoreid){
  int device = 0;
  for (; device < NUMBER_OF_DEVICES; device++){
    pid_t pid = fork();
    if(pid == -1){
      printf("<Server> Processo Device non creato\n");
    }else if(pid == 0){
      startDevice(filePath, semaphoreid, device, board_sharedmemory, acklist_sharedmemory);
    }
  }
}

void createAckManager(int messageQueueKey, int semaphoreid){
  pid_t pid = fork();
  if (pid == -1)
    printf("<Server> Processo AckManager non creato\n");
  else if (pid == 0){
    printf("<Server> Processo AckManager creato\n");
    startAckManager(semaphoreid, acklist_sharedmemory, messageQueueKey);
  }
}

int main(int argc, char * argv[]) {
  //Inizio Programma
  printf("<Server> Inizio Programma...\n");
  printf("<Server> Ottenimento Message Queue Key... ");
  //Ottenimento Message Queue Key dato in input da terminale
  int message_queue_key;
  if (argv[1] != NULL){
    printf("OK!\n");
    message_queue_key = atoi(argv[1]); //Converte la stringa in intero
  }else{
    printf("Errore!\n<Server Error> La Message Queue Key e' nulla!\n");
    return 1; //ErrExit non funziona per questo
  }

  //Creazione puntatore per i devices
  char *filepositions = argv[2];
  //Creazione Semafori
  // I primi 5 sono per i device, partendo dal primo device, gli ultimi due
  // servono per l'accesso alle memorie condivise scacchiera (o board)
  // e la lista di acknowledgment
  unsigned short semaphoreInitialValues[] = {1, 0, 0, 0, 0, 1, 1};
  int semaphoreid = createSemaphoreSet(IPC_PRIVATE, NUMBER_OF_DEVICES + 2, semaphoreInitialValues);
  //Creazione Shared Memories
  acklist_sharedmemory = alloc_shared_memory(message_queue_key, sizeof(Acknowledgment) * MAX_NUMBER_ACKS);
  board_sharedmemory = alloc_shared_memory(IPC_PRIVATE, sizeof(Board));
  //Setta la maschera e l'handler
  serverSignalMask();
  //Creazione Devices e Ack
  printf("<Server> Creazione processi!\n");
  createDevices(filepositions, semaphoreid);
  createAckManager(message_queue_key, semaphoreid);
  //Contatore per comprendere quante volte sta accadendo la lettura
  int repetitions = 0;
  //Ogni device si muove ogni due secondi
  while(1){
    sleep(2);
    repetitions++;
    //Libera la scacchiera
    semOp(semaphoreid, NUMBER_OF_DEVICES, -1);
    printf("<Server> Step %i: posizioni device \n", repetitions);
  }
  while(wait(NULL) != -1);
}
