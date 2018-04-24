#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/shm.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "SharedMemorySetup.h"
#include "Define.h"

int main(int argc, char **argv)
{
  int verbose=0;

  char* myRead = NULL;
  FILE* file;
  char* fileName;
  char* name1;
  char* name2;
  char* baseFileName;
  DaqSharedMemory* cDaqSharedMemory;
  circBuffer* cSharedBuffer;
  int myc=0;
  int scalerAlfa;
  char chanMask;
  uint32_t boardId;
  int NAlfa=0;
  struct timespec runTimeStamp;
  char Device[10];
  uint32_t test, bufferSize=0;
  int bufcnt=0;
  int isCalo=0, isCompton=0;
  char strnrun[100];
  char strDate[15];
  char logFilename[100];
  uint32_t RunConfig = 0x00000000;
    
  while ((myc = getopt (argc, argv, "f:d:")) != -1)
  switch (myc) {
    case 'f':
      baseFileName=(char *) malloc (strlen (optarg) + 1);
      strcpy ((char *) baseFileName, optarg);
      break;
    case 'd':
      strcpy ((char *) Device, optarg);
      if(strstr(Device, "GCAL")!=NULL) isCalo=1;
      else if(strstr(Device, "CSPEC")!=NULL) isCompton=1;
      break;
  }

  char* posPoint = strstr(baseFileName, "Data");
  int pos = posPoint-baseFileName;
  name1=(char *) malloc (strlen (baseFileName) + 100);
  strcpy(name1,"");
  strncpy(name1,baseFileName,pos+8);
  name1[pos+8]='\0';
  //printf("Consumer: name1 %s\n",name1);
  name2=(char *) malloc (strlen (baseFileName) + 100);
  strcpy(name2,"");
  strcat(name2,posPoint+12);
  //printf("Consumer: name2 %s\n",name2);

  printf("\n%s Consumer: Configuring SharedMemory \n",Device);
  cDaqSharedMemory=configDaqSharedMemory("Consumer");

  printf("\n%s Consumer:: Configuring SharedBuffer \n",Device);
  if(isCompton) {
    cSharedBuffer=configSharedBuffer(keyCompton);
    scalerAlfa=cDaqSharedMemory->AlfaScaler;
  } 
  else cSharedBuffer=configSharedBuffer(keyCalo);
 
  while(!cDaqSharedMemory->stopDAQ) {
    if(cSharedBuffer->head==0) break;
  }
  cSharedBuffer->tail=0;

  fileName=(char *) malloc (strlen (baseFileName) + 100);

  while(1) {
    if(cDaqSharedMemory->stopDAQ) break;

    printf("\n%s Consumer: Resetting event counters \n",Device);
    if(isCompton) {
      cDaqSharedMemory->EventsCons[BaFGamma]=0;
      cDaqSharedMemory->EventsCons[BaFAlfa]=0;
      cDaqSharedMemory->EventsCons[HpGe]=0;
      cDaqSharedMemory->EventsCons[PamSi]=0;

      if(cDaqSharedMemory->IncludeBaF)      RunConfig|= (0x1 << 0);
      if(cDaqSharedMemory->IncludeHpGe)     RunConfig|= (0x1 << 1);
      if(cDaqSharedMemory->testHpGe)        RunConfig|= (0x1 << 2);
      if(cDaqSharedMemory->IncludePamSi)    RunConfig|= (0x1 << 3);
      RunConfig|= (cDaqSharedMemory->AlfaScaler << 18);
    } 
    else {
      RunConfig |= (0x1 << 4);
      cDaqSharedMemory->EventsCons[Calo]=0;
    }
    if(cDaqSharedMemory->softwareTrigger) {
      RunConfig|= (0x1 << 5);
      RunConfig|= (cDaqSharedMemory->SoftwareTrgRate << 6);
    }
    printf("RunConfig %u \n",RunConfig);
    
    int nRun=cDaqSharedMemory->runNumber;
    sprintf(strnrun,"%i",nRun);
    sprintf(logFilename,"Log/LogFile_Run%i",nRun);
    freopen (logFilename,"a",stdout);
    printf("\n%s Consumer: Opening %s \n",Device,logFilename);
    fflush(stdout);

    clock_gettime(CLOCK_REALTIME, &runTimeStamp);
    struct tm* mytime=localtime(&runTimeStamp.tv_sec);
    sprintf(strDate,"%02i%02i%i_%02i%02i%02i",mytime->tm_mday,mytime->tm_mon+1,mytime->tm_year+1900,mytime->tm_hour, mytime->tm_min, mytime->tm_sec);
    strcpy(fileName,"");
    strcat(fileName,name1);
    strcat(fileName,strnrun);
    strcat(fileName,"_");
    strcat(fileName,name2);
    strcat(fileName,"_");
    strcat(fileName,strDate);

    printf("%s Consumer: Opening %s \n",Device,fileName);
    file = fopen (fileName, "w+");
    if(file==NULL) printf("ERROR IN OPENING FILE %s \n",fileName);
    fflush(stdout);

    // Write the run header
    myRead = (char*)malloc(runHeaderSize);  
    myRead = (char*) &nRun;
    fwrite (myRead, runHeaderSize, 1, file);

    myRead = (char*)malloc(runHeaderSize);  
    myRead = (char*) &RunConfig;
    fwrite (myRead, runHeaderSize, 1, file);

    while(cDaqSharedMemory->runNumber==nRun) { // Loop on run number

      if((cSharedBuffer->head - cSharedBuffer->tail)<=evtHeaderSize) {
        if(cDaqSharedMemory->stopDAQ) break;
        usleep(1000); // waiting for new data
        continue; 
      }

      bufferSize=readEventSize(cSharedBuffer, cSharedBuffer->tail);
      myRead=(char*)malloc(bufferSize+evtHeaderSize);  
      cSharedBuffer->tail = readCircularBuffer(cSharedBuffer,myRead,bufferSize+evtHeaderSize,cSharedBuffer->tail);
      fwrite (myRead, bufferSize+evtHeaderSize, 1, file);

      if(isCompton) {
	chanMask = *(long *)(myRead+evtHeaderSize+caenHeaderSize) & 0x0000000F;
	 boardId = *(long *)(myRead+evtHeaderSize+caenHeaderSize) & 0xF8000000;
	if(boardId==DT5780Id) {
	  cDaqSharedMemory->EventsCons[HpGe]++;
	}
	else if(boardId == V1742BaFId) { //se e' BaF
	  if(!(chanMask & 0x1)) {  // se e' solo alfa
	    NAlfa++;
	    if((NAlfa%scalerAlfa)==0) 
	      cDaqSharedMemory->EventsCons[BaFAlfa]++;
	  } else {  
	    cDaqSharedMemory->EventsCons[BaFGamma]++;
	    if((chanMask>>2) & 0x1) cDaqSharedMemory->EventsCons[BaFAlfa]++;
	  }
	} else {  // e' PamSi
	  cDaqSharedMemory->EventsCons[PamSi]++;
	}
      } else {  // is Calo
	cDaqSharedMemory->EventsCons[Calo]++;
      }
      free(myRead);
     
    } // end loop on runNumber
 
    fclose(file);

    if(isCalo) printf("\n\n%s Consumer: written %i %s events to file %s \n",Device,cDaqSharedMemory->EventsCons[Calo],Device,fileName);
    
    if(isCompton) {
      printf("\n\n%s Consumer: written %i events to file %s \n",Device,
      cDaqSharedMemory->EventsCons[BaFGamma]+cDaqSharedMemory->EventsCons[BaFAlfa]+cDaqSharedMemory->EventsCons[HpGe]+cDaqSharedMemory->EventsCons[PamSi],fileName);
      printf("%s Consumer: written %i BaF Gamma events \n",Device,cDaqSharedMemory->EventsCons[BaFGamma]);
      printf("%s Consumer: written %i BaF Alfa events \n",Device,cDaqSharedMemory->EventsCons[BaFAlfa]);
      printf("%s Consumer: written %i HpGe events \n",Device,cDaqSharedMemory->EventsCons[HpGe]);
      printf("%s Consumer: written %i PamSi events \n",Device,cDaqSharedMemory->EventsCons[PamSi]);
    }

    fflush(stdout);
    fclose (stdout); 

  }// end while daq active

}
