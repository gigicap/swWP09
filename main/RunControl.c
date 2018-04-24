#include "RunControl.h"

DaqSharedMemory* rcDaqSharedMemory;

GtkWidget *sbNEvents, *sbTime, *sbComptonV1742Ch, *sbComptonV1742Gr, *sbCaloV1742Ch, *sbCaloV1742Gr, *sbComptonDT5780Ch, *sbComptonPlotScaler, *sbCaloPlotScaler,  *sbAlfaScaler, *sbRate;  // spin buttton
GtkWidget *eFileName, *eRunNumber; // entry text
GtkWidget *eMonitorProd[5], *eMonitorCons[5];
GtkWidget *tMonitor; // table
GtkWidget *bConfigure, *bAdvancedSetup, *bStartDaq, *bStopDaq, *bStartSlowControl, *bQuit; //buttons
GtkWidget *rTime, *rNEvents, *rCaloV1742Ch, *rCaloV1742Gr , *rBaFV1742Ch, *rBaFV1742Gr; // radio buttons
GtkWidget *cConsumer, *cComptonPlotter, *cCaloPlotter, *cMonitor, *cSoftTrg, *cTestHpGe, *cCalo, *cBaF, *cHpGe, *cPamSi; //check buttons
GtkWidget *cbRecipeList; //combo box
GtkTextIter iter;
GtkTextBuffer *buffer;

char currentDir[100], defaultDir[100], defaultFile[100];
int connectionParams[4];
int V1742Ready=0, DT5780Ready=0, V1485Ready=0, V1742CaloReady;
int running=0;
int ret;
char logFilename[100];
char timeNow[22];
char text[100];

int main(int argc, char *argv[]) {

  GtkWidget* main_window;

  gtk_init(&argc, &argv);
  main_window = create_main_window();
  gtk_widget_show_all(main_window);
  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  rcDaqSharedMemory=configDaqSharedMemory("RunControl");
  sprintf(text,"RunControl: Configuring the DAQ shared memory\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 

  updateRunNumber(0);

  gtk_main();
  
  return 0;
}


/*************************************************************************************************************/
GtkWidget* create_main_window() {
/*************************************************************************************************************/
  getcwd(currentDir, sizeof(currentDir));

  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0xD8D8;
  color.green = 0xBFBF;
  color.blue = 0xD8D8;

  //static GdkPixmap *pixmap = NULL;
  //GtkWidget *lNEvents, *lTime, *lFileName, *lNRun, *lRunNumber, *lAlfaScaler, *lRate;

  GtkWidget *lFileName, *lRunNumber, *lAlfaScaler, *lRate; // labels

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "RUN CONTROL");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

  GtkWidget* fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);

  int xframe,yframe,xbsize,ybsize;

  /////////////////////////////////////////////////////////////////////
  // Device configuration frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;  yframe=10;  xbsize=150;  ybsize=40;
  GtkWidget* frameConfig = gtk_frame_new ("Device Config");
  gtk_fixed_put(GTK_FIXED(fixed), frameConfig, xframe,yframe);
  gtk_widget_set_size_request (frameConfig, 600, 130);

  cbRecipeList=gtk_combo_box_text_new();
  gtk_widget_set_size_request (cbRecipeList, 300, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), cbRecipeList,xframe+120 , yframe+30 );
  gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(cbRecipeList),1);
  gtk_widget_show (cbRecipeList);
  FillRecipeList(cbRecipeList, "Default",1);
  GtkWidget* lRecipes=gtk_label_new("RECIPE NAME");
  gtk_fixed_put(GTK_FIXED(fixed), lRecipes, xframe+10, yframe+40);
  g_signal_connect(G_OBJECT(cbRecipeList), "set-focus-child", G_CALLBACK(loadRecipeList), NULL);
  g_signal_connect(G_OBJECT(cbRecipeList), "changed", G_CALLBACK(changeRecipe), NULL);

  bConfigure=gtk_button_new_with_label("Configure Devices");
  gtk_widget_set_size_request(bConfigure, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bConfigure, xframe+120, yframe+80);
  g_signal_connect(G_OBJECT(bConfigure), "clicked", G_CALLBACK(ConfigDigi), NULL);

  bAdvancedSetup=gtk_button_new_with_label("Advanced Setup");
  gtk_widget_set_size_request(bAdvancedSetup, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bAdvancedSetup, xframe+430, yframe+80);
  g_signal_connect(G_OBJECT(bAdvancedSetup), "clicked", G_CALLBACK(openAdvancedSetup), NULL);


  /////////////////////////////////////////////////////////////////////
  // Device selection frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;   yframe=160;
  GtkWidget *frameDevices = gtk_frame_new ("Device Selection");
  gtk_fixed_put(GTK_FIXED(fixed), frameDevices, xframe,yframe);
  gtk_widget_set_size_request (frameDevices, 600, 140);

  cBaF = gtk_check_button_new_with_label ("BaF");
  gtk_fixed_put(GTK_FIXED(fixed), cBaF, xframe+30, yframe+20);
  gtk_widget_set_sensitive(cBaF, FALSE);
  g_signal_connect(G_OBJECT(cBaF), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cHpGe = gtk_check_button_new_with_label("HpGe");
  gtk_fixed_put(GTK_FIXED(fixed), cHpGe, xframe+30, yframe+50);
  gtk_widget_set_sensitive(cHpGe, FALSE);
  g_signal_connect(G_OBJECT(cHpGe), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cPamSi = gtk_check_button_new_with_label ("PamSi");
  gtk_fixed_put(GTK_FIXED(fixed), cPamSi, xframe+30, yframe+80);
  gtk_widget_set_sensitive(cPamSi, FALSE);
  g_signal_connect(G_OBJECT(cPamSi), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cCalo = gtk_check_button_new_with_label("Calo"); 
  gtk_fixed_put(GTK_FIXED(fixed), cCalo, xframe+370, yframe+20);
  gtk_widget_set_sensitive(cCalo, FALSE);
  g_signal_connect(G_OBJECT(cCalo), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  sbAlfaScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbAlfaScaler, xframe+210, yframe+20);
  lAlfaScaler = gtk_label_new("Alpha Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lAlfaScaler, 140, 185);


  /////////////////////////////////////////////////////////////////////
  // Run configuration frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;   yframe=320;
  GtkWidget *frameRunConfig = gtk_frame_new ("Run Config");
  gtk_fixed_put(GTK_FIXED(fixed), frameRunConfig, xframe,yframe);
  gtk_widget_set_size_request (frameRunConfig, 600, 170);

  rTime = gtk_radio_button_new_with_label(NULL,"Time (s)");
  gtk_fixed_put(GTK_FIXED(fixed), rTime, xframe+10, yframe+20);
  rNEvents = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rTime),"N Events");
  gtk_fixed_put(GTK_FIXED(fixed), rNEvents, xframe+10, yframe+60);
  g_signal_connect (GTK_TOGGLE_BUTTON (rTime), "toggled",G_CALLBACK (selectRunType), NULL);

  sbTime = gtk_spin_button_new_with_range(0, 1000000,1000);
  gtk_spin_button_set_value((gpointer) sbTime, 3600);
  gtk_fixed_put(GTK_FIXED(fixed), sbTime, xframe+110, yframe+20);

  sbNEvents = gtk_spin_button_new_with_range(0, 10000000,1000);
  gtk_spin_button_set_value((gpointer) sbNEvents, 100000);
  gtk_fixed_put(GTK_FIXED(fixed), sbNEvents, xframe+110, yframe+60);
  gtk_widget_set_sensitive(sbNEvents, FALSE);

  eFileName = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eFileName, xframe+90, yframe+120);
  gtk_widget_set_size_request(eFileName, 500, 30);
  lFileName =  gtk_label_new("File Name");
  gtk_fixed_put(GTK_FIXED(fixed), lFileName, xframe+10, yframe+125);
  strcat(defaultFile,currentDir);
  strcat(defaultFile,"/Data/");
  gtk_entry_set_text((gpointer) eFileName,defaultFile);

  cSoftTrg = gtk_check_button_new_with_label("Software Trigger"); 
  gtk_fixed_put(GTK_FIXED(fixed), cSoftTrg, xframe+290, yframe+60);
 
  cConsumer = gtk_check_button_new_with_label("Consumer");
  gtk_fixed_put(GTK_FIXED(fixed), cConsumer, xframe+290, yframe+20);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConsumer),1);

  cTestHpGe = gtk_check_button_new_with_label("Pulser HpGe"); 
  gtk_fixed_put(GTK_FIXED(fixed), cTestHpGe, xframe+450, yframe+20);
  gtk_widget_set_sensitive(cTestHpGe,FALSE);
  g_signal_connect(G_OBJECT(cTestHpGe), "toggled", G_CALLBACK(CheckTestHpGe), NULL);

  sbRate = gtk_spin_button_new_with_range(0, 500,10);
  gtk_spin_button_set_value((gpointer) sbRate, 10);
  gtk_fixed_put(GTK_FIXED(fixed), sbRate, xframe+520, yframe+60);
  lRate =  gtk_label_new("Rate (Hz)");
  gtk_fixed_put(GTK_FIXED(fixed), lRate, xframe+450, yframe+65);
  gtk_signal_connect (GTK_OBJECT (sbRate), "value_changed", G_CALLBACK(updateRate),NULL);



  /////////////////////////////////////////////////////////////////////
  // Scrolled window for the logbook
  /////////////////////////////////////////////////////////////////////
  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request(scrolled_window,600,300);

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
  gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 10, 530);


  /////////////////////////////////////////////////////////////////////
  // Start/Stop buttons, Run number display
  /////////////////////////////////////////////////////////////////////
  eRunNumber = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eRunNumber, 650, 550);
  gtk_widget_set_size_request(eRunNumber, 100, 25);
  lRunNumber =  gtk_label_new("Current Run Number");
  gtk_fixed_put(GTK_FIXED(fixed), lRunNumber, 650, 520);
  gtk_entry_set_text((gpointer) eRunNumber,"1");

  bStartSlowControl = gtk_button_new_with_label("Start Slow Control");
  gtk_widget_set_size_request(bStartSlowControl, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStartSlowControl, 700, 620);
  gtk_widget_set_sensitive(bStartSlowControl, FALSE);
  g_signal_connect(G_OBJECT(bStartSlowControl), "clicked", G_CALLBACK(StartSlowControl), NULL);

  bStartDaq = gtk_button_new_with_label("Start DAQ");
  gtk_widget_set_size_request(bStartDaq, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStartDaq, 900, 620);
  gtk_widget_set_sensitive(bStartDaq, FALSE);
  g_signal_connect(G_OBJECT(bStartDaq), "clicked", G_CALLBACK(StartDaq), NULL);

  bStopDaq = gtk_button_new_with_label("Stop DAQ");
  gtk_widget_set_size_request(bStopDaq, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStopDaq, 1100, 620);
  gtk_widget_set_sensitive(bStopDaq, FALSE);
  g_signal_connect(G_OBJECT(bStopDaq), "clicked", G_CALLBACK(StopDaq), NULL);


  /////////////////////////////////////////////////////////////////////
  // CSPEC plotter frame
  /////////////////////////////////////////////////////////////////////
  xframe=650;
  yframe=160;
  GtkWidget *frameComptonPlotter = gtk_frame_new ("CSPEC Plotter");
  gtk_fixed_put(GTK_FIXED(fixed), frameComptonPlotter, xframe,yframe);
  gtk_widget_set_size_request (frameComptonPlotter, 315, 140);

  cComptonPlotter = gtk_check_button_new_with_label("Plotter Enable");
  gtk_fixed_put(GTK_FIXED(fixed), cComptonPlotter, xframe+20, yframe+30);
  g_signal_connect(G_OBJECT(cComptonPlotter), "toggled", G_CALLBACK(checkComptonPlotter), NULL);

  sbComptonV1742Ch = gtk_spin_button_new_with_range (0,36,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonV1742Ch, xframe+100, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbComptonV1742Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);
 
  sbComptonV1742Gr = gtk_spin_button_new_with_range (0,3,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonV1742Gr, xframe+100, yframe+110);
  GtkWidget *lComptonV1742Ch = gtk_label_new("V1742");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonV1742Ch, xframe+20, yframe+60);
  gtk_signal_connect (GTK_OBJECT (sbComptonV1742Gr), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  rBaFV1742Ch = gtk_radio_button_new_with_label(NULL,"Channel");
  gtk_fixed_put(GTK_FIXED(fixed), rBaFV1742Ch, xframe+20, yframe+80);
  rBaFV1742Gr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rBaFV1742Ch),"Group");
  gtk_fixed_put(GTK_FIXED(fixed), rBaFV1742Gr, xframe+20, yframe+110);
  g_signal_connect (GTK_TOGGLE_BUTTON (rBaFV1742Ch), "toggled", G_CALLBACK (selectV1742PlotType), NULL);


  sbComptonDT5780Ch = gtk_spin_button_new_with_range (0,1,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonDT5780Ch, xframe+240, yframe+80);
  GtkWidget *lComptonDT5780 = gtk_label_new("DT5780");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonDT5780, xframe+180, yframe+60);
  GtkWidget *lComptonDT5780Ch = gtk_label_new("Channel");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonDT5780Ch, xframe+180, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbComptonDT5780Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);


  sbComptonPlotScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonPlotScaler, xframe+240, yframe+25);
  gtk_spin_button_set_value((gpointer) sbComptonPlotScaler, 100);
  GtkWidget *lComptonPlotScaler = gtk_label_new("Plot Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonPlotScaler, xframe+160, yframe+30);
  gtk_signal_connect (GTK_OBJECT (sbComptonPlotScaler), "value_changed",G_CALLBACK (updatePlotScaler),NULL);


  /////////////////////////////////////////////////////////////////////
  // GCAL plotter frame
  /////////////////////////////////////////////////////////////////////
  xframe=975;
  yframe=160;
  GtkWidget *frameCaloPlotter = gtk_frame_new ("GCAL Plotter");
  gtk_fixed_put(GTK_FIXED(fixed), frameCaloPlotter, xframe,yframe);
  gtk_widget_set_size_request (frameCaloPlotter, 315, 140);
  cCaloPlotter = gtk_check_button_new_with_label("Plotter Enable");
  gtk_fixed_put(GTK_FIXED(fixed), cCaloPlotter, xframe+20, yframe+30);
  g_signal_connect(G_OBJECT(cCaloPlotter), "toggled", G_CALLBACK(checkCaloPlotter), NULL);

  sbCaloV1742Ch = gtk_spin_button_new_with_range (0,36,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbCaloV1742Ch, xframe+100, yframe+80);
  sbCaloV1742Gr = gtk_spin_button_new_with_range (0,3,1);
  gtk_signal_connect (GTK_OBJECT (sbCaloV1742Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  gtk_fixed_put(GTK_FIXED(fixed), sbCaloV1742Gr, xframe+100, yframe+110);
  GtkWidget *lCaloV1742Ch = gtk_label_new("V1742");
  gtk_fixed_put(GTK_FIXED(fixed), lCaloV1742Ch, xframe+20, yframe+60);
  gtk_signal_connect (GTK_OBJECT (sbCaloV1742Gr), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  rCaloV1742Ch = gtk_radio_button_new_with_label(NULL,"Channel");
  gtk_fixed_put(GTK_FIXED(fixed), rCaloV1742Ch, xframe+20, yframe+80);
  rCaloV1742Gr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rCaloV1742Ch),"Group");
  gtk_fixed_put(GTK_FIXED(fixed), rCaloV1742Gr, xframe+20, yframe+110);
  g_signal_connect (GTK_TOGGLE_BUTTON (rCaloV1742Ch), "toggled",G_CALLBACK (selectV1742PlotType), NULL);


  sbCaloPlotScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbCaloPlotScaler, xframe+240, yframe+25);
  gtk_spin_button_set_value((gpointer) sbCaloPlotScaler, 100);
  GtkWidget *lCaloPlotScaler = gtk_label_new("Plot Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lCaloPlotScaler, xframe+160, yframe+30);
  gtk_signal_connect (GTK_OBJECT (sbCaloPlotScaler), "value_changed", G_CALLBACK (updatePlotScaler),NULL);


  /////////////////////////////////////////////////////////////////////
  // Monitor frame
  /////////////////////////////////////////////////////////////////////
  xframe=650;
  yframe=320;
  GtkWidget *frameMonitor = gtk_frame_new ("Monitoring");
  gtk_fixed_put(GTK_FIXED(fixed), frameMonitor, xframe,yframe);
  gtk_widget_set_size_request (frameMonitor, 640, 170);
  
  tMonitor=gtk_table_new (5,2,TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), tMonitor, xframe+100,yframe+100);
  int et=0;
  for(et=0;et<5;et++) {
    eMonitorProd[et] = gtk_entry_new();
    gtk_widget_set_size_request(eMonitorProd[et], 100, 25);
    gtk_table_attach_defaults (GTK_TABLE (tMonitor), eMonitorProd[et], et, et+1, 0, 1);
    eMonitorCons[et] = gtk_entry_new();
    gtk_widget_set_size_request(eMonitorCons[et], 100, 25);
    gtk_table_attach_defaults (GTK_TABLE (tMonitor), eMonitorCons[et], et, et+1, 1, 2);
  }
  GtkWidget* lProd = gtk_label_new("Producer");
  gtk_fixed_put(GTK_FIXED(fixed), lProd, xframe+10, yframe+105);
  GtkWidget* lCons = gtk_label_new("Consumer");
  gtk_fixed_put(GTK_FIXED(fixed), lCons, xframe+10, yframe+125);
  GtkWidget* lBaFGamma = gtk_label_new("BaF Gamma");
  gtk_fixed_put(GTK_FIXED(fixed), lBaFGamma, xframe+100, yframe+80);
  GtkWidget* lBaFAlfa = gtk_label_new("BaF Alfa");
  gtk_fixed_put(GTK_FIXED(fixed), lBaFAlfa, xframe+200, yframe+80);
  GtkWidget* lHpGe = gtk_label_new("HpGe");
  gtk_fixed_put(GTK_FIXED(fixed), lHpGe, xframe+300, yframe+80);
  GtkWidget* lPamSi = gtk_label_new("PamSi");
  gtk_fixed_put(GTK_FIXED(fixed), lPamSi, xframe+400, yframe+80);
  GtkWidget* lCalo = gtk_label_new("Calo");
  gtk_fixed_put(GTK_FIXED(fixed), lCalo, xframe+500, yframe+80);

  cMonitor = gtk_check_button_new_with_label("Monitor Enable");
  gtk_fixed_put(GTK_FIXED(fixed), cMonitor, xframe+20, yframe+40);


  /////////////////////////////////////////////////////////////////////
  // Quit button
  /////////////////////////////////////////////////////////////////////
  bQuit = gtk_button_new_with_label("Quit");
  gtk_widget_set_size_request(bQuit, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, 1000, 750);
  g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(Quit), NULL);


  /////////////////////////////////////////////////////////////////////
  // INFN logo
  /////////////////////////////////////////////////////////////////////
  GtkWidget* image = gtk_image_new_from_file("Setup/logoINFN200x117.png");
  gtk_widget_set_size_request(image,145,80);
  gtk_fixed_put(GTK_FIXED(fixed), image, 1100, 20);



  return window; 
}



/*************************************************************************************************************/
void WriteLogHeader() {
/*************************************************************************************************************/
  sprintf(logFilename,"Log/LogFile_Run%i",rcDaqSharedMemory->runNumber);
  freopen (logFilename,"a",stdout);
  printf("\nRunControl: WriteLogHeader \nOpening %s  \n",logFilename); 
  
  struct timespec currTime;
  clock_gettime(CLOCK_REALTIME, &currTime);
  time_t mysec=currTime.tv_sec;
  printf("Date and Time: %s \n",ctime(&mysec));
  char* currentRecipe=getCurrentRecipe(cbRecipeList);
  if(strstr(currentRecipe, "Default")!=NULL)  {
    char str[100];
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", str);
    pclose(deffile);
    currentRecipe=str;
  }
  printf("Selected Recipe: %s \n",currentRecipe);
  printf("Device Selection for this run: ");
  if(rcDaqSharedMemory->IncludeBaF)   printf("BaF\n");
  if(rcDaqSharedMemory->IncludeHpGe)  printf("HpGe\n");
  if(rcDaqSharedMemory->IncludePamSi) printf("PamSi\n");
  if(rcDaqSharedMemory->IncludeGcal) printf("Gcal\n");
  //if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) printf("Calo\n");

  printf("Run Configuration: \n");
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) {
    printf("  Run Lenght %i s \n",gtk_spin_button_get_value_as_int((gpointer) sbTime));
  } else printf("  Run Lenght %i events \n",gtk_spin_button_get_value_as_int((gpointer) sbNEvents));
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSoftTrg))) 
    printf("  Software Trigger is ON (Rate %i Hz) \n",gtk_spin_button_get_value_as_int((gpointer) sbRate));
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHpGe))) 
    printf("  Pulser HpGe is ON (Rate %i Hz) \n",gtk_spin_button_get_value_as_int((gpointer) sbRate));

  fflush(stdout);
}


/*************************************************************************************************************/
void loadRecipeList() {
/*************************************************************************************************************/
  FillRecipeList(cbRecipeList,getCurrentRecipe(cbRecipeList),0);
}


/*************************************************************************************************************/
void changeRecipe() {
/*************************************************************************************************************/
  V1742Ready=0;
  V1742CaloReady=0;
  DT5780Ready=0;
  V1485Ready=0;

  gtk_widget_set_sensitive(cBaF, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cBaF), FALSE);
  gtk_widget_set_sensitive(cHpGe, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cHpGe), FALSE);
  gtk_widget_set_sensitive(cPamSi, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cPamSi), FALSE);
  gtk_widget_set_sensitive(cCalo, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cCalo), FALSE);
 
  gtk_widget_set_sensitive(bStartDaq, FALSE);
  gtk_widget_set_sensitive(bStopDaq, FALSE);

}

/*************************************************************************************************************/
void updateChannelToPlot() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rBaFV1742Ch))) 
    rcDaqSharedMemory->PlotChannelBaF = gtk_spin_button_get_value_as_int ((gpointer) sbComptonV1742Ch);
  else rcDaqSharedMemory->PlotChannelBaF = gtk_spin_button_get_value_as_int ((gpointer) sbComptonV1742Gr);

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rCaloV1742Ch))) 
    rcDaqSharedMemory->PlotChannelCalo = gtk_spin_button_get_value_as_int ((gpointer) sbCaloV1742Ch);
  else rcDaqSharedMemory->PlotChannelCalo = gtk_spin_button_get_value_as_int ((gpointer) sbCaloV1742Gr);

  rcDaqSharedMemory->PlotChannelHpGe = gtk_spin_button_get_value_as_int ((gpointer) sbComptonDT5780Ch);
}


/*************************************************************************************************************/
void updatePlotScaler() {
/*************************************************************************************************************/
  rcDaqSharedMemory->ComptonPlotScaler = gtk_spin_button_get_value_as_int ((gpointer) sbComptonPlotScaler);
  rcDaqSharedMemory->CaloPlotScaler = gtk_spin_button_get_value_as_int ((gpointer) sbCaloPlotScaler);
}




/*************************************************************************************************************/
void updateRate() {
/*************************************************************************************************************/
  rcDaqSharedMemory->SoftwareTrgRate = gtk_spin_button_get_value_as_int ((gpointer) sbRate);
}


/*************************************************************************************************************/
void updateRunNumber(int increment) {
/*************************************************************************************************************/
  char strRun[100];
  FILE* file = fopen(runNumberFile,"r");
  int nrun=0;
  fscanf(file, "%i", &nrun);
  if(increment) nrun++;
  rcDaqSharedMemory->runNumber=nrun;
  fclose(file);
  file = fopen(runNumberFile,"w");
  fprintf(file,"%i",nrun);
  sprintf(strRun,"%i",nrun);
  gtk_entry_set_text((gpointer) eRunNumber,(gpointer) strRun);
  fclose(file);
}


/*************************************************************************************************************/
void updateMonitor() {
/*************************************************************************************************************/
  int et=0;
  char strEvt[100];

    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[BaFGamma]);
    gtk_entry_set_text((gpointer) eMonitorProd[BaFGamma],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[BaFAlfa]);
    gtk_entry_set_text((gpointer) eMonitorProd[BaFAlfa],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[HpGe]);
    gtk_entry_set_text((gpointer) eMonitorProd[HpGe],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[PamSi]);
    gtk_entry_set_text((gpointer) eMonitorProd[PamSi],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[Calo]);
    gtk_entry_set_text((gpointer) eMonitorProd[Calo],(gpointer) strEvt);

    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[BaFGamma]);
    gtk_entry_set_text((gpointer) eMonitorCons[BaFGamma],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[BaFAlfa]);
    gtk_entry_set_text((gpointer) eMonitorCons[BaFAlfa],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[HpGe]);
    gtk_entry_set_text((gpointer) eMonitorCons[HpGe],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[PamSi]);
    gtk_entry_set_text((gpointer) eMonitorCons[PamSi],(gpointer) strEvt);
    sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[Calo]);
    gtk_entry_set_text((gpointer) eMonitorCons[Calo],(gpointer) strEvt);

}

/*************************************************************************************************************/
void checkIncludedDevice() {
/*************************************************************************************************************/
  int countDevice=0;
  if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cBaF)) || 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHpGe)) || 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cPamSi)))  countDevice++;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) countDevice++;

  if(countDevice>1) gtk_widget_set_sensitive(rNEvents, FALSE);
  else gtk_widget_set_sensitive(rNEvents, TRUE);

  if( !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cBaF)) && 
       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHpGe)) && 
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cPamSi)) && 
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo)) && V1485Ready) gtk_widget_set_sensitive(cTestHpGe, TRUE);
  else 
    gtk_widget_set_sensitive(cTestHpGe, FALSE);
}

/*************************************************************************************************************/
void checkRunNumber() {
/*************************************************************************************************************/
  int RunLenght,TimeBasedRunLength;
  double currentTime,startTime;
  int count;
  int delay=0;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) {
    RunLenght=gtk_spin_button_get_value_as_int((gpointer) sbTime);
    TimeBasedRunLength=1;
  } else if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rNEvents))) {
    RunLenght=gtk_spin_button_get_value_as_int((gpointer) sbNEvents);
    TimeBasedRunLength=0;
  }

  if(TimeBasedRunLength) startTime=getTime();

  while(running) {
    while (gtk_events_pending()) gtk_main_iteration();
    usleep(1000); delay++;

    if(delay%(2*SlowControlSleepTime*1000)==0) { //Check slow control system
      //if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) DecodeSlowControlStatus(Calo); 
      if(rcDaqSharedMemory->IncludeGcal) DecodeSlowControlStatus(Calo); 
      if(rcDaqSharedMemory->IncludeBaF) DecodeSlowControlStatus(BaF); 
      delay=0;
    }

    if(TimeBasedRunLength) {
      currentTime=getTime();
      count=currentTime-startTime;
    } 
    else if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) {
      count=rcDaqSharedMemory->EventsCons[Calo];
    } 
    else {
      count=rcDaqSharedMemory->EventsCons[BaFGamma]+rcDaqSharedMemory->EventsCons[BaFAlfa]+rcDaqSharedMemory->EventsCons[HpGe]+rcDaqSharedMemory->EventsCons[PamSi];
    }

    if(count >= RunLenght) {
      updateRunNumber(1);
      count=0;
      if(TimeBasedRunLength) startTime=getTime();
      for(int id=0;id<5;id++) rcDaqSharedMemory->EventsProd[id]=0;
      WriteLogHeader();
    }
      
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cMonitor))) updateMonitor(); 
  }
}


/*************************************************************************************************************/
void DecodeSlowControlStatus(Device_t detector) {
/*************************************************************************************************************/
  uint32_t u32;
  char name[10];

  if(detector==Calo) {
    sprintf(name,"%s","GCAL");
    u32=rcDaqSharedMemory->gcalStatus;
  }
  else if(detector==BaF) {    
    sprintf(name,"%s","BaF");
    u32=rcDaqSharedMemory->bafStatus;
  }

  GetTime(timeNow);
 
  if(! (u32 & (0x1<<0)) ) {
    sprintf(text,"%s ***WARNING: The %s slow control is not running\n",timeNow,name);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
    printf(text);
  }

  if(u32 & (0x1 << 1)) {
      sprintf(text,"%s ***WARNING: %s HV channel OFF\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }

  if(u32 & (0x1 << 2) || u32 & (0x1 << 3) || u32 & (0x1 << 4)) {
      sprintf(text,"%s ***WARNING: %s HV status alarm\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }  

  if(u32 & (0x1 << 17)) {
      sprintf(text,"%s ***WARNING: %s LV channel OFF\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }

  if(u32 & (0x1 << 18) || u32 & (0x1 << 19) || u32 & (0x1 << 20)) {
      sprintf(text,"%s ***WARNING: %s LV status alarm\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }  


}



/*************************************************************************************************************/
void CheckTestHpGe() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHpGe)))
    rcDaqSharedMemory->testHpGe=0;
  else
    rcDaqSharedMemory->testHpGe=1;
}



/*************************************************************************************************************/
void selectRunType() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) {
    //    rcDaqSharedMemory->RunTimeLength=gtk_spin_button_get_value_as_int((gpointer) sbTime);
    //    rcDaqSharedMemory->NumberOfEvents=0;
    gtk_widget_set_sensitive(sbTime, TRUE);
    gtk_widget_set_sensitive(sbNEvents, FALSE);
  } else {
    //    rcDaqSharedMemory->RunTimeLength=0;
    //    rcDaqSharedMemory->NumberOfEvents=gtk_spin_button_get_value_as_int((gpointer) sbNEvents);
    gtk_widget_set_sensitive(sbTime, FALSE);
    gtk_widget_set_sensitive(sbNEvents, TRUE);
  }
}


/*************************************************************************************************************/
void selectV1742PlotType() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rCaloV1742Ch))) 
    rcDaqSharedMemory->isCaloPlotGroup=0;
  else 
    rcDaqSharedMemory->isCaloPlotGroup=1;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rBaFV1742Ch))) 
    rcDaqSharedMemory->isBaFPlotGroup=0;
  else 
    rcDaqSharedMemory->isBaFPlotGroup=1;
  updateChannelToPlot();
}

/*************************************************************************************************************/
void startPlotter(char* device) {
/*************************************************************************************************************/
  printf("RunControl: starting %s Plotter for Run %i\n",device,rcDaqSharedMemory->runNumber);
  char command[200]="bin/Plotter -d ";
  strcat(command,device);
  strcat(command,"&");
  ret = system(command);

}

/*************************************************************************************************************/
void checkCaloPlotter() {
/*************************************************************************************************************/
  if(running) {
    // check Calo Plotter
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cCaloPlotter))) {
      rcDaqSharedMemory->CaloPlotScaler=gtk_spin_button_get_value_as_int((gpointer) sbCaloPlotScaler);
      rcDaqSharedMemory->stopCaloPlot=0;
      startPlotter("GCAL");
    }
    else {
      rcDaqSharedMemory->stopCaloPlot=1;
    }
    selectV1742PlotType();
  }
}

/*************************************************************************************************************/
void checkComptonPlotter() {
/*************************************************************************************************************/
  if(running) {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cComptonPlotter))) {
      rcDaqSharedMemory->ComptonPlotScaler=gtk_spin_button_get_value_as_int((gpointer) sbComptonPlotScaler);
      rcDaqSharedMemory->stopComptonPlot=0;
      startPlotter("CSPEC");
    }
    else {
      rcDaqSharedMemory->stopComptonPlot=1;
    }
    selectV1742PlotType();
  }
}


/*************************************************************************************************************/
void startConsumer(char* device) {
/*************************************************************************************************************/
  char filename[400];
  strcpy(filename,"");
  strcat(filename, (char*) gtk_entry_get_text((GtkEntry *) eFileName ));

  char strnrun[100];
  sprintf(strnrun,"Run%i",rcDaqSharedMemory->runNumber);
  strcat(filename,strnrun);

  strcat(filename,"_");
  strcat(filename,device);
  strcat(filename,"_");

  char* recipe;
  char defrecipe[100];
  recipe=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *) cbRecipeList );
  if(strstr(recipe, "Default")!=NULL) {
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", defrecipe);
    pclose(deffile);
    strcat(filename,defrecipe);
  } else {
    strcat(filename,recipe);
  }

  printf("RunControl: starting %s Consumer for Run %i\n",device,rcDaqSharedMemory->runNumber);
  fflush(stdout);
  char command[200]="bin/Consumer -f ";
  strcat(command,filename);
  strcat(command," -d ");
  strcat(command,device);
  strcat(command," &");
  ret = system(command);  
}



/*************************************************************************************************************/
void startProducer(char* device) {
/*************************************************************************************************************/
  printf("RunControl: starting %s Producer for Run %i\n",device,rcDaqSharedMemory->runNumber);
  fflush(stdout);
  char command[200]="bin/Producer -d ";
  strcat(command,device);
  strcat(command," &");
  ret = system(command);
}

/*************************************************************************************************************/
void ConfigDigi() { 
/*************************************************************************************************************/
  char* V812configfile=getConfigFile(cbRecipeList,Discr);
  if( access( V812configfile, F_OK ) != -1 ) { 
    ret = ProgramV812(V812configfile);   
    if(ret==0)  g_print("Discriminator V812 successfully configured \n");
    else g_print("***RunControl: ERROR configuring V812: %i \n",ret);
  }

  char* V1485configfile=getConfigFile(cbRecipeList,PamSi);
  if( access( V1485configfile, F_OK ) != -1 ) { 
    V1485Params_t Params;
    ParseConfigFileV1485(V1485configfile, &Params);
    rcDaqSharedMemory->connectionParamsV1485[0]=Params.LinkNum;
    rcDaqSharedMemory->connectionParamsV1485[1]=Params.BaseAddress;
    ret = TestConnection(&Params);
    if(ret==cvSuccess) {
      g_print("RunControl: IO board V1485 successfully configured \n");
      V1485Ready=1;
      gtk_widget_set_sensitive(cPamSi, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cPamSi), TRUE);
      gtk_widget_set_sensitive(bStartDaq, TRUE);
    } 
    else g_print("***RunControl: ERROR configuring V1485: %i \n",CAENVME_DecodeError(ret));
  }

  char* DT5780configfile=getConfigFile(cbRecipeList,HpGe);
  if( access( DT5780configfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerDT5780(connectionParams,DT5780configfile);
    if(ret==0) {
      g_print("RunControl: Digitizer DT5780 successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsDT5780[c]=connectionParams[c];
      DT5780Ready=1;
      gtk_widget_set_sensitive(cHpGe, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cHpGe), TRUE);
      //gtk_widget_set_sensitive(bStartDaq, TRUE);
      gtk_widget_set_sensitive(bStartSlowControl, TRUE);
    } 
    else g_print("***RunControl: ERROR configuring DT5780: %i \n",ret);
  }

  char* V1742BaFconfigfile=getConfigFile(cbRecipeList,BaF);
  if( access( V1742BaFconfigfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerV1742(connectionParams,V1742BaFconfigfile,0);
    if(ret==0) {
      g_print("RunControl: Digitizer V1742 (BaF) successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsV1742BaF[c]=connectionParams[c];
      V1742Ready=1;
      gtk_widget_set_sensitive(cBaF, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cBaF), TRUE);
      gtk_widget_set_sensitive(bStartDaq, TRUE);
    } 
    else g_print("***RunControl: ERROR configuring V1742 (BaF): %i \n",ret);
  }

  char* V1742Caloconfigfile=getConfigFile(cbRecipeList,Calo);
  if( access( V1742Caloconfigfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerV1742(connectionParams,V1742Caloconfigfile,1);
    if(ret==0) {
      g_print("RunControl: Digitizer V1742 (GCAL) successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsV1742Calo[c]=connectionParams[c];
      V1742CaloReady=1;
      gtk_widget_set_sensitive(cCalo, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cCalo), TRUE);
      //gtk_widget_set_sensitive(bStartDaq, TRUE);
      gtk_widget_set_sensitive(bStartSlowControl, TRUE);
    } 
    else g_print("***RunControl: ERROR configuring V1742 (GCAL): %i \n",ret);
  }

}

/*************************************************************************************************************/
void StartSlowControl() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cBaF))) rcDaqSharedMemory->IncludeBaF=TRUE;
  else rcDaqSharedMemory->IncludeBaF=FALSE;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHpGe))) rcDaqSharedMemory->IncludeHpGe=TRUE;
  else rcDaqSharedMemory->IncludeHpGe=FALSE;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cPamSi))) rcDaqSharedMemory->IncludePamSi=TRUE;
  else rcDaqSharedMemory->IncludePamSi=FALSE;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) rcDaqSharedMemory->IncludeGcal=TRUE;
  else rcDaqSharedMemory->IncludeGcal=FALSE;

  if(rcDaqSharedMemory->IncludeBaF) rcDaqSharedMemory->bafStatus=0;
  if(rcDaqSharedMemory->IncludeHpGe) rcDaqSharedMemory->hpgeStatus=0;
  if(rcDaqSharedMemory->IncludePamSi) rcDaqSharedMemory->pamsiStatus=0;
  if(rcDaqSharedMemory->IncludeGcal) rcDaqSharedMemory->gcalStatus=0;
  //if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cCalo))) rcDaqSharedMemory->gcalStatus=0;

  sprintf(text,"Starting the Slow Control for Run %i\n",rcDaqSharedMemory->runNumber+1);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);

  char command[100];
  sprintf(command,"bin/SlowControl &");
  ret = system(command);

  gtk_widget_set_sensitive(bStartDaq, TRUE);
  gtk_widget_set_sensitive(bStartSlowControl, FALSE);



/*
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHpGe))) {
    int handle;

    sprintf(text,"RunControl: starting HPGeMonitor for Run %i\n",rcDaqSharedMemory->runNumber);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);

    // Open connection with the DT5780 board
    ret = CAEN_DGTZ_OpenDigitizer(rcDaqSharedMemory->connectionParamsDT5780[0],rcDaqSharedMemory->connectionParamsDT5780[1],
                                  rcDaqSharedMemory->connectionParamsDT5780[2],rcDaqSharedMemory->connectionParamsDT5780[3], 
                                  &handle);
    GetTime(timeNow);
    if(!ret) {
     sprintf(text,"%s Connection opened with DT5780 board \n",timeNow);
     gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
     printf(text);
    }
    else {
     sprintf(text,"***HPGeMonitor: %s ERROR opening DT5780 board - code %u \n",timeNow,ret);
     gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
     printf(text);
     exit(1);
    }

    char command[100];
    sprintf(command," bin/HPGeMonitor &");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, command, -1, "lmarg", "red_bg", NULL);
    printf(command);
    ret = system(command);
    gtk_widget_set_sensitive(bStartDaq, TRUE);
    gtk_widget_set_sensitive(bStartSlowControl, FALSE);
  }

*/


}


/*************************************************************************************************************/
void StartDaq() {
/*************************************************************************************************************/
  int et=0;
  for(int et=0;et<5;et++) {
    rcDaqSharedMemory->EventsProd[et]=0;
    rcDaqSharedMemory->EventsCons[et]=0;
  }
  rcDaqSharedMemory->stopDAQ=0; 
  gtk_widget_set_sensitive(bStopDaq, TRUE);
  gtk_widget_set_sensitive(bStartDaq, FALSE);
  gtk_widget_set_sensitive(cBaF, FALSE);
  gtk_widget_set_sensitive(cHpGe, FALSE);
  gtk_widget_set_sensitive(cPamSi, FALSE);

  gtk_widget_set_sensitive(cSoftTrg,FALSE);
  gtk_widget_set_sensitive(cTestHpGe,FALSE);
  gtk_widget_set_sensitive(sbRate,FALSE);
  gtk_widget_set_sensitive(cConsumer, FALSE);
  gtk_widget_set_sensitive(bConfigure,FALSE);

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSoftTrg))) rcDaqSharedMemory->softwareTrigger=1; 
  else rcDaqSharedMemory->softwareTrigger=0;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHpGe))) rcDaqSharedMemory->testHpGe=1; 
  else rcDaqSharedMemory->testHpGe=0;


  rcDaqSharedMemory->AlfaScaler = gtk_spin_button_get_value_as_int ((gpointer) sbAlfaScaler);
  rcDaqSharedMemory->isLoggerBusy = 0;
  updateRunNumber(1);
  updateRate();
  WriteLogHeader();

  if(rcDaqSharedMemory->IncludeBaF || rcDaqSharedMemory->IncludeHpGe || rcDaqSharedMemory->IncludePamSi) {    
    startProducer("CSPEC");
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cConsumer))) startConsumer("CSPEC");
  }

  if(rcDaqSharedMemory->IncludeGcal) {
    startProducer("GCAL");
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cConsumer))) startConsumer("GCAL");
  } 

  running=1;
  checkCaloPlotter();
  checkComptonPlotter();
  checkRunNumber();
}


/*************************************************************************************************************/
void StopDaq() {
/*************************************************************************************************************/ 
  g_print("Acquisition Stopped \n");
  rcDaqSharedMemory->stopDAQ=1;
  rcDaqSharedMemory->stopComptonPlot=1;
  rcDaqSharedMemory->stopCaloPlot=1;

  running=0;
  gtk_widget_set_sensitive(bStopDaq, FALSE);
  gtk_widget_set_sensitive(bStartDaq, TRUE);
  if(V1742Ready) gtk_widget_set_sensitive(cBaF, TRUE);
  if(V1742CaloReady) gtk_widget_set_sensitive(cBaF, TRUE);
  if(DT5780Ready) gtk_widget_set_sensitive(cHpGe, TRUE);
  if(V1485Ready) gtk_widget_set_sensitive(cPamSi, TRUE);
  gtk_widget_set_sensitive(cSoftTrg,TRUE);
  gtk_widget_set_sensitive(cTestHpGe,TRUE);
  gtk_widget_set_sensitive(sbRate,TRUE);
  gtk_widget_set_sensitive(bConfigure,TRUE);
  gtk_widget_set_sensitive(cConsumer, TRUE);
  gtk_widget_set_sensitive(cComptonPlotter, TRUE);
  gtk_widget_set_sensitive(cCaloPlotter, TRUE);
}


/*************************************************************************************************************/
void Quit() {
/*************************************************************************************************************/
  deleteDaqSharedMemory();
  fclose (stdout);
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
