/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"
#include "defines.h"

void createFIFO(char *pathfifo){
    //Crea la fifo del path selezionato
    int createfifo = mkfifo(pathfifo, S_IRUSR | S_IWUSR);
    if(createfifo == -1)
        ErrExit("<Device>Errore! MKFIFO fallita!\n");
}
int openFIFO(char *pathfifo, int flag){
  // Apre la fifo del path con i flag desiderati
  int openfifo = open(pathfifo,flag);
  if (openfifo == -1)
    ErrExit("<Device>Errore! OPEN FIFO fallita!\n");
  return openfifo;
}

void writeFIFO(int openfifo, Message message){
    int writefifo = write(openfifo, &message, sizeof(message));
    if(writefifo == -1)
        ErrExit("<Device>Errore! WRITE FIFO fallita!\n");

    if(close(openfifo) == -1)
        ErrExit("<Device>Errore! CLOSE FIFO fallita!");
}

void unlinkFIFO(char *pathfifo){
    if(unlink(pathfifo) != 0)
        ErrExit("<device> unlink fifo failed\n");
}
