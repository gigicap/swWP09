#include "SlowControl.h"

DaqSharedMemory* scDaqSharedMemory;

GtkWidget *window;
GtkWidget *scrolled_window;
GtkWidget *notebook;
GtkWidget *fixedMain;
GtkWidget *cGCAL, *cBaF;
GtkWidget *fixedGCAL, *fixedBaF;
GtkWidget *bStart, *bStop, *bExit, *bAlarm;
GtkWidget *eUpdateTime;
GtkTextBuffer *buffer;
GtkTextIter iter;

gcalWidget_t gcal;
bafWidget_t baf;

int handle;
int running=0;
unsigned short updateAllHVParam, updateAllLVParam;
int bafMasked, gcalMasked;

char timeNow[22];
char text[100];
char status[20];
char strEntry[10];

bool isStandAlone=false;

int main(int argc, char *argv[]) {
  int opt=0;

  while ((opt = getopt(argc, argv, "s :")) != -1) {
    switch (opt) {
      case 's':
        isStandAlone=true;
        break;
      case '?':
        printf("usage: SlowControl [ -s ] \n");   
        exit(1);
        break;
    }
  }
    
  gcal.HVList[0]=0; gcal.HVList[1]=1; gcal.HVList[2]=2; gcal.HVList[3]=3; gcal.HVList[4]=4; 
  gcal.HVList[5]=5; gcal.HVList[6]=6; gcal.HVList[7]=7; gcal.HVList[8]=8; gcal.HVList[9]=9; 
  gcal.HVList[10]=10; 
  gcal.LVList[0]=0; gcal.LVList[0]=1;

  baf.HVList[0]=0; 
  baf.LVList[0]=2; baf.LVList[1]=3;

  slotA7030=-1; slotA2518=-1; slotA1536=-1;

  bafMasked=0;
  gcalMasked=0;

  gtk_init(&argc, &argv);

  create_main_window();

 
  // Configure the shared memory
  scDaqSharedMemory=configDaqSharedMemory("SlowControl");
  sprintf(text,"Configuring the DAQ shared memory\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);

  if(isStandAlone) {
    scDaqSharedMemory->IncludeBaF=1;
    scDaqSharedMemory->IncludeGcal=1;
  }

  sprintf(text,"The following systems are present:\n");
  if(scDaqSharedMemory->IncludeBaF==1) strcat(text," BaF ");
  if(scDaqSharedMemory->IncludeGcal==1) strcat(text," GCAL ");
  strcat(text,"\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);

  int page=-1;
  if(scDaqSharedMemory->IncludeBaF) {
    page++;
    setupFrameBaF();
    scDaqSharedMemory->bafStatus=0;

    cBaF = gtk_check_button_new_with_label("BaF"); 
    gtk_fixed_put(GTK_FIXED(fixedMain), cBaF, 150, 15);
    g_signal_connect(G_OBJECT(cBaF), "toggled", G_CALLBACK(maskBaF), NULL);

  }
  if(scDaqSharedMemory->IncludeGcal) {
    page++;
    setupFrameGCAL();
    scDaqSharedMemory->gcalStatus=0;

    cGCAL = gtk_check_button_new_with_label("GCAL"); 
    gtk_fixed_put(GTK_FIXED(fixedMain), cGCAL, 200, 15);
    g_signal_connect(G_OBJECT(cGCAL), "toggled", G_CALLBACK(maskGCAL), NULL);
  }

  gtk_widget_show_all(window);



  // Open logfile 
  char logFileName[100];
  sprintf(logFileName,"Log/SlowControlLog_Run%i",scDaqSharedMemory->runNumber+1);
  freopen(logFileName,"a",stdout);
  sprintf(text,"Opening logfile: %s \n",logFileName);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  // Open the connection with the SY5527 mainframe
  // 3->SY5527   
  // 0->TCP/IP
  int err_code =CAENHV_InitSystem(3,0,"172.16.3.164","admin","admin",&handle);
  GetTime(timeNow);
  if(!err_code) {
    sprintf(text,"%s Connection opened with the SY5527 mainframe \n",timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
  }
  else {
    sprintf(text,"%s ERROR opening the SY5527 mainframe - code %u \n",timeNow,err_code);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
    exit(1);
  }

  CrateMap(handle);

  gtk_main();
  
  return 0;
}  


/*************************************************************************************************************/
void gcalSendLVCommand() {
/*************************************************************************************************************/
  int SelectedChannel=-1;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gcal.cLVSelector)->entry));
  if(strcmp(channel,"CH00")==0)      SelectedChannel=0;
  else if(strcmp(channel,"CH01")==0) SelectedChannel=1;
  else if(strcmp(channel,"All ")==0) SelectedChannel=50;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gcal.cLVCommand)->entry));
  char cmd[30];
  sprintf(cmd,"%s",strCommand);

  float value = gtk_spin_button_get_value((gpointer) gcal.sbLVvalue);

  ApplyCommand(6, SelectedChannel, cmd, value);

}


/*************************************************************************************************************/
void gcalSendHVCommand() {
/*************************************************************************************************************/
  int SelectedChannel=-1;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gcal.cHVSelector)->entry));
  if(strcmp(channel,"CH00")==0)      SelectedChannel=0;
  else if(strcmp(channel,"CH01")==0) SelectedChannel=1;
  else if(strcmp(channel,"CH02")==0) SelectedChannel=2;
  else if(strcmp(channel,"CH03")==0) SelectedChannel=3;
  else if(strcmp(channel,"CH04")==0) SelectedChannel=4;
  else if(strcmp(channel,"CH05")==0) SelectedChannel=5;
  else if(strcmp(channel,"CH06")==0) SelectedChannel=6;
  else if(strcmp(channel,"CH07")==0) SelectedChannel=7;
  else if(strcmp(channel,"CH08")==0) SelectedChannel=8;
  else if(strcmp(channel,"CH09")==0) SelectedChannel=9;
  else if(strcmp(channel,"CH10")==0) SelectedChannel=10;
  else if(strcmp(channel,"All ")==0) SelectedChannel=50;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gcal.cHVCommand)->entry));
  char cmd[30];
  sprintf(cmd,"%s",strCommand);

  float value = gtk_spin_button_get_value((gpointer) gcal.sbHVvalue);

  ApplyCommand(7, SelectedChannel, cmd, value);

}



/*************************************************************************************************************/
void bafSendLVCommand() {
/*************************************************************************************************************/

  int SelectedChannel=-1;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (baf.cLVSelector)->entry));
  if(strcmp(channel,"CH02")==0)      SelectedChannel=0;
  else if(strcmp(channel,"CH03")==0) SelectedChannel=1;
  else if(strcmp(channel,"All ")==0) SelectedChannel=50;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (baf.cLVCommand)->entry));
  char cmd[30];
  sprintf(cmd,"%s",strCommand);

  float value = gtk_spin_button_get_value((gpointer) baf.sbLVvalue);

  ApplyCommand(0, SelectedChannel, cmd, value);
}

/*************************************************************************************************************/
void bafSendHVCommand() {
/*************************************************************************************************************/
  int SelectedChannel=0;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (baf.cHVCommand)->entry));
  char cmd[30];
  sprintf(cmd,"%s",strCommand);

  float value = gtk_spin_button_get_value((gpointer) baf.sbHVvalue);

  ApplyCommand(1, SelectedChannel, cmd, value);
}


/*************************************************************************************************************/
void ApplyCommand(int id, int ch, char command[30], float value) {
/*************************************************************************************************************/
  int ret;

  int isPower=-1;
  if(strcmp(command,"POWER ON")==0) isPower=1;
  else if(strcmp(command,"POWER OFF")==0) isPower=0;
  
  int nch, slot;
  unsigned short int* list;
  int* Power;
  float* ParVal;

  if(id==0) { //BaF LV module
    slot=slotA2518; 

    if(ch==50) nch=nbafLV;
    else nch=1; 
    list = (unsigned short int*) malloc(nch * sizeof(unsigned short int));
    Power = (int*) malloc(nch * sizeof(int));   
    ParVal = (float*) malloc(nch * sizeof(float));

    list[0]=baf.LVList[ch]; 
    for(int i=0; i<nch; i++) {
      if(ch==50) list[i]=baf.LVList[i];
      if(isPower>=0) Power[i]=isPower;
      else ParVal[i]=value;
    }
  }
  else if(id==1) { //BaF HV module - only 1 channel
    slot=slotA1536; 

    nch=1; 
    list = (unsigned short int*) malloc(nch * sizeof(unsigned short int));
    Power = (int*) malloc(nch * sizeof(int));   
    ParVal = (float*) malloc(nch * sizeof(float));

    list[0]=baf.HVList[0]; 
    if(isPower>=0) Power[0]=isPower;
    else ParVal[0]=value;
  }
  if(id==6) { //GCAL LV module
    slot=slotA2518; 

    if(ch==50) nch=ngcalLV;
    else nch=1; 
    list = (unsigned short int*) malloc(nch * sizeof(unsigned short int));
    Power = (int*) malloc(nch * sizeof(int));   
    ParVal = (float*) malloc(nch * sizeof(float));

    list[0]=gcal.LVList[ch]; 
    for(int i=0; i<nch; i++) {
      if(ch==50) list[i]=gcal.LVList[i];
      if(isPower>=0) Power[i]=isPower;
      else ParVal[i]=value;
    }
  }
  if(id==7) { //GCAL HV module
    slot=slotA7030; 

    if(ch==50) nch=ngcalHV;
    else nch=1; 
    list = (unsigned short int*) malloc(nch * sizeof(unsigned short int));
    Power = (int*) malloc(nch * sizeof(int));   
    ParVal = (float*) malloc(nch * sizeof(float));

    list[0]=gcal.HVList[ch]; 
    for(int i=0; i<nch; i++) {
      if(ch==50) list[i]=gcal.HVList[i];
      if(isPower>=0) Power[i]=isPower;
      else ParVal[i]=value;
    }
  }

  GetTime(timeNow);
  if(isPower==0) sprintf(text,"%s Setting POWER OFF on Slot%u - CH%u\n",timeNow,slot,ch);
  else if(isPower==1) sprintf(text,"%s Setting POWER ON on Slot%u - CH%u\n",timeNow,slot,ch);
  else sprintf(text,"%s Setting %s to value %.2f on Slot%u - CH%u\n",timeNow,command,value,slot,ch);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
  printf(text);

  if(isPower>=0) ret=CAENHV_SetChParam(handle,slot,"Pw",nch,list,Power);
  else ret=CAENHV_SetChParam(handle,slot,command,nch,list,ParVal);
     printf(" command: %s   nch %u  ret on CAENHV_SetChParam: 0x%X \n", command, nch,ret); 


  //Force monitor refresh
  updateAllLVParam=1; 
  updateAllHVParam=1; 

  free(ParVal);
  free(Power);
  free(list);
}


/*************************************************************************************************************/
void startSC() {
/*************************************************************************************************************/
  gtk_widget_set_sensitive(bStart, FALSE);
  gtk_widget_set_sensitive(bAlarm, TRUE);
  gtk_widget_set_sensitive(bStop, TRUE);

  running=1;
  scDaqSharedMemory->gcalStatus |= (0x1 << 0);
  scDaqSharedMemory->bafStatus  |= (0x1 << 0);

  updateAllHVParam=1;   //Force the refresh all parameters every time the monitor is started
  updateAllLVParam=1;

  GetTime(timeNow);
  sprintf(text,"%s Starting the slow control \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  while(running)  {

    while (gtk_events_pending()) gtk_main_iteration();

    sleep(SlowControlSleepTime);

    if(scDaqSharedMemory->IncludeBaF) {
      scDaqSharedMemory->bafStatus &= 0x1;  // Clear HV/LV alarms before the new monitor cycle
      bafHVmonitor();
      bafLVmonitor();
     // if(bafMasked) scDaqSharedMemory->bafStatus=1; //...Decide if to suppress the alarm messages only or to suppress also the alarm 
    }

    if(scDaqSharedMemory->IncludeGcal) {
      scDaqSharedMemory->gcalStatus &= 0x1; 
      // gcalHVmonitor(); the module A7030 is broken...(23/03/2018)
      gcalLVmonitor();
     // if(gcalMasked) scDaqSharedMemory->gcalStatus=1; //...Decide if to suppress the alarm messages only or to suppress also the alarm 
    }

    GetTime(timeNow);
    gtk_entry_set_text((gpointer) eUpdateTime, (gpointer) timeNow);

    updateAllHVParam=0;
    updateAllLVParam=0;
  }

}



/*************************************************************************************************************/
void bafHVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA1536,"VMon",nbafHV,baf.HVList,baf.ParVMon);
  if(ret) MonitorError("BaF VMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA1536,"IMon",nbafHV,baf.HVList,baf.ParIMon);
  if(ret) MonitorError("BaF IMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA1536,"Status",nbafHV,baf.HVList,baf.ParStatus);
  if(ret) MonitorError("BaF Status-HV",ret);

  if(updateAllHVParam) {
    ret=CAENHV_GetChParam(handle,slotA1536,"V0Set",nbafHV,baf.HVList,baf.ParV0Set);
    if(ret) MonitorError("BaF V0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"I0Set",nbafHV,baf.HVList,baf.ParI0Set);
    if(ret) MonitorError("BaF I0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"RUp",nbafHV,baf.HVList,baf.ParRup);
    if(ret) MonitorError("BaF RUp-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"RDWn",nbafHV,baf.HVList,baf.ParRdwn);
    if(ret) MonitorError("BaF RDWn-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"Trip",nbafHV,baf.HVList,baf.ParTrip);
    if(ret) MonitorError("BaF Trip-HV",ret);
  }

  for(int n=0; n<nbafHV; n++) {
    sprintf(strEntry,"%4.2f",baf.ParVMon[n]);
    gtk_entry_set_text((gpointer) baf.eVMon[n],(gpointer) strEntry);
    
    sprintf(strEntry,"%4.3f",baf.ParIMon[n]);
    gtk_entry_set_text((gpointer) baf.eIMon[n],(gpointer) strEntry);

    if((baf.ParStatus[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) baf.ePw[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) baf.ePw[n],(gpointer) "OFF");
      scDaqSharedMemory->bafStatus |= (0x1 << 1);
      scDaqSharedMemory->bafStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor:  bafStatus %i   ch %i off\n",scDaqSharedMemory->bafStatus,n);
      if(!bafMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus = DecodeStatus(baf.ParStatus[n], status);
    gtk_entry_set_text((gpointer) baf.eStatus[n],(gpointer) status);
    if(alarmStatus) { 
      scDaqSharedMemory->bafStatus |= (alarmStatus << 2);
      scDaqSharedMemory->bafStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor: %s  alarmStatus %i   bafStatus %i   ch %i\n",status,alarmStatus,scDaqSharedMemory->bafStatus,n);
      if(!bafMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllHVParam) {
      sprintf(strEntry,"%4.2f",baf.ParV0Set[n]);
      gtk_entry_set_text((gpointer) baf.eV0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%4.2f",baf.ParI0Set[n]);
      gtk_entry_set_text((gpointer) baf.eI0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",baf.ParRup[n]);
      gtk_entry_set_text((gpointer) baf.eRup[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",baf.ParRdwn[n]);
      gtk_entry_set_text((gpointer) baf.eRdwn[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.1f",baf.ParTrip[n]);
      gtk_entry_set_text((gpointer) baf.eTrip[n],(gpointer) strEntry);
    }
  }

}

/*************************************************************************************************************/
void gcalHVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA7030,"VMon",ngcalHV,gcal.HVList,gcal.ParVMon);
  if(ret) MonitorError("GCAL VMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA7030,"IMon",ngcalHV,gcal.HVList,gcal.ParIMon);
  if(ret) MonitorError("GCAL IMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA7030,"Status",ngcalHV,gcal.HVList,gcal.ParStatus);
  if(ret) MonitorError("GCAL Status-HV",ret);

  if(updateAllHVParam) {
    ret=CAENHV_GetChParam(handle,slotA7030,"V0Set",ngcalHV,gcal.HVList,gcal.ParV0Set);
    if(ret) MonitorError("GCAL V0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA7030,"I0Set",ngcalHV,gcal.HVList,gcal.ParI0Set);
    if(ret) MonitorError("GCAL I0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA7030,"RUp",ngcalHV,gcal.HVList,gcal.ParRup);
    if(ret) MonitorError("GCAL RUp-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA7030,"RDWn",ngcalHV,gcal.HVList,gcal.ParRdwn);
    if(ret) MonitorError("GCAL RDWn-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA7030,"Trip",ngcalHV,gcal.HVList,gcal.ParTrip);
    if(ret) MonitorError("GCAL Trip-HV",ret);
  }

  for(int n=0; n<ngcalHV; n++) {
    sprintf(strEntry,"%4.2f",gcal.ParVMon[n]);
    gtk_entry_set_text((gpointer) gcal.eVMon[n],(gpointer) strEntry);
    
    sprintf(strEntry,"%4.3f",gcal.ParIMon[n]);
    gtk_entry_set_text((gpointer) gcal.eIMon[n],(gpointer) strEntry);

    if((gcal.ParStatus[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) gcal.ePw[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) gcal.ePw[n],(gpointer) "OFF");
      scDaqSharedMemory->gcalStatus |= (0x1 << 1);
      scDaqSharedMemory->gcalStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor:  gcalStatus %i   ch %i off\n",scDaqSharedMemory->gcalStatus,n);
      if(!gcalMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus = DecodeStatus(gcal.ParStatus[n], status);
    gtk_entry_set_text((gpointer) gcal.eStatus[n],(gpointer) status);
    if(alarmStatus) { 
      scDaqSharedMemory->gcalStatus |= (alarmStatus << 2);
      scDaqSharedMemory->gcalStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor: %s  alarmStatus %i   gcalStatus %i   ch %i\n",status,alarmStatus,scDaqSharedMemory->gcalStatus,n);
      if(!gcalMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllHVParam) {
      sprintf(strEntry,"%4.2f",gcal.ParV0Set[n]);
      gtk_entry_set_text((gpointer) gcal.eV0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%4.2f",gcal.ParI0Set[n]);
      gtk_entry_set_text((gpointer) gcal.eI0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",gcal.ParRup[n]);
      gtk_entry_set_text((gpointer) gcal.eRup[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",gcal.ParRdwn[n]);
      gtk_entry_set_text((gpointer) gcal.eRdwn[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.1f",gcal.ParTrip[n]);
      gtk_entry_set_text((gpointer) gcal.eTrip[n],(gpointer) strEntry);
    }
  }

}



/*************************************************************************************************************/
void bafLVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA2518,"VMon",nbafLV,baf.LVList,baf.ParVMonL);
  if(ret) MonitorError("BaF VMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"VCon",nbafLV,baf.LVList,baf.ParVConL);
  if(ret) MonitorError("BaF Trip-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"IMon",nbafLV,baf.LVList,baf.ParIMonL);
  if(ret) MonitorError("BaF IMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"Status",nbafLV,baf.LVList,baf.ParStatusL);
  if(ret) MonitorError("BaF Status-LV",ret);

  if(updateAllLVParam) {
    ret=CAENHV_GetChParam(handle,slotA2518,"V0Set",nbafLV,baf.LVList,baf.ParV0SetL);
    if(ret) MonitorError("BaF V0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"I0Set",nbafLV,baf.LVList,baf.ParI0SetL);
    if(ret) MonitorError("BaF I0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RUpTime",nbafLV,baf.LVList,baf.ParRupL);
    if(ret) MonitorError("BaF RUp-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RDwTime",nbafLV,baf.LVList,baf.ParRdwnL);
    if(ret) MonitorError("BaF RDWn-LV",ret);
  }

  for(int n=0; n<nbafLV; n++) {
    sprintf(strEntry,"%5.3f",baf.ParVMonL[n]);
    gtk_entry_set_text((gpointer) baf.eVMonL[n],(gpointer) strEntry);

    sprintf(strEntry,"%5.3f",baf.ParVConL[n]);
    gtk_entry_set_text((gpointer) baf.eVConL[n],(gpointer) strEntry);

    sprintf(strEntry,"%6.3f",baf.ParIMonL[n]);
    gtk_entry_set_text((gpointer) baf.eIMonL[n],(gpointer) strEntry);

    if ((baf.ParStatusL[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) baf.ePwL[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) baf.ePwL[n],(gpointer) "OFF");
      scDaqSharedMemory->bafStatus |= (0x1 << 17);
      scDaqSharedMemory->bafStatus |= (0x1 << (21+n));
      sprintf(text,"LVmonitor:  bafStatus %i   ch %i off\n",scDaqSharedMemory->bafStatus,n);
      if(!bafMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus=DecodeStatus(baf.ParStatusL[n], status);
    gtk_entry_set_text((gpointer) baf.eStatusL[n],(gpointer) status);
    if(alarmStatus) { 
      scDaqSharedMemory->bafStatus |= (alarmStatus<<18);
      scDaqSharedMemory->bafStatus |= (0x1 << (21+n)); 
      sprintf(text,"LVmonitor: %s   alarmStatus %i    bafStatus %i   ch %i\n",status,alarmStatus,scDaqSharedMemory->bafStatus,n);
      if(!bafMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllLVParam) {
      sprintf(strEntry,"%5.3f",baf.ParV0SetL[n]);
      gtk_entry_set_text((gpointer) baf.eV0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%5.2f",baf.ParI0SetL[n]);
      gtk_entry_set_text((gpointer) baf.eI0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",baf.ParRupL[n]);
      gtk_entry_set_text((gpointer) baf.eRupL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",baf.ParRdwnL[n]);
      gtk_entry_set_text((gpointer) baf.eRdwnL[n],(gpointer) strEntry);
    }
  }

}



/*************************************************************************************************************/
void gcalLVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA2518,"VMon",ngcalLV,gcal.LVList,gcal.ParVMonL);
  if(ret) MonitorError("GCAL VMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"VCon",ngcalLV,gcal.LVList,gcal.ParVConL);
  if(ret) MonitorError("GCAL Trip-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"IMon",ngcalLV,gcal.LVList,gcal.ParIMonL);
  if(ret) MonitorError("GCAL IMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"Status",ngcalLV,gcal.LVList,gcal.ParStatusL);
  if(ret) MonitorError("GCAL Status-LV",ret);

  if(updateAllLVParam) {
    ret=CAENHV_GetChParam(handle,slotA2518,"V0Set",ngcalLV,gcal.LVList,gcal.ParV0SetL);
    if(ret) MonitorError("GCAL V0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"I0Set",ngcalLV,gcal.LVList,gcal.ParI0SetL);
    if(ret) MonitorError("GCAL I0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RUpTime",ngcalLV,gcal.LVList,gcal.ParRupL);
    if(ret) MonitorError("GCAL RUp-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RDwTime",ngcalLV,gcal.LVList,gcal.ParRdwnL);
    if(ret) MonitorError("GCAL RDWn-LV",ret);
  }

  for(int n=0; n<ngcalLV; n++) {
    sprintf(strEntry,"%5.3f",gcal.ParVMonL[n]);
    gtk_entry_set_text((gpointer) gcal.eVMonL[n],(gpointer) strEntry);

    sprintf(strEntry,"%5.3f",gcal.ParVConL[n]);
    gtk_entry_set_text((gpointer) gcal.eVConL[n],(gpointer) strEntry);

    sprintf(strEntry,"%6.3f",gcal.ParIMonL[n]);
    gtk_entry_set_text((gpointer) gcal.eIMonL[n],(gpointer) strEntry);

    if ((gcal.ParStatusL[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) gcal.ePwL[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) gcal.ePwL[n],(gpointer) "OFF");
      scDaqSharedMemory->gcalStatus |= (0x1 << 17);
      scDaqSharedMemory->gcalStatus |= (0x1 << (21+n)); 
      sprintf(text,"LVmonitor:  gcalStatus %i   ch %i off\n",scDaqSharedMemory->gcalStatus,n);
      if(!gcalMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus=DecodeStatus(gcal.ParStatusL[n], status);
    gtk_entry_set_text((gpointer) gcal.eStatusL[n],(gpointer) status);
    if(alarmStatus) { 
      scDaqSharedMemory->gcalStatus |= (alarmStatus<<18);
      scDaqSharedMemory->gcalStatus |= (0x1 << (21+n)); 
      sprintf(text,"LVmonitor: %s   alarmStatus %i    gcalStatus %i   ch %i\n",status,alarmStatus,scDaqSharedMemory->gcalStatus,n);
      if(!gcalMasked) gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllLVParam) {
      sprintf(strEntry,"%5.3f",gcal.ParV0SetL[n]);
      gtk_entry_set_text((gpointer) gcal.eV0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%5.2f",gcal.ParI0SetL[n]);
      gtk_entry_set_text((gpointer) gcal.eI0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",gcal.ParRupL[n]);
      gtk_entry_set_text((gpointer) gcal.eRupL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",gcal.ParRdwnL[n]);
      gtk_entry_set_text((gpointer) gcal.eRdwnL[n],(gpointer) strEntry);
    }
  }

}



/*************************************************************************************************************/
int DecodeStatus(int val, char *status) {
/*************************************************************************************************************/
  char mesg[20];
  int alarm=0;
      
  if(val==0 || val==1) sprintf(mesg,"%s"," ");
  if(val & (0x1 << 1)) sprintf(mesg,"%s", "RampUp");
  if(val & (0x1 << 2)) sprintf(mesg,"%s", "RampDown");
  if(val & (0x1 << 3)) sprintf(mesg,"%s", "OvCurrent");
  if(val & (0x1 << 4)) sprintf(mesg,"%s", "OverVoltage");
  if(val & (0x1 << 5)) sprintf(mesg,"%s", "UnderVoltage");
  if(val & (0x1 << 6)) sprintf(mesg,"%s", "ExternalTrip");
  if(val & (0x1 << 7)) sprintf(mesg,"%s", "OverVMAX");
  if(val & (0x1 << 8)) sprintf(mesg,"%s", "ExternalDisable");
  if(val & (0x1 << 9)) {
    sprintf(mesg,"%s", "Trip");
    alarm |= (0x1 << 0);
  }
  if(val & (0x1 << 10)) sprintf(mesg,"%s", "CalibError"); 
  if(val & (0x1 << 11)) sprintf(mesg,"%s", "Unplugged");
  if(val & (0x1 << 13)) sprintf(mesg,"%s", "OverVoltageProt");
  if(val & (0x1 << 14)) sprintf(mesg,"%s", "PowerFail");
  if(val & (0x1 << 15)) {
    sprintf(mesg,"%s", "Temperature");
    alarm |= (0x1 << 1);
  }

  mesg[20]='\0';
  strcpy(status, mesg);

  if((val>>3) !=0  && alarm==0) alarm |= (0x1 << 2);

  return alarm;
}




/*************************************************************************************************************/
void MonitorError(char message[20], int err_code) {
/*************************************************************************************************************/
   sprintf(text," ERROR reading %s: code 0x%X \n",message, err_code);
   gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
   printf(text);

}


/*************************************************************************************************************/
void stopSC() {
/*************************************************************************************************************/
  running=0;
  scDaqSharedMemory->bafStatus &= ~(0x1 << 0);
  scDaqSharedMemory->gcalStatus &= ~(0x1 << 0);

  gtk_widget_set_sensitive(bAlarm, FALSE);
  gtk_widget_set_sensitive(bStop, FALSE);
  gtk_widget_set_sensitive(bStart, TRUE);

  GetTime(timeNow);
  sprintf(text,"%s Stopping the slow control system \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  printf(text);

}



/*************************************************************************************************************/
void exitSC() {
/*************************************************************************************************************/
  running=0;
  scDaqSharedMemory->bafStatus &= ~(0x1 << 0);
  scDaqSharedMemory->gcalStatus &= ~(0x1 << 0);
  
  CAENHV_DeinitSystem(handle);
  GetTime(timeNow);
  sprintf(text,"%s Connection closed with the SY5527 mainframe \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  
  fflush(stdout);
  fclose(stdout);  // close logFile

  gtk_main_quit();

}



/*************************************************************************************************************/
void GetTime(char *timeNow) {
/*************************************************************************************************************/
  time_t now;
  struct tm *tm;
  char bufTime[21];
  char mesgTime[22];

  now = time(NULL);
  tm = localtime(&now);
  strftime(bufTime, sizeof(bufTime), "%d-%m-%Y  %H:%M:%S", tm);
  sprintf(mesgTime,"%s", bufTime);
  mesgTime[22]='\0';
      
  strcpy(timeNow, mesgTime);
     
} 


/*************************************************************************************************************/
void CrateMap(int handle) {
/*************************************************************************************************************/

  unsigned short NrOfSl, *SerNumList, *NrOfCh;
  char                   *ModelList, *DescriptionList;
  unsigned char          *FmwRelMinList, *FmwRelMaxList;
      
  int ret = CAENHV_GetCrateMap(handle, &NrOfSl, &NrOfCh, &ModelList, &DescriptionList, &SerNumList,
                                       &FmwRelMinList, &FmwRelMaxList );
      
  int  ii;
  char *m = ModelList, *d = DescriptionList;

  sprintf(text,"          *** SY5527 CRATE MAP ***          \n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
  printf(text);

  for(ii = 0; ii < NrOfSl; ii++ , m += strlen(m) + 1, d += strlen(d) + 1 ) {
    if( *m == '\0' ) {
      sprintf(text,"Board %2d: Not Present\n", ii);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
      printf(text);
    }
    else {
      sprintf(text,"Board %2d: %s %s  Nr. Ch: %d  Ser. %d   Rel. %d.%d\n",
                 ii, m, d, NrOfCh[ii], SerNumList[ii], FmwRelMaxList[ii],FmwRelMinList[ii]);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
      printf(text);
      //test
      if(strcmp(m,"A7030TP")==0) { printf("Found module A7030TP in slot %2d\n",ii); slotA7030=ii; }
      if(strcmp(m,"A2518")==0) { printf("Found module A2518 in slot %2d\n",ii); slotA2518=ii; }
      if(strcmp(m,"A1536D")==0) { printf("Found module A1536D in slot %2d\n",ii); slotA1536=ii; }
    }
   }

  free(SerNumList); free(ModelList); free(DescriptionList); free(FmwRelMinList); free(FmwRelMaxList); free(NrOfCh);
}



/*************************************************************************************************************/
void maskBaF() {
/*************************************************************************************************************/

  GetTime(timeNow);

  bafMasked = !bafMasked;

  if(bafMasked) sprintf(text,"%s BaF alarms masked \n",timeNow);
  else sprintf(text,"%s BaF alarms unmasked \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  printf(text);

}


/*************************************************************************************************************/
void setupFrameBaF() {
/*************************************************************************************************************/
  GtkWidget *lVMon, *lIMon, *lV0Set, *lI0Set, *lRup, *lRdwn, *lTrip, *lPw, *lStatus;
  GtkWidget *lVMonL, *lIMonL, *lV0SetL, *lI0SetL, *lRupL, *lRdwnL, *lVConL, *lPwL, *lStatusL;  
  GtkWidget *lChannel[ngcalHV], *lChannelL[ngcalLV];

  fixedBaF = gtk_fixed_new();

  GtkWidget *tabLabelBaF = gtk_label_new ("BaF SLOW CONTROL");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedBaF, tabLabelBaF);


  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0x6666;
  color.green = 0x9999;
  color.blue = 0x6666;
  gtk_widget_modify_bg(notebook, GTK_STATE_NORMAL, &color);
  // test GTK_STATE:
  // ACTIVE FOCUSED INCONSISTENT INSENSITIVE NORMAL PRELIGHT SELECTED 
  
    
  /////////////////////////////////////////////////////////////////////
  // HV setup frame
  /////////////////////////////////////////////////////////////////////
  GtkWidget *frameHV = gtk_frame_new ("  HV - A1536  ");
  gtk_fixed_put(GTK_FIXED(fixedBaF), frameHV, 5, 10);
  gtk_widget_set_size_request (frameHV, 800, 560);
     
  int xsize=65, ysize=30;
  int xspace=10, yspace=10;
  int xoffset=0, yoffset=50;
  char strChan[2];
  char labelChan[10];
  for(int n=0; n<nbafHV; n++) {
    baf.eVMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eVMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eVMon[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eVMon[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eVMon[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    baf.eIMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eIMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eIMon[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eIMon[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eIMon[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    baf.eV0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eV0Set[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eV0Set[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eV0Set[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eV0Set[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    baf.eI0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eI0Set[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eI0Set[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eI0Set[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eI0Set[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    baf.eRup[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eRup[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eRup[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eRup[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eRup[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    baf.eRdwn[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eRdwn[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eRdwn[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eRdwn[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eRdwn[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    baf.eTrip[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eTrip[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eTrip[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eTrip[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eTrip[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    baf.ePw[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.ePw[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.ePw[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.ePw[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.ePw[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    baf.eStatus[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eStatus[n], xsize+30, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eStatus[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eStatus[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eStatus[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n);
    else sprintf(strChan,"%i",n);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannel[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixedBaF), lChannel[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  int lxoff=12, lyoff=20;
  xspace += xsize;
  lVMon =  gtk_label_new("VMon");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lVMon, lxoff+xspace, yoffset-lyoff);

  lIMon =  gtk_label_new("IMon");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lIMon, lxoff+(xspace*2), yoffset-lyoff);

  lV0Set =  gtk_label_new("VSet");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lV0Set, lxoff+(xspace*3), yoffset-lyoff);

  lI0Set =  gtk_label_new("ISet");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lI0Set, lxoff+(xspace*4), yoffset-lyoff);

  lRup =  gtk_label_new("Rup");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lRup, lxoff+(xspace*5), yoffset-lyoff);

  lRdwn =  gtk_label_new("Rdwn");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lRdwn, lxoff+(xspace*6), yoffset-lyoff);

  lTrip =  gtk_label_new("Trip");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lTrip, lxoff+(xspace*7), yoffset-lyoff);

  lPw =  gtk_label_new("Pw");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lPw, lxoff+(xspace*8), yoffset-lyoff);

  lStatus =  gtk_label_new("Status");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lStatus, lxoff+(xspace*9), yoffset-lyoff);


  /////////////////////////////////////////////////////////////////////
  // ComboBox for the HV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *csel = NULL;
  csel = g_list_append (csel, "CH00");
  baf.cHVSelector=gtk_combo_new();
  gtk_widget_set_size_request(baf.cHVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(baf.cHVSelector), csel);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.cHVSelector, 100, 500);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cbitems = NULL;
  cbitems = g_list_append (cbitems, "V0Set");
  cbitems = g_list_append (cbitems, "I0Set");
  cbitems = g_list_append (cbitems, "RUp");
  cbitems = g_list_append (cbitems, "RDWn");
  cbitems = g_list_append (cbitems, "Trip");
  cbitems = g_list_append (cbitems, "POWER ON");
  cbitems = g_list_append (cbitems, "POWER OFF");
  baf.cHVCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(baf.cHVCommand), cbitems);
  gtk_widget_set_size_request(baf.cHVCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.cHVCommand,220, 500);

  GtkAdjustment *adjustment = (GtkAdjustment *) gtk_adjustment_new (600.0, 0.0, 1000.0, 50.0, 1.0, 0.0);
  baf.sbHVvalue = gtk_spin_button_new (adjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.sbHVvalue, 450, 500);

  GtkWidget *bSend;
  bSend=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSend, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixedBaF), bSend, 600, 500);
  g_signal_connect(G_OBJECT(bSend), "clicked", G_CALLBACK(bafSendHVCommand), NULL);


  /////////////////////////////////////////////////////////////////////
  // LV setup frame
  ///////////////////////////////////////////////////////////////////// 
  GtkWidget *frameLV = gtk_frame_new ("  LV - A2518  ");
  gtk_fixed_put(GTK_FIXED(fixedBaF), frameLV, 5, 590);
  gtk_widget_set_size_request (frameLV, 800, 210);


  xsize=65; ysize=30; xspace=10; yspace=10;
  yoffset=650;
  for(int n=0; n<ngcalLV; n++) {
    baf.eVMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eVMonL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eVMonL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eVMonL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eVMonL[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    baf.eVConL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eVConL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eVConL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eVConL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eVConL[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    baf.eIMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eIMonL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eIMonL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eIMonL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eIMonL[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    baf.eV0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eV0SetL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eV0SetL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eV0SetL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eV0SetL[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    baf.eI0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eI0SetL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eI0SetL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eI0SetL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eI0SetL[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    baf.eRupL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eRupL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eRupL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eRupL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eRupL[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    baf.eRdwnL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eRdwnL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eRdwnL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eRdwnL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eRdwnL[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    baf.ePwL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.ePwL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.ePwL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.ePwL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.ePwL[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    baf.eStatusL[n] = gtk_entry_new();
    gtk_widget_set_size_request(baf.eStatusL[n], xsize+20, ysize);
    gtk_entry_set_editable(GTK_ENTRY(baf.eStatusL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(baf.eStatusL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedBaF), baf.eStatusL[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n+2);
    else sprintf(strChan,"%i",n+2);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannelL[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixedBaF), lChannelL[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  xspace += xsize;
  lVMonL =  gtk_label_new("VMon");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lVMonL, lxoff+xspace, yoffset-lyoff);

  lVConL =  gtk_label_new("VCon");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lVConL, lxoff+(xspace*2), yoffset-lyoff);

  lIMonL =  gtk_label_new("IMon");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lIMonL, lxoff+(xspace*3), yoffset-lyoff);

  lV0SetL =  gtk_label_new("VSet");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lV0SetL, lxoff+(xspace*4), yoffset-lyoff);

  lI0SetL =  gtk_label_new("ISet");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lI0SetL, lxoff+(xspace*5), yoffset-lyoff);

  lRupL =  gtk_label_new("Rup");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lRupL, lxoff+(xspace*6), yoffset-lyoff);

  lRdwnL =  gtk_label_new("Rdwn");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lRdwnL, lxoff+(xspace*7), yoffset-lyoff);

  lPwL =  gtk_label_new("Pw");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lPwL, lxoff+(xspace*8), yoffset-lyoff);

  lStatusL =  gtk_label_new("Status");
  gtk_fixed_put(GTK_FIXED(fixedBaF), lStatusL, lxoff+(xspace*9), yoffset-lyoff);

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVsel = NULL;
  cLVsel = g_list_append (cLVsel, "CH02");
  cLVsel = g_list_append (cLVsel, "CH03");
  cLVsel = g_list_append (cLVsel, "All ");
  baf.cLVSelector=gtk_combo_new();
  gtk_widget_set_size_request(baf.cLVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(baf.cLVSelector), cLVsel);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.cLVSelector, 100, 750);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVitems = NULL;
  cLVitems = g_list_append (cLVitems, "V0Set");
  cLVitems = g_list_append (cLVitems, "I0Set");
  cLVitems = g_list_append (cLVitems, "RUpTime");
  cLVitems = g_list_append (cLVitems, "RDwTime");
  cLVitems = g_list_append (cLVitems, "POWER ON");
  cLVitems = g_list_append (cLVitems, "POWER OFF");
  baf.cLVCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(baf.cLVCommand), cLVitems);
  gtk_widget_set_size_request(baf.cLVCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.cLVCommand,220, 750);

  GtkAdjustment *LVadjustment = (GtkAdjustment *) gtk_adjustment_new (5.0, 3.0, 500.0, 1.0, 1.0, 0.0);
  baf.sbLVvalue = gtk_spin_button_new (LVadjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixedBaF), baf.sbLVvalue, 450, 750);

  GtkWidget *bSendLV;
  bSendLV=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSendLV, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixedBaF), bSendLV, 600, 750);
  g_signal_connect(G_OBJECT(bSendLV), "clicked", G_CALLBACK(bafSendLVCommand), NULL);


  //gtk_widget_hide(fixedBaF);
}

/*************************************************************************************************************/
void maskGCAL() {
/*************************************************************************************************************/

  GetTime(timeNow);

  gcalMasked = !gcalMasked ;

  if(gcalMasked) sprintf(text,"%s GCAL alarms masked \n",timeNow);
  else sprintf(text,"%s GCAL alarms unmasked \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  printf(text);

}




/*************************************************************************************************************/
void setupFrameGCAL() {
/*************************************************************************************************************/
  GtkWidget *lVMon, *lIMon, *lV0Set, *lI0Set, *lRup, *lRdwn, *lTrip, *lPw, *lStatus;
  GtkWidget *lVMonL, *lIMonL, *lV0SetL, *lI0SetL, *lRupL, *lRdwnL, *lVConL, *lPwL, *lStatusL;  
  GtkWidget *lChannel[ngcalHV], *lChannelL[ngcalLV];

  fixedGCAL = gtk_fixed_new();

  GtkWidget *tabLabelGCAL = gtk_label_new ("GCAL SLOW CONTROL");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedGCAL, tabLabelGCAL);


  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0x6666;
  color.green = 0x9999;
  color.blue = 0x6666;
  gtk_widget_modify_bg(notebook, GTK_STATE_NORMAL, &color);
  // test GTK_STATE:
  // ACTIVE FOCUSED INCONSISTENT INSENSITIVE NORMAL PRELIGHT SELECTED 
  
    
  /////////////////////////////////////////////////////////////////////
  // HV setup frame
  /////////////////////////////////////////////////////////////////////
  GtkWidget *frameHV = gtk_frame_new ("  HV - A7030TP  ");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), frameHV, 5, 10);
  gtk_widget_set_size_request (frameHV, 800, 560);
     
  int xsize=65, ysize=30;
  int xspace=10, yspace=10;
  int xoffset=0, yoffset=50;
  char strChan[2];
  char labelChan[10];
  for(int n=0; n<ngcalHV; n++) {
    gcal.eVMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eVMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eVMon[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eVMon[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eVMon[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    gcal.eIMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eIMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eIMon[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eIMon[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eIMon[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    gcal.eV0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eV0Set[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eV0Set[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eV0Set[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eV0Set[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    gcal.eI0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eI0Set[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eI0Set[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eI0Set[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eI0Set[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    gcal.eRup[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eRup[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eRup[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eRup[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eRup[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    gcal.eRdwn[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eRdwn[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eRdwn[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eRdwn[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eRdwn[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    gcal.eTrip[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eTrip[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eTrip[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eTrip[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eTrip[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    gcal.ePw[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.ePw[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.ePw[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.ePw[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.ePw[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    gcal.eStatus[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eStatus[n], xsize+30, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eStatus[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eStatus[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eStatus[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n);
    else sprintf(strChan,"%i",n);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannel[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), lChannel[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  int lxoff=12, lyoff=20;
  xspace += xsize;
  lVMon =  gtk_label_new("VMon");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lVMon, lxoff+xspace, yoffset-lyoff);

  lIMon =  gtk_label_new("IMon");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lIMon, lxoff+(xspace*2), yoffset-lyoff);

  lV0Set =  gtk_label_new("VSet");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lV0Set, lxoff+(xspace*3), yoffset-lyoff);

  lI0Set =  gtk_label_new("ISet");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lI0Set, lxoff+(xspace*4), yoffset-lyoff);

  lRup =  gtk_label_new("Rup");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lRup, lxoff+(xspace*5), yoffset-lyoff);

  lRdwn =  gtk_label_new("Rdwn");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lRdwn, lxoff+(xspace*6), yoffset-lyoff);

  lTrip =  gtk_label_new("Trip");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lTrip, lxoff+(xspace*7), yoffset-lyoff);

  lPw =  gtk_label_new("Pw");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lPw, lxoff+(xspace*8), yoffset-lyoff);

  lStatus =  gtk_label_new("Status");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lStatus, lxoff+(xspace*9), yoffset-lyoff);


  /////////////////////////////////////////////////////////////////////
  // ComboBox for the HV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *csel = NULL;
  csel = g_list_append (csel, "CH00");
  csel = g_list_append (csel, "CH01");
  csel = g_list_append (csel, "CH02");
  csel = g_list_append (csel, "CH03");
  csel = g_list_append (csel, "CH04");
  csel = g_list_append (csel, "CH05");
  csel = g_list_append (csel, "CH06");
  csel = g_list_append (csel, "CH07");
  csel = g_list_append (csel, "CH08");
  csel = g_list_append (csel, "CH09");
  csel = g_list_append (csel, "CH10");
  csel = g_list_append (csel, "All ");
  gcal.cHVSelector=gtk_combo_new();
  gtk_widget_set_size_request(gcal.cHVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(gcal.cHVSelector), csel);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.cHVSelector, 100, 500);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cbitems = NULL;
  cbitems = g_list_append (cbitems, "V0Set");
  cbitems = g_list_append (cbitems, "I0Set");
  cbitems = g_list_append (cbitems, "RUp");
  cbitems = g_list_append (cbitems, "RDWn");
  cbitems = g_list_append (cbitems, "Trip");
  cbitems = g_list_append (cbitems, "POWER ON");
  cbitems = g_list_append (cbitems, "POWER OFF");
  gcal.cHVCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(gcal.cHVCommand), cbitems);
  gtk_widget_set_size_request(gcal.cHVCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.cHVCommand,220, 500);

  GtkAdjustment *adjustment = (GtkAdjustment *) gtk_adjustment_new (600.0, 0.0, 1000.0, 50.0, 1.0, 0.0);
  gcal.sbHVvalue = gtk_spin_button_new (adjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.sbHVvalue, 450, 500);

  GtkWidget *bSend;
  bSend=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSend, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), bSend, 600, 500);
  g_signal_connect(G_OBJECT(bSend), "clicked", G_CALLBACK(gcalSendHVCommand), NULL);


  /////////////////////////////////////////////////////////////////////
  // LV setup frame
  ///////////////////////////////////////////////////////////////////// 
  GtkWidget *frameLV = gtk_frame_new ("  LV - A2518  ");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), frameLV, 5, 590);
  gtk_widget_set_size_request (frameLV, 800, 210);


  xsize=65; ysize=30; xspace=10; yspace=10;
  yoffset=650;
  for(int n=0; n<ngcalLV; n++) {
    gcal.eVMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eVMonL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eVMonL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eVMonL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eVMonL[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    gcal.eVConL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eVConL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eVConL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eVConL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eVConL[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    gcal.eIMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eIMonL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eIMonL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eIMonL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eIMonL[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    gcal.eV0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eV0SetL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eV0SetL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eV0SetL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eV0SetL[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    gcal.eI0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eI0SetL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eI0SetL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eI0SetL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eI0SetL[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    gcal.eRupL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eRupL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eRupL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eRupL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eRupL[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    gcal.eRdwnL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eRdwnL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eRdwnL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eRdwnL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eRdwnL[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    gcal.ePwL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.ePwL[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.ePwL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.ePwL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.ePwL[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    gcal.eStatusL[n] = gtk_entry_new();
    gtk_widget_set_size_request(gcal.eStatusL[n], xsize+20, ysize);
    gtk_entry_set_editable(GTK_ENTRY(gcal.eStatusL[n]), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(gcal.eStatusL[n]), FALSE);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.eStatusL[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n);
    else sprintf(strChan,"%i",n);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannelL[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixedGCAL), lChannelL[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  xspace += xsize;
  lVMonL =  gtk_label_new("VMon");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lVMonL, lxoff+xspace, yoffset-lyoff);

  lVConL =  gtk_label_new("VCon");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lVConL, lxoff+(xspace*2), yoffset-lyoff);

  lIMonL =  gtk_label_new("IMon");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lIMonL, lxoff+(xspace*3), yoffset-lyoff);

  lV0SetL =  gtk_label_new("VSet");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lV0SetL, lxoff+(xspace*4), yoffset-lyoff);

  lI0SetL =  gtk_label_new("ISet");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lI0SetL, lxoff+(xspace*5), yoffset-lyoff);

  lRupL =  gtk_label_new("Rup");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lRupL, lxoff+(xspace*6), yoffset-lyoff);

  lRdwnL =  gtk_label_new("Rdwn");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lRdwnL, lxoff+(xspace*7), yoffset-lyoff);

  lPwL =  gtk_label_new("Pw");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lPwL, lxoff+(xspace*8), yoffset-lyoff);

  lStatusL =  gtk_label_new("Status");
  gtk_fixed_put(GTK_FIXED(fixedGCAL), lStatusL, lxoff+(xspace*9), yoffset-lyoff);

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVsel = NULL;
  cLVsel = g_list_append (cLVsel, "CH00");
  cLVsel = g_list_append (cLVsel, "CH01");
  cLVsel = g_list_append (cLVsel, "All ");
  gcal.cLVSelector=gtk_combo_new();
  gtk_widget_set_size_request(gcal.cLVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(gcal.cLVSelector), cLVsel);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.cLVSelector, 100, 750);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVitems = NULL;
  cLVitems = g_list_append (cLVitems, "V0Set");
  cLVitems = g_list_append (cLVitems, "I0Set");
  cLVitems = g_list_append (cLVitems, "RUpTime");
  cLVitems = g_list_append (cLVitems, "RDwTime");
  cLVitems = g_list_append (cLVitems, "POWER ON");
  cLVitems = g_list_append (cLVitems, "POWER OFF");
  gcal.cLVCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(gcal.cLVCommand), cLVitems);
  gtk_widget_set_size_request(gcal.cLVCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.cLVCommand,220, 750);

  GtkAdjustment *LVadjustment = (GtkAdjustment *) gtk_adjustment_new (5.0, 3.0, 500.0, 1.0, 1.0, 0.0);
  gcal.sbLVvalue = gtk_spin_button_new (LVadjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), gcal.sbLVvalue, 450, 750);

  GtkWidget *bSendLV;
  bSendLV=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSendLV, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixedGCAL), bSendLV, 600, 750);
  g_signal_connect(G_OBJECT(bSendLV), "clicked", G_CALLBACK(gcalSendLVCommand), NULL);


  //gtk_widget_hide(fixedGCAL);
}

/*************************************************************************************************************/
void create_main_window(void) {
/*************************************************************************************************************/

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "SLOW CONTROL");
  gtk_window_set_default_size(GTK_WINDOW(window), 1550, 830);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0x1111;
  color.green = 0x010F;
  color.blue = 0x664A;
  //gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

  fixedMain = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixedMain);
  g_signal_connect(window, "destroy", G_CALLBACK(exitSC), NULL); 

  // Create a new notebook, place the position of the tabs 
  notebook = gtk_notebook_new();
  gtk_fixed_put(GTK_FIXED(fixedMain), notebook, 10, 50);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_widget_set_size_request(notebook, 820, 850);

  // Scrolled window for the logbook
  scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request(scrolled_window,650,450);
    
  GtkWidget *textview = gtk_text_view_new();
  buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW (textview));
  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_create_tag(buffer,"lmarg","left_margin",5,NULL);
  gtk_text_buffer_create_tag(buffer,"blue_fg","foreground","blue",NULL);
  gtk_text_buffer_create_tag(buffer,"green_fg","foreground","green",NULL);
  gtk_text_buffer_create_tag(buffer,"red_bg","background","red",NULL);
  gtk_text_buffer_create_tag(buffer,"blue_bg","background","blue",NULL);
  gtk_text_buffer_create_tag(buffer,"yellow_bg","background","yellow",NULL);
  
  gtk_container_add(GTK_CONTAINER (scrolled_window), textview);
  gtk_fixed_put(GTK_FIXED(fixedMain), scrolled_window, 850, 150);

  /////////////////////////////////////////////////////////////////////
  // START/STOP/EXIT buttons
  ///////////////////////////////////////////////////////////////////// 
  int xbsize=110, ybsize=60;
  int xpos=890, ypos=680;
  bStart = gtk_button_new_with_label("Start");
  gtk_widget_set_size_request(bStart, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixedMain), bStart, xpos, ypos); 
  gtk_widget_set_sensitive(bStart, TRUE);
  g_signal_connect(G_OBJECT(bStart), "clicked", G_CALLBACK(startSC), NULL);
  
  bStop = gtk_button_new_with_label("Stop");
  gtk_widget_set_size_request(bStop, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixedMain), bStop, xpos+125, ypos);
  gtk_widget_set_sensitive(bStop, FALSE);
  g_signal_connect(G_OBJECT(bStop), "clicked", G_CALLBACK(stopSC), NULL);
 
  bExit = gtk_button_new_with_label("     EXIT\nSlow Control");
  gtk_widget_set_size_request(bExit, 150, 50);
  gtk_fixed_put(GTK_FIXED(fixedMain), bExit, 1320, 810);
  g_signal_connect(G_OBJECT(bExit), "clicked", G_CALLBACK(exitSC), NULL); 
 
  /////////////////////////////////////////////////////////////////////
  // Clear crate alarm cycle button
  ///////////////////////////////////////////////////////////////////// 
  bAlarm = gtk_button_new_with_label("Clear SY5527\n     Alarm");
  gtk_widget_set_size_request(bAlarm, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixedMain), bAlarm, xpos+250, ypos);
  gtk_widget_set_sensitive(bAlarm, FALSE);
  g_signal_connect(G_OBJECT(bAlarm), "clicked", G_CALLBACK(ClearCrateAlarm), NULL);


  /////////////////////////////////////////////////////////////////////
  // Update time display
  /////////////////////////////////////////////////////////////////////
  GtkWidget *lUpdateTime =  gtk_label_new("Last Update");
  gtk_fixed_put(GTK_FIXED(fixedMain), lUpdateTime, 845, 90);

  eUpdateTime = gtk_entry_new();
  gtk_widget_set_size_request(eUpdateTime, 170, 30); 
  gtk_fixed_put(GTK_FIXED(fixedMain), eUpdateTime, 845, 110);
  gtk_entry_set_editable(GTK_ENTRY(eUpdateTime), FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(eUpdateTime), FALSE);


  /////////////////////////////////////////////////////////////////////
  // INFN logo
  /////////////////////////////////////////////////////////////////////
  GtkWidget* image = gtk_image_new_from_file("Setup/logoINFN200x117.png");
  gtk_widget_set_size_request(image,145,80);
  gtk_fixed_put(GTK_FIXED(fixedMain), image, 1300, 20);

  /////////////////////////////////////////////////////////////////////
  // Alarm mask frame
  /////////////////////////////////////////////////////////////////////
  GtkWidget *mask = gtk_frame_new (" Mask Alarms ");
  gtk_fixed_put(GTK_FIXED(fixedMain), mask, 20, 0);
  gtk_widget_set_size_request (mask, 250, 40);
 

}



/*************************************************************************************************************/
void ClearCrateAlarm() {
/*************************************************************************************************************/
  GetTime(timeNow);

  int ret=CAENHV_ExecComm(handle,"ClearAlarm");
  if(ret) {
    sprintf(text,"%s ERROR clearing crate alarm \n", timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
  }
  else {
    sprintf(text,"%s Crate alarms cleared \n",timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
  }

}
