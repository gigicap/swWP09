#include <CAENDigitizer.h>
#include <CAENVMElib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>
#include "SharedMemorySetup.h"
#include "UtilsFunctions.h"
#include "V1485Configurator.h"

circBuffer* pSharedBuffer;
int handleV1742, handleDT5780, handleV1485;
DaqSharedMemory* pDaqSharedMemory;
uint32_t pBufferSizeV1742, pAllocatedSizeV1742,pBufferSizeDT5780, pAllocatedSizeDT5780;
int err_code;
char *pbufferV1742 = NULL;  
char *pbufferDT5780 = NULL;  
uint32_t *pbufferV1485;        // Memory buffer for blt
uint32_t baseAddressV1485;     // Base Address
int isCalo=0, isCompton=0;
char logFilename[100];

double mytimestart, mytimestop;
int isV1742=0;
int isDT5780=0;
int isV1485=0;
int isSoftwareTrigger=0;
int isTestHpGe = 0;
int  TrgCntV1742 = 0;
int  TrgCntAlfa = 0;
int  TrgCntDT5780 = 0;
int  TrgCntPamSi = 0;
int  TrgCntGamma = 0;
char Device[10];

void startDaqV1742() {
  err_code=0;
  if(isCalo) 
    err_code |= CAEN_DGTZ_OpenDigitizer(pDaqSharedMemory->connectionParamsV1742Calo[0],pDaqSharedMemory->connectionParamsV1742Calo[1],pDaqSharedMemory->connectionParamsV1742Calo[2] , pDaqSharedMemory->connectionParamsV1742Calo[3], &handleV1742);
  else 
    err_code |= CAEN_DGTZ_OpenDigitizer(pDaqSharedMemory->connectionParamsV1742BaF[0],pDaqSharedMemory->connectionParamsV1742BaF[1],pDaqSharedMemory->connectionParamsV1742BaF[2] , pDaqSharedMemory->connectionParamsV1742BaF[3], &handleV1742);

  if(err_code!=0) printf("open digitizer error %i \n",err_code);
  err_code |= CAEN_DGTZ_MallocReadoutBuffer(handleV1742, &pbufferV1742,&pAllocatedSizeV1742);
  if(err_code!=0) printf("malloc readout buffer error %i \n",err_code);
  err_code |= CAEN_DGTZ_ClearData(handleV1742);
  if(err_code!=0) printf("clear data error %i \n",err_code);
  err_code |= CAEN_DGTZ_SWStartAcquisition(handleV1742);

  if(!err_code) printf("\n%s Producer: acquisition started for board V1742 %i \n",Device,err_code);
  else {
    printf("***%s Producer: ERROR opening V1742 device %i \n",Device,err_code);
    exit(1);
  }
}

void startDaqDT5780() {
  // The slow control starter opens the connection to the DT5780 board, the handle is passed via the shared memory structure
  err_code |= CAEN_DGTZ_OpenDigitizer(pDaqSharedMemory->connectionParamsDT5780[0],pDaqSharedMemory->connectionParamsDT5780[1],pDaqSharedMemory->connectionParamsDT5780[2] , pDaqSharedMemory->connectionParamsDT5780[3], &handleDT5780);
  //handleDT5780 = pDaqSharedMemory->DT5780handle;

  err_code |= CAEN_DGTZ_MallocReadoutBuffer(handleDT5780, &pbufferDT5780,&pAllocatedSizeDT5780);
  err_code |= CAEN_DGTZ_ClearData(handleDT5780);
  err_code |= CAEN_DGTZ_SWStartAcquisition(handleDT5780);
  
  if(!err_code) {
    printf("\n%s Producer: acquisition started for board DT5780 %i \n",Device,err_code);
  }
  else {
    printf("*** %s Producer: ERROR opening DT5780 device %i \n",Device,err_code);
    exit(1);
  }
 
}



void startDaqV1485() {
  if( CAENVME_Init(cvV2718,pDaqSharedMemory->connectionParamsV1485[0] , 0, &handleV1485) == cvSuccess ) 
    printf("\n%s Producer: Acquisition started for board V1485 %i \n",Device,err_code);
  else {
    printf("*** %s Producer: ERROR opening V1485 device %i \n",Device,CAENVME_DecodeError(err_code));
    exit(1);
  }

  pbufferV1485 = (uint32_t *)malloc(1024);
  baseAddressV1485 = pDaqSharedMemory->connectionParamsV1485[1]<<16;
}



void stopDaq(int handle) {
  err_code = CAEN_DGTZ_SWStopAcquisition(handle);
  CAEN_DGTZ_ClearData(handle);
  err_code = CAEN_DGTZ_CloseDigitizer(handle);
}


void PrintStat() {
   mytimestop=getTime();
   if(isCalo) {
     printf("\n\n%s Producer: Trigger count V1742: %i \n",Device,TrgCntV1742);
     printf("%s Producer: Trigger Rate  V1742: %f \n",Device,TrgCntV1742/(mytimestop-mytimestart));
   } else {
     if(isV1742) {
       printf("\n\n%s Producer: Trigger count V1742: %i \n",Device, TrgCntV1742);
       printf("%s Producer: Trigger Rate  V1742: %f \n",Device,TrgCntV1742/(mytimestop-mytimestart));
       printf("%s Producer: Trigger count Gamma: %i \n",Device,TrgCntGamma);
       printf("%s Producer: Trigger Rate  Gamma: %f \n",Device,TrgCntGamma/(mytimestop-mytimestart));
       printf("%s Producer: Trigger count Alpha: %i \n",Device,TrgCntAlfa);
       printf("%s Producer: Trigger Rate  Alpha: %f \n",Device,TrgCntAlfa/(mytimestop-mytimestart));
     }
     if(isDT5780) {
       printf("%s Producer: Trigger count HpGe: %i \n",Device,TrgCntDT5780);
       printf("%s Producer: Trigger Rate  HpGe: %f \n",Device,TrgCntDT5780/(mytimestop-mytimestart));
     }
     
     if(isV1485) {
       printf("%s Producer: Trigger count PamSi: %i \n",Device,TrgCntPamSi);
       printf("%s Producer: Trigger Rate  PamSi: %f \n",Device,TrgCntPamSi/(mytimestop-mytimestart));
     }
   }

}


void UpdateLogFile() {
  PrintStat();
  fflush(stdout);
  fclose (stdout);

  TrgCntV1742 = 0;
  TrgCntAlfa = 0;
  TrgCntDT5780 = 0;
  TrgCntPamSi = 0;
  TrgCntGamma = 0;

  sprintf(logFilename,"Log/LogFile_Run%i",pDaqSharedMemory->runNumber);
  freopen (logFilename,"a",stdout);
  printf("%s Producer: Opening %s \n",Device,logFilename);
  fflush(stdout);
}




int main(int argc, char **argv)  {

  int stopdaq;
  char chanMask;
  int timeoutDT5780 = 10; // 1ms
  uint32_t data;          // Data
  CVAddressModifier  AM = cvA32_U_DATA ;// Addressing Mode
  CVDataWidth DW = cvD32 ;		// Data Format
  uint32_t  bltsV1485 = 4096 ;		// Block size for blt (bytes)
  int SleepTime;
  int ready,nbyte,nword32bit;
  int myc=0;
  int currRun;


  while ((myc = getopt (argc, argv, "d:")) != -1)
    switch (myc)
      {
      case 'd':
	strcpy ((char *) Device, optarg);
	if(strstr(Device, "GCAL")!=NULL) isCalo=1;
	else if(strstr(Device, "CSPEC")!=NULL) isCompton=1;
	break;
      }

  printf("\n%s Producer: Configuring DAQSharedMemory \n",Device);
  pDaqSharedMemory=configDaqSharedMemory("Producer");

  sprintf(logFilename,"Log/LogFile_Run%i",pDaqSharedMemory->runNumber);
  freopen (logFilename,"a",stdout);
  printf("\n%s Producer: Opening %s \n",Device,logFilename);

  printf("\n%s Producer: Configuring SharedBuffer \n",Device);
  if(isCompton) {
    isV1742=pDaqSharedMemory->IncludeBaF;
    isDT5780=pDaqSharedMemory->IncludeHpGe;
    isV1485=pDaqSharedMemory->IncludePamSi;
    isTestHpGe=pDaqSharedMemory->testHpGe;
    pSharedBuffer=configSharedBuffer(keyCompton);
  } 
  else if(isCalo){
    pSharedBuffer=configSharedBuffer(keyCalo);
  }
  pSharedBuffer->tail=-1;
  pSharedBuffer->head=0;
  isSoftwareTrigger=pDaqSharedMemory->softwareTrigger;
  if(isSoftwareTrigger || isTestHpGe) SleepTime= (int) 1000000/pDaqSharedMemory->SoftwareTrgRate;

  if(isDT5780) startDaqDT5780();
  if(isV1742 || isCalo) startDaqV1742();
  if(isV1485 || isTestHpGe) startDaqV1485();
  fflush(stdout);

  mytimestart=getTime();
  currRun=pDaqSharedMemory->runNumber;
 
  while(pDaqSharedMemory->stopDAQ==0) {
 
    if(pDaqSharedMemory->runNumber!=currRun) {
      currRun=pDaqSharedMemory->runNumber;
      UpdateLogFile();
      printf("\n%s Producer: Starting new run %i\n",Device,currRun);
      mytimestart=getTime();
    }

    if (isSoftwareTrigger) usleep(SleepTime);

    if(isV1742 || isCalo) {    
      if (isSoftwareTrigger) CAEN_DGTZ_SendSWtrigger(handleV1742);  
      err_code = CAEN_DGTZ_ReadData(handleV1742, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, pbufferV1742, &pBufferSizeV1742);
      if(err_code!=0) {
        printf("*** %s Producer: ERROR reading V1742 data %i\n",Device,err_code);
        fflush(stdout);
      }
      if (pBufferSizeV1742 == 0) {
        usleep(100);
        if(pDaqSharedMemory->stopDAQ==0) continue;
        else break;
      } 

      if(isCalo) writeCircularBuffer(pSharedBuffer, (char*) &pDaqSharedMemory->gcalStatus,4);
      else writeCircularBuffer(pSharedBuffer, (char*) &pDaqSharedMemory->bafStatus,4);
      writeTimeStamp(pSharedBuffer);
      writeCircularBuffer(pSharedBuffer, pbufferV1742, pBufferSizeV1742);
      TrgCntV1742++;
 
      if(isCompton) {
	chanMask = *(long *)(pbufferV1742+4) & 0x0000000F;
	if((chanMask >> 2) & 0x1)  {
	  pDaqSharedMemory->EventsProd[BaFAlfa]++;
	  TrgCntAlfa++;
	}
	if(!(chanMask & 0x1)) { // se non c'e' il gamma
	  continue;  // it is only alpha particle
        } 
        else {
	  TrgCntGamma++;
	  pDaqSharedMemory->EventsProd[BaFGamma]++;
	}
      } 
      else {
        pDaqSharedMemory->EventsProd[Calo]=TrgCntV1742;
        continue;
      }
    } //end V1742 block

    if(isDT5780) {
      if(isTestHpGe) {
	usleep(SleepTime);
	data = DATA_V1485_SENDEXTTRG;
	err_code =CAENVME_WriteCycle(handleV1485, baseAddressV1485 | REG_V1485_SOFTTRG, &data,AM, DW);
	if(err_code != cvSuccess) {
	  printf("*** %s Producer: ERROR sending external trigger to V1485: %s \n",Device,CAENVME_DecodeError(err_code)) ; 
	  fflush(stdout);
	}
      }

      if (isSoftwareTrigger) CAEN_DGTZ_SendSWtrigger(handleDT5780);

      int nloop=0;
      while(nloop<=timeoutDT5780) {
        err_code = CAEN_DGTZ_ReadData(handleDT5780, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, pbufferDT5780, &pBufferSizeDT5780);
	if(err_code!=0) {
          printf("*** %s Producer: ERROR reading DT5780 data %i\n",Device,err_code);
          fflush(stdout);
	}
	if(pBufferSizeDT5780 == 0) {
	  nloop++;
	  usleep(100);
	  if(pDaqSharedMemory->stopDAQ) break;
	} 
        else break;
      }

      if(nloop < timeoutDT5780) {
        writeCircularBuffer(pSharedBuffer, (char*) &pDaqSharedMemory->hpgeStatus,4);
        writeTimeStamp(pSharedBuffer);
        writeCircularBuffer(pSharedBuffer, pbufferDT5780, pBufferSizeDT5780);
        TrgCntDT5780++;
        pDaqSharedMemory->EventsProd[HpGe]++;
      } 
      else {
        if(isV1742) {
          printf("*** %s Producer: WARNING - timeout %i us waiting HpGe events \n",Device,timeoutDT5780*100);
	  fflush(stdout);
        }
      }
    } // end DT5780 block


    if(isV1485) {

      if(isSoftwareTrigger) { 
	data = DATA_V1485_SENDSOFTTRG;
	err_code =CAENVME_WriteCycle(handleV1485, baseAddressV1485 | REG_V1485_SOFTTRG, &data,AM, DW);
	if(err_code != cvSuccess) {
	  printf("\n***%s Producer: ERROR sending software trigger to V1485 : %s \n",Device,CAENVME_DecodeError(err_code)) ;
	  fflush(stdout);
	}
      }

      ready=0;
      while(!ready) {
	if(pDaqSharedMemory->stopDAQ) break;
	err_code = CAENVME_ReadCycle(handleV1485, baseAddressV1485 | REG_V1485_DATAREADY,&ready,AM,DW);
	if(err_code != cvSuccess) { 
          printf("\n***%s Producer: ERROR reading V1485 ready register: %s \n",Device,CAENVME_DecodeError(err_code)) ; 
          fflush(stdout);
          usleep(1000);
	}
      }

      printf(" ready: %i \n", ready) ;

      for (int j=0;j<(bltsV1485/4);j++) pbufferV1485[j]=0;
      err_code = CAENVME_BLTReadCycle(handleV1485,baseAddressV1485,(char *)pbufferV1485,bltsV1485,cvA32_U_BLT,DW,&nbyte);
      if(err_code != cvSuccess) {
        printf("\n***%s Producer: ERROR during V1485 BLT read Cycle %s \n",Device,CAENVME_DecodeError(err_code)); 
        printf("                   Read %u bytes \n",nbyte); 
        fflush(stdout);
      }
      writeCircularBuffer(pSharedBuffer, (char*) &pDaqSharedMemory->pamsiStatus,4);
      writeTimeStamp(pSharedBuffer);
      nword32bit=1+nbyte/4; // 1 word di header + le word a 32 bit di dati
      writeCircularBuffer(pSharedBuffer, (char*) &nword32bit, caenHeaderSize);
      writeCircularBuffer(pSharedBuffer, (char*) pbufferV1485, nbyte);
      TrgCntPamSi++;
      pDaqSharedMemory->EventsProd[PamSi]++;

      printf(" Event number: %i  -   Read %u bytes \n", pDaqSharedMemory->EventsProd[PamSi], nbyte) ;

    }

  } // end main loop (stopDAQ==0)

  if(isV1742||isCalo) stopDaq(handleV1742);
  if(isDT5780) stopDaq(handleDT5780);
  if(isV1485) CAENVME_End(handleV1485);
   //   CAEN_DGTZ_FreeReadoutBuffer(&pbufferV1742);
   //   CAEN_DGTZ_FreeReadoutBuffer(&pbufferDT5780);

  PrintStat();
  fflush(stdout);
  fclose (stdout);
}
