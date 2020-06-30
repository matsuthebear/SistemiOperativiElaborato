/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once
#include "defines.h"
#define PATHFIFO "/tmp/dev_fifo."

void createFIFO(char *pathfifo);
void writeFIFO(int openfifo, Message message);
void unlinkFIFO(char *pathfifo);
int openFIFO(char *pathfifo, int flag);
