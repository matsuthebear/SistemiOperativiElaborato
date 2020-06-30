/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "shared_memory.h"
#include "err_exit.h"
#include "fifo.h"

Position getPositionAndMove(int filedescriptor, Position position, int device, char * buffer, Board * board_position){
  Position new_position;
  int file, row = 0;
  for(; row < position.row; row++){
    file = read(filedescriptor, buffer, BUFFER_LENGTH);
    if ( file == -1)
      ErrExit("<Device> File corrotto");
    else if (file == 0)
      return position;
  }
  char number;
  int counter = 0, x = -1, y = -1;
  for(int character = 0; character <  BUFFER_LENGTH; character++){
    number = *(buffer + character);
    if (isdigit(number)){
      if (x == -1)
        x = (int)number - '0';
      else
        y = (int)number - '0';
    }
    if (x != -1 && y != -1){
      if (counter == device){
        break;
      }else{
        counter++;
        x = -1;
        y = -1;
      }
    }
  }
  new_position.x = x;
  new_position.y = y;
  new_position.row = row;
  //Attacco la memoria condivisa
  if (x > -1 && y > -1){
      if (position.x != new_position.x || position.y != new_position.y){
        board_position ->board[new_position.x][new_position.y] = getpid();
        if(position.x > -1 && position.y > -1)
          board_position ->board[position.x][position.y] = 0;
      }else{
        board_position ->board[position.x][position.y] = getpid();
      }
    }else{
      new_position = position;
    }
  return new_position;

}

int checkAckMessage(Acknowledgment * acklist, Message message){
  Acknowledgment ack;
  int check = 0;
  for (int i = 0; i < MAX_NUMBER_ACKS; i++){
    ack = acklist[i];
    if (ack.message_id == message.message_id && ack.pid_receiver == message.pid_receiver){
      check = 1;
      break;
    }
  }
  return check;
}

void checkAndSendMessages(int fifodescriptor, Position position, Acknowledgment * acklist, Board * board_position){
  Message message;
  int message_fifo;
  printf("<Device %d> %d %d msgs:", getpid(), position.x, position.y);
  do{
    message_fifo = read(fifodescriptor, &message, sizeof(Message));
    if (message_fifo > 0 && message.pid_sender != getpid()){
      printf("%d ", message.message_id);
      //printf("<Device %d> Ho ricevuto un messaggio da %d\n", getpid(), message.pid_sender);
      //Check per vedere se il messaggio e' gia stato recapitato con lo stesso message_id e device
      // Il messaggio non e' ancora arrivato al device con il message_id richiesto
      if (checkAckMessage(acklist,message) == 0){
        //printf("<Device %d> Il messaggio non e' ancora stato registrato\n", getpid());
        //Trova la cella vuota piu' vicina, per poi scriverci sopra l'ack
        for (int i = 0; i < MAX_NUMBER_ACKS; i++){
          if (acklist[i].message_id == 0){
            acklist[i].pid_sender = message.pid_sender;
            acklist[i].pid_receiver = message.pid_receiver;
            acklist[i].message_id = message.message_id;
            acklist[i].timestamp = time(NULL);
            //printf("<Device %d> Ho scritto un ack in %d\n", getpid(),i);
            break;
          }
        }
      }else{
        //printf("<Device %d> Il messaggio risulta gia' presente in ack\n", getpid());
        message.message_id = -1; //Messaggio da scartare perchè già presente
      }
      if (message.message_id != -1){
        //printf("<Device %d> Mi preparo a inviare il messaggio a un device vicino\n", getpid());
        int x = position.x;
        int y = position.y;
        message.pid_sender = getpid();
        double min_distance = 0, radius;
        pid_t device, min_device;
        for (int i = 0; i < BOARD_SIZE; i++){
          for (int j = 0; j < BOARD_SIZE; j++){
            device = board_position->board[i][j];
            if (device > 0 && device != getpid()){
              radius = sqrt((i - x) * (i - x) + (j - y) * (j - y));
              if (radius <= message.max_distance){
                if (min_distance == 0 || radius < min_distance){
                  //printf("<Device %d> Device trovato : %d\n", getpid(), device);
                  message.pid_receiver = device;
                  if (checkAckMessage(acklist,message) == 0){
                    min_distance = radius;
                    min_device = device;
                    //printf("<Device %d> Device piu' vicino trovato %f %d\n", getpid(), radius, min_device);
                  }
                }
              }
            }
          }
        }

        if (min_device > 0){
          message.pid_receiver = min_device;
          //printf("<Device %d> Invio messaggio a %d\n",getpid(),message.pid_receiver);
          char pathfifo[20];
          sprintf(pathfifo, "%s%d", PATHFIFO, min_device);
          //printf("%s\n", pathfifo);
          int fifodescriptor_receiver = openFIFO(pathfifo, O_WRONLY | O_NONBLOCK);
          writeFIFO(fifodescriptor_receiver, message);
          //printf("<Device %d> Messaggio inviato\n", getpid());
        }else{
          //printf("<Device %d> Non ho trovato alcun device: %f %d\n", getpid(), radius, min_device);
        }

      }
    }
  }while(message_fifo > 0);
  printf("\n");
}


int getMessageQueue(key_t key ,int flags){
    int msqid = msgget(key, flags);
    if(msqid == -1)
      ErrExit("MSQID fallita!");
    
    
    return msqid;
}

void readMessageQueue(int msqid, AckClient *ackclient, size_t msgsz, long msgtype, int msgflg){
  if(msgrcv(msqid, ackclient,msgsz, msgtype ,msgflg) == -1)
    ErrExit("<Client>Errore! Lettura da msgqueue fallita");
}

void writeMessageQueue(int msqid, AckClient *ackclient, size_t msgsz, int msgflg){
   if(msgsnd(msqid, ackclient,msgsz,msgflg) == -1)
      ErrExit("<AckManager>Scrittura da msgqueue fallita");
}

int checkAckDevices(int message_id, Acknowledgment * acklist){
  int count = 0;
  for (int ack = 0; ack < MAX_NUMBER_ACKS; ack++){
    if (acklist[ack].message_id != 0 && acklist[ack].message_id == message_id){
      count++; 
    }
  }
  if (count == NUMBER_OF_DEVICES)
  //Il messaggio è stato ricevuto da tutti i devices
      return 1;
  else
  //O il messaggio non è stato recapitato ai device  o c'è stato un errore
    return 0;
}

AckClient getAckClient(Acknowledgment * acklist){
  AckClient ackclient;
  for (int i = 0; i < NUMBER_OF_DEVICES; i++){
    ackclient.acklist[i].pid_sender = 0;
    ackclient.acklist[i].pid_receiver = 0;
    ackclient.acklist[i].message_id = 0;
    ackclient.acklist[i].timestamp = 0;
  }
  int message_id;
  ackclient.check = 0;
  //Controlla se l'id corrispondente ha all'interno altri 4 acks
  for (int i = 0; i < MAX_NUMBER_ACKS; i++){
    message_id = acklist[i].message_id;
    if (checkAckDevices(message_id, acklist) == 1){
      //Se uguale a 1, ha trovato i client
      //printf("%d",checkAckDevices(message_id, acklist));
      //printf("%d", message_id);
      ackclient.check = 1;
      break;
    }
  }
  int count = 0;
  for (int i = 0; i < MAX_NUMBER_ACKS; i++)
    //printf("%d ", acklist[i]);
  if (ackclient.check == 1){
    for (int i = 0; i < MAX_NUMBER_ACKS; i++){
      if (acklist[i].message_id == message_id && message_id != 0){
        ackclient.acklist[count] = acklist[i];
        count++;
        acklist[i].pid_sender = 0;
        acklist[i].pid_receiver = 0;
        acklist[i].message_id = 0;
        acklist[i].timestamp = 0;
      }
      if (count == 5)
        break;
    }
  }
  return ackclient;
}
