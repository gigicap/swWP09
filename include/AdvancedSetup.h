#ifndef ADVANCED_SETUP
#define ADVANCED_SETUP
#include "../include/Define.h"
/*
#define configBaF "ConfigV1742BaF.txt"
#define configCalo "ConfigV1742Calo.txt"
#define configHpGe "ConfigDT5780.txt"
#define configDiscr "ConfigV812.txt"
#define MaxV1742 2
*/

/*
typedef enum  {
  BaF,
  HpGe,
  Calo,
  Discr
}Device_t;
*/

typedef struct 
{
  GtkWidget *sbV1485LinkNum;
  GtkWidget* sbV1485BaseAddress;
} V1485Widget_t;

typedef struct 
{
  GtkWidget *sbV812LinkNum;
  GtkWidget* sbV812BaseAddress;
  GtkWidget* sbV812Majority;
  GtkWidget *sbV812DeadTime[MaxV812NGroups];
  GtkWidget *sbV812OutputWidth[MaxV812NGroups];
  GtkWidget *sbV812Threshold[MaxV812NChannels];
  GtkWidget *cV812Channel[MaxV812NChannels];
} V812Widget_t;

typedef struct 
{
  GtkWidget *sbDT5780LinkNum;
  GtkWidget *cbDT5780LinkType;
  GtkWidget* sbDT5780RecordLength;
  GtkWidget* sbDT5780PreTriggerSize;
  GtkWidget *cDT5780ExternalTrigger;
  GtkWidget *cbDT5780AcquisitionMode;
  GtkWidget *cbDT5780IOLevel;
  GtkWidget *cbDT5780ChannelTriggerMode;
  GtkWidget *cbDT5780ChannelEnable[MaxDT5780NChannels];
  GtkWidget *cbDT5780Polarity[MaxDT5780NChannels];
  GtkWidget *sbDT5780TriggerThreshold[MaxDT5780NChannels];
  GtkWidget *sbDT5780TrapezoidRiseTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780TrapezoidFlatTop[MaxDT5780NChannels];
  GtkWidget *sbDT5780DecayTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780PeakingTime[MaxDT5780NChannels];
  GtkWidget *cbDT5780TriggerSmoothing[MaxDT5780NChannels];
  GtkWidget *sbDT5780InputRiseTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780TriggerHoldOff[MaxDT5780NChannels];
  GtkWidget *sbDT5780PeakHoldOff[MaxDT5780NChannels];
  GtkWidget *sbDT5780BaselineHoldOff[MaxDT5780NChannels];
  GtkWidget *cbDT5780InputRange[MaxDT5780NChannels];
  GtkWidget *sbDT5780DCOffset[MaxDT5780NChannels];
  GtkWidget *cbDT5780SelfTrigger[MaxDT5780NChannels];
  GtkWidget *cbDT5780TriggerMode[MaxDT5780NChannels];
} DT5780Widget_t;

typedef struct 
{
  GtkWidget *sbV1742LinkNum[MaxV1742];
  GtkWidget* sbV1742PostTrigger[MaxV1742];
  GtkWidget *cbV1742Frequency[MaxV1742];
  GtkWidget *cbV1742FastTrgDigitizing[MaxV1742];
  GtkWidget *cbV1742TriggerEdge[MaxV1742];
  GtkWidget *cV1742GroupEnable[MaxV1742NGroups][MaxV1742];
  GtkWidget *sbV1742DCOffset[MaxV1742NChannels][MaxV1742];
  GtkWidget *sbV1742FastTriggerOffset[MaxV1742NGroups/2][MaxV1742];
  GtkWidget *sbV1742FastTriggerThreshold[MaxV1742NGroups/2][MaxV1742];

} V1742Widget_t;

char* getCurrentRecipe(GtkWidget* combo);
char* getConfigFile(GtkWidget* combo, Device_t device);
void FillRecipeList(GtkWidget* combo, char* stractive, int isFirst);
void openAdvancedSetup();

#endif
