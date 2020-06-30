#include "includes/defines.h"
#include "includes/fifo.h"
#include "includes/semaphore.h"
#include "includes/shared_memory.h"
#include "includes/device.h"
#include "includes/acknowledgment.h"

void serverSignalMask();
void serverSignalHandler(int signal);
void createDevices(char * filePath, int semaphoreid);
void createAckManager(int messageQueueKey, int semaphoreid);
