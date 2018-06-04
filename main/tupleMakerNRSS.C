/******************************************************************************************** 
* tupleMakerNRSS                                                                            *
*  standalone build:                                                                        *
*  g++ -std=gnu++11 -g `root-config --cflags --libs` tupleMakerNRSS.C -o ./tupleMakerNRSS   *
*                                                                                           *
*  usage: ./tupleMakerNRSS -f datafile [-r nRawEvents]                                     *
*                                                                                           *
*********************************************************************************************/

#include <sys/time.h>
#include <time.h>

#include "../include/DecodeDT5743.h"
#include "../CAENDigitizer.h"
#include "../include/NRSSTree.h"


#define headerSize 4
#define runHeaderSize 4
#define MAX_CH 8
#define MAX_RL 1024

/* Board Id (registro 0xEF08) */
//#define DT5743Id   ??? //To be determined


int DecodeHeader(uint32_t header);

int main(int argc, char **argv) {

int ret;

FILE* file;
//stringhe di comodo per copiare il file 
char* fileName=NULL;    
char* buffer = NULL;
char* header = NULL;
char* scstatus= NULL;
char* timesec= NULL;     
char* timensec= NULL;
char* runnumber= NULL;
char* runconfig= NULL;

char* runnumber = NULL;
char* runconfig = NULL;

// run variables
uint32_t bufferSize=0, test, boardId;
//time variables 
int timeStampSize=timeSecSize+timenSecSize;
uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
uint32_t status=0;

  char  raw;
  int nEvents=0;
  int myc=0;
  int deviceConfig;
  int verbose=0, writeRaw=0;

CAEN_DGTZ_X743_EVENT_t *Evt743= NULL;

int dataSignalDT5743[MAX_CH][MAX_RL];

//root file and Trees;


//Open date file--- 
while ((myc = getopt (argc, argv, "f:r:")) != -1) {
    switch (myc)
      {
      case 'f':
	fileName=(char *) malloc (strlen (optarg) + 1);
	strcpy ((char *) fileName, optarg);
	break;
      case 'r':
	strcpy(&raw,optarg);
	writeRaw=atoi(&raw);
        printf("%i raw events will be written out \n", writeRaw);
        break;
      }
  }

  if(fileName==NULL) {
    printf("usage: tupleMakerNRSS -f fileName [-r nRawEvents] \n");
    exit(1);
  }
  file = fopen (fileName, "r");

header=(char*) malloc(headerSize);

// Read the run header...
runnumber=(char*) malloc(runHeaderSize);
if(!fread(runnumber,runHeaderSize,1,file)) {
    printf("*** Cannot find the run number! ");
    return(0);
	}
uint32_t runNumber=*(uint32_t *)(runnumber);
printf("tupleMakerNRSS: building events for run %i \n",runNumber);
runconfig=(char*) malloc(runHeaderSize);
if(!fread(runconfig,runHeaderSize,1,file)) { 
    printf("*** Cannot find the run configuration! ");
    return(0);
  }
uint32_t runConfig=*(uint32_t *)(runconfig);
deviceConfig=DecodeHeader(runConfig);  


//Create the root file
char rootFile[30],rn[10];
strcpy(rootFile,"Run");
sprintf(rn,"%i",runNumber);
strcat(rootFile,rn);
strcat(rootFile,"_eventsNRSS.root");
NRSSTree* myEvt= new NRSSTree(rootFile);


//Main loop over datafile

  while(!feof(file)) {        
    nEvents++;

   	//status
   	if(!fread(scstatus,scstatusSize,1,file)) break;
    status= *(uint32_t *)(scstatus);
    printf("status: %d \n",status);

    //PC timestamp 
    if(!fread(timesec,timeSecSize,1,file)) break;
    evTime=*(uint32_t *)(timesec);
    if(nEvents==1) startTime=evTime;
    printf("   evTime: %u \n",evTime);
    printf("startTime: %u \n",startTime);
    time_t mysec=*(uint32_t *)(timesec);
    if((nEvents % 100)==0)  printf("DataTime: %s \n",ctime(&mysec));
    if(!fread(timensec,timenSecSize,1,file)) break;
    nsecTime=*(uint32_t*)(timensec);
    if(nEvents==1) nsecStartTime=nsecTime;
    printf("     nsecTime: %u \n",nsecTime);
    printf("nsecStartTime: %u \n",nsecStartTime);
    timeStamp=(evTime-startTime)*1.e9 + nsecTime - nsecStartTime;
    timeStamp /= 1.e9;
    printf("timeStamp: %f \n",timeStamp);

    //buffer size
    if(!fread(header,headerSize,1,file)) break;
    bufferSize=*(long *) (header) & 0x0FFFFFFF;
    bufferSize=bufferSize*4;				//???????
    
    buffer=(char *)malloc(bufferSize);	//<--questo è il buffer evento!

    //Decode event
    ret = CAEN_DGTZ_DecodeEventDT5743(buffer,Evt743);

    //extract all info from Evt743
    //and put them inside the base tree (EvTree)

    myEvt->Store(Evt743, timeStamp, nEvents, runNumber);
    myEvt->FillTree();

	}

return 0;
}




int DecodeHeader(uint32_t header) {
  unsigned int myHeader=0;

  //read header... 

  return myHeader;
}