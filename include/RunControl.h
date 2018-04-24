#ifndef RUN_CONTROL
#define RUN_CONTROL

#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>

#include "V1742Configurator.h"
#include "V812Configurator.h"
#include "DT5780Configurator.h"
#include "V1485Configurator.h"
#include "AdvancedSetup.h"
#include "SharedMemorySetup.h"
#include "UtilsFunctions.h"

GtkWidget* create_main_window();
void WriteLogHeader();
void loadRecipeList();
void changeRecipe();
void updateRunNumber(int);
void updateChannelToPlot();
void updatePlotScaler();
void updateRate();
void updateMonitor();
void checkIncludedDevice();
void checkRunNumber();
void CheckTestHpGe();
void selectRunType();
void selectV1742PlotType();
void startPlotter(char* device);
void checkCaloPlotter();
void checkComptonPlotter();
void startConsumer(char* device);
void startProducer(char* device);
void ConfigDigi();
void StartSlowControl();
void StartDaq();
void StopDaq();
void Quit();
void GetTime(char*);
void DecodeSlowControlStatus(Device_t detector);

#endif
