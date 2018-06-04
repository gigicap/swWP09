/************************************
*
*  Routine to decode DT5743 events  
*  to avoid connection to Digitizer 
*  (to be called instead of DecodeEvent)
* 
*************************************/

#define MAX_GR 4
#define HEA_SZ 4 

CAEN_DGTZ_ErrorCode CAENDGTZ_API CAEN_DGTZ_DecodeEventDT5743(uint32_t *data, CAEN_DGTZ_X743_EVENT_t *5743Event){

//check for Board failure
if (!(data[1] & 0x4000000))
    return CAEN_DGTZ_InvalidEvent;

//total event size
uint32_t size = data[0] & 0xFFFFFFF;
//active groups in the event 
uint32_t gr = data[1] & 0xF;

for (int i = 0; i < MAX_GR; ++i)
	5743Event->GrPresent[i] = 0;

if(gr & 0x1) 5743Event->GrPresent[0] = 1;
if(gr & 0x2) 5743Event->GrPresent[1] = 1; 
if(gr & 0x4) 5743Event->GrPresent[2] = 1; 
if(gr & 0x8) 5743Event->GrPresent[3] = 1; 

//check record length
if ((size - HEA_SZ) % MAX_GR)
    return CAEN_DGTZ_InvalidEvent;

uint32_t RecordLength = (size-4)/4;

//event counter 
uint32_t EvtCnt = data[1] & 0x3FFFFF;


5743Info.recordlength = RecordLength;
5743Info.evtcnt = EvtCnt;



//ReadPerGroup
for (i=0; i< MAX_GR; i++) {
if(5743Event->GrPresent[i]==0) continue;
CAEN_DGTZ_GROUP_t 5743Group;
int idx = HEA_SZ + i*RecordLength;

uint64_t extendedTimestamp = (data[idx+13] & 0xFF000000) +
(data[idx+14] & 0xFF000000)>>0xFF +
(data[idx+15] & 0xFF000000)>>0xFFFF +
(data[idx+16] & 0xFF000000)>>0xFFFF +
(data[idx+17] & 0xFF000000)>>0xFFFFFF;
5743Group.TDC = extendedTimestamp; 

5743Group.EventId = (uint8_t)(data[idx+10] & 0xFF000000);

for(j=0; j<RecordLength; j++){
5743Group.DataChannel[0][j] = 
data[idx+j]& 0xFFF;
5743Group.DataChannel[1][j] = 
data[idx+j]& 0xFFF000; 
}

5743Event->DataGroup[i] = 5743Group;
}




return CAEN_DGTZ_Success;
}

