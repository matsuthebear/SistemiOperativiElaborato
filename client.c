/// @file client.c
/// @brief Contiene l'implementazione del client.
#include "includes/defines.h"
#include "includes/fifo.h"
#include "includes/err_exit.h"
#include <stdio.h>

char fifoPID[50] = "";
char charPID[20] = "";
char int_to_char[11];
int messageQueueId;


void clientSignalHandler(int sig){
    printf("<Client> Terminazione programma\n");
    if (msgctl(messageQueueId, IPC_RMID, NULL) == -1)
      ErrExit("<AckManager>MSGCTL fallito");
    exit(0);
}

void clientSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);
    signal(SIGUSR1, clientSignalHandler);
}


int main(int argc, char * argv[]) {
    clientSignalMask();
    if (argc != 2) {
      ErrExit("<Client Error> Elementi non corretti");
      return 1;
    }
    Message message;
    //Inserimento dati da argv in message
    printf("Inserire il PID, l'id del message e la massima distanza:\n");
    scanf("%d %d %lf", &message.pid_receiver, &message.message_id, &message.max_distance);
    printf("Inserire il message (massimo 256 caratteri):\n");
    scanf("%s", message.message);
    message.pid_sender = getpid();
    //Ottenimento fifo 
    printf("Dati ottenuti!\n");
    int msq_queue_key = atoi(argv[1]);
    
    strcpy(fifoPID, PATHFIFO);
    sprintf(int_to_char, "%d", message.pid_receiver);
    strcat(fifoPID, int_to_char);
    printf("<Client> Apertura FIFO %s...\n", fifoPID);
    // Apertura e scrittura della fifo
    int deviceFIFO = open(fifoPID, O_WRONLY);
    if (deviceFIFO == -1)
        ErrExit("<Client Error> Apertura della FIFO fallita");
    printf("<Client> Scrittura del message nella FIFO\n");
    if (write(deviceFIFO, &message, sizeof(Message)) != sizeof(Message))
        ErrExit("<Client Error> Scrittura Fallita");
    printf("<Client> Scrittura Riuscita!\n<Client> Attendo l'ack...\n");
    if (close(deviceFIFO) != 0)
      ErrExit("<Client> Chiusura FIFO Fallita");
    //Ottenimento risposta dall'ack manager
    AckClient ackclient;
    int message_id = message.message_id;
    messageQueueId = getMessageQueue(msq_queue_key, IPC_EXCL | S_IRUSR | S_IWUSR);
    printf("<Client> In attesa del messaggio con message_id %d\n", message_id);
    size_t messageSize = sizeof(AckClient) - sizeof(long);
    readMessageQueue(messageQueueId, &ackclient, messageSize, message_id, 0);
    printf("<Client> Ack ricevuto!\n<Client> Creazione file ack\n"); 
    char buffer[150],filename[50],header[500];
    sprintf(filename,"out_%d.txt", message_id);
    int file_output = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    sprintf(header, "Messagio %d:\n%s\nLista Acks:\n", message_id, message.message);
    if(write(file_output, header,strlen(header)) == -1)
        ErrExit("<Client> Scrittura fallita");
    char date[50];
    time_t time;
    for (int i = 0; i < NUMBER_OF_DEVICES; i++){
      memset(buffer, 0 , sizeof(buffer));
      time = ackclient.acklist[i].timestamp;
      strftime(date, 50, "%Y-%m-%d %H:%M:%S", localtime(&time));
      sprintf(buffer, "%d, %d, %s\n", ackclient.acklist[i].pid_sender,
      ackclient.acklist[i].pid_receiver, date);
      printf("%s\n", buffer);
      if(write(file_output, buffer,strlen(buffer)) == -1)
        ErrExit("<Client> Scrittura fallita");
    }
    if (close(file_output) != 0)
      ErrExit("<Client> Chiusura file output fallita");
    // Chiusura della fifo e del client
    
    
    return 0;
}