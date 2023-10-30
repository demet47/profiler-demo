#ifndef KERNEL_H
#define KERNEL_H


//returns 0 for termination flag raise, 1 for not
int psx_devctl(int fdDev, int cmd, void* dataChannel, int size, int* extraData);

//returns fDev
int psx_devctl_open();

void psx_devctl_close();


#endif