//gcc `pkg-config --libs --cflags gtk+-2.0` -fPIC -DLINUX -Wall src/AdvancedSetup.c src/V812Configurator.o src/DT5780Configurator.o src/UtilsFunctions.o -o bin/AdvancedSetup -lCAENVME -lCAENDigitizer -lCAENDPPLib -lrt 

#include <gtk/gtk.h>
#include "../include/V812Configurator.h"
#include "../include/DT5780Configurator.h"
#include "../include/V1742Configurator.h"
#include "../include/V1485Configurator.h"
#include "../include/AdvancedSetup.h"
#include "../include/X742CorrectionRoutines.h"

#include <math.h>
#include <string.h>

int err_code;
int xSizeTabs=800;
int ySizeTabs=830;
int borders=20;
int nitems=0;
GtkWidget *cbRecipes; //combo box
GtkWidget *fixed,*fixedV812,*fixedDT5780,*fixedV1742[MaxV1742],*fixedV1485;
GtkWidget *cDiscr, *cConfigBaF, *cConfigHpGe, *cConfigPamSi, *cConfigCalo;  // check buttons
V812Widget_t V812Widgets;
DT5780Widget_t HpGeWidgets;
V1742Widget_t BaFWidgets;
V1485Widget_t V1485Widgets;
V1742Widget_t CaloWidgets;
GtkWidget *window;
int isNewRecipe;
GtkWidget *notebook;
int nc, ng, ind;
char configfile[200];
GtkWidget* eOffset, *eThreshold, *sbThreshold,*cbTrCopy;
int isOpen=0;
int Login=0;

void copyTriggerSetup() {
  int page=gtk_notebook_get_current_page ((GtkNotebook *) notebook);
  int idigi;
  if(page==2) idigi=0;
  else idigi=1;
  int thr = atoi(gtk_entry_get_text((gpointer) eThreshold));
  int dco= atoi(gtk_entry_get_text((gpointer) eOffset));
  int itr = gtk_combo_box_get_active(GTK_COMBO_BOX(cbTrCopy));
  gtk_spin_button_set_value((gpointer)BaFWidgets.sbV1742FastTriggerOffset[itr][idigi],dco);
  gtk_spin_button_set_value((gpointer)BaFWidgets.sbV1742FastTriggerThreshold[itr][idigi],thr);
}
void updateThresholdmV() {
  float trvaluemv=gtk_spin_button_get_value((GtkSpinButton*) sbThreshold);
  int trvalue = round(26214.+13.2*trvaluemv);
  char strvalue[10];
  sprintf(strvalue,"%i",trvalue);
  gtk_entry_set_text((gpointer) eOffset,"32768");
  gtk_entry_set_text((gpointer) eThreshold,strvalue);  
}
void updateTrHelp(GtkWidget* combo) {
    char* currentText=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *) combo );
    if(strcmp(currentText,"mV")==0) {
      gtk_widget_set_sensitive(sbThreshold, TRUE);
      updateThresholdmV();
    } else {
      gtk_widget_set_sensitive(sbThreshold, FALSE);
      if(strcmp(currentText,"TTL")==0) {
	gtk_entry_set_text((gpointer) eOffset,"43008");
	gtk_entry_set_text((gpointer) eThreshold,"26214");
      } else if(strcmp(currentText,"NIM")==0) {
	gtk_entry_set_text((gpointer) eOffset,"32768");
	gtk_entry_set_text((gpointer) eThreshold,"20934");
      }

    }
}
void OpenTriggerHelp() {

  GtkWidget *wTrHelp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(wTrHelp), "Trigger Setup");
  gtk_window_set_default_size(GTK_WINDOW(wTrHelp), 300, 300);
  gtk_container_set_border_width(GTK_CONTAINER(wTrHelp), 10);
  gtk_window_set_position(GTK_WINDOW(wTrHelp), GTK_WIN_POS_CENTER);
  GtkWidget *fixedTrHelp = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(wTrHelp), fixedTrHelp);

  GtkWidget* cbTrHelp =gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), cbTrHelp,40 , 40);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) cbTrHelp,"NIM");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) cbTrHelp,"TTL");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) cbTrHelp,"mV");

  gtk_combo_box_set_active(GTK_COMBO_BOX(cbTrHelp), 0);
  eOffset = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), eOffset, 120, 100);
  gtk_widget_set_size_request(eOffset, 80, 30);
  gtk_editable_set_editable ((GtkEditable *) eOffset,0);
  GtkWidget* lOffset = gtk_label_new("DC Offset");
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), lOffset, 40, 105);

  eThreshold = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), eThreshold, 120, 140);
  gtk_widget_set_size_request(eThreshold, 80, 30);
  gtk_editable_set_editable ((GtkEditable *) eThreshold,0);
  GtkWidget* lThreshold = gtk_label_new("Threshold");
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), lThreshold, 40, 145);

  sbThreshold = gtk_spin_button_new_with_range(-1000,0, 10);
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), sbThreshold, 160, 40);
  gtk_spin_button_set_value((gpointer) sbThreshold, -400);
  g_signal_connect(G_OBJECT(sbThreshold), "value_changed", G_CALLBACK(updateThresholdmV), NULL);
  GtkWidget *lThresholdmV = gtk_label_new("Threshold (mV)");
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), lThresholdmV, 160, 20);
  
  cbTrCopy =gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), cbTrCopy,120 , 200);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) cbTrCopy,"TR0");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) cbTrCopy,"TR1");
  gtk_combo_box_set_active(GTK_COMBO_BOX(cbTrCopy), 0);
  GtkWidget* bTrCopy = gtk_button_new_with_label("Copy to");
  gtk_fixed_put(GTK_FIXED(fixedTrHelp), bTrCopy, 40, 200); 

  updateTrHelp(cbTrHelp);
  g_signal_connect(G_OBJECT(cbTrHelp), "changed", G_CALLBACK(updateTrHelp), NULL);
  g_signal_connect(G_OBJECT(bTrCopy), "clicked", G_CALLBACK(copyTriggerSetup), NULL);
  gtk_widget_show_all(wTrHelp);
}

void OpenLogin() {
  Login=0;
  GtkWidget* dLogin=gtk_dialog_new_with_buttons("Login",NULL,
						GTK_DIALOG_MODAL,
						GTK_STOCK_CANCEL, 
						GTK_RESPONSE_CANCEL,
						GTK_STOCK_OK, 
						GTK_RESPONSE_ACCEPT,
						NULL);
  GtkWidget* ePassword = gtk_entry_new();
  gtk_entry_set_visibility( (GtkEntry *) ePassword, FALSE );
  gtk_widget_set_size_request(ePassword, 300, 30);
  gtk_container_add((GtkContainer *) gtk_dialog_get_content_area((GtkDialog*)dLogin), ePassword);
  gtk_widget_show (dLogin);
  gtk_widget_show (ePassword);
  if (gtk_dialog_run (GTK_DIALOG (dLogin)) == GTK_RESPONSE_ACCEPT)
    { 
      if(strcmp((char*) gtk_entry_get_text((GtkEntry *) ePassword),AdvSetupPasswd)==0)
	Login=1;
      else {
	GtkWidget* Message = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
"WRONG PASSWORD");
	if(gtk_dialog_run(GTK_DIALOG(Message))==GTK_RESPONSE_OK) 
	  gtk_widget_destroy(Message);
	Login=0;

      }
    } else {
    Login=0;
  } 
  //  return ret;
  gtk_widget_destroy (dLogin);      

}

void UpdateAddress(GtkWidget* spin) {
  char saddr[4];

  int addr = gtk_spin_button_get_value_as_int((gpointer) spin);
  sprintf(saddr,"0x%X",addr);
  gtk_entry_set_text((gpointer) spin,saddr);

}

char* getConfigFile(GtkWidget* combo, Device_t device) {
  strcpy(configfile,"");
  strcpy(configfile,"Setup/");
  strcat(configfile,getCurrentRecipe(combo));
  strcat(configfile,"/");
  if(device==BaF) 
    strcat(configfile,configBaF);
  else if(device==HpGe) 
    strcat(configfile,configHpGe);
  else if(device==Discr) 
    strcat(configfile,configDiscr);
  else if(device==PamSi) 
    strcat(configfile,configPamSi);
  else if(device==Calo) 
    strcat(configfile,configCalo);
  return configfile;
}

char* getCurrentRecipe(GtkWidget* combo) {
  char* currentRecipe=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *) combo);
  if(strstr(currentRecipe, "Default")!=NULL) currentRecipe="Default";
  /*
  if(strstr(currentRecipe, "Default")!=NULL)  {
    char str[100];
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", str);
    pclose(deffile);
    currentRecipe=str;
  }
  */
  return currentRecipe;
}


void DownloadCalibrationTables() {
  char stringa[1000];
  bzero(stringa,1000);
  int handle;
  int page=gtk_notebook_get_current_page ((GtkNotebook *) notebook);
  int idigi;
  if(page==2) idigi=0;
  else idigi=1;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
  int LinkNum = gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742LinkNum[idigi]);
  err_code = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, LinkNum, 0,0, &handle);
  if(err_code==0) {
    int f;
    for(f=0;f<3;f++) {
      if(idigi==0) sprintf(stringa, "%s_%s%d", "./CorrectionTables/BaF/Corrections","freq",f);
      else sprintf(stringa, "%s_%s%d", "./CorrectionTables/Calo/Corrections","freq",f);
      err_code = CAEN_DGTZ_EnableDRS4Correction(handle);
      err_code = CAEN_DGTZ_GetCorrectionTables(handle, f, (void*)X742Tables);
      SaveCorrectionTables(stringa, 15, X742Tables);
    }
  } else {
    printf("Open Digitizer: %s \n",CAENVME_DecodeError(err_code));
  }
  err_code |= CAEN_DGTZ_CloseDigitizer(handle);
}

void showDiscrFrame() {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cDiscr))) {
    gtk_widget_show(fixedV812);
    gtk_notebook_set_current_page((GtkNotebook *)notebook,0); 
    //    UpdateAddress();
  } else gtk_widget_hide(fixedV812);
}

void showHpGeFrame() {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigHpGe))) {
    gtk_widget_show(fixedDT5780);
    gtk_notebook_set_current_page((GtkNotebook *)notebook,1);
  } else gtk_widget_hide(fixedDT5780);
}

void showBaFFrame() {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigBaF))) {
    gtk_widget_show(fixedV1742[0]);
    gtk_notebook_set_current_page((GtkNotebook *)notebook,2);
  } else gtk_widget_hide(fixedV1742[0]);
}

void showPamSiFrame() {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigPamSi))) {
    gtk_widget_show(fixedV1485);
    gtk_notebook_set_current_page((GtkNotebook *)notebook,3);
    //    UpdateAddress();
  } else {
    gtk_widget_hide(fixedV1485);
  }
}

void showCaloFrame() {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigCalo))) {
    gtk_widget_show(fixedV1742[1]);
    gtk_notebook_set_current_page((GtkNotebook *)notebook,4);
  } else {
    gtk_widget_hide(fixedV1742[1]);
  }
}


void CloseAdvancedSetup() {
  isOpen=0;
  gtk_widget_destroy(GTK_WIDGET(window));
}
void LoadV812Params(V812Params_t *Params) {
 
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cDiscr),1);
  //  gtk_spin_button_set_value_as_int((gpointer) cDiscr,1);
  gtk_spin_button_set_value((gpointer) V812Widgets.sbV812LinkNum, Params->LinkNum  );
  gtk_spin_button_set_value((gpointer) V812Widgets.sbV812BaseAddress, Params->BaseAddress);
  //  UpdateAddress();
  gtk_spin_button_set_value((gpointer) V812Widgets.sbV812Majority, Params->Majority );
  for(ng=0; ng< MaxV812NGroups; ng++) {
    gtk_spin_button_set_value((gpointer) V812Widgets.sbV812OutputWidth[ng],Params->OutputWidth[ng]  );
    gtk_spin_button_set_value((gpointer) V812Widgets.sbV812DeadTime[ng],Params->DeadTime[ng] );
      
    for(nc=0; nc<MaxV812NChannels/MaxV812NGroups; nc++) {
      ind=nc+ng*(MaxV812NChannels/MaxV812NGroups);
      gtk_spin_button_set_value((gpointer) V812Widgets.sbV812Threshold[ind], Params->Threshold[ind]);
     
      if((Params->InhibitPattern%2)==0) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (V812Widgets.cV812Channel[ind]),0);
      } else {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (V812Widgets.cV812Channel[ind]),1);
      }
      Params->InhibitPattern=Params->InhibitPattern/2;		
    }	
    
  }
}

void LoadV1485Params(V1485Params_t *Params) {
 
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigPamSi),1);

  gtk_spin_button_set_value((gpointer) V1485Widgets.sbV1485LinkNum, Params->LinkNum  );
  gtk_spin_button_set_value((gpointer) V1485Widgets.sbV1485BaseAddress, Params->BaseAddress);
  //  UpdateAddress();
}

void LoadDT5780Params(DT5780Params_t *Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams) {
 
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigHpGe),1);
  
  gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780LinkNum, Params->LinkNum);
  gtk_combo_box_set_active ((GtkComboBox *)  HpGeWidgets.cbDT5780LinkType, Params->LinkType);
  gtk_combo_box_set_active ((GtkComboBox *)  HpGeWidgets.cbDT5780AcquisitionMode, Params->AcqMode);
  gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780RecordLength,Params->RecordLength);
  gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780PreTriggerSize,Params->PreTriggerSize);
  gtk_combo_box_set_active ((GtkComboBox *)  HpGeWidgets.cbDT5780IOLevel, Params->IOLevel);
  if(Params->ExtTrigger==0) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (HpGeWidgets.cDT5780ExternalTrigger),1);
  if(Params->TriggerOut==0x103)  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelTriggerMode), 0);
  else gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelTriggerMode), 1);
  if((Params->ChannelMask >> 0) & 0x1) gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelEnable[0]), 0);
  else gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelEnable[0]), 1);
  if((Params->ChannelMask >> 1) & 0x1) gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelEnable[1]), 0);
  else gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelEnable[1]), 1);
 
  int ch;
  for(ch=0;ch<MaxDT5780NChannels;ch++) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780Polarity[ch]),Params->PulsePolarity[ch] );
    if(Params->SelfTrigger[ch]==0) gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780SelfTrigger[ch]),0 );
    else  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780SelfTrigger[ch]),1 );
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780TriggerMode[ch]),Params->TrgMode[ch] );
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780InputRange[ch]),Params->InputRange[ch] );
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780DCOffset[ch],Params->DCOffset[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780TriggerThreshold[ch],DPPParams->thr[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780TrapezoidRiseTime[ch],DPPParams->k[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780TrapezoidFlatTop[ch],DPPParams->m[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780DecayTime[ch],DPPParams->M[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780PeakingTime[ch],DPPParams->ftd[ch]);

    int list[6]={1,2,4,8,16,32};
    int ns;
    for(int ns=0;ns<6;ns++) if(DPPParams->a[ch]==list[ns]) gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780TriggerSmoothing[ch]), list[ns]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780InputRiseTime[ch],DPPParams->b[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780TriggerHoldOff[ch],DPPParams->trgho[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780PeakHoldOff[ch],DPPParams->pkho[ch]);
    gtk_spin_button_set_value((gpointer) HpGeWidgets.sbDT5780BaselineHoldOff[ch],DPPParams->blho[ch]);
  }
				
}
void LoadV1742Params(V1742Params_t *Params, int idigi) {
  if(idigi==0) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigBaF),1);
  else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigCalo),1);
  gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742LinkNum[idigi], Params->LinkNum);
  gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742PostTrigger[idigi], Params->PostTrigger );
  gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742Frequency[idigi]),Params->DRS4Frequency);
  gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742FastTrgDigitizing[idigi]),Params->FastTriggerEnabled);
gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742TriggerEdge[idigi]),Params->TriggerEdge);
  for(ng=0; ng< MaxV1742NGroups; ng++) {
    if(ng<2) {
      gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742FastTriggerOffset[ng][idigi],Params->FastTriggerDCOffset[ng*2]  );
      gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742FastTriggerThreshold[ng][idigi],Params->FastTriggerThreshold[ng*2] );
    }
    if((Params->GroupEnableMask%2)==0) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (BaFWidgets.cV1742GroupEnable[ng][idigi]),0);
      } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (BaFWidgets.cV1742GroupEnable[ng][idigi]),1);
      }
      Params->GroupEnableMask=Params->GroupEnableMask/2;		
  }
  for(nc=0; nc<MaxV1742NChannels; nc++) {
    gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742DCOffset[nc][idigi],Params->DCOffset[nc]);
    //    gtk_spin_button_set_value((gpointer) BaFWidgets.sbV1742DCOffset[nc], ((100.*Params->DCOffset[nc]/65535.)-50.));	
    //   printf("ch %i param %i val %f \n", nc,Params->DCOffset[nc],((100.*Params->DCOffset[nc]/65535.)-50.));
  }
} 

void LoadRecipeParams() {

  if(!isNewRecipe) {

    char filename[200]="Setup/";
    strcat(filename,getCurrentRecipe(cbRecipes));

    char* V812configfile=getConfigFile(cbRecipes,Discr);
    if( access( V812configfile, F_OK ) != -1 ) {
      V812Params_t Params;
      memset(&Params, 0, sizeof(V812Params_t));
      ParseConfigFileV812(V812configfile,&Params);
      LoadV812Params(&Params);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cDiscr),0);
    
    }

    char* DT5780configfile=getConfigFile(cbRecipes,HpGe);
    if( access( DT5780configfile, F_OK ) != -1 ) {
      DT5780Params_t Params;
      CAEN_DGTZ_DPP_PHA_Params_t DPPParams; 
      memset(&Params, 0, sizeof(DT5780Params_t));
      memset(&DPPParams, 0, sizeof(CAEN_DGTZ_DPP_PHA_Params_t));
      ParseConfigFileDT5780(DT5780configfile,&Params,&DPPParams);
      LoadDT5780Params(&Params,&DPPParams);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigHpGe),0);
    
    }

    char* V1485configfile=getConfigFile(cbRecipes,PamSi);

    if( access( V1485configfile, F_OK ) != -1 ) {
      V1485Params_t Params;
      memset(&Params, 0, sizeof(V1485Params_t));
      ParseConfigFileV1485(V1485configfile,&Params);
      LoadV1485Params(&Params);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigPamSi),0);
    
    }

    char* V1742BaFconfigfile=getConfigFile(cbRecipes,BaF);
    if( access( V1742BaFconfigfile, F_OK ) != -1 ) {

      V1742Params_t Params;
      memset(&Params, 0, sizeof(V1742Params_t));
      ParseConfigFileV1742(V1742BaFconfigfile,&Params);
      LoadV1742Params(&Params,0);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigBaF),0);     
    }

    char* V1742Caloconfigfile=getConfigFile(cbRecipes,Calo);
    if( access( V1742Caloconfigfile, F_OK ) != -1 ) {

      V1742Params_t Params;
      memset(&Params, 0, sizeof(V1742Params_t));
      ParseConfigFileV1742(V1742Caloconfigfile,&Params);
      LoadV1742Params(&Params,1);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConfigCalo),0);     
    }


  }
}

void ReadV812Params(V812Params_t* Params) {

  Params->LinkNum = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812LinkNum );
  Params->BaseAddress = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812BaseAddress );

  Params->Majority = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812Majority);
  Params->InhibitPattern = 0;
  for(ng=0; ng< MaxV812NGroups; ng++) {
    Params->OutputWidth[ng] = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812OutputWidth[ng]  );
    Params->DeadTime[ng] = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812DeadTime[ng] );

    for(nc=0; nc<MaxV812NChannels/MaxV812NGroups; nc++) {
      ind=nc+ng*(MaxV812NChannels/MaxV812NGroups);
      Params->Threshold[ind] = gtk_spin_button_get_value_as_int((gpointer) V812Widgets.sbV812Threshold[ind]);
      Params->InhibitPattern += (pow(2,ind))*gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (V812Widgets.cV812Channel[ind]));
   }

  }
}

void ReadDT5780Params(DT5780Params_t* Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams) {
  char* read;
  Params->LinkNum = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780LinkNum );
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780LinkType);
  if(strcmp(read, "Optical")==0) Params->LinkType=1;
  else if(strcmp(read, "USB")==0) Params->LinkType=0;
  else printf("Link Type %s: invalid setup", read);
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780AcquisitionMode);
  if(strcmp(read, "OSCILLOSCOPE")==0) Params->AcqMode=0;
  else if(strcmp(read, "LIST")==0) Params->AcqMode=1;
  else if(strcmp(read, "MIXED")==0) Params->AcqMode=2;
  else printf("Acquisition Mode %s: invalid setup",read);
  Params->RecordLength = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780RecordLength);
  Params->PreTriggerSize = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780PreTriggerSize);
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780IOLevel);
  if(strcmp(read, "NIM")==0) Params->IOLevel=0;
  else if(strcmp(read, "TTL")==0) Params->IOLevel=1;
  else printf("IO Level %s: invalid setup",read);
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (HpGeWidgets.cDT5780ExternalTrigger))==1) Params->ExtTrigger=0;
  else Params->ExtTrigger=1;
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780ChannelTriggerMode);
  if(strcmp(read, "AND")==0) Params->TriggerOut=0x103;
  else if(strcmp(read, "OR")==0) Params->TriggerOut=0x3;
  else printf("Channel Trigger Mode %s: invalid setup",read);
  Params->ChannelMask=0;
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780ChannelEnable[0]);
  if(strcmp(read, "ENABLE")==0) Params->ChannelMask=1;
  read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780ChannelEnable[1]);
  if(strcmp(read, "ENABLE")==0) Params->ChannelMask +=2;
  int ch;
  for(ch=0;ch<MaxDT5780NChannels;ch++) {

    read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780Polarity[ch]);
    if(strcmp(read, "POSITIVE")==0) Params->PulsePolarity[ch] = 1;
    else if(strcmp(read, "NEGATIVE")==0) Params->PulsePolarity[ch]=0;
    else printf("Pulse Polarity %s: invalid setup", read);
    read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780SelfTrigger[ch]);
    if(strcmp(read, "ENABLE")==0) Params->SelfTrigger[ch] = 0x0000000;
    else if(strcmp(read, "DISABLE")==0) Params->SelfTrigger[ch] = 0x1000000;
    else printf("Channel Enable %s: invalid setup", read);
    read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780TriggerMode[ch]);
    if(strcmp(read, "INDIVIDUAL")==0) Params->TrgMode[ch] = 0;
    else if(strcmp(read, "COINCIDENCE")==0) Params->TrgMode[ch] = 1;
    else if(strcmp(read, "ANTICOINCIDENCE")==0) Params->TrgMode[ch] = 2;
    else printf("Trigger Mode %s: invalid setup", read);
    read=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)  HpGeWidgets.cbDT5780InputRange[ch]);
    if(strcmp(read, "0.6 V")==0) Params->InputRange[ch] = 0;
    else if(strcmp(read, "1.4 V")==0) Params->InputRange[ch] = 1;
    else if(strcmp(read, "3.7 V")==0) Params->InputRange[ch] = 2;
    else if(strcmp(read, "9.5 V")==0) Params->InputRange[ch] = 3;
    else printf("Input Range %s: invalid setup", read);
    Params->DCOffset[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780DCOffset[ch] );
    DPPParams->thr[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780TriggerThreshold[ch] );
    DPPParams->k[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780TrapezoidRiseTime[ch] );
    DPPParams->m[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780TrapezoidFlatTop[ch] );
    DPPParams->M[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780DecayTime[ch] );
    DPPParams->ftd[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780PeakingTime[ch] );
    
    DPPParams->a[ch]=gtk_combo_box_get_active (GTK_COMBO_BOX(HpGeWidgets.cbDT5780TriggerSmoothing[ch]));
    DPPParams->b[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780InputRiseTime[ch] );
    DPPParams->trgho[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780TriggerHoldOff[ch] );
    DPPParams->pkho[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780PeakHoldOff[ch] );
    DPPParams->blho[ch] = gtk_spin_button_get_value_as_int((gpointer) HpGeWidgets.sbDT5780BaselineHoldOff[ch] );
  }
}

void ReadV1742Params(V1742Params_t* Params, int idigi) {

  Params->LinkNum = gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742LinkNum[idigi]);
  Params->PostTrigger = gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742PostTrigger[idigi]);
  Params->DRS4Frequency =  gtk_combo_box_get_active(GTK_COMBO_BOX(BaFWidgets.cbV1742Frequency[idigi]));
  Params->FastTriggerEnabled = gtk_combo_box_get_active(GTK_COMBO_BOX(BaFWidgets.cbV1742FastTrgDigitizing[idigi]));
  Params->TriggerEdge = gtk_combo_box_get_active(GTK_COMBO_BOX(BaFWidgets.cbV1742TriggerEdge[idigi]));
  Params->GroupEnableMask= 0;
  
  for(ng=0; ng< MaxV1742NGroups; ng++) {
    if(ng<2) {
      Params->FastTriggerDCOffset[ng*2]=  gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742FastTriggerOffset[ng][idigi]);
      Params->FastTriggerDCOffset[ng*2+1]=Params->FastTriggerDCOffset[ng*2];
      Params->FastTriggerThreshold[ng*2]=  gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742FastTriggerThreshold[ng][idigi]);
      Params->FastTriggerThreshold[ng*2+1]=Params->FastTriggerThreshold[ng*2];
    }
    Params->GroupEnableMask += (pow(2,ng))*gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (BaFWidgets.cV1742GroupEnable[ng][idigi]));		
  }
  for(nc=0; nc<MaxV1742NChannels; nc++) {
    Params->DCOffset[nc]= (gtk_spin_button_get_value_as_int((gpointer) BaFWidgets.sbV1742DCOffset[nc][idigi]));
  }
  
}

void ReadV1485Params(V1485Params_t* Params) {

  Params->LinkNum = gtk_spin_button_get_value_as_int((gpointer) V1485Widgets.sbV1485LinkNum );
  Params->BaseAddress = gtk_spin_button_get_value_as_int((gpointer) V1485Widgets.sbV1485BaseAddress );
  printf("BasdeAddress %X \n",Params->BaseAddress);
}

void FillRecipeList(GtkWidget* combo, char* stractive, int isFirst) {

  if(nitems!=0) {
    int ni;
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo),0);
    for(ni=0;ni<nitems;ni++) {
      gtk_combo_box_text_remove((GtkComboBoxText *) combo,nitems-ni);
      // non rimuovo il primo item perche' non riesco a rimuovere l'item attivo
    } 
  }
   
  FILE* file=popen("ls Setup/","r");

  char str[1000],str1[100];
  int read;
  ind=0;
  nitems=0;
  
  while(!feof(file)) {
    read=fscanf(file, "%s", str);
    if( !read || (read == EOF) || !strlen(str)) continue;
    if (strcmp(str, stractive)==0) ind=nitems;
    if(strstr(str, "Default")!=NULL) {
      strcat(str, " ->");
      FILE* deffile = popen("readlink Setup/Default","r");
      read=fscanf(deffile, "%s", str1);
      pclose(deffile);
      strcat(str,str1);
    }
    gtk_combo_box_text_append_text ((GtkComboBoxText *) combo,str);
    nitems++;
  }
  
  pclose(file);

  if(!isFirst) ind++;
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), ind);
 
  if(!isFirst) gtk_combo_box_text_remove((GtkComboBoxText *) combo,0);


}

void SaveRecipe() {

  isNewRecipe=1;
  char command[200];
  char* currentRecipe=getCurrentRecipe(cbRecipes);
  if(strstr(currentRecipe, "Default")!=NULL)  {
    char str[100];
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", str);
    pclose(deffile);
    currentRecipe=str;
  }
  char* pch=strstr(currentRecipe,"-v");
  if(pch==NULL) 
    {
      strcpy(command,"");
      strcat(command,"rm -r Setup/");
      strcat(command,currentRecipe);
      err_code=system(command);
      strcat(currentRecipe,"-v0");
      pch=strstr(currentRecipe,"-v");
    }
  int ver=atoi(pch+2);
  char newver[10];
  ver++;
  sprintf(newver,"%i",ver);
  strncpy (pch+2,newver,strlen(newver));

  char directory[100];
  strcpy(directory,"Setup/");
  strcat(directory,currentRecipe);
  while( access(directory, F_OK ) != -1 ) {
    sprintf(newver,"%i",ver++);
    strncpy (pch+2,newver,strlen(newver));
    strcpy(directory,"Setup/");
    strcat(directory,currentRecipe);
  }

  strcpy(command,"");
  strcat(command,"mkdir Setup/");
  strcat(command,currentRecipe);
  err_code=system(command);
  
  FillRecipeList(cbRecipes,currentRecipe,0);  
  
  char* V812configfile=getConfigFile(cbRecipes,Discr);
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cDiscr))) {
    V812Params_t Params;
    ReadV812Params(&Params);
    printf("scrivo file %s\n",V812configfile);
    WriteConfigFileV812(V812configfile, &Params);
  } 
  
  char* DT5780configfile=getConfigFile(cbRecipes,HpGe);
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigHpGe))) {
    printf("scrivo file %s\n",DT5780configfile);
    DT5780Params_t Params;
    CAEN_DGTZ_DPP_PHA_Params_t DPPParams;
    ReadDT5780Params(&Params,&DPPParams);
    WriteConfigFileDT5780(DT5780configfile, &Params, &DPPParams);
  } 
  
  char* V1742BaFconfigfile=getConfigFile(cbRecipes,BaF);
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigBaF))) {
    printf("scrivo file %s\n",V1742BaFconfigfile);
    V1742Params_t Params;
    ReadV1742Params(&Params,0);
    WriteConfigFileV1742(V1742BaFconfigfile, &Params);
  } 
  
  char* V1485configfile=getConfigFile(cbRecipes,PamSi);
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigPamSi))) {
    V1485Params_t Params;
    ReadV1485Params(&Params);
    printf("scrivo file %s\n",V1485configfile);
    WriteConfigFileV1485(V1485configfile, &Params);
  } 
    
  char* V1742Caloconfigfile=getConfigFile(cbRecipes,Calo);
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cConfigCalo))) {
    printf("scrivo file %s\n",V1742Caloconfigfile);
    V1742Params_t Params;
    ReadV1742Params(&Params,1);
    WriteConfigFileV1742(V1742Caloconfigfile, &Params);
  } 
  
  isNewRecipe=0;

}
void SaveAsRecipe() {
  
  char* foldername=NULL;
  GtkWidget* dialog=gtk_dialog_new_with_buttons("Recipe Name",NULL,
						GTK_DIALOG_MODAL,
						GTK_STOCK_CANCEL, 
						GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, 
						GTK_RESPONSE_ACCEPT,
						NULL);
  GtkWidget* eRecipeName = gtk_entry_new();
  gtk_widget_set_size_request(eRecipeName, 300, 30);
  gtk_container_add((GtkContainer *) gtk_dialog_get_content_area((GtkDialog*)dialog), eRecipeName);
  gtk_widget_show (dialog);
  gtk_widget_show (eRecipeName);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    { 
      isNewRecipe=1;
      foldername=(char*) gtk_entry_get_text((GtkEntry *) eRecipeName );
      char command[200]="mkdir Setup/";
      //      strcat(foldername,"-v0");
      strcat(command,foldername);
      err_code=system(command);
      FillRecipeList(cbRecipes,foldername,0);  
      SaveRecipe();
      //      strcpy(command,"");
      //      strcat(command,"rm -r Setup/");
      //      strcat(command, foldername);
      //      system(command);
    }
  gtk_widget_destroy (dialog);      
  isNewRecipe=0;
}

void RemoveRecipe() {

    char* foldername=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)cbRecipes);
    char command[200]="rm -r Setup/";
    strcat(command,foldername);
    system(command);
    FillRecipeList(cbRecipes,"Default",0); 
 
}

void MakeDefault() {

  if( access("Setup/Default", F_OK ) != -1 ) {
    system("rm Setup/Default");
  }
  char* foldername=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *)cbRecipes);
  char command[200]="ln -s ";
  strcat(command,foldername);
  strcat(command," Setup/Default");
  system(command);
  FillRecipeList(cbRecipes,"Default",0); 
   
}

void SetupFrameV812() {
  char strChan[2], labelChan[10];
  int yscale;
  GtkWidget *lV812Channel[MaxV812NChannels];
  //  GtkWidget *fixedDiscr = gtk_fixed_new();
  fixedV812 = gtk_fixed_new();
  GtkWidget *tabLabelDiscr = gtk_label_new ("Discriminator");
  GtkWidget *lV812Threshold[MaxV812NGroups];
  GtkWidget *lV812Enable[MaxV812NGroups];
  GtkWidget *lV812DeadTime[MaxV812NGroups];
  GtkWidget *lV812OutputWidth[MaxV812NGroups];

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedV812, tabLabelDiscr);
  // add frame to Discriminator tab
  GtkWidget *frameDiscr = gtk_frame_new ("V812");
  gtk_fixed_put(GTK_FIXED(fixedV812), frameDiscr, borders,0);
  gtk_widget_set_size_request (frameDiscr, xSizeTabs-2*borders, ySizeTabs-2*borders);
  // add Setup buttons to Discriminator tab

  V812Widgets.sbV812LinkNum = gtk_spin_button_new_with_range(0,3, 1);
  gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812LinkNum, 200, 20);
  GtkWidget *lV812LinkNum = gtk_label_new("Optical Link Number");
  gtk_fixed_put(GTK_FIXED(fixedV812), lV812LinkNum, 40, 25);
  
  V812Widgets.sbV812BaseAddress = gtk_spin_button_new_with_range (0, 65535, 1);
  gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812BaseAddress, 390, 20);
  gtk_widget_set_size_request (V812Widgets.sbV812BaseAddress, 80, 25);
  // g_signal_connect(G_OBJECT(V812Widgets.sbV812BaseAddress), "value_changed", G_CALLBACK(UpdateAddress), NULL);

  GtkWidget *lV812BaseAddress = gtk_label_new("BaseAddress");
  gtk_fixed_put(GTK_FIXED(fixedV812), lV812BaseAddress, 300, 25);
  // gtk_spin_button_set_value((gpointer) V812Widgets.sbV812BaseAddress, 60928);
  gtk_spin_button_set_numeric((gpointer) V812Widgets.sbV812BaseAddress,0);
  gtk_spin_button_set_value((gpointer) V812Widgets.sbV812BaseAddress, 60928);
  V812Widgets.sbV812Majority = gtk_spin_button_new_with_range (0, 16, 1);
  gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812Majority, 640, 20);
  GtkWidget *lV812Majority = gtk_label_new("Majority Threshold");
  gtk_fixed_put(GTK_FIXED(fixedV812), lV812Majority, 500, 25);
  
  GtkWidget *frameGr0 = gtk_frame_new ("Group 0");
  gtk_fixed_put(GTK_FIXED(fixedV812), frameGr0, 40,60);
  gtk_widget_set_size_request (frameGr0, 500, 360);

  GtkWidget *frameGr1 = gtk_frame_new ("Group 1");
  gtk_fixed_put(GTK_FIXED(fixedV812), frameGr1, 40,420);
  gtk_widget_set_size_request (frameGr1, 500, 360);


  for(ng=0; ng< MaxV812NGroups; ng++) {
    lV812Threshold[ng] = gtk_label_new("Threshold (mV)");
    gtk_fixed_put(GTK_FIXED(fixedV812), lV812Threshold[ng], 160, 80+360*ng);
    lV812Enable[ng] = gtk_label_new("Enable");
    gtk_fixed_put(GTK_FIXED(fixedV812), lV812Enable[ng], 260, 80+360*ng);

    V812Widgets.sbV812OutputWidth[ng] = gtk_spin_button_new_with_range (0,255,1);
    gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812OutputWidth[ng], 460,100+360*ng );
    lV812OutputWidth[ng] = gtk_label_new("Output Width");
    gtk_fixed_put(GTK_FIXED(fixedV812), lV812OutputWidth[ng], 360, 105+360*ng);
 
    V812Widgets.sbV812DeadTime[ng] = gtk_spin_button_new_with_range (0,255,1);
    gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812DeadTime[ng], 460,200+360*ng );
    lV812DeadTime[ng] = gtk_label_new("Dead Time");
    gtk_fixed_put(GTK_FIXED(fixedV812), lV812DeadTime[ng], 360, 205+360*ng);

    for(nc=0; nc<MaxV812NChannels/MaxV812NGroups; nc++) {
      ind=nc+ng*(MaxV812NChannels/MaxV812NGroups);
      yscale=70+30*ng+40*(ind+1);
      
      V812Widgets.sbV812Threshold[ind] = gtk_spin_button_new_with_range (0,255,1);
      gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.sbV812Threshold[ind], 180, yscale);
      sprintf(strChan,"%i",ind);
      strcpy(labelChan,"CHANNEL ");
      strcat(labelChan,strChan);
      lV812Channel[ind] =  gtk_label_new(labelChan);
      gtk_fixed_put(GTK_FIXED(fixedV812), lV812Channel[ind], 60, yscale+5);
      V812Widgets.cV812Channel[ind] = gtk_check_button_new(); 
      gtk_fixed_put(GTK_FIXED(fixedV812), V812Widgets.cV812Channel[ind], 260,yscale );
   }

  }
  gtk_widget_hide(fixedV812);
}
void SetupFrameDT5780() {

  int i,x,xscale=180,yscale=35;
  int xpos = 220, ypos=180;
  fixedDT5780 = gtk_fixed_new();
  GtkWidget *tabLabelHpGe = gtk_label_new ("HpGe");

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedDT5780, tabLabelHpGe);
  // add frame to Discriminator tab
  GtkWidget *frameHpGe = gtk_frame_new ("DT5780");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), frameHpGe, borders,0);
  gtk_widget_set_size_request (frameHpGe, xSizeTabs-2*borders, ySizeTabs-2*borders);

  // add Setup buttons to Discriminator tab

  /* Link Number */

  HpGeWidgets.sbDT5780LinkNum = gtk_spin_button_new_with_range (0,3,1);
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780LinkNum, 400, 20);
  GtkWidget *lDT5780LinkNum = gtk_label_new("Optical Link Number");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780LinkNum, 250, 25);
 
  /* Comunication Type */
  HpGeWidgets.cbDT5780LinkType=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780LinkType,120 , 20);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780LinkType,"USB");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780LinkType,"Optical");
  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780LinkType), 1);
  GtkWidget *lDT5780LinkType = gtk_label_new("Link Type");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780LinkType, 40, 25);

  /* Acquisition Mode */
  HpGeWidgets.cbDT5780AcquisitionMode=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780AcquisitionMode,170 , 60);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780AcquisitionMode,"OSCILLOSCOPE");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780AcquisitionMode,"LIST");
 gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780AcquisitionMode,"MIXED");
  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780AcquisitionMode), 0);
  GtkWidget *lDT5780AcquisitionMode = gtk_label_new("Acquisition Mode");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780AcquisitionMode, 40, 65);

  /* Record Length */

  HpGeWidgets.sbDT5780RecordLength = gtk_spin_button_new_with_range(0,100000,1000);
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780RecordLength, 450, 60);
  GtkWidget *lDT5780RecordLength = gtk_label_new("Record length");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780RecordLength, 350, 65);

  /* PreTrigger Size */

  HpGeWidgets.sbDT5780PreTriggerSize = gtk_spin_button_new_with_range(0,500,10);
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780PreTriggerSize, 700, 60);
  GtkWidget *lDT5780PreTriggerSize = gtk_label_new("PreTrigger Size");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780PreTriggerSize, 600, 65);
 
  /* IO Level */
  HpGeWidgets.cbDT5780IOLevel=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780IOLevel,100 , 100);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780IOLevel,"NIM");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780IOLevel,"TTL");
  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780IOLevel), 0);
  GtkWidget *lDT5780IOLevel = gtk_label_new("IO Level");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780IOLevel, 40, 105);

  /* External Trigger */
  HpGeWidgets.cDT5780ExternalTrigger = gtk_check_button_new_with_label("Enable External Trigger"); 
  gtk_fixed_put(GTK_FIXED(fixedDT5780),HpGeWidgets.cDT5780ExternalTrigger , 260, 100);
 
  /* Channel Trigger Mode */
  HpGeWidgets.cbDT5780ChannelTriggerMode=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780ChannelTriggerMode,700 , 100);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780ChannelTriggerMode,"AND");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780ChannelTriggerMode,"OR");
  gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelTriggerMode), 0);
  GtkWidget *lDT5780ChannelTriggerMode = gtk_label_new("Channel Trigger Mode");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780ChannelTriggerMode, 540, 105);
 
  /* Channels Params */
  GtkWidget *lDT5780Ch0 = gtk_label_new("CHANNEL 0");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780Ch0, xpos, ypos-yscale*0.8);
  GtkWidget *lDT5780Ch1 = gtk_label_new("CHANNEL 1");
  gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780Ch1, xpos+xscale, ypos-yscale*0.8);
  for(nc=0; nc<MaxDT5780NChannels; nc++) {
    x=xpos+xscale*nc;
    i=0;
    /* Enable */
    HpGeWidgets.cbDT5780ChannelEnable[nc] = gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780),HpGeWidgets.cbDT5780ChannelEnable[nc] , x, ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780ChannelEnable[nc],"ENABLE");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780ChannelEnable[nc],"DISABLE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780ChannelEnable[nc]), nc);
    /* Polarity */
    i++;
    HpGeWidgets.cbDT5780Polarity[nc]=gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780Polarity[nc],x , ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780Polarity[nc],"NEGATIVE");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780Polarity[nc],"POSITIVE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780Polarity[nc]), 0);
    /* SELF TRIGGER */
    i++;
    HpGeWidgets.cbDT5780SelfTrigger[nc]=gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780SelfTrigger[nc],x , ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780SelfTrigger[nc],"ENABLE");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780SelfTrigger[nc],"DISABLE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780SelfTrigger[nc]), 0);
   /* TRIGGER MODE*/
    i++;
    HpGeWidgets.cbDT5780TriggerMode[nc]=gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780TriggerMode[nc],x , ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerMode[nc],"INDIVIDUAL");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerMode[nc],"COINCIDENCE");
gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerMode[nc],"ANTICOINCIDENCE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780TriggerMode[nc]), 0);
   /* INPUT RANGE*/
    i++;
    HpGeWidgets.cbDT5780InputRange[nc]=gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780InputRange[nc],x , ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780InputRange[nc],"0.6 V");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780InputRange[nc],"1.4 V");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780InputRange[nc],"3.7 V");
gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780InputRange[nc],"9.5 V");
    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780InputRange[nc]), 3);
    /* DC OFFSET */
    i++;
    HpGeWidgets.sbDT5780DCOffset[nc] = gtk_spin_button_new_with_range (0,65535,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780DCOffset[nc], x, ypos+i*yscale);
   /* TRIGGER THRESHOLD */
    i++;
    HpGeWidgets.sbDT5780TriggerThreshold[nc] = gtk_spin_button_new_with_range(0,16383,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780TriggerThreshold[nc], x, ypos+i*yscale);
   /* TRAPEZOID RISE TIME */
    i++;
    HpGeWidgets.sbDT5780TrapezoidRiseTime[nc] = gtk_spin_button_new_with_range(0,10230,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780TrapezoidRiseTime[nc], x, ypos+i*yscale);
   /* TRAPEZOID FLAT TOP */
    i++;
    HpGeWidgets.sbDT5780TrapezoidFlatTop[nc] = gtk_spin_button_new_with_range(0,10230,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780TrapezoidFlatTop[nc], x, ypos+i*yscale);
   /* DECAY TIME */
    i++;
    HpGeWidgets.sbDT5780DecayTime[nc] = gtk_spin_button_new_with_range(0,655350,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780DecayTime[nc], x, ypos+i*yscale);
   /* PEAKING TIME */
    i++;
    HpGeWidgets.sbDT5780PeakingTime[nc] = gtk_spin_button_new_with_range(0,20470,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780PeakingTime[nc], x, ypos+i*yscale);
   /* TRIGGER SMOOTHING*/
    i++;
    HpGeWidgets.cbDT5780TriggerSmoothing[nc]=gtk_combo_box_text_new();
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.cbDT5780TriggerSmoothing[nc],x , ypos+i*yscale);
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"1");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"2");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"4");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"8");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"16");
    gtk_combo_box_text_append_text ((GtkComboBoxText *) HpGeWidgets.cbDT5780TriggerSmoothing[nc],"32");

    gtk_combo_box_set_active(GTK_COMBO_BOX(HpGeWidgets.cbDT5780TriggerSmoothing[nc]), 0);
   /* INPUT RISE TIME */
    i++;
    HpGeWidgets.sbDT5780InputRiseTime[nc] = gtk_spin_button_new_with_range(0,2550,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780InputRiseTime[nc], x, ypos+i*yscale);
   /* TRIGGER HOLD OFF */
    i++;
    HpGeWidgets.sbDT5780TriggerHoldOff[nc] = gtk_spin_button_new_with_range(0,5120,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780TriggerHoldOff[nc], x, ypos+i*yscale);
   /* PEAK HOLD OFF */
    i++;
    HpGeWidgets.sbDT5780PeakHoldOff[nc] = gtk_spin_button_new_with_range(0,20480,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780PeakHoldOff[nc], x, ypos+i*yscale);
   /* BASELINE HOLD OFF */
    i++;
    HpGeWidgets.sbDT5780BaselineHoldOff[nc] = gtk_spin_button_new_with_range(0,2550,1000);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), HpGeWidgets.sbDT5780BaselineHoldOff[nc], x, ypos+i*yscale);

  }

  /* Labels */

  GtkWidget *lDT5780Labels[16];
  char LabelName[16][100]={"ENABLE","POLARITY","SELF TRIGGER","TRIGGER MODE","INPUT RANGE","DC OFFSET (LSB)","TRIGGER THRESHOLD (LSB)", "TRAPEZOID RISE TIME (ns)","TRAPEZOID FLAT TOP (ns)", "DECAY TIME (ns)","PEAKING TIME (ns)", "TRIGGER SMOOTHING", "INPUT RISE TIME (ns)", "TRIGGER HOLD OFF (ns)", "PEAK HOLD OFF (ns)", "BASELINE HOLD OFF (ns)"};
  for(nc=0;nc<16;nc++){
    lDT5780Labels[nc]=gtk_label_new(LabelName[nc]);
    gtk_fixed_put(GTK_FIXED(fixedDT5780), lDT5780Labels[nc], 40, ypos+nc*yscale);
  }
  gtk_widget_hide(fixedDT5780);
}

void SetupFrameV1742(char* label, int idigi) {

  GtkWidget *lV1742DCOffset[MaxV1742NGroups];
  GtkWidget *frameGr[MaxV1742NGroups],*frameTr[MaxV1742NGroups/2] ;
  GtkWidget *lV1742Channel[MaxV1742NChannels], *lV1742TrgThreshold[2], *lV1742TrgDCOffset[2];
  fixedV1742[idigi] = gtk_fixed_new();
  GtkWidget *tabLabelBaF = gtk_label_new (label);
  GtkWidget *bDownloadCalibrationTables;
  char strChan[2], labelChan[10];
  char strGr[2], labelGr[10],labelTr[4];

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedV1742[idigi], tabLabelBaF);
  // add frame to Discriminator tab
  GtkWidget *frameBaF = gtk_frame_new ("V1742");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), frameBaF, borders,0);
  gtk_widget_set_size_request (frameBaF, xSizeTabs-2*borders, ySizeTabs-2*borders);
  /* Link Number */

  BaFWidgets.sbV1742LinkNum[idigi] = gtk_spin_button_new_with_range(0,3,1);
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.sbV1742LinkNum[idigi], 200, 20);
  GtkWidget *lV1742LinkNum = gtk_label_new("Optical Link Number");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742LinkNum, 50, 25);
 
  /* Acquisition Frequency */
  BaFWidgets.cbV1742Frequency[idigi]=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.cbV1742Frequency[idigi], 400 , 20);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742Frequency[idigi],"5 GHz");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742Frequency[idigi],"2.5 GHz");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742Frequency[idigi],"1 GHz");
  gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742Frequency[idigi]), 2);
  GtkWidget *lV1742Frequency = gtk_label_new("Frequency");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742Frequency, 300, 25);
  
  /* Load Calibration Tables */
  bDownloadCalibrationTables = gtk_button_new_with_label("Download Calibration Tables");
  gtk_widget_set_size_request(bDownloadCalibrationTables, 200, 30);
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), bDownloadCalibrationTables, 560, 20);
  g_signal_connect(G_OBJECT(bDownloadCalibrationTables), "clicked", G_CALLBACK(DownloadCalibrationTables), NULL);
  /* Post Trigger */

  BaFWidgets.sbV1742PostTrigger[idigi] = gtk_spin_button_new_with_range(0,100,10);
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.sbV1742PostTrigger[idigi], 200, 60);
  GtkWidget *lV1742PostTrigger = gtk_label_new("Post Trigger (%)");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742PostTrigger, 50, 65);
  
  /* Fast Trigger Digitizing */
  BaFWidgets.cbV1742FastTrgDigitizing[idigi]=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.cbV1742FastTrgDigitizing[idigi], 460 , 60);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742FastTrgDigitizing[idigi],"NO");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742FastTrgDigitizing[idigi],"YES");
  gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742FastTrgDigitizing[idigi]), 0);
  GtkWidget *lV1742FastTrgDigitizing = gtk_label_new("Fast Trigger Digitizing");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742FastTrgDigitizing, 300, 65);
  
  /* Trigger Edge */
  BaFWidgets.cbV1742TriggerEdge[idigi]=gtk_combo_box_text_new();
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.cbV1742TriggerEdge[idigi], 660 , 60);
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742TriggerEdge[idigi],"RISING");
  gtk_combo_box_text_append_text ((GtkComboBoxText *) BaFWidgets.cbV1742TriggerEdge[idigi],"FALLING");
  gtk_combo_box_set_active(GTK_COMBO_BOX(BaFWidgets.cbV1742TriggerEdge[idigi]), 0);
  GtkWidget *lV1742TriggerEdge = gtk_label_new("Trigger Edge");
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742TriggerEdge, 560, 65);
  int x[MaxV1742NGroups]={60,320,60,320};
  int y[MaxV1742NGroups]={120,120,460,460};
  
  for(ng=0; ng<MaxV1742NGroups; ng++) {
    sprintf(strGr,"%i",ng);
    if(ng<2) {
      BaFWidgets.sbV1742FastTriggerOffset[ng][idigi]=gtk_spin_button_new_with_range(0,65536,10);
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]),BaFWidgets.sbV1742FastTriggerOffset[ng][idigi], x[1]+350,y[ng*2]+10);
      lV1742TrgDCOffset[ng]=gtk_label_new("DC Offset");
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]),lV1742TrgDCOffset[ng], x[1]+250,y[ng*2]+15);
      BaFWidgets.sbV1742FastTriggerThreshold[ng][idigi]=gtk_spin_button_new_with_range(0,65536,10);
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]),BaFWidgets.sbV1742FastTriggerThreshold[ng][idigi], x[1]+350,y[ng*2]+50);
      lV1742TrgThreshold[ng]=gtk_label_new("Threshold");
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]),lV1742TrgThreshold[ng], x[1]+250,y[ng*2]+55);

      strcpy(labelTr,"Tr");
      strcat(labelTr,strGr);
      frameTr[ng]=gtk_frame_new (labelTr);
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), frameTr[ng], x[1]+240,y[2*ng]-20);
      gtk_widget_set_size_request (frameTr[ng], 200, 180);
    }
    strcpy(labelGr,"Group ");
    strcat(labelGr,strGr);
    frameGr[ng]=gtk_frame_new (labelGr);
    gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), frameGr[ng], x[ng]-20,y[ng]-20);
    gtk_widget_set_size_request (frameGr[ng], 240, 340);
    
    BaFWidgets.cV1742GroupEnable[ng][idigi] = gtk_check_button_new_with_label("Enable"); 
    gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]),BaFWidgets.cV1742GroupEnable[ng][idigi], x[ng],y[ng]+10);

    lV1742DCOffset[ng] = gtk_label_new("DC Offset (%)");
    gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742DCOffset[ng], x[ng]+110, y[ng]+10);
    int ich=0;
    for(nc=0; nc<MaxV1742NChannels/MaxV1742NGroups; nc++) {
      
      ind=nc+ng*(MaxV1742NChannels/MaxV1742NGroups);
      
      BaFWidgets.sbV1742DCOffset[ind][idigi] = gtk_spin_button_new_with_range(-50.0,50.0,0.1);
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), BaFWidgets.sbV1742DCOffset[ind][idigi], x[ng]+120, y[ng]+35*(ich+1));
      sprintf(strChan,"%i",ind);
      strcpy(labelChan,"CHANNEL ");
      strcat(labelChan,strChan);
      lV1742Channel[ind] =  gtk_label_new(labelChan);
      gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), lV1742Channel[ind], x[ng], y[ng]+35*(ich+1) +5);
      ich++;
   }

  }
  GtkWidget* bTrHelp = gtk_button_new_with_label("Trigger Setup Wizard");
  gtk_widget_set_size_request(bTrHelp, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixedV1742[idigi]), bTrHelp, x[1]+250, y[0]+240);
  g_signal_connect(bTrHelp, "clicked", G_CALLBACK(OpenTriggerHelp), NULL); 
  gtk_widget_hide(fixedV1742[idigi]);
}

void SetupFrameV1485() {
  char strChan[2], labelChan[10];
  int yscale;
  fixedV1485 = gtk_fixed_new();
  GtkWidget *tabLabelPamSi = gtk_label_new ("PamSi");

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), fixedV1485, tabLabelPamSi);
  // add frame to Discriminator tab
  GtkWidget *framePamSi = gtk_frame_new ("V1485");
  gtk_fixed_put(GTK_FIXED(fixedV1485), framePamSi, borders,0);
  gtk_widget_set_size_request (framePamSi, xSizeTabs-2*borders, ySizeTabs-2*borders);
  // add Setup buttons to Discriminator tab

  V1485Widgets.sbV1485LinkNum = gtk_spin_button_new_with_range(0,3, 1);
  gtk_fixed_put(GTK_FIXED(fixedV1485), V1485Widgets.sbV1485LinkNum, 200, 20);
  GtkWidget *lV1485LinkNum = gtk_label_new("Optical Link Number");
  gtk_fixed_put(GTK_FIXED(fixedV1485), lV1485LinkNum, 40, 25);
  
  V1485Widgets.sbV1485BaseAddress = gtk_spin_button_new_with_range (0, 65535, 1);
  gtk_fixed_put(GTK_FIXED(fixedV1485), V1485Widgets.sbV1485BaseAddress, 390, 20);
  gtk_widget_set_size_request (V1485Widgets.sbV1485BaseAddress, 80, 25);
  //  g_signal_connect(G_OBJECT(V1485Widgets.sbV1485BaseAddress), "value_changed", G_CALLBACK(UpdateAddress), NULL);

  GtkWidget *lV1485BaseAddress = gtk_label_new("BaseAddress");
  gtk_fixed_put(GTK_FIXED(fixedV1485), lV1485BaseAddress, 300, 25);
  // gtk_spin_button_set_value((gpointer) V812Widgets.sbV812BaseAddress, 60928);

  gtk_spin_button_set_numeric((gpointer) V1485Widgets.sbV1485BaseAddress,0);
  gtk_spin_button_set_value((gpointer) V1485Widgets.sbV1485BaseAddress, 8738);
 
  gtk_widget_hide(fixedV1485);
}

void openAdvancedSetup() {
  int argc;
  char *argv[4];
  //  GtkWidget *window;
  GtkWidget *bSaveRecipe, *bSaveRecipeAs, *bRemoveRecipe, *bMakeDefault; //buttons

  if(!isOpen) {
    OpenLogin();
    if(Login==1) {
      isOpen=1;
      window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      
      gtk_window_set_title(GTK_WINDOW(window), "Devices Setup");
      gtk_window_set_default_size(GTK_WINDOW(window), 1100, 600);
      gtk_container_set_border_width(GTK_CONTAINER(window), 10);
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
      fixed = gtk_fixed_new();
      gtk_container_add(GTK_CONTAINER(window), fixed);
      // Create a new notebook, place the position of the tabs 
      notebook = gtk_notebook_new ();
      gtk_fixed_put(GTK_FIXED(fixed), notebook, 20, 60);
      gtk_widget_set_size_request(notebook, xSizeTabs, ySizeTabs);
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    
      cDiscr = gtk_check_button_new_with_label("Discriminator"); 
      gtk_fixed_put(GTK_FIXED(fixed), cDiscr, 20, 20);
      
      cConfigBaF = gtk_check_button_new_with_label("BaF"); 
      gtk_fixed_put(GTK_FIXED(fixed), cConfigBaF, 220, 20);
      
      cConfigHpGe = gtk_check_button_new_with_label("HpGe"); 
      gtk_fixed_put(GTK_FIXED(fixed), cConfigHpGe, 150, 20);

      cConfigPamSi = gtk_check_button_new_with_label("Si"); 
      gtk_fixed_put(GTK_FIXED(fixed), cConfigPamSi, 290, 20);

      cConfigCalo = gtk_check_button_new_with_label("Calo"); 
      gtk_fixed_put(GTK_FIXED(fixed), cConfigCalo, 360, 20);

      cbRecipes=gtk_combo_box_text_new();
      gtk_fixed_put(GTK_FIXED(fixed), cbRecipes,850 , 120);
      gtk_widget_show (cbRecipes);
      
      GtkWidget* lRecipes=gtk_label_new("Recipe List");
      gtk_fixed_put(GTK_FIXED(fixed), lRecipes, 850, 100);
      gtk_widget_set_size_request (cbRecipes, 250, 30);
      
      GtkWidget *menu, *menuBar, *rootMenu, *saveItem, *saveAsItem, *removeItem, *makeDefaultItem;
      
      menu = gtk_menu_new();
      rootMenu = gtk_menu_item_new_with_label("Manage Recipes..");
      gtk_widget_show(rootMenu);
      saveItem = gtk_menu_item_new_with_label("Save");
      gtk_menu_append(GTK_MENU (menu), saveItem);
      gtk_widget_show(saveItem);
      saveAsItem = gtk_menu_item_new_with_label("Save As..");
      gtk_menu_append(GTK_MENU (menu), saveAsItem);
      gtk_widget_show(saveAsItem);
      removeItem = gtk_menu_item_new_with_label("Remove");
      gtk_menu_append(GTK_MENU (menu), removeItem);
      gtk_widget_show(removeItem);
      makeDefaultItem = gtk_menu_item_new_with_label("Make Default");
      gtk_menu_append(GTK_MENU (menu), makeDefaultItem);
      gtk_widget_show(makeDefaultItem);
      gtk_menu_item_set_submenu(GTK_MENU_ITEM (rootMenu), menu);
      GtkWidget* vbox = gtk_vbox_new (FALSE, 0);
      gtk_widget_set_size_request(vbox, 150, 70);
      gtk_fixed_put(GTK_FIXED(fixed), vbox, 850, 160);
      gtk_widget_show (vbox);
      menuBar = gtk_menu_bar_new ();
      gtk_box_pack_start (GTK_BOX (vbox), menuBar, FALSE, FALSE, 2);
      gtk_widget_show (menuBar);
      gtk_menu_bar_append (GTK_MENU_BAR (menuBar), rootMenu);

      GtkWidget* bExit = gtk_button_new_with_label("Exit");
      gtk_widget_set_size_request(bExit, 150,40);
      gtk_fixed_put(GTK_FIXED(fixed), bExit, 950, 800);
      g_signal_connect(G_OBJECT(bExit), "clicked", G_CALLBACK(CloseAdvancedSetup), NULL); 

      FillRecipeList(cbRecipes, "Default",1);

      SetupFrameV812();
      SetupFrameDT5780();
      SetupFrameV1742("BaF",0);
      SetupFrameV1485();
      SetupFrameV1742("Calo",1);
      LoadRecipeParams();
      gtk_widget_show_all(window);
      showCaloFrame();
      showBaFFrame();
      showDiscrFrame();
      showHpGeFrame();
      showPamSiFrame();

      g_signal_connect(window, "destroy", G_CALLBACK(CloseAdvancedSetup), NULL); 
      //      g_signal_connect(G_OBJECT(notebook), "switch-page", G_CALLBACK(UpdateAddress), NULL);
      g_signal_connect(G_OBJECT(V812Widgets.sbV812BaseAddress), "value_changed", G_CALLBACK(UpdateAddress), V812Widgets.sbV812BaseAddress);
      g_signal_connect(G_OBJECT(V812Widgets.sbV812BaseAddress), "focus-in-event", G_CALLBACK(UpdateAddress), V812Widgets.sbV812BaseAddress);
      g_signal_connect(G_OBJECT(V1485Widgets.sbV1485BaseAddress), "value_changed", G_CALLBACK(UpdateAddress),V1485Widgets.sbV1485BaseAddress );
      g_signal_connect(G_OBJECT(V1485Widgets.sbV1485BaseAddress), "focus-in-event", G_CALLBACK(UpdateAddress),V1485Widgets.sbV1485BaseAddress );

      g_signal_connect(G_OBJECT(saveItem), "activate", G_CALLBACK(SaveRecipe), NULL);
      g_signal_connect(G_OBJECT(saveAsItem), "activate", G_CALLBACK(SaveAsRecipe), NULL);
      g_signal_connect(G_OBJECT(removeItem), "activate", G_CALLBACK(RemoveRecipe), NULL);
      g_signal_connect(G_OBJECT(makeDefaultItem), "activate", G_CALLBACK(MakeDefault), NULL);
      g_signal_connect(G_OBJECT(cDiscr), "toggled", G_CALLBACK(showDiscrFrame), NULL);
      g_signal_connect(G_OBJECT(cConfigBaF), "toggled", G_CALLBACK(showBaFFrame), NULL);
      g_signal_connect(G_OBJECT(cConfigCalo), "toggled", G_CALLBACK(showCaloFrame), NULL);
      g_signal_connect(G_OBJECT(cConfigHpGe), "toggled", G_CALLBACK(showHpGeFrame), NULL);
      g_signal_connect(G_OBJECT(cConfigPamSi), "toggled", G_CALLBACK(showPamSiFrame), NULL);
      g_signal_connect(G_OBJECT(cbRecipes), "changed", G_CALLBACK(LoadRecipeParams), NULL);
    }
  } else {
    gtk_window_present((gpointer) window); 
  }
 
}
