#include "SharedMemorySetup.h"

/************************************************************************************ */
DaqSharedMemory* configDaqSharedMemory(char* caller) {
/************************************************************************************ */

  printf("configDaqSharedMemory: configuring the shared memory for %s \n",caller);

  DaqSharedMemory* myDaqSharedMemory = malloc(sizeof(*myDaqSharedMemory));

  keyDaq=7789;
  //keyDaq=9515;
  int shmSize=sizeof(*myDaqSharedMemory);

  // create the segment
  if((shmidDaq = shmget(keyDaq, shmSize, IPC_CREAT | 0666))<0) {
     printf("*** configDaqSharedMemory: ERROR - memory segment not created \n");
     return NULL;
  } 
  else {
     printf("configDaqSharedMemory: DaqSharedMemory created (size %i) \n",shmSize);
  }

  // attach the segment to our data space
  myDaqSharedMemory = shmat(shmidDaq, NULL,0);
  if( myDaqSharedMemory == (char*)-1) {
    printf("***configDaqSharedMemory ERROR -  memory segment not attached to data space \n");
    return NULL;
  }

  return myDaqSharedMemory;
}


/*************************************************************************************/
void deleteDaqSharedMemory() {
/*************************************************************************************/
  if ( (shmidDaq = shmget (keyDaq, 0, 0)) >= 0 ) {
    printf("deleting DaqSharedMemory.. \n");
    if ( shmctl (shmidDaq, IPC_RMID, NULL) < 0 )
      printf("error in deleting DaqSharedMemory \n");
    else  printf("DaqSharedMemory deleted \n");
  }
}



/*************************************************************************************/
circBuffer* configSharedBuffer(key_t key) {
/*************************************************************************************/
  circBuffer* SharedBuffer = malloc(sizeof(*SharedBuffer));

  uint32_t shmSize=CIRCBUFFER_SIZE;
  SharedBuffer->buffer=(char*)malloc(shmSize);

  // create the segment
  int shmid;
  if((shmid = shmget(key, shmSize, IPC_CREAT | 0666))<0) {
    printf("SharedMemorySetup::configSharedBuffer Error: memory segment not created \n");
    return NULL;
  } 
  else {
    printf("configSharedBuffer:: shared memory size %i \n",shmSize);
  }

  // attach the segment to our data space
  SharedBuffer = shmat(shmid, NULL,0);
  if( SharedBuffer == (char*)-1) {
    printf("*** configSharedBuffer: ERROR - memory segment not attached to data space \n");
    return NULL;
  }

  //  SharedBuffer->head=0;  //la azzera il producer, altrimenti quando parte il plotter si riazzera di nuovo
  //  SharedBuffer->tail=-1;
  SharedBuffer->maxSize=shmSize;

  printf("SharedMemorySetup: SharedBuffer configured - Key: %i\n",key);

  return SharedBuffer;
}



/*******************************************************************************************/
void writeCircularBuffer(circBuffer* sharedBuffer, char* bufferData, uint32_t bufferSize) {
/*******************************************************************************************/
  uint32_t size1, size2;

  int32_t dist=sharedBuffer->tail-sharedBuffer->head;
  if(dist <= 0) dist+=sharedBuffer->maxSize;
  //  printf("bufferSize %u dist %i \n",bufferSize,dist);
  if(bufferSize > dist && sharedBuffer->tail!=-1 ) { //se tail=-1 il consumer non sta girando
    printf("WARNING:: No space left on sharedBuffer \n");
    sharedBuffer->head -= evtHeaderSize;
  } 
  else { 
    if(sharedBuffer->head+bufferSize <= sharedBuffer->maxSize) {
      // write in a single step
      memcpy((void*) &(sharedBuffer->buffer)+sharedBuffer->head, (void*)bufferData, bufferSize);
      sharedBuffer->head += bufferSize;
      if(sharedBuffer->head == sharedBuffer->maxSize) sharedBuffer->head =0;
    } 
    else {
      // write in two steps 
      size1=sharedBuffer->maxSize-sharedBuffer->head;
      memcpy((void*) &(sharedBuffer->buffer)+sharedBuffer->head, (void*)bufferData, size1);
      size2=bufferSize-size1;
      memcpy((void*) &(sharedBuffer->buffer), (void*)bufferData+size1, size2);
      sharedBuffer->head=size2;
    }
  } 
}



/*************************************************************************************/
void writeTimeStamp(circBuffer* sharedBuffer) {
/*************************************************************************************/
  int timeWordSize=4;
  struct timespec currTime;
  clock_gettime(CLOCK_REALTIME, &currTime);
  uint32_t mysec=currTime.tv_sec;
  uint32_t mynsec=currTime.tv_nsec;
  //printf("mysec %i sizeof %i\n", mysec,sizeof(mysec));
  //printf("mynsec %i sizeof %i\n", mynsec,sizeof(mynsec));
  writeCircularBuffer(sharedBuffer, (char*) &mysec, timeWordSize);
  writeCircularBuffer(sharedBuffer, (char*) &mynsec,timeWordSize);
}


/************************************************************************************************************/
uint32_t readCircularBuffer(circBuffer* sharedBuffer, char* readData, uint32_t bufferSize, uint32_t mytail) {
/************************************************************************************************************/
  uint32_t size1, size2;

  if(mytail+bufferSize <= sharedBuffer->maxSize) {
    // read in a single step
    memcpy((void*) readData, (void*) &(sharedBuffer->buffer)+mytail,bufferSize);
    mytail += bufferSize;
    if(mytail == sharedBuffer->maxSize) mytail =0;
  } else {
    // read in two steps 
    size1=sharedBuffer->maxSize-mytail;
    memcpy((void*) readData, (void*) &(sharedBuffer->buffer)+mytail, size1);
    size2=bufferSize-size1;
    memcpy((void*) readData +size1,(void*) &(sharedBuffer->buffer), size2);
    mytail=size2;
  }
  return mytail;
}




/*************************************************************************************/
uint32_t readEventSize(circBuffer* sharedBuffer, uint32_t mytail) {
/*************************************************************************************/
  char* caenHeader = NULL;
  caenHeader = (char*) malloc(caenHeaderSize);
  mytail += evtHeaderSize;
  readCircularBuffer(sharedBuffer, caenHeader, caenHeaderSize, mytail);

  uint32_t bufferSize=*(long *) (caenHeader) & 0x0FFFFFFF;
  bufferSize=bufferSize*4;
  //  printf("readEventSize: bufferSize %u sharedtail %u mytail %u  \n",bufferSize,sharedBuffer->tail,mytail );
  free(caenHeader);
  return bufferSize;
}
