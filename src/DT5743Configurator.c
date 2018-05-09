#include <CAENDigitizer.h>
#include "../include/DT5743Configurator.h"
#include "../include/UtilsFunctions.h"

#define verbose 0

/*************************************************************************************************************/
void WriteConfigFileDT5743(char* filename, DT5743Params_t *Params, CAEN_DGTZ_DPP_X743_Params_t *DT5743_DPP_Params) {
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

  if(Params->TriggerMode==0) fprintf(file,"EXTTRIGGER SW \n\n");
  else if(Params->TriggerMode==1) fprintf(file,"EXTTRIGGER NORMAL \n\n");
  else if(Params->TriggerMode==2) fprintf(file,"EXTTRIGGER AUTO \n\n");
  else fprintf(file,"EXTTRIGGER EXTERNAL \n\n");

  if(Params->TriggerOut==0) fprintf(file,"TRIGGEROUT NO \n\n");
  else if(Params->TriggerOut==1) fprintf(file,"TRIGGEROUT YES \n\n");
  else fprintf(file,"TRIGGEROUT %i \n\n",Params->TriggerOut);

  fprintf(file,"TRIGGERGATE %i \n\n",Params->TriggerGate);

  fprintf(file,"TRIGGERPAIRLOGIC 0x%x \n\n",Params->TriggerPairLogic);

  fprintf(file,"GLOBALTRIGGERLOGIC %i \n\n",Params->GlobalTriggerLogic);

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
      fprintf(file, "CH_THRES %i \n\n", DT5743_DPP_Params->chargeThreshold[ch]);
      fprintf(file, "CH_REF_CELL %i \n\n", DT5743_DPP_Params->startCell[ch]);
      fprintf(file, "CH_LENGTH %i \n\n", DT5743_DPP_Params->chargeLength[ch]);
            

  }
  fclose(file);
}

/*************************************************************************************************************/
int ParseConfigFileDT5743(char* filename, DT5780Params_t *Params, CAEN_DGTZ_DPP_X743_Params_t *DT5743_DPP_Params) {
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

    else if (strstr(str, "TRIGGERGATE")!=NULL)
    {
      read = fscanf(file, "%i", &Params->TriggerGate);

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
    Params->SelfTrigger[ch] = 0x1;
  else if (strcmp(str1, "DISABLE")==0)
    Params->SelfTrigger[ch] = 0x0;
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


  // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
  // Not programmable via configuration file
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    
  // Set the number of samples for each waveform
  ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength);
  int rl;
  CAEN_DGTZ_GetRecordLength(handle,&rl);
  // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
  ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOLevel);

  // This function enables/disables the groups for the acquisition. Disabled channels don’t give any trigger and don’t participate to the event data.
  ret |= CAEN_DGTZ_SetGroupEnableMask(handle, Params.GroupMask);


  /* Set the digitizer's behaviour when an external trigger arrives:
    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger
  */
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);

  
  // Set how many events to accumulate in the board memory before being available for readout
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle,Params.MaxEventsBlt);


 uint32_t chmask = 0x0000;
//PER CHANNEL
  for(i=0; i<MaxDT5743NChannels; i++) {
    int gr_i = i/2;
    if(Params.GroupMask & (1<<gr_i)) {
                        

      // Set the Post-Trigger size (in samples)
      ret |= CAEN_DGTZ_SetSAMPostTriggerSize(handle, gr_i, Params.PosTrigger);
      // Set a DC offset to the input signal to adapt it to digitizer's dynamic range 
      ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, Params.DCOffset[i]);
 
      // channel autotrigger
      chmask =+ Params.SelfTrigger[i]<<i;
    }
  }
  ret |= CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, chmask);


//sostituire con TestPattern

//trigger between channels
  int j;

  for (j = 0; j < MaxDT5743Blocks; ++j)
  {
    CAEN_DGTZ_TriggerLogic_t pairLogic;
  
    uint32_t pairmask = 0x0000;
    pairmask |= 1<<j;
    if(pairmask == (pairmask&Params.TriggerPairLogic)) pairLogic = 1;
    else pairLogic = 0;
    ret |= CAEN_DGTZ_SetChannelPairTriggerLogic(handle, j*2, j*2+1, pairLogic, Params.TriggerGate);
  }

//global trigger logic
  ret |= CAEN_DGTZ_SetTriggerLogic(handle, globalLogic, majorityLevel);


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

