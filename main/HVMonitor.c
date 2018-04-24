#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>
#include <CAENHVWrapper.h>

#define nChannel 10
int ret;
int handle;
unsigned short ChList[nChannel], StatusList[2*nChannel];
int ParStatus[nChannel];
float ParVMon[nChannel],ParIMon[nChannel], ParV0Set[nChannel], ParI0Set[nChannel], ParPw[nChannel], ParRup[nChannel], ParRdwn[nChannel], ParTrip[nChannel], ParVal[nChannel];
int Power[nChannel];
GtkWidget *eVMon[nChannel], *eIMon[nChannel], *eV0Set[nChannel], *eI0Set[nChannel], *ePw[nChannel], *eStatus[nChannel],  *eRup[nChannel], *eRdwn[nChannel], *eTrip[nChannel], *eChCommand, *sbValueCommand ;  //entry text
GtkWidget *bQuit, *bStart, *bSend;
GtkWidget *cCommand;
char strEntry[10];
int stop=0;

void StopAndQuit(GtkWidget *button, gpointer   data) {
  stop=1;
  printf("stop %i \n",stop);
  CAENHV_DeinitSystem(handle);
  G_CALLBACK(gtk_main_quit);
}

void SendCommand(GtkWidget *button, gpointer   data) {
  //  ret=CAENHV_InitSystem(3,0,"172.16.3.164","admin","admin",&handle);
  //  printf("ret %i handle %i \n",ret,handle);
  //  sleep(2);
  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cCommand)->entry));

  float value =gtk_spin_button_get_value((gpointer) sbValueCommand);
  printf("Setting %s to value %i \n",strCommand,value);
  int isInt=0;
  int n=0;
  while(n<nChannel) {
    if(strcmp(strCommand,"POWER ON")==0) {Power[n]=1; isInt=1;}
    else if(strcmp(strCommand,"POWER OFF")==0) {Power[n]=0; isInt=1;}
    else ParVal[n]=value;
    n++;
  }
  printf("isInt %i \n",isInt);
  if(isInt==0) ret=CAENHV_SetChParam(handle,0,strCommand,nChannel,ChList,ParVal); 
  else ret=CAENHV_SetChParam(handle,0,"Pw",nChannel,ChList,Power);
  printf("ret %x handle %i \n",ret,handle);
  //  CAENHV_DeinitSystem(handle);
}

void StartMonitor() {

  ret=CAENHV_InitSystem(3,0,"172.16.3.164","admin","admin",&handle);
  printf("ret %x handle %i \n",ret,handle);
  sleep(2);
  ret=0;
  while(ret==0) {
    if(stop==1) break;
    while (gtk_events_pending()) gtk_main_iteration();
    usleep(100);
    ret|=CAENHV_GetChParam(handle,0,"VMon",nChannel,ChList,ParVMon);
    if(ret!=0)    printf("ret reading VMon %x handle %i \n", ret, handle);
    //  if(ret != 0 ) printf("CAENHV_GetChParam: %s (num. %d)\n\n", 
    //			 CAENHV_GetError(handle), ret);
    ret|=CAENHV_GetChParam(handle,0,"IMon",nChannel,ChList,ParIMon);
    if(ret!=0)   printf("ret reading IMon %x handle %i \n", ret,handle);
    ret|=CAENHV_GetChParam(handle,0,"V0Set",nChannel,ChList,ParV0Set);
    if(ret!=0)    printf("ret reading V0Set %x handle %i \n", ret, handle);
    ret|=CAENHV_GetChParam(handle,0,"I0Set",nChannel,ChList,ParI0Set);
    if(ret!=0) printf("ret reading I0Set %x handle %i \n", ret, handle);
    ret=CAENHV_GetChParam(handle,0,"RUp",nChannel,ChList,ParRup);
    if(ret!=0)  printf("ret reading Rup %x handle %i \n", ret, handle);
    ret=CAENHV_GetChParam(handle,0,"RDWn",nChannel,ChList,ParRdwn);
    if(ret!=0)  printf("ret reading RDWn %x handle %i \n", ret, handle);
    ret=CAENHV_GetChParam(handle,0,"Trip",nChannel,ChList,ParTrip);
    if(ret!=0)  printf("ret reading Trip %x handle %i\n", ret, handle);

  //  sleep(2);
    ret=CAENHV_GetChParam(handle,0,"Status",nChannel,ChList,ParStatus);
     if(ret!=0) printf("ret reading Status %x handle %i \n", ret, handle);
    //    ret=1;

    int n=0;
    while(n<nChannel) {
      //    printf("channel %i VMon %f \n",n,ParVMon[n]);
      sprintf(strEntry,"%4.2f",ParVMon[n]);
      gtk_entry_set_text((gpointer) eVMon[n],(gpointer) strEntry);
    
    //    printf("channel %i IMon %f \n",n,ParIMon[n]);
      sprintf(strEntry,"%4.3f",ParIMon[n]);
      gtk_entry_set_text((gpointer) eIMon[n],(gpointer) strEntry);

    //    printf("channel %i V0Set %f \n",n,ParV0Set[n]);
      sprintf(strEntry,"%4.2f",ParV0Set[n]);
      gtk_entry_set_text((gpointer) eV0Set[n],(gpointer) strEntry);

    //    printf("channel %i I0Set %f \n",n,ParI0Set[n]);
      sprintf(strEntry,"%4.2f",ParI0Set[n]);
      gtk_entry_set_text((gpointer) eI0Set[n],(gpointer) strEntry);

     sprintf(strEntry,"%3.0f",ParRup[n]);
     gtk_entry_set_text((gpointer) eRup[n],(gpointer) strEntry);

    sprintf(strEntry,"%3.0f",ParRdwn[n]);
     gtk_entry_set_text((gpointer) eRdwn[n],(gpointer) strEntry);

    sprintf(strEntry,"%3.1f",ParTrip[n]);
     gtk_entry_set_text((gpointer) eTrip[n],(gpointer) strEntry);

     if ((ParStatus[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) ePw[n],(gpointer) "ON");
      else gtk_entry_set_text((gpointer) ePw[n],(gpointer) "OFF");
     if ((ParStatus[n] >> 9) & 0x1)  gtk_entry_set_text((gpointer) eStatus[n],(gpointer) "Trip");
     else  if ((ParStatus[n] >> 4) & 0x1)  gtk_entry_set_text((gpointer) eStatus[n],(gpointer) "OVER VOLTAGE");
     else if ((ParStatus[n] >> 1) & 0x1)  gtk_entry_set_text((gpointer) eStatus[n],(gpointer) "RAMP UP");
else if ((ParStatus[n] >> 2) & 0x1)  gtk_entry_set_text((gpointer) eStatus[n],(gpointer) "RAMP DOWN");
 else gtk_entry_set_text((gpointer) eStatus[n],(gpointer) "");
      n++;
    }
  }
}

int main(int argc, char *argv[]) {

  GtkWidget *window;

  GtkWidget *fixed;
  

  GtkWidget *lVMon, *lIMon, *lV0Set, *lI0Set, *lPw, *lStatus, *lRup, *lRdwn, *lTrip, *lChannel[nChannel]; // labels


  int xsize=100;
  int ysize=30;
  int xspace=10;
  int yspace=10;
  int yoffset=50;
 

  int n=0;
  while(n<nChannel) {
    ChList[n]=n;
    n++;
  }
  n=0;
  while(n<2*nChannel) {
    StatusList[n]=n;
    n++;
  }

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "HV MONITOR");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 600);
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);

  n=0;
 while(n<nChannel) {
  eVMon[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eVMon[n], xsize+xspace, yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eVMon[n], xsize, ysize);
  eIMon[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eIMon[n], 2*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eIMon[n], xsize, ysize);
  eV0Set[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eV0Set[n], 3*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eV0Set[n], xsize, ysize);
  eI0Set[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eI0Set[n], 4*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eI0Set[n], xsize, ysize);
  ePw[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), ePw[n], 5*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(ePw[n], xsize, ysize);
  eStatus[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eStatus[n], 6*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eStatus[n], xsize, ysize);

  eRup[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eRup[n], 7*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eRup[n], xsize, ysize);
 eRdwn[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eRdwn[n], 8*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eRdwn[n], xsize, ysize);

  eTrip[n] = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eTrip[n], 9*(xsize+xspace), yoffset+(ysize+yspace)*n);
  gtk_widget_set_size_request(eTrip[n], xsize, ysize);
  char strChan[2];
  sprintf(strChan,"%i",n);
  char labelChan[10];
  strcpy(labelChan,"CHANNEL ");
  strcat(labelChan,strChan);
  lChannel[n]=gtk_label_new(labelChan);
  gtk_fixed_put(GTK_FIXED(fixed), lChannel[n], 10, yoffset+5+(ysize+yspace)*n);
  n++;
 }

 int lyoff=20;
 int lxoff=110;
 lVMon =  gtk_label_new("VMon");
 gtk_fixed_put(GTK_FIXED(fixed), lVMon, lxoff, yoffset-lyoff);

 lIMon =  gtk_label_new("IMon");
 gtk_fixed_put(GTK_FIXED(fixed), lIMon, 2*lxoff, yoffset-lyoff);

 lV0Set =  gtk_label_new("V0Set");
 gtk_fixed_put(GTK_FIXED(fixed), lV0Set, 3*lxoff, yoffset-lyoff);

 lI0Set =  gtk_label_new("I0Set");
 gtk_fixed_put(GTK_FIXED(fixed), lI0Set, 4*lxoff, yoffset-lyoff);

 lPw =  gtk_label_new("Pw");
 gtk_fixed_put(GTK_FIXED(fixed), lPw, 5*lxoff, yoffset-lyoff);

 lStatus =  gtk_label_new("Status");
 gtk_fixed_put(GTK_FIXED(fixed), lStatus, 6*lxoff, yoffset-lyoff);

 lRup =  gtk_label_new("Rup");
 gtk_fixed_put(GTK_FIXED(fixed), lRup, 7*lxoff, yoffset-lyoff);

 lRdwn =  gtk_label_new("Rdwn");
 gtk_fixed_put(GTK_FIXED(fixed), lRdwn, 8*lxoff, yoffset-lyoff);

 lTrip =  gtk_label_new("Trip Time");
 gtk_fixed_put(GTK_FIXED(fixed), lTrip, 9*lxoff, yoffset-lyoff);

  bQuit=gtk_button_new_with_label("Stop Monitoring");
  gtk_widget_set_size_request(bQuit, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, 950, 520 );

  bStart=gtk_button_new_with_label("Start Monitoring");
  gtk_widget_set_size_request(bStart, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixed), bStart, 750, 520 );

  GList *cbitems = NULL;
  cbitems = g_list_append (cbitems, "V0Set");
  cbitems = g_list_append (cbitems, "I0Set");
  cbitems = g_list_append (cbitems, "RUp");
  cbitems = g_list_append (cbitems, "RDWn");
  cbitems = g_list_append (cbitems, "Trip");
  cbitems = g_list_append (cbitems, "POWER ON");
  cbitems = g_list_append (cbitems, "POWER OFF");
  cCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(cCommand), cbitems);
  gtk_fixed_put(GTK_FIXED(fixed), cCommand,200 , 520);

  bSend=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSend, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixed), bSend, 500, 520 );

  GtkAdjustment *adjustment = (GtkAdjustment *) gtk_adjustment_new (100.0, 0.0, 1000.0, 1000.0, 5.0, 0.0);
  sbValueCommand = gtk_spin_button_new (adjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixed), sbValueCommand, 400, 520);
  //  lNEvents = gtk_label_new("N Events");
  //  gtk_fixed_put(GTK_FIXED(fixed), lNEvents, 20, 205);

  gtk_widget_show_all(window);
 // updateMonitor();
  // sleep(3);

 g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);  
 g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(StopAndQuit), NULL);
 g_signal_connect(G_OBJECT(bStart), "clicked", G_CALLBACK(StartMonitor), NULL);
 g_signal_connect(G_OBJECT(bSend), "clicked", G_CALLBACK(SendCommand), NULL);
 gtk_main();


 return 0;
}
