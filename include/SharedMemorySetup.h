#ifndef _SHAREDMEMORYSETUP__H
#define _SHAREDMEMORYSETUP__H

#include <sys/shm.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "Define.h"

typedef struct {
  int stopDAQ;
  int stopComptonPlot;
  int stopCaloPlot;
  int softwareTrigger;
  int testHpGe;
  int SoftwareTrgRate;
  int isLoggerBusy;
  int IncludeBaF; 
  int IncludeHpGe;
  int IncludePamSi; 
  int IncludeGcal; 
  int connectionParamsV1742BaF[4];
  int connectionParamsV1742Calo[4];
  int connectionParamsDT5780[4];
  int connectionParamsV1485[2];
  int runNumber;
  int NumberOfEvents;
  int RunTimeLength;
  int PlotChannelBaF;
  int PlotChannelCalo;
  int PlotChannelHpGe;
  int ComptonPlotScaler;
  int CaloPlotScaler;
  int isCaloPlotGroup;
  int isBaFPlotGroup;
  int AlfaScaler;
  int EventsProd[5];
  int EventsCons[5];

  uint32_t gcalStatus;
  uint32_t bafStatus;
  uint32_t hpgeStatus;
  uint32_t pamsiStatus;

} DaqSharedMemory;

int shmidDaq;
key_t keyDaq;

typedef struct {
  uint32_t head;
  uint32_t tail;
  uint32_t maxSize;
  char* buffer;
} circBuffer;


DaqSharedMemory* configDaqSharedMemory(char* caller); 
void deleteDaqSharedMemory(); 
circBuffer* configSharedBuffer(key_t key); 

void writeCircularBuffer(circBuffer* sharedBuffer, char* bufferData, uint32_t bufferSize);
void writeTimeStamp(circBuffer* sharedBuffer);

uint32_t readCircularBuffer(circBuffer* sharedBuffer, char* readData, uint32_t bufferSize, uint32_t mytail);
uint32_t readEventSize(circBuffer* sharedBuffer, uint32_t mytail);

#endif // _SHAREDMEMORYSETUP__H
