#ifndef DEFINE_H
#define DEFINE_H

/* Configuration file names */
#define configBaF "ConfigV1742BaF.txt"
#define configCalo "ConfigV1742Calo.txt"
#define configHpGe "ConfigDT5780.txt"
#define configDiscr "ConfigV812.txt"
#define configPamSi "ConfigV1485.txt"
#define runNumberFile "RunNumber.txt"

/* Max number of V1742 digitizers */
#define MaxV1742 2

/* Size of Shared memory Buffer */
#define CIRCBUFFER_SIZE 3000000

/* Compton Shared Memory key */
#define keyCompton 5684

/* Calo Shared Memory key */
#define keyCalo 5802

/* Run header (in bytes) */
#define runHeaderSize 4

// Each event data block is preceeed by 3 words of 4 bytes each:
// SlowControlStatus, timeSec, timenSec
#define evtHeaderSize 12

/* Data block header size (in bytes) */
#define caenHeaderSize 4

// Board Id (registro 0xEF08) 
#define DT5780Id 3758096384 //0x1C
#define V1742CaloId  4026531840 //0x1E
#define V1742BaFId  4160749568 //0x1F


/* Password for Advanced Setup */
#define AdvSetupPasswd "eli"

typedef enum {
  BaFGamma,
  BaFAlfa,
  HpGe,
  PamSi,
  Calo,
  Discr,
  BaF,
  Compton,
} Device_t;

// Update rate in sec for the slow control
#define SlowControlSleepTime 2

#endif
