#include "HPGeMonitor.h"

#define nHV 2
#define nCMD 8

HighVoltageParams_t HVParams[nHV];
int handleDT5780;
   
GtkWidget *main_window;
GtkWidget *eVMon[nHV], *eIMon[nHV], *eVSet[nHV], *eISet[nHV], *ePw[nHV], *eStatus[nHV], *eRup[nHV], *eRdwn[nHV], *eChCommand;
GtkWidget *sbValueCommand;
GtkWidget *eCoolerMonitor[nCMD], *eUpdateTime; //entries
GtkWidget *tHV, *tCooler;    //tables
GtkWidget *lVMon, *lIMon, *lVSet, *lISet, *lPw, *lStatus, *lRup, *lRdwn, *lChannel[nHV]; // labels
GtkWidget *lColdTipT, *lCoolerStatus, *lColdHeadT, *lCompressorT, *lBoardT, *lFaultStatus, *lHVinhibit, *lPower, *lUpdateTime;
GtkWidget *bStart, *bStop, *bQuit, *bSend;  //buttons
GtkWidget *cCommand, *cChannelSelector; //ComboBox
GtkTextIter iter;
GtkTextBuffer *buffer;

int cport_nr=16;
unsigned char buf[4096];
char mode[]={'8','N','1',0};

char timeNow[22];
char text[100];

int running=0;

int main(int argc, char* argv[]) {
  int bdrate=9600;       

  GtkWidget* main_window;
  gtk_init(&argc, &argv);
  main_window = create_main_window();

  gtk_widget_show_all(main_window);
  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


// Open logfile
  char logFileName[100];
  sprintf(logFileName,"Log/HPGeMonitorLog");
  char bufTime[20];
  char strDateTime[21];
  time_t now = time(NULL);
  struct tm* tm = localtime(&now);
  strftime(bufTime, sizeof(bufTime), "%d-%m-%Y_%H-%M-%S", tm);
  sprintf(strDateTime,"%s", bufTime);
  strDateTime[21]='\0';
  strcat(logFileName,"_");
  strcat(logFileName,strDateTime);
  freopen(logFileName,"a",stdout);
  sprintf(text,"Opening logfile: %s \n",logFileName);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  if(RS232_OpenComport(cport_nr, bdrate, mode)) {
    sprintf(text,"***ERROR Can not open COM port %u \n",cport_nr);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
    printf(text);    
    exit(1);
  } 
  else {
      GetTime(timeNow);
      sprintf(text,"%s Connection opened with HPGe Cooler on COM port %u \n",timeNow,cport_nr);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
      printf(text);
    }


  // Load HV Parameters and informations with default values for the DT780
  for (int ch=0; ch<nHV; ch++) FillHVParameterDT5780(&HVParams[ch]);

  // Open connection with the board
  long int LinkType=1;
  int LinkNum=1;
  int err_code = CAEN_DGTZ_OpenDigitizer(LinkType,LinkNum,0,0,&handleDT5780);
  GetTime(timeNow);
  if(!err_code) {
   sprintf(text,"%s Connection opened with DT5780 board - handle is %i\n",timeNow,handleDT5780);
   gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
   printf(text);
  }
  else {
    sprintf(text,"***HPGeMonitor: %s ERROR opening DT5780 board - code %u \n",timeNow,err_code);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
    exit(1);
  }


  // HV initialization
  sprintf(text,"Starting HV system initialization\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);
  int32_t ret = ProgramHV(HVParams);
  if(ret) {
    sprintf(text,"***HPGeMonitor: Some error occured during the setting of the HV parameters\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
    exit(1);
  }
  else {
    GetTime(timeNow);
    sprintf(text,"%s HV system initialized\n",timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
  }


  gtk_main();

return 0;

}

/*************************************************************************************************************/
void startMonitor() {
/*************************************************************************************************************/
  int n,ready,first=0;
  char str[10][512];

  uint32_t retCode=0;
  uint32_t address,data;
  char strEntry[10];

  running=1;

  gtk_widget_set_sensitive(bStart, FALSE);
  gtk_widget_set_sensitive(bStop, TRUE);
  gtk_widget_set_sensitive(bQuit, TRUE);

  strcpy(str[0], "COOLER\r\n");
  strcpy(str[1], "TEMPC\r\n");
  strcpy(str[2], "TWE\r\n");
  strcpy(str[3], "TCO\r\n");
  strcpy(str[4], "TBRD\r\n");
  strcpy(str[5], "ERROR\r\n");
  strcpy(str[6], "HVINH\r\n");
  strcpy(str[7], "PWR\r\n");


  GetTime(timeNow);   
  sprintf(text,"%s Starting the monitoring system \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);


  while(running) {
    if(first) sleep(1);
    else first=1;

    while (gtk_events_pending()) gtk_main_iteration();

    for(int ch=0; ch<nHV; ch++) {
      PrintHVMonitor(ch);
    }

    for(int i=0; i<nCMD; i++) {
      RS232_cputs(cport_nr, str[i]);
      usleep(100000);  /* sleep for 100 milliSeconds */
      ready=0;
      while(n==0 || ready==0)  {
        n = RS232_PollComport(cport_nr, buf, 4095);
        if(n > 0 ) {
          buf[n] = 0;   // always put a "null" at the end of a string
          for(int j=0; j<n; j++) { // replace unreadable control-codes by empty spaces
            if(buf[j]<32) buf[j] = ' ';
          }
          //printf(" %s:   %s\n", str[i], (char *)buf);
          gtk_entry_set_text((gpointer) eCoolerMonitor[i], (gpointer) buf);
          ready = 1;
        }
      }
    }

    GetTime(timeNow);   
    gtk_entry_set_text((gpointer) eUpdateTime, (gpointer) timeNow); 
  }   
}





/*************************************************************************************************************/
void SendCommand(GtkWidget *button, gpointer data) {
/*************************************************************************************************************/
  uint32_t offset = 2;
  int32_t ret, u32, powerStatus;
  char strEntry[10];
  double VMonRes = 0.1;  // 0.1 V resolution
  double IMonRes = 0.01; // 10 nA resolution

  uint32_t SelectedChannel=0;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cChannelSelector)->entry));
  if(strcmp(channel,"CH1")==0) SelectedChannel=1;
  uint32_t chb = SelectedChannel + offset;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cCommand)->entry));
  
  double value=gtk_spin_button_get_value((gpointer) sbValueCommand);

  GetTime(timeNow);
  if(strcmp(strCommand,"POWER ON")==0) sprintf(text," %s Switching ON CH%u\n",timeNow,SelectedChannel);
  else if(strcmp(strCommand,"POWER OFF")==0) sprintf(text," %s Switching OFF CH%u\n",timeNow,SelectedChannel);
  else sprintf(text," %s Setting %s to value %.1f on CH%u\n",timeNow,strCommand,value,SelectedChannel);
  gtk_text_buffer_insert(buffer,&iter,text,-1);
  printf(text);

  if(strcmp(strCommand,"VSet")==0) {
    HVParams[SelectedChannel].VSet.Value=value;
    uint32_t VSet = HighVoltageUnitsToLSB(&HVParams[SelectedChannel].VSet);
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_VSET_ADDR | (chb << 8), VSet)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting VSet CH%u: code %u \n", SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_VSET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR reading address 0x%X CH%u: code %u \n", HV_VSET_ADDR,SelectedChannel,ret);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{
        sprintf(strEntry,"%.1f", u32*VMonRes);
        gtk_entry_set_text((gpointer) eVSet[SelectedChannel],(gpointer) strEntry);
      }
    }
  }
  else if(strcmp(strCommand,"ISet")==0) {
    HVParams[SelectedChannel].ISet.Value=value;
    uint32_t ISet = HighVoltageUnitsToLSB(&HVParams[SelectedChannel].ISet);
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_ISET_ADDR | (chb << 8), ISet)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting ISet CH%u: code %u \n",SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_ISET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR reading address 0x%X CH%u: code %u \n",HV_ISET_ADDR,SelectedChannel,ret);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(strEntry,"%.2f", u32*IMonRes);
        gtk_entry_set_text((gpointer) eISet[SelectedChannel],(gpointer) strEntry);
      }
    }
  }
  else if(strcmp(strCommand,"RUp")==0) {
    HVParams[SelectedChannel].RampUp.Value=value;
    uint32_t RampUp = HighVoltageUnitsToLSB(&HVParams[SelectedChannel].RampUp);
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_RAMPUP_ADDR | (chb << 8), RampUp)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting RampUp CH%u: code %u \n",SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPUP_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR reading address 0x%X CH%u: code %u \n", HV_RAMPUP_ADDR, SelectedChannel,ret);
        printf(text);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      }
      else{
        sprintf(strEntry,"%u", u32);
        gtk_entry_set_text((gpointer) eRup[SelectedChannel],(gpointer) strEntry);
      }
    }
  }
  else if(strcmp(strCommand,"RDwn")==0) {
    HVParams[SelectedChannel].RampDown.Value=value;
    uint32_t RampDown = HighVoltageUnitsToLSB(&HVParams[SelectedChannel].RampDown);
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_RAMPDOWN_ADDR | (chb << 8), RampDown)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting RampDown CH%u: code %u \n", SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPDOWN_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR reading address 0x%X CH%u: code %u \n",HV_RAMPDOWN_ADDR, SelectedChannel,ret);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{
        sprintf(strEntry,"%u", u32);
        gtk_entry_set_text((gpointer) eRdwn[SelectedChannel],(gpointer) strEntry);
      }
    }
  }
  else if(strcmp(strCommand,"POWER ON")==0) {
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) { 
      sprintf(text," ERROR %u reading address 0x%X  CH%u \n", ret, HV_POWER_ADDR,SelectedChannel);
      printf(text);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }
    u32 |= 0x1;   
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR switching power on CH%u: code %u \n", SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
  }
  else if(strcmp(strCommand,"POWER OFF")==0) {
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) { 
      sprintf(text," ERROR %u reading address 0x%X  CH%u \n", ret, HV_POWER_ADDR,SelectedChannel);
      printf(text);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }
    u32 &= ~(0x1); 
    if((ret = CAEN_DGTZ_WriteRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR switching power off CH%u: code %u \n",SelectedChannel,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
  }

}



/*************************************************************************************************************/
//  GtkWidget* create_main_window(void) 
//  Create the main window 
/*************************************************************************************************************/
GtkWidget* create_main_window(void) {

  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255 
  color.red = 0xFFFF;
  color.green = 0xDCDC;
  color.blue = 0x7878;

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window), "HPGe SYSTEM MONITOR");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 1230, 700);
  gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
  gtk_widget_modify_bg(main_window, GTK_STATE_NORMAL, &color);
  GtkWidget *fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(main_window), fixed);


  /////////////////////////////////////////////////////////////////////
  // HV setup frame
  /////////////////////////////////////////////////////////////////////
  int xPosFrame=10, yPosFrame=10;
  GtkWidget *frameHV = gtk_frame_new ("DT5780 HV Setup");
  gtk_fixed_put(GTK_FIXED(fixed), frameHV, xPosFrame,yPosFrame);
  gtk_widget_set_size_request (frameHV, 450, 520);

  int xpos=xPosFrame+370, ypos=yPosFrame+30;
  tHV=gtk_table_new (9,3,TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), tHV, xpos, ypos);

  char strChan[2];
  char labelChan[10];
  int xsize=80, ysize=30;
  int xoffset=120, xspace=40, yspace=10;
  for(int n=0; n<nHV; n++) {
    sprintf(strChan,"%i",n);
    strcpy(labelChan,"CHANNEL ");
    strcat(labelChan,strChan);
    lChannel[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixed), lChannel[n], xoffset+(xsize+xspace)*n, (ysize+yspace)+5);

    eVMon[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), eVMon[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*2);
    gtk_widget_set_size_request(eVMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(eVMon[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eVMon[n]), FALSE); 

    eIMon[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), eIMon[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*3);
    gtk_widget_set_size_request(eIMon[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(eIMon[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eIMon[n]), FALSE); 

    eVSet[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), eVSet[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*4);
    gtk_widget_set_size_request(eVSet[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(eVSet[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eVSet[n]), FALSE); 

    eISet[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), eISet[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*5);
    gtk_widget_set_size_request(eISet[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(eISet[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eISet[n]), FALSE); 

    eRup[n] = gtk_entry_new(); 
    gtk_fixed_put(GTK_FIXED(fixed), eRup[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*6); 
    gtk_widget_set_size_request(eRup[n], xsize, ysize); 
    gtk_entry_set_editable(GTK_ENTRY(eRup[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eRup[n]), FALSE); 

    eRdwn[n] = gtk_entry_new();  
    gtk_fixed_put(GTK_FIXED(fixed), eRdwn[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*7); 
    gtk_widget_set_size_request(eRdwn[n], xsize, ysize); 
    gtk_entry_set_editable(GTK_ENTRY(eRdwn[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eRdwn[n]), FALSE); 

    eStatus[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), eStatus[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*8);
    gtk_widget_set_size_request(eStatus[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(eStatus[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eStatus[n]), FALSE); 

    ePw[n] = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), ePw[n], xoffset+(xsize+xspace)*n, (ysize+yspace)*9);
    gtk_widget_set_size_request(ePw[n], xsize, ysize);
    gtk_entry_set_editable(GTK_ENTRY(ePw[n]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(ePw[n]), FALSE); 
 
  }
  
  int lxoff=30, lyoff=40;

  lVMon =  gtk_label_new("VMon (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lVMon, lxoff, lyoff*2);
   
  lIMon =  gtk_label_new("IMon (uA)");
  gtk_fixed_put(GTK_FIXED(fixed), lIMon, lxoff, lyoff*3);

  lVSet =  gtk_label_new("VSet (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lVSet, lxoff, lyoff*4);
  
  lISet =  gtk_label_new("ISet (uA)");
  gtk_fixed_put(GTK_FIXED(fixed), lISet, lxoff, lyoff*5);

  lRup =  gtk_label_new("Rup (V/s)");
  gtk_fixed_put(GTK_FIXED(fixed), lRup, lxoff, lyoff*6);
 
  lRdwn =  gtk_label_new("Rdwn (V/s)");
  gtk_fixed_put(GTK_FIXED(fixed), lRdwn, lxoff, lyoff*7);

  lStatus =  gtk_label_new("Status");
  gtk_fixed_put(GTK_FIXED(fixed), lStatus, lxoff, lyoff*8);

  lPw =  gtk_label_new("Pw");
  gtk_fixed_put(GTK_FIXED(fixed), lPw, lxoff, lyoff*9);
 
  GList *csel = NULL;
  csel = g_list_append (csel, "CH0");
  csel = g_list_append (csel, "CH1");
  cChannelSelector=gtk_combo_new();
  gtk_widget_set_size_request(cChannelSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(cChannelSelector), csel);
  gtk_fixed_put(GTK_FIXED(fixed), cChannelSelector, 30, 440);

  GList *cbitems = NULL;
  cbitems = g_list_append (cbitems, "VSet");
  cbitems = g_list_append (cbitems, "ISet");
  cbitems = g_list_append (cbitems, "RUp");
  cbitems = g_list_append (cbitems, "RDwn");
  cbitems = g_list_append (cbitems, "POWER ON");
  cbitems = g_list_append (cbitems, "POWER OFF");
  cCommand=gtk_combo_new();
  gtk_widget_set_size_request(cCommand, 150, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(cCommand), cbitems);
  gtk_fixed_put(GTK_FIXED(fixed), cCommand, 120, 440);

  GtkAdjustment *adjustment = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 3000.0, 10.0, 50.0, 0.0);
  sbValueCommand = gtk_spin_button_new (adjustment, 1.0, 0);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON (sbValueCommand), TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), sbValueCommand, 300, 440);


  bSend=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSend, 100, 30); 
  gtk_fixed_put(GTK_FIXED(fixed), bSend, 170, 490); 
  g_signal_connect(G_OBJECT(bSend), "clicked", G_CALLBACK(SendCommand), NULL);
 

  /////////////////////////////////////////////////////////////////////
  // Cooler frame
  /////////////////////////////////////////////////////////////////////
  xPosFrame=510;  yPosFrame=10;
  GtkWidget *frameCooler = gtk_frame_new ("HPGe Cooler Monitor");
  gtk_fixed_put(GTK_FIXED(fixed), frameCooler, xPosFrame,yPosFrame);
  gtk_widget_set_size_request (frameCooler, 450, 300);

  xpos=xPosFrame+270; ypos=yPosFrame+30;
  tCooler=gtk_table_new (9,2,TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), tCooler, xpos, ypos);

  int et, xe=80, ye=30;
  for(et=0;et<nCMD;et++) {
    eCoolerMonitor[et] = gtk_entry_new();
    gtk_widget_set_size_request(eCoolerMonitor[et], xe, ye);
    gtk_table_attach_defaults (GTK_TABLE(tCooler), eCoolerMonitor[et], 0, 1, et, et+1);
    gtk_entry_set_editable(GTK_ENTRY(eCoolerMonitor[et]), FALSE); 
    gtk_widget_set_can_focus(GTK_WIDGET(eCoolerMonitor[et]), FALSE); 

  }
  xpos=xPosFrame+25; ypos+=10;
  GtkWidget* lCoolerStatus = gtk_label_new("Cooler Status (ON/OFF)");
  gtk_fixed_put(GTK_FIXED(fixed), lCoolerStatus, xpos, ypos);

  ypos += ye;
  GtkWidget* lColdTipT = gtk_label_new("Cold-tip temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lColdTipT, xpos, ypos);

  ypos += ye;
  GtkWidget* lColdHeadT = gtk_label_new("Cold-head temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lColdHeadT, xpos, ypos);

  ypos += ye;
  GtkWidget* lCompressorT = gtk_label_new("Compressor temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lCompressorT, xpos, ypos);

  ypos += ye;
  GtkWidget* lBoardT = gtk_label_new("Board Temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lBoardT, xpos, ypos);

  ypos += ye;
  GtkWidget* lFaultStatus = gtk_label_new("Fault status");
  gtk_fixed_put(GTK_FIXED(fixed), lFaultStatus, xpos, ypos);

  ypos += ye;
  GtkWidget* lHVinhibit = gtk_label_new("HV inhibit status (ON/OFF)");
  gtk_fixed_put(GTK_FIXED(fixed), lHVinhibit, xpos, ypos);

  ypos += ye;
  GtkWidget* lPower = gtk_label_new("Power (W)");
  gtk_fixed_put(GTK_FIXED(fixed), lPower, xpos, ypos);


  /////////////////////////////////////////////////////////////////////
  // Scrolled window for the logbook
  /////////////////////////////////////////////////////////////////////
  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request(scrolled_window,650,300);

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
  gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 510, 360);

  /////////////////////////////////////////////////////////////////////
  // START/STOP/QUIT buttons
  /////////////////////////////////////////////////////////////////////
  int xbsize=80, ybsize=50;
  xpos=120; ypos=600;
  bStart = gtk_button_new_with_label("Start");
  gtk_widget_set_size_request(bStart, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStart, xpos, ypos);
  gtk_widget_set_sensitive(bStart, TRUE);
  g_signal_connect(G_OBJECT(bStart), "clicked", G_CALLBACK(startMonitor), NULL);  

  bStop = gtk_button_new_with_label("Stop");
  gtk_widget_set_size_request(bStop, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStop, xpos+100, ypos);
  gtk_widget_set_sensitive(bStop, FALSE);
  g_signal_connect(G_OBJECT(bStop), "clicked", G_CALLBACK(stopMonitor), NULL);  

  bQuit = gtk_button_new_with_label("Quit");
  gtk_widget_set_size_request(bQuit, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, xpos+200, ypos);
  gtk_widget_set_sensitive(bQuit, TRUE);
  g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(quitMonitor), NULL);  

  /////////////////////////////////////////////////////////////////////
  // Update time display
  /////////////////////////////////////////////////////////////////////
  lUpdateTime =  gtk_label_new("Last Update");
  gtk_fixed_put(GTK_FIXED(fixed), lUpdateTime, 980, 250 );
  eUpdateTime = gtk_entry_new();
  gtk_widget_set_size_request(eUpdateTime, 170, ye);
  gtk_fixed_put(GTK_FIXED(fixed), eUpdateTime, 980, 270);
  gtk_entry_set_editable(GTK_ENTRY(eUpdateTime), FALSE); 
  gtk_widget_set_can_focus(GTK_WIDGET(eUpdateTime), FALSE); 


  /////////////////////////////////////////////////////////////////////
  // INFN logo
  /////////////////////////////////////////////////////////////////////
  GtkWidget* image = gtk_image_new_from_file("Setup/logoINFN200x117.png");
  gtk_widget_set_size_request(image,145,80);
  gtk_fixed_put(GTK_FIXED(fixed), image, 1000, 20);


  return main_window;
}


/*************************************************************************************************************/
void stopMonitor() {
/*************************************************************************************************************/
  running=0;
  gtk_widget_set_sensitive(bStop, FALSE);
  gtk_widget_set_sensitive(bStart, TRUE);

  GetTime(timeNow);
  sprintf(text,"%s Stopping the monitoring system \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  printf(text);

}



/*************************************************************************************************************/
void quitMonitor() {
/*************************************************************************************************************/
  running=0;

  CAEN_DGTZ_CloseDigitizer(handleDT5780);
  sprintf(text,"%s Quitting HPGeMonitor \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);

  fflush(stdout);
  fclose(stdout);  // close logFile

  gtk_main_quit();

}




/*************************************************************************************************************/
void PrintHVMonitor(uint32_t ch) {
/*************************************************************************************************************/
  char strEntry[10];
  uint32_t offset = 2;
  int32_t ret;
  uint32_t u32;
  uint32_t chb = ch + offset; 

  double VMonRes = 0.1;  // 0.1 V resolution
  double IMonRes = 0.01; // 10 nA resolution

  // Read and print VMon
  if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_VMON_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
    sprintf(text,"ERROR reading address 0x%X  CH%u: code %u \n",HV_VMON_ADDR,ch,ret);
    printf(text);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  }
  else {
    sprintf(strEntry,"%.1f", u32*VMonRes);
    if(ch==1) sprintf(strEntry,"%.1f", u32*(-VMonRes)); //***CH1 HAS NEGATIVE HV !!
    gtk_entry_set_text((gpointer) eVMon[ch],(gpointer) strEntry);
  }
              
  // Read and print IMon
  if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_IMON_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
    sprintf(text,"ERROR reading address 0x%X  CH%u: code %u \n",HV_IMON_ADDR,ch,ret);
    printf(text);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  }
  else {
    sprintf(strEntry,"%.2f", u32*IMonRes);
    gtk_entry_set_text((gpointer) eIMon[ch],(gpointer) strEntry);
  }


  // Read and print Power 
  if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
    sprintf(text,"ERROR reading address 0x%X  CH%u: code %u \n", HV_POWER_ADDR,ch,ret);
    printf(text);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  }
  else {
    sprintf(strEntry,"Off");
    if(u32 & (0x1<<0)) sprintf(strEntry,"On");
    gtk_entry_set_text((gpointer) ePw[ch],(gpointer) strEntry);
  }


  // Read and print Status
  static uint32_t prevStatus[2]={0,0};  
  if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_STATUS_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
    sprintf(text,"ERROR reading address 0x%X  CH%u: code %u \n",HV_STATUS_ADDR,ch,ret);
    printf(text);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  }
  else {
    if(u32==0) sprintf(strEntry,"%s"," ");
    if(u32 & (0x1 << 0)) sprintf(strEntry,"%s", "PowerON");
    if(u32 & (0x1 << 1)) sprintf(strEntry,"%s", "RampUp");
    if(u32 & (0x1 << 2)) sprintf(strEntry,"%s", "RampDown");
    if(u32 & (0x1 << 3)) sprintf(strEntry,"%s", "OvCurrent");
    if(u32 & (0x1 << 4)) sprintf(strEntry,"%s", "OverVoltage");
    if(u32 & (0x1 << 5)) sprintf(strEntry,"%s", "UnderVoltage");
    if(u32 & (0x1 << 6)) sprintf(strEntry,"%s", "OverVMAX");
    if(u32 & (0x1 << 7)) sprintf(strEntry,"%s", "OverIMAX");
    if(u32 & (0x1 << 8)) sprintf(strEntry,"%s", "TempWarning");
    if(u32 & (0x1 << 9)) sprintf(strEntry,"%s", "OverTemp");
    if(u32 & (0x1 << 10)) sprintf(strEntry,"%s", "HVInhibit");
    if(u32 & (0x1 << 11)) sprintf(strEntry,"%s", "CalibError");
    if(u32 & (0x1 << 12)) sprintf(strEntry,"%s", "AlarmReset");
    if(u32 & (0x1 << 13)) sprintf(strEntry,"%s", "HVshutDown");
    if(u32 & (0x1 << 14)) sprintf(strEntry,"%s", "MaxPower");
    if(u32 & (0x1 << 15)) sprintf(strEntry,"%s", "FanSpeedHigh");
    gtk_entry_set_text((gpointer) eStatus[ch],(gpointer) strEntry);

    if(u32 != prevStatus[ch]) {
      prevStatus[ch]=u32;
      GetTime(timeNow);  
      sprintf(text,"%s %s CH%u\n",timeNow,strEntry,ch);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
      printf(text);
    }      
  }

}



/*************************************************************************************************************/
int32_t ProgramHV(HighVoltageParams_t HVParams[]) {
/*************************************************************************************************************/
  // On DT5780 boards, HV channels 0/1 are reached through board channels 2/3.
  // we then use an offset of 2 when we act on channels 0/1.

  uint32_t chb,offset=2,ret=0,u32;

  double Vres = 0.1;  // 0.1 V resolution
  double Ires = 0.01; // 10 nA resolution
  double Rres = 1.;   // 1 s resolution
  double VMres = 20.; // 20 V resolution
   
  char strEntry[10];

  // Set only the parameters not accessible to the user, read & print the others
  uint32_t ch=0;
  while(ch < nHV) {

    uint32_t chb = ch + offset;

    sprintf(text,"ProgramHV - handleDT5780 %i\n",handleDT5780);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);

    // Read Vset (0x1n20)
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_VSET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u Reading VSet for CH%u \n", ret,ch);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{ 
      sprintf(strEntry,"%.1f", u32*Vres);
      gtk_entry_set_text((gpointer) eVSet[ch],(gpointer) strEntry);
    }

    // Read Iset (0x1n24)
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_ISET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u Reading ISet for CH%u \n", ret,ch);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{ 
      sprintf(strEntry,"%.1f", u32*Ires);
      gtk_entry_set_text((gpointer) eISet[ch],(gpointer) strEntry);
    }

    // Read RampUp (0x1n28)
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPUP_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u Reading Rup for CH%u \n", ret,ch);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{ 
      sprintf(strEntry,"%.0f", u32*Rres);
      gtk_entry_set_text((gpointer) eRup[ch],(gpointer) strEntry);
    }

    // Read RampDown (0x1n2C)
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPDOWN_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u Reading Rdown for CH%u \n", ret,ch);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{ 
      sprintf(strEntry,"%.0f", u32*Rres);
      gtk_entry_set_text((gpointer) eRdwn[ch],(gpointer) strEntry);
    }

    // Set VMax (0x1n30)
    uint32_t VMax = HighVoltageUnitsToLSB(&HVParams[ch].VMax);
    if ((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_VMAX_ADDR | (chb << 8), VMax)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting VMax  CH%u: code %u \n",ch,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_VMAX_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u setting VMax to %.1f for CH%u \n", ret,VMax*Vres,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(text," Setting VMax to %.0f for CH%u \n", u32*VMres,ch);
        gtk_text_buffer_insert(buffer,&iter,text,-1);
        printf(text);
      }
    }

    // Read On/Off status
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u reading address 0x%X  CH%u \n", ret, HV_POWER_ADDR,ch);
      printf(text);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }
    else {
      sprintf(strEntry,"Off");
      if(u32 & (0x1<<0)) {
        sprintf(strEntry,"On");
      }    
      gtk_entry_set_text((gpointer) ePw[ch],(gpointer) strEntry);  
    }


    // Set Power Down Mode - HV control (0x1n34)
    uint32_t PwDownMode;
    uint32_t mode = (HVParams[ch].PWDownMode == HighVoltage_PWDown_Kill) ? HV_PWDOWN_BITVALUE_KILL : HV_PWDOWN_BITVALUE_RAMP;
    printf(" mode %u \n", mode);
    if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &PwDownMode)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR %u reading address 0x%X  CH%u \n", ret, HV_POWER_ADDR,ch);
      printf(text);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }
    else {
      printf(" CH%u  PwDownMode %u (first)\n",ch,PwDownMode);
      PwDownMode &= ~(0x1 << HV_REGBIT_PWDOWN); // erase bit HV_REGBIT_PWDOWN
      printf(" CH%u  PwDownMode %u (second)\n",ch,PwDownMode);
      PwDownMode |= (mode << HV_REGBIT_PWDOWN);
      printf(" CH%u  PwDownMode %u (third)\n",ch,PwDownMode);
      if((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), PwDownMode)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u setting PowerDown mode  CH%u \n",ret,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else {
        if((ret = CAEN_DGTZ_ReadRegister(handleDT5780, HV_POWER_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
          sprintf(text," ERROR %u reading PowerDownMode for CH%u \n",ret,ch);
          gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
          printf(text);
        }
        else { 
          if(u32 & (0x1 << HV_REGBIT_PWDOWN)) {
            sprintf(text," PowerDown mode RAMP for CH%u \n",ch);
            gtk_text_buffer_insert(buffer,&iter,text,-1);
            printf(text);
          }
          else {
            sprintf(text," PowerDown mode KILL for CH%u \n",ch);
            gtk_text_buffer_insert(buffer,&iter,text,-1);
            printf(text);
            sprintf(text," ***WARNING: PowerDown mode is set to KILL. This can seriously damage your detector \n",ch);
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
            printf(text);
          }
        }
      }
    }
  
    ch++;
  }


///////////////////////////////////  
// TO BE REMOVED FROM THE CODE 
///////////////////////////////////  
/*   
    // Set VSet (0x1n20)
    uint32_t VSet = HighVoltageUnitsToLSB(&HVParams[ch].VSet);
    if((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_VSET_ADDR | (chb << 8), VSet)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting VSet CH%u: code %u \n",ch,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_VSET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u Setting VSet to %.1f for CH%u \n", ret,VSet*Vres,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(strEntry,"%.1f", u32*Vres);
        gtk_entry_set_text((gpointer) eVSet[ch],(gpointer) strEntry);
        sprintf(text," Setting VSet to %.1f for CH%u \n", u32*Vres,ch);
        gtk_text_buffer_insert(buffer,&iter,text,-1); 
        printf(text);
      }
    }

    // Set ISet (0x1n24)
    uint32_t ISet = HighVoltageUnitsToLSB(&HVParams[ch].ISet);
    if((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_ISET_ADDR | (chb << 8), ISet)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting ISet CH%u: code %u \n",ch, ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_ISET_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u Setting ISet to %.1f for CH%u \n", ret,ISet*Ires,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(strEntry,"%.1f", u32*Ires);
        gtk_entry_set_text((gpointer) eISet[ch],(gpointer) strEntry);
        sprintf(text," Setting ISet to %.1f for CH%u \n", u32*Ires,ch);
        gtk_text_buffer_insert(buffer,&iter,text,-1);
        printf(text);
      }
    }

    // Set RampUp (0x1n28)   
    uint32_t RampUp = HighVoltageUnitsToLSB(&HVParams[ch].RampUp);
    if((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_RAMPUP_ADDR | (chb << 8), RampUp)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting RampUp  CH%u: code %u \n",ch,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPUP_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u Setting Rup to %.1f for CH%u \n", ret,RampUp*Rres,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(strEntry,"%.0f", u32*Rres);
        gtk_entry_set_text((gpointer) eRup[ch],(gpointer) strEntry);
        sprintf(text," Setting Rup to %.0f for CH%u \n", u32*Rres,ch);
        gtk_text_buffer_insert(buffer,&iter,text,-1);
        printf(text);
      }
    }

    // Set RampDown (0x1n2C)
    uint32_t RampDown = HighVoltageUnitsToLSB(&HVParams[ch].RampDown);
    if((ret |= CAEN_DGTZ_WriteRegister(handleDT5780, HV_RAMPDOWN_ADDR | (chb << 8), RampDown)) != CAEN_DGTZ_Success) {
      sprintf(text," ERROR setting RampDown CH%u: code %u \n",ch,ret);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
      printf(text);
    }
    else{
      if((ret |= CAEN_DGTZ_ReadRegister(handleDT5780, HV_RAMPDOWN_ADDR | (chb << 8), &u32)) != CAEN_DGTZ_Success) {
        sprintf(text," ERROR %u Setting Rdown to %.1f for CH%u \n", ret,RampDown*Rres,ch);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
        printf(text);
      }
      else{ 
        sprintf(strEntry,"%.0f", u32*Rres);
        gtk_entry_set_text((gpointer) eRdwn[ch],(gpointer) strEntry);
        sprintf(text," Setting Rdown to %.0f for CH%u \n", u32*Rres,ch);
        gtk_text_buffer_insert(buffer,&iter,text,-1);
        printf(text);
      }
    }
 
*/


  return ret;
}
  



/*************************************************************************************************************/
uint32_t HighVoltageUnitsToLSB(HighVoltageParameter_t *parameter) {
/*************************************************************************************************************/
//  Return the value of HV parameter converted from Parameter Units to LSB
  if (parameter->Value > parameter->Infos.Max)
      parameter->Value = parameter->Infos.Max;
  if (parameter->Value < parameter->Infos.Min)
      parameter->Value = parameter->Infos.Min;

  return (uint32_t)(parameter->Value/parameter->Infos.Res);
}




/*************************************************************************************************************/
void FillHVParameterDT5780(HighVoltageParams_t *Params) {
/*************************************************************************************************************/
//   Set default the values of the HV system
    
  // VSet
  Params->VSet.Infos.Max = 2560.0; // V
  Params->VSet.Infos.Min = 0.0;    // V
  Params->VSet.Infos.Res = 0.1;    // Volts/LSB
  Params->VSet.Value = 1.0;        // V

  // ISet
  Params->ISet.Infos.Max = 10.0;    // uA
  Params->ISet.Infos.Min =  0.0;    // uA   
  Params->ISet.Infos.Res =  0.01;   // uA/LSB
  Params->ISet.Value =  1.0;        // uA
        
  // VMax
  Params->VMax.Infos.Max = 2600.0; // V
  Params->VMax.Infos.Min = 0.0;    // V
  Params->VMax.Infos.Res = 20.0;   // V/LSB
  Params->VMax.Value = 2600.0;     // V

  // RampDown
  Params->RampDown.Infos.Max = 50.0; // V/s
  Params->RampDown.Infos.Min = 1.0;   // V/s
  Params->RampDown.Infos.Res = 1.0;   // V/s/LSB
  Params->RampDown.Value = 10.0;      // V/s

  // RampUp
  Params->RampUp.Infos.Max = 50.0; // V/s
  Params->RampUp.Infos.Min = 1.0;  // V/s
  Params->RampUp.Infos.Res = 1.0;  // V/s/LSB
  Params->RampUp.Value = 5.0;      // V/s

  // Power down mode 
  Params->PWDownMode = HighVoltage_PWDown_Ramp;

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
