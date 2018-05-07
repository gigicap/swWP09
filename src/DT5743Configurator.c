#include <CAENDigitizer.h>
#include "../include/DT5743Configurator.h"
#include "../include/UtilsFunctions.h"

#define verbose 0

/*************************************************************************************************************/
void WriteConfigFileDT5743(char* filename, DT5743Params_t *Params) {
/*************************************************************************************************************/
  
  FILE* file = fopen(filename,"w");
  if(Params->LinkType==0) fprintf(file,"LINKTYPE USB \n\n");
  else fprintf(file,"LINKTYPE PCI \n\n");
  fprintf(file,"LINKNUM %i \n\n",Params->LinkNum);
  fprintf(file,"RECORDLENGTH %i \n\n",Params->RecordLength);
  fprintf(file,"MAXEVENTSBLT %i \n\n",Params->MaxEventsBlt);
  fprintf(file,"POSTRIGGER %i \n\n",Params->PosTrigger);

  if(Params->AcqMode==0) fprintf(file,"ACQUISITIONMODE WAVEFORM \n\n");
  else fprintf(file,"ACQUISITIONMODE CHARGE \n\n");

  if(Params->TestPattern==0) fprintf(file,"TESTPATTERN NO \n\n");
  else fprintf(file,"TESTPATTERN YES \n\n");  

  if(Params->IOLevel == 0) fprintf(file,"IOLEVEL NIM \n\n");
  else fprintf(file,"IOLEVEL TTL \n\n");

  if(Params->ExtTrigger==0) fprintf(file,"EXTTRIGGER ENABLE \n\n");
  else fprintf(file,"EXTTRIGGER DISABLE \n\n");

  if(Params->TriggerOut==3) fprintf(file,"TRIGGEROUT AND \n\n");
  else if(Params->TriggerOut==0) fprintf(file,"TRIGGEROUT OR \n\n");
  else fprintf(file,"TRIGGEROUT %i \n\n",Params->TriggerOut);

  fprintf(file,"GROUPMASK 0x%x \n\n",Params->GroupMask);

  int ch;
  for(ch=0;ch<MaxDT5743NChannels;ch++) {
      fprintf(file,"[CH%i] \n\n",ch);
      fprintf(file,"DCOFFSET %i \n\n",Params->DCOffset[ch]);
      if(Params->SelfTrigger[ch] == 0)  fprintf(file,"SELFTRIGGER DISABLE \n\n");
      else fprintf(file,"SELFTRIGGER ENABLE \n\n");
      fprintf(file,"TRIGGERTHRESHOLD %i \n\n",Params->TriggerLevel[ch]);
      if(Params->Polarity[ch] == 0) fprintf(file, "POLARITY NEGATIVE\n\n", );
      else fprintf(file, "POLARITY POSITIVE\n\n", );
      fprintf(file, "CH_TRES %i \n\n", Params->ChTres[ch]);
      fprintf(file, "CH_REF_CELL %i \n\n", Params->ChRefCell[ch]);
      fprintf(file, "CH_LENGTH %i \n\n", Params->ChLength[ch]);
            

  }
  fclose(file);
}

/*************************************************************************************************************/
int ParseConfigFileDT5743(char* filename, DT5780Params_t *Params) {
/*************************************************************************************************************/
  FILE* file = fopen(filename,"r");

  char str[1000], str1[1000];
  int read;
  int value;
  int ch;
  while(!feof(file)) {
    
    read = fscanf(file, "%s", str);
    if( !read || (read == EOF) || !strlen(str)) continue;
    if(str[0] == '#') {fgets(str, 1000, file); continue;}    // skip comments
    if (strstr(str, "LINKTYPE")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "USB")==0)
	  Params->LinkType = CAEN_DGTZ_USB;
	else if (strcmp(str1, "PCI")==0)
	  Params->LinkType = CAEN_DGTZ_OpticalLink;
	 if(verbose) printf("Link Type %s \n", str1);
	continue;
      }  
    else if (strstr(str, "LINKNUM")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->LinkNum);
	 if(verbose) printf("Link Number %i \n", Params->LinkNum);
	continue;
      }  

   else if (strstr(str, "RECORDLENGTH")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->RecordLength);
	 if(verbose) printf("RecordLength %i channels \n", Params->RecordLength);
	continue;
      }  
   else if (strstr(str, "PRETRIGGERSIZE")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->PosTrigger);
	 if(verbose) printf("PosTrigger Size %i \n", Params->PosTrigger);
	continue;
      }
   else if (strstr(str, "ACQUISITIONMODE")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "WAVEFORM")==0)
	  Params->AcqMode = 0;
	else if (strcmp(str1, "CHARGE")==0)
	  Params->AcqMode = 1;
	 if(verbose) printf("Acquisition Mode %s \n", str1);
	continue;
      }  

   else if (strstr(str, "IOLEVEL")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "NIM")==0)
	  Params->IOLevel = CAEN_DGTZ_IOLevel_NIM;
	else if (strcmp(str1, "TTL")==0)
	  Params->IOLevel = CAEN_DGTZ_IOLevel_TTL;
	 if(verbose) printf("Front Panel IO Level %s \n", str1);
	continue;
      }  

   else if (strstr(str, "GROUPMASK")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->GroupMask );
	 if(verbose) printf("Group Mask %x \n",Params->GroupMask);
	continue;
      }

   else if (strstr(str, "TRIGGEROUT")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
  Params->TriggerOut = atoi(read);
	if(verbose) printf("Trigger Out %s \n",str1 );
	continue;
      }

   else if (strstr(str, "EXTTRIGGER")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
	if (strcmp(str1, "ENABLE")==0)
	  Params->ExtTrigger = 0;
	else if (strcmp(str1, "DISABLE")==0)
	  Params->ExtTrigger = 1;
	 if(verbose) printf("External Trigger  %s \n",str1 );
	continue;
      }


  else if (strstr(str, "[CH0]")!=NULL) {
      ch=0;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH1]")!=NULL) {
      ch=1;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH2]")!=NULL) {
      ch=2;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH3]")!=NULL) {
      ch=3;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH4]")!=NULL) {
      ch=4;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH5]")!=NULL) {
      ch=5;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH6]")!=NULL) {
      ch=6;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
  else if (strstr(str, "[CH7]")!=NULL) {
      ch=7;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }



  else if (strstr(str, "POLARITY")!=NULL) 
      {
	     read = fscanf(file, "%s", str1 );
	     if (strcmp(str1, "POSITIVE")==0)
	         Params->Polarity[ch]=1;
	     else if (strcmp(str1, "NEGATIVE")==0)
	         Params->Polarity[ch]=0;
	     if(Params->Polarity[ch]==1 && verbose==1) printf("Ch%i: Polarity Positive \n",ch);
	     else if(verbose)  printf("Ch%i: Polarity Negative \n",ch);
	continue;
      }
    
  else if (strstr(str, "TRIGGERTHRESHOLD")!=NULL) 
      {
	read = fscanf(file, "%i",&Params->TriggerLevel[ch]);
	 if(verbose) printf("Ch%i: Trigger Threshold %i ADC counts \n",ch,&Params->TriggerLevel[ch]);
	continue;
      }

  else if (strstr(str, "DCOFFSET")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->DCOffset[ch]);
	if(verbose) printf("Ch%i: DC OffSet %i \n", ch,Params->DCOffset[ch]);
	continue;
      }

  else if (strstr(str, "SELFTRIGGER")!=NULL) 
      {
  read = fscanf(file, "%s", str1 );
  if (strcmp(str1, "ENABLE")==0)
    Params->SelfTrigger[ch] = 0x0000000;
  else if (strcmp(str1, "DISABLE")==0)
    Params->SelfTrigger[ch] = 0x1000000;
  if(verbose) printf("Ch%i: Self Trigger  %s \n",ch, str1 );
  continue;
      }

  else if (strstr(str, "CH_TRES")!=NULL) 
      {
  read = fscanf(file, "%i", &Params->ChTres[ch]);
  if(verbose) printf("Ch%i: CHARGE MODE THRESHOLD %i \n", ch,Params->ChTres[ch]);
  continue;
      }
  else if (strstr(str, "CH_REF_CELL")!=NULL) 
      {
  read = fscanf(file, "%i", &Params->ChRefCell[ch]);
  if(verbose) printf("Ch%i: CHARGE REFERENCE CELL %i \n", ch,Params->ChRefCell[ch]);
  continue;
      }
  else if (strstr(str, "CH_LENGTH")!=NULL) 
      {
  read = fscanf(file, "%i", &Params->ChLength[ch]);
  if(verbose) printf("Ch%i: CHARGE INTEGRATION LENGTH %i \n", ch,Params->ChLength[ch]);
  continue;
      }
  
  }

  fclose(file);

}


/*************************************************************************************************************/
int  ProgramDigitizerDT5743(int connectionParams[4], char* filename) {
/*************************************************************************************************************/
  //CAEN_DGTZ_DPP_PHA_Params_t DPPParams;
  DT5780Params_t Params;

  CAEN_DGTZ_BoardInfo_t           BoardInfo;
  int MajorNumber;
  int ret=0, handle;
  uint32_t InputRangeRegister[2]={0x10B4,0x11B4};
  uint32_t InputRangeValues[4]={0x5,0x6,0x9,0xA};
  uint32_t TriggerModeRegister[2]={0x1080,0x1180};
  uint32_t TriggerModeMask= 0xC0000;
  uint32_t TriggerModeValues[3]={0x00000,0x80000,0xC0000};

  //Set to 0 
  memset(&Params, 0, sizeof(DT5743Params_t));
  //Parse from file    
  ParseConfigFileDT5743(filename,&Params);

 
  /* *************************************************************************************** */
  /* Open the digitizer and read board information                                           */
  /* *************************************************************************************** */
  /* The following function is used to open the digitizer with the given connection parameters
    and get the handler to it */

  ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,Params.LinkNum , 0, 0, &handle);

  if (ret) {
    sleep(1);
    ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,Params.LinkNum , 0, 0, &handle); //retry opening
    if(ret) {
      printf("Can't open digitizer DT5743\n");
      return ret;
    }
  }
        
  printf("ProgramDigitizerDT5743: handle = %i\n",handle);

  /* Once we have the handler to the digitizer, we use it to call the other functions */
  ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
  if (ret) {
    printf("Can't read board info\n");
    return ret;
  }


  printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, 0);
  printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
  printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);


  /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
  int i;
  uint32_t val;

  /* Reset the digitizer */
  ret |= CAEN_DGTZ_Reset(handle);
  printf("ProgramDigitizerDT5743: --handle = %i\n",handle);
  if (ret) {
      printf("ERROR: can't reset the digitizer.\n");
      return -1;
  }

//???????????
  ret |= CAEN_DGTZ_WriteRegister(handle, 0x8000, 0x01040114);  // This register contains general settings for the board configuration.
  ret |= CAEN_DGTZ_WriteRegister(handle, 0x811C, 0xC003C);  // This register manages the front panel I/O connectors
  // ret |= CAEN_DGTZ_WriteRegister(handle, 0x811C, 0x801);  // This register manages the front panel I/O connectors


  // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    
  // Set the number of samples for each waveform
  ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength);
  int rl;
  CAEN_DGTZ_GetRecordLength(handle,&rl);
  // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
  ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOLevel);

//??????
  CAEN_DGTZ_WriteRegister(handle, 0xEF08, 0x1C);  // Board id set to 1C (=11100)

  /* Set the digitizer's behaviour when an external trigger arrives:
    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger
  */
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);

  // This function enables/disables the groups for the acquisition. Disabled channels don’t give any trigger and don’t participate to the event data.
  ret |= CAEN_DGTZ_SetGroupEnableMask(handle, Params.ChannelMask);

  // Set how many events to accumulate in the board memory before being available for readout
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle,Params.MaxEventsBlt);


//SI PUO' FARE ANCHE CANALE PER CANALE 
  for(i=0; i<MaxDT5743NChannels; i++) {
    int gr_i = i/2;
    if(Params.GroupMask & (1<<gr_i)) {
                        

      // Set the Post-Trigger size (in samples)
      ret |= CAEN_DGTZ_SetSAMPostTriggerSize(handle, gr_i, Params.PosTrigger);

      // Set a DC offset to the input signal to adapt it to digitizer's dynamic range 
      ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, Params.DCOffset[i]);
 
      // printf("TriggerModeRegister %x,TriggerModeValues %x,TriggerModeMask %x \n",TriggerModeRegister[i],TriggerModeValues[TrgMode[i]],TriggerModeMask);
      ret |= WriteRegisterBitmask(handle, TriggerModeRegister[i],TriggerModeValues[Params.TrgMode[i]],TriggerModeMask);
      ret |= WriteRegisterBitmask(handle, TriggerModeRegister[i],Params.SelfTrigger[i],0x1000000);
    }
  }


//sostituire con TestPattern

  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Trapezoid);
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_2,CAEN_DGTZ_DPP_DIGITALPROBE_Trigger );
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1,CAEN_DGTZ_DPP_DIGITALPROBE_Peaking );

  // Enable propagation of individual trigger   
  ret |= WriteRegisterBitmask(handle, 0x8000, 0x4, 0x4);



  // Enable trigger out AND o OR between channels
  ret |= CAEN_DGTZ_WriteRegister(handle, 0x8110,Params.TriggerOut); 
  ret |= WriteRegisterBitmask(handle, 0x817C,Params.ExtTrigger,0x1);
  //    ret |= WriteRegisterBitmask(handle, 0x817C,0x0,0x1); // enable ext trigger ATTENZIONE: 0->enable 1->disable
  //    ret |= WriteRegisterBitmask(handle, 0x1080,0x1000000,0x1000000); // disable(=1)/enable(=0) acquisition on self trigger
  //    ret |= WriteRegisterBitmask(handle, 0x1080,0x0000000,0x1000000); // disable(=1)/enable(=0) acquisition on self trigger

  // disable(=0)/enable(=1) energy evaluation also for piled-up events
  ret |= WriteRegisterBitmask(handle, 0x1080,0x1000000,0x8000000);


  ret |= WriteRegisterBitmask(handle, 0x1184, 0xFF, 0xFF);// gate (x10ns)
  ret |= WriteRegisterBitmask(handle, 0x1084, 0xFF, 0xFF);// gate (x10ns)
  ret |= WriteRegisterBitmask(handle, 0x8188, 0x4000010C, 0xFFFFFFFF);// 
  ret |= WriteRegisterBitmask(handle, 0x818C, 0xC000010C, 0xFFFFFFFF);// 
  //    ret |= WriteRegisterBitmask(handle, 0x818C, 0x10C, 0xFFFFFFFF);
  //    ret |= WriteRegisterBitmask(handle, 0x818C, 0x10C, 0xFFFFFFFF); // se non setto 818C allora salvo solo il canale 0 (ma solo quando ho la coincidenza tra i due)
 
  ret |= CAEN_DGTZ_CloseDigitizer(handle);

  if(ret) {
    printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
    return ret;
  } else {
    connectionParams[0]=Params.LinkType;
    connectionParams[1]=Params.LinkNum;
    connectionParams[2]=0;
    connectionParams[3]=0;
    return 0;
  }
    
}

