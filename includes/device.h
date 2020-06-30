#include "defines.h"

void deviceSignalHandler(int signal);
void setDeviceSignalMask();
void startDevice(char * filepath, int semaphore, int device, int board_sharedmemory, int acklist_sharedmemory);
