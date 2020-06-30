#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"

Acknowledgment * acklist;
Acknowledgment ackclient[NUMBER_OF_DEVICES];
int messageQueueId;

void ackSignalHandler(int signal){
  printf("<AckManager> Terminazione ackmanager\n");
  free_shared_memory(acklist);
  msgctl(messageQueueId, IPC_RMID, NULL);
  exit(0);
}

void setAckSignalMask(){
  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGTERM);
  sigprocmask(SIG_SETMASK, &set, NULL);
  signal(SIGTERM, ackSignalHandler);
}


void startAckManager(int semaphoreid, int acklist_sharedmemory, int messageQueueKey){
  setAckSignalMask();
  AckClient ackclient;
  acklist = (Acknowledgment *)get_shared_memory(acklist_sharedmemory,0);
  int messageQueueId = getMessageQueue(messageQueueKey, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  size_t messageSize= sizeof(AckClient) - sizeof(long);
  while(1){
    sleep(5);
    printf("<AckManager> Check in corso\n");
    semOp(semaphoreid, NUMBER_OF_DEVICES + 1, -1);
    //Gestione ACKS
    do{
      ackclient = getAckClient(acklist);
      if (ackclient.check != 0){
          ackclient.mtype = ackclient.acklist[0].message_id;
          printf("<AckManager> Mi preparo ad inviare la risposta a %ld!\n", ackclient.mtype);
          
          writeMessageQueue(messageQueueId, &ackclient, messageSize, 0);
          printf("<AckManager> Risposta inviata!\n");
        }
    }while(ackclient.check != 0);
    semOp(semaphoreid, NUMBER_OF_DEVICES + 1, 1);
  }
}
