#ifndef SLOW_CONTROL
#define SLOW_CONTROL

#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"
#include <CAENHVWrapper.h>
#include "rs232.h"
#include "SharedMemorySetup.h"
#include "Define.h"

#define nbafHV 1
#define nbafLV 2

#define ngcalHV 11
#define ngcalLV 2

typedef struct
{
  unsigned short HVList[nbafHV];
  unsigned short LVList[nbafLV];

  int ParStatus[nbafHV], ParStatusL[nbafLV];

  float ParVMon[nbafHV],ParIMon[nbafHV], ParV0Set[nbafHV], ParI0Set[nbafHV], ParRup[nbafHV], ParRdwn[nbafHV], ParTrip[nbafHV];
  float ParVMonL[nbafLV],ParIMonL[nbafLV], ParV0SetL[nbafLV], ParI0SetL[nbafLV], ParRupL[nbafLV], ParRdwnL[nbafLV], ParVConL[nbafLV];

  GtkWidget *eVMon[nbafHV], *eIMon[nbafHV], *eV0Set[nbafHV], *eI0Set[nbafHV], *ePw[nbafHV], *eStatus[nbafHV], *eRup[nbafHV], *eRdwn[nbafHV], *eTrip[nbafHV];
  GtkWidget *eVMonL[nbafLV], *eVConL[nbafLV], *eIMonL[nbafLV], *eV0SetL[nbafLV], *eI0SetL[nbafLV], *ePwL[nbafLV], *eStatusL[nbafLV], *eRupL[nbafLV], *eRdwnL[nbafLV];

  GtkWidget *sbHVvalue, *sbLVvalue;

  GtkWidget *cHVCommand, *cHVSelector, *cLVCommand, *cLVSelector;

} bafWidget_t;


typedef struct
{
  unsigned short HVList[ngcalHV];
  unsigned short LVList[ngcalLV];

  int ParStatus[ngcalHV], ParStatusL[ngcalLV];

  float ParVMon[ngcalHV],ParIMon[ngcalHV], ParV0Set[ngcalHV], ParI0Set[ngcalHV], ParRup[ngcalHV], ParRdwn[ngcalHV], ParTrip[ngcalHV];
  float ParVMonL[ngcalLV],ParIMonL[ngcalLV], ParV0SetL[ngcalLV], ParI0SetL[ngcalLV], ParRupL[ngcalLV], ParRdwnL[ngcalLV], ParVConL[ngcalLV];

  GtkWidget *eVMon[ngcalHV], *eIMon[ngcalHV], *eV0Set[ngcalHV], *eI0Set[ngcalHV], *ePw[ngcalHV], *eStatus[ngcalHV], *eRup[ngcalHV], *eRdwn[ngcalHV], *eTrip[ngcalHV];
  GtkWidget *eVMonL[ngcalLV], *eVConL[ngcalLV], *eIMonL[ngcalLV], *eV0SetL[ngcalLV], *eI0SetL[ngcalLV], *ePwL[ngcalLV], *eStatusL[ngcalLV], *eRupL[ngcalLV], *eRdwnL[ngcalLV];

  GtkWidget *sbHVvalue, *sbLVvalue;

  GtkWidget *cHVCommand, *cHVSelector, *cLVCommand, *cLVSelector;

} gcalWidget_t;


// Slot number for HV/LV boards in the SY5527 mainframe
unsigned short slotA7030, slotA2518, slotA1536;


// main
void create_main_window();
void setupFrameBaF();
void setupFrameGCAL();
void startSC();
void stopSC();
void exitSC();
void ClearCrateAlarm();
void CrateMap(int);
void GetTime(char timeNow[21]);
//void ApplyCommand(int, int, char*, float);
void ApplyCommand(int, int, char command[30], float);
int DecodeStatus(int, char status[20]);
void maskBaF();
void maskGCAL();


//BaF
void bafHVmonitor();
void bafLVmonitor();
void bafSendHVCommand();
void bafSendLVCommand();

// GCAL
void gcalHVmonitor();
void gcalLVmonitor();
void gcalSendHVCommand();
void gcalSendLVCommand();
void MonitorError(char parName[20], int);


#endif
