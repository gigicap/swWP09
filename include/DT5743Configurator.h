#ifndef DT5743_CONFIGURATION
#define DT5743_CONFIGURATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"

#define MaxDT5743NChannels 8
/*
typedef enum  {
  ENABLED,
  DISABLED,
} TriggerModeDT5743;
*/
typedef struct
{
  CAEN_DGTZ_ConnectionType LinkType;
  int LinkNum;
  uint32_t RecordLength;
  uint32_t MaxEventsBlt;
  uint32_t GroupMask;
  int TestPattern;
  int AcqMode;   //cambiare in charge-wave mode
  CAEN_DGTZ_IOLevel_t IOLevel;
  int ExtTrigger;
  uint8_t PosTrigger;

  uint32_t GroupTrigger; //0x... 0=OR GRUPPO 1=AND GRUPPO
  uint32_t TriggerOut;    //AND OR MAJORITY  0 = OR , 1 = MAJ 1, 2 = MAJ 2, 3 = AND


  int DCOffset[MaxDT5743NChannels];
  int TriggerLevel[MaxDT5743NChannels];
  uint32_t TrgMode[MaxDT5743NChannels];
  int Polarity[MaxDT5743NChannels];      //for charge mode
  int ChTres[MaxDT5743NChannels];
  int ChRefCell[MaxDT5743NChannels];
  int ChLength[MaxDT5743NChannels];

} DT5743Params_t;

int ParseConfigFileDT5743(char* filename, DT5743Params_t *Params);
void WriteConfigFileDT5743(char* filename, DT5743Params_t *Params);

int  ProgramDigitizerDT5743(int connectionParams[4], char* filename);

#endif
