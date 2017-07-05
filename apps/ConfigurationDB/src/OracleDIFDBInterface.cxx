#include "OracleDIFDBInterface.hh"
#include <map>
lydaq::OracleDIFDBInterface::OracleDIFDBInterface(State* s ) : _state(s)
{}
void lydaq::OracleDIFDBInterface::initialize()
{}

void lydaq::OracleDIFDBInterface::LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v)
{
  LOG4CXX_INFO(_logOracle,"Oracle initialisation ");
  DifConfiguration* dd=_state->getDifConfiguration();
  //  std::cout<<"diffs "<<(int) dd<<std::endl;
  std::vector<ConfigObject*> dim=_state->getDifConfiguration()->getVector();
  std::cout <<dim.size()<<std::endl;
  LOG4CXX_INFO(_logOracle," Number of DIF found "<<dim.size());
  for (std::vector<ConfigObject*>::iterator itDIFp=dim.begin();itDIFp!=dim.end();itDIFp++)
    {
      Dif* itDIF= (Dif*) (*itDIFp);

      //      std::cout<<"DIF found "<<itDIF->getInt("ID")<<std::endl;
      if (itDIF->getInt("ID")!=difid) continue;
      v->Masked=(itDIF->getInt("MASK")!=0);
      v->MonitoredChannel = itDIF->getInt("MONITORED_CHANNEL");
      v->MonitorSequencer = itDIF->getInt("MONITOR_SEQUENCER");
      v->NumericalReadoutMode= itDIF->getInt("NUMERICAL_READOUT_MODE");
      v->NumericalReadoutStartMode= itDIF->getInt("NUMERICAL_READOUT_START_MODE");
      v->AVDDShdn= itDIF->getInt("AVDD_SHDN");
      v->DVDDShdn= itDIF->getInt("DVDD_SHDN");
      v->PowerAnalog= itDIF->getInt("POWER_ANALOG");
      v->PowerDigital= itDIF->getInt("POWER_DIGITAL");
      v->PowerDAC= itDIF->getInt("POWER_DAC");
      v->PowerADC= itDIF->getInt("POWER_ADC");
      v->PowerSS= itDIF->getInt("POWER_SS");
      v->TimerHoldRegister= itDIF->getInt("TIMER_HOLD_REGISTER");
      v->EnableMonitoring= itDIF->getInt("ENABLE_MONITORING");
      v->MemoryDisplayLimitMax= itDIF->getInt("MEMORY_DISPLAY_LIMIT_MAX");
      v->MemoryDisplayLimitMin= itDIF->getInt("MEMORY_DISPLAY_LIMIT_MIN");
      v->MemoryEfficiencyLimitMax= itDIF->getInt("MEMORY_EFFICIENCY_LIMIT_MAX");
      v->MemoryEfficiencyLimitMin= itDIF->getInt("MEMORY_EFFICIENCY_LIMIT_MIN");
      v->DIFIMonGain= itDIF->getInt("DIF_IMON_GAIN");
      v->SlabIMonGain= itDIF->getInt("SLAB_IMON_GAIN");
				
      if (v->DIFIMonGain==50) v->DIFIMonGain=0;		else v->DIFIMonGain=1;
      if (v->SlabIMonGain==50) v->SlabIMonGain=0;	else v->SlabIMonGain=1;
      if (	v->MemoryDisplayLimitMax==-1) v->MemoryDisplayLimitMax =  INT_MAX;
      if (	v->MemoryDisplayLimitMin==1) v->MemoryDisplayLimitMin = - INT_MAX;
      if (	v->MemoryEfficiencyLimitMax==-1) v->MemoryEfficiencyLimitMax =  INT_MAX;
      if (	v->MemoryEfficiencyLimitMin==1) v->MemoryEfficiencyLimitMin = - INT_MAX;
#undef  DEBUG_DB
#ifdef DEBUG_DB		
      printf ("************** DIF parameters **************\n");
      printf ("			v->MonitoredChannel = %d\n",v->MonitoredChannel);
      printf ("			CurrentMonitorSequencer = %d\n",v->MonitorSequencer);
      printf ("			v->NumericalReadoutMode = %d\n",v->NumericalReadoutMode);
      printf ("			CurrentNumericalReadoutStartMode = %d\n",v->NumericalReadoutStartMode);
      printf ("			v->AVDDShdn = %d\n",v->AVDDShdn);
      printf ("			CurrentDVDDShdn = %d\n",v->DVDDShdn);
      printf ("			CurrentPowerAnalog = %d\n",v->PowerAnalog);
      printf ("			CurrentPowerDigital = %d\n",v->PowerDigital);
      printf ("			CurrentPowerDAC = %d\n",v->PowerDAC);
      printf ("			CurrentPowerADC = %d\n",v->PowerADC);
      printf ("			CurrentPowerSS = %d\n",v->PowerSS);
      printf ("			CurrentTimerHoldRegister = %d\n",v->TimerHoldRegister);
      printf ("			CurrentMonitoringStatus = %d\n",v->EnableMonitoring);
      printf ("			CurrentMemoryDisplayLimitMax = %d\n",v->MemoryDisplayLimitMax);
      printf ("			CurrentMemoryDisplayLimitMin = %d\n",v->MemoryDisplayLimitMin);
      printf ("			CurrentMemoryEfficiencyLimitMax = %d\n",v->MemoryEfficiencyLimitMax);
      printf ("			CurrentMemoryEfficiencyLimitMin = %d\n",v->MemoryEfficiencyLimitMin);
      printf ("			CurrentDifIMonGain = %d\n",v->DIFIMonGain);
      printf ("			CurrentSlabIMonGain = %d\n\n",v->SlabIMonGain);
#endif				

    }//for (std::map<uint32_t,DIFDefCpp*>::iterator itDIF=dim.begin();itDIF!=dim.end();itDIF++)
  return;
}
#undef BUGINDB
uint32_t lydaq::OracleDIFDBInterface::LoadDIFHardrocV2Parameters(uint32_t difid_request,SingleHardrocV2ConfigurationFrame* ConfigHR2)
{
  uint8_t thr=0;
	uint8_t thrl1=0;
	uint8_t thrl2=0;
	uint8_t thrl3=0;
	uint8_t thrl4=0;
	
  std::cout<<"On initialise Oracle ASICs "<<std::endl;
  LOG4CXX_INFO(_logOracle," Filling vector ");
  uint32_t difid=difid_request;
#ifdef BUGINDB
  if (difid==72) difid=27;
#endif
  std::vector<ConfigObject*> asic_vector_=_state->getAsicConfiguration()->getVector();
  for (int iasic=0;iasic<MAX_NB_OF_ASICS;iasic++)
    {

      for (std::vector<ConfigObject*>::iterator itHR2p=asic_vector_.begin();itHR2p!=asic_vector_.end();itHR2p++)
	{
	  Asic* itHR2=(Asic*) (*itHR2p);
	  if (itHR2->getInt("DIF_ID")!=difid) continue;

	  if (itHR2->getInt("ENABLED")==0) continue;      
	  // xml conf file stores hardroc headers in chain order
	  try 
	    {
	      if (	(iasic+1)==itHR2->getInt("HEADER"))
		{
		  printf ("%d ",itHR2->getInt("HEADER"));
		  unsigned long long tmask0;
		  unsigned long long tmask1;
		  unsigned long long tmask2;
		  sscanf(itHR2->getString("MASK0").c_str(),"%llx\n",&tmask0);	
		  sscanf(itHR2->getString("MASK1").c_str(),"%llx\n",&tmask1);	
		  sscanf(itHR2->getString("MASK2").c_str(),"%llx\n",&tmask2);

		  //	      printf(" Header %d Mask0 %llx \n",itHR2->getInt("HEADER"),tmask0);
		  for (int32_t ib=0;ib<HARDROCV2_SLC_FRAME_SIZE;ib++)
		    ConfigHR2[thr][ib]=0;

		  ConfigHR2[thr][0]|=((itHR2->getInt("ENOCDOUT1B")&0x01)<<7);
		  ConfigHR2[thr][0]|=((itHR2->getInt("ENOCDOUT2B")&0x01)<<6);
		  ConfigHR2[thr][0]|=((itHR2->getInt("ENOCTRANSMITON1B")&0x01)<<5);
		  ConfigHR2[thr][0]|=((itHR2->getInt("ENOCTRANSMITON2B")&0x01)<<4);
		  ConfigHR2[thr][0]|=((itHR2->getInt("ENOCCHIPSATB")&0x01)<<3);
		  ConfigHR2[thr][0]|=((itHR2->getInt("SELSTARTREADOUT")&0x01)<<2);
		  ConfigHR2[thr][0]|=((itHR2->getInt("SELENDREADOUT")&0x01)<<1);
		  ConfigHR2[thr][1]|=((itHR2->getInt("CLKMUX")&0x01)<<4);
		  ConfigHR2[thr][1]|=((itHR2->getInt("SCON")&0x01)<<3);
		  ConfigHR2[thr][1]|=((itHR2->getInt("RAZCHNEXTVAL")&0x01)<<2);
		  ConfigHR2[thr][1]|=((itHR2->getInt("RAZCHNINTVAL")&0x01)<<1);
		  ConfigHR2[thr][1]|=((itHR2->getInt("TRIGEXTVAL")&0x01)<<0);
		  ConfigHR2[thr][2]|=((itHR2->getInt("DISCROROR")&0x01)<<7);
		  ConfigHR2[thr][2]|=((itHR2->getInt("ENTRIGOUT")&0x01)<<6);
		  ConfigHR2[thr][2]|=((itHR2->getInt("TRIG0B")&0x01)<<5);
		  ConfigHR2[thr][2]|=((itHR2->getInt("TRIG1B")&0x01)<<4);
		  ConfigHR2[thr][2]|=((itHR2->getInt("TRIG2B")&0x01)<<3);
		  ConfigHR2[thr][2]|=((itHR2->getInt("OTABGSW")&0x01)<<2);
		  ConfigHR2[thr][2]|=((itHR2->getInt("DACSW")&0x01)<<1);
		  ConfigHR2[thr][2]|=((itHR2->getInt("SMALLDAC")&0x01)<<0);
		  ConfigHR2[thr][3]= ((itHR2->getInt("B2")>>2)&0xFF);
		  ConfigHR2[thr][4]|=((itHR2->getInt("B2")&0x03)<<6);
		  ConfigHR2[thr][4]|=((itHR2->getInt("B1")>>4)&0x3F);
		  ConfigHR2[thr][5]|=((itHR2->getInt("B1")&0x0F)<<4);
		  ConfigHR2[thr][5]|=((itHR2->getInt("B0")>>6)&0x0F);
		  ConfigHR2[thr][6]|=((itHR2->getInt("B0")&0x3F)<<2);
		  ConfigHR2[thr][6]|=(itHR2->getInt("HEADER")&0x01)<<1;
		  ConfigHR2[thr][6]|=(itHR2->getInt("HEADER")&0x02)>>1;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x04)<<5;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x08)<<3;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x10)<<1;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x20)>>1;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x40)>>3;
		  ConfigHR2[thr][7]|=(itHR2->getInt("HEADER")&0x80)>>5;

		  ConfigHR2[thr][7]|=((tmask2&0x8000000000000000llu)>>62); //63	ok
		  ConfigHR2[thr][8]|=((tmask2&0x4000000000000000llu)>>56); //62	ok
		  ConfigHR2[thr][8]|=((tmask2&0x2000000000000000llu)>>58); //61	ok
		  ConfigHR2[thr][8]|=((tmask2&0x1000000000000000llu)>>60); //60	ok
		  ConfigHR2[thr][9]|=((tmask2&0x0800000000000000llu)>>54); //59	ok
		  ConfigHR2[thr][9]|=((tmask2&0x0400000000000000llu)>>56); //58	ok
		  ConfigHR2[thr][10]|=((tmask2&0x0200000000000000llu)>>50); //57	ok
		  ConfigHR2[thr][10]|=((tmask2&0x0100000000000000llu)>>52); //56	ok
		  ConfigHR2[thr][10]|=((tmask2&0x0080000000000000llu)>>54); //55	ok
		  ConfigHR2[thr][11]|=((tmask2&0x0040000000000000llu)>>48); //54	ok
		  ConfigHR2[thr][11]|=((tmask2&0x0020000000000000llu)>>50); //53	ok
		  ConfigHR2[thr][11]|=((tmask2&0x0010000000000000llu)>>52); //52	ok
		  ConfigHR2[thr][12]|=((tmask2&0x0008000000000000llu)>>46); //51	ok
		  ConfigHR2[thr][12]|=((tmask2&0x0004000000000000llu)>>48); //50	ok
		  ConfigHR2[thr][13]|=((tmask2&0x0002000000000000llu)>>42); //49	ok
		  ConfigHR2[thr][13]|=((tmask2&0x0001000000000000llu)>>44); //48	ok
		  ConfigHR2[thr][13]|=((tmask2&0x0000800000000000llu)>>46); //47	ok
		  ConfigHR2[thr][14]|=((tmask2&0x0000400000000000llu)>>40); //46	ok
		  ConfigHR2[thr][14]|=((tmask2&0x0000200000000000llu)>>42); //45	ok
		  ConfigHR2[thr][14]|=((tmask2&0x0000100000000000llu)>>44); //44	ok
		  ConfigHR2[thr][15]|=((tmask2&0x0000080000000000llu)>>38); //43	ok
		  ConfigHR2[thr][15]|=((tmask2&0x0000040000000000llu)>>40); //42	ok
		  ConfigHR2[thr][16]|=((tmask2&0x0000020000000000llu)>>34); //41 ok	
		  ConfigHR2[thr][16]|=((tmask2&0x0000010000000000llu)>>36); //40	ok
		  ConfigHR2[thr][16]|=((tmask2&0x0000008000000000llu)>>38); //39	ok
		  ConfigHR2[thr][17]|=((tmask2&0x0000004000000000llu)>>32); //38	ok
		  ConfigHR2[thr][17]|=((tmask2&0x0000002000000000llu)>>34); //37	ok
		  ConfigHR2[thr][17]|=((tmask2&0x0000001000000000llu)>>36); //36	ok
		  ConfigHR2[thr][18]|=((tmask2&0x0000000800000000llu)>>30); //35	ok
		  ConfigHR2[thr][18]|=((tmask2&0x0000000400000000llu)>>32); //34	ok
		  ConfigHR2[thr][19]|=((tmask2&0x0000000200000000llu)>>26); //33	ok
		  ConfigHR2[thr][19]|=((tmask2&0x0000000100000000llu)>>28); //32	ok 
		  ConfigHR2[thr][19]|=((tmask2&0x0000000080000000llu)>>30); //31	ok
		  ConfigHR2[thr][20]|=((tmask2&0x0000000040000000llu)>>24); //30	ok
		  ConfigHR2[thr][20]|=((tmask2&0x0000000020000000llu)>>26); //29	ok
		  ConfigHR2[thr][20]|=((tmask2&0x0000000010000000llu)>>28); //28	ok
		  ConfigHR2[thr][21]|=((tmask2&0x0000000008000000llu)>>22); //27	ok
		  ConfigHR2[thr][21]|=((tmask2&0x0000000004000000llu)>>24); //26	ok
		  ConfigHR2[thr][22]|=((tmask2&0x0000000002000000llu)>>18); //25	ok
		  ConfigHR2[thr][22]|=((tmask2&0x0000000001000000llu)>>20); //24	ok
		  ConfigHR2[thr][22]|=((tmask2&0x0000000000800000llu)>>22); //23	ok
		  ConfigHR2[thr][23]|=((tmask2&0x0000000000400000llu)>>16); //22	ok
		  ConfigHR2[thr][23]|=((tmask2&0x0000000000200000llu)>>18); //21	ok 
		  ConfigHR2[thr][23]|=((tmask2&0x0000000000100000llu)>>20); //20	ok
		  ConfigHR2[thr][24]|=((tmask2&0x0000000000080000llu)>>14); //19	ok
		  ConfigHR2[thr][24]|=((tmask2&0x0000000000040000llu)>>16); //18	ok
		  ConfigHR2[thr][25]|=((tmask2&0x0000000000020000llu)>>10); //17	ok
		  ConfigHR2[thr][25]|=((tmask2&0x0000000000010000llu)>>12); //16	ok
		  ConfigHR2[thr][25]|=((tmask2&0x0000000000008000llu)>>14); //15 ok
		  ConfigHR2[thr][26]|=((tmask2&0x0000000000004000llu)>>8);//14 ok
		  ConfigHR2[thr][26]|=((tmask2&0x0000000000002000llu)>>10); //13 ok
		  ConfigHR2[thr][26]|=((tmask2&0x0000000000001000llu)>>12); //12 ok
		  ConfigHR2[thr][27]|=((tmask2&0x0000000000000800llu)>>6);//11 ok
		  ConfigHR2[thr][27]|=((tmask2&0x0000000000000400llu)>>8);//10 ok
		  ConfigHR2[thr][28]|=((tmask2&0x0000000000000200llu)>>2);//9 ok
		  ConfigHR2[thr][28]|=((tmask2&0x0000000000000100llu)>>4);//8 ok
		  ConfigHR2[thr][28]|=((tmask2&0x0000000000000080llu)>>6);//7 ok
		  ConfigHR2[thr][29]|=((tmask2&0x0000000000000040llu)>>0);//6 ok
		  ConfigHR2[thr][29]|=((tmask2&0x0000000000000020llu)>>2);//5 ok
		  ConfigHR2[thr][29]|=((tmask2&0x0000000000000010llu)>>4);//4 ok
		  ConfigHR2[thr][30]|=((tmask2&0x0000000000000008llu)<<2);//3 ok
		  ConfigHR2[thr][30]|=((tmask2&0x0000000000000004llu)<<0);//2 ok
		  ConfigHR2[thr][31]|=((tmask2&0x0000000000000002llu)<<6);//1 ok
		  ConfigHR2[thr][31]|=((tmask2&0x0000000000000001llu)<<4);//0 ok

		  ConfigHR2[thr][7]|= ((tmask1&0x8000000000000000llu)>>63); //63 ok		
		  ConfigHR2[thr][8]|= ((tmask1&0x4000000000000000llu)>>57); //62	ok	
		  ConfigHR2[thr][8]|= ((tmask1&0x2000000000000000llu)>>59); //61	ok	
		  ConfigHR2[thr][9]|= ((tmask1&0x1000000000000000llu)>>53); //60	ok	
		  ConfigHR2[thr][9]|= ((tmask1&0x0800000000000000llu)>>55); //59	ok	
		  ConfigHR2[thr][9]|= ((tmask1&0x0400000000000000llu)>>57); //58	ok	
		  ConfigHR2[thr][10]|=((tmask1&0x0200000000000000llu)>>51); //57	ok	
		  ConfigHR2[thr][10]|=((tmask1&0x0100000000000000llu)>>53); //56		ok
		  ConfigHR2[thr][10]|=((tmask1&0x0080000000000000llu)>>55); //55	ok	
		  ConfigHR2[thr][11]|=((tmask1&0x0040000000000000llu)>>49); //54		ok
		  ConfigHR2[thr][11]|=((tmask1&0x0020000000000000llu)>>51); //53	ok	
		  ConfigHR2[thr][12]|=((tmask1&0x0010000000000000llu)>>45); //52		ok
		  ConfigHR2[thr][12]|=((tmask1&0x0008000000000000llu)>>47); //51		ok
		  ConfigHR2[thr][12]|=((tmask1&0x0004000000000000llu)>>49); //50		ok
		  ConfigHR2[thr][13]|=((tmask1&0x0002000000000000llu)>>43); //49		ok
		  ConfigHR2[thr][13]|=((tmask1&0x0001000000000000llu)>>45); //48	ok	
		  ConfigHR2[thr][13]|=((tmask1&0x0000800000000000llu)>>47); //47	ok	
		  ConfigHR2[thr][14]|=((tmask1&0x0000400000000000llu)>>41); //46		ok
		  ConfigHR2[thr][14]|=((tmask1&0x0000200000000000llu)>>43); //45		ok
		  ConfigHR2[thr][15]|=((tmask1&0x0000100000000000llu)>>37); //44		ok
		  ConfigHR2[thr][15]|=((tmask1&0x0000080000000000llu)>>39); //43		ok
		  ConfigHR2[thr][15]|=((tmask1&0x0000040000000000llu)>>41); //42		ok
		  ConfigHR2[thr][16]|=((tmask1&0x0000020000000000llu)>>35); //41		ok
		  ConfigHR2[thr][16]|=((tmask1&0x0000010000000000llu)>>37); //40		ok
		  ConfigHR2[thr][16]|=((tmask1&0x0000008000000000llu)>>39);  //39	ok	
		  ConfigHR2[thr][17]|=((tmask1&0x0000004000000000llu)>>33); //38		ok
		  ConfigHR2[thr][17]|=((tmask1&0x0000002000000000llu)>>35); //37		ok
		  ConfigHR2[thr][18]|=((tmask1&0x0000001000000000llu)>>29); //36		ok
		  ConfigHR2[thr][18]|=((tmask1&0x0000000800000000llu)>>31); //35		ok
		  ConfigHR2[thr][18]|=((tmask1&0x0000000400000000llu)>>33); //34		ok
		  ConfigHR2[thr][19]|=((tmask1&0x0000000200000000llu)>>27); //33		ok
		  ConfigHR2[thr][19]|=((tmask1&0x0000000100000000llu)>>29); //32		ok
		  ConfigHR2[thr][19]|=((tmask1&0x0000000080000000llu)>>31); //31	ok	
		  ConfigHR2[thr][20]|=((tmask1&0x0000000040000000llu)>>25); //30		ok
		  ConfigHR2[thr][20]|=((tmask1&0x0000000020000000llu)>>27); //29		ok
		  ConfigHR2[thr][21]|=((tmask1&0x0000000010000000llu)>>21); //28		ok
		  ConfigHR2[thr][21]|=((tmask1&0x0000000008000000llu)>>23); //27		ok
		  ConfigHR2[thr][21]|=((tmask1&0x0000000004000000llu)>>25); //26		ok
		  ConfigHR2[thr][22]|=((tmask1&0x0000000002000000llu)>>19); //25		ok
		  ConfigHR2[thr][22]|=((tmask1&0x0000000001000000llu)>>21); //24		ok
		  ConfigHR2[thr][22]|=((tmask1&0x0000000000800000llu)>>23); //23	ok	
		  ConfigHR2[thr][23]|=((tmask1&0x0000000000400000llu)>>17); //22		ok
		  ConfigHR2[thr][23]|=((tmask1&0x0000000000200000llu)>>19); //21		ok
		  ConfigHR2[thr][24]|=((tmask1&0x0000000000100000llu)>>13); //20		ok
		  ConfigHR2[thr][24]|=((tmask1&0x0000000000080000llu)>>15); //19		ok
		  ConfigHR2[thr][24]|=((tmask1&0x0000000000040000llu)>>17); //18		ok
		  ConfigHR2[thr][25]|=((tmask1&0x0000000000020000llu)>>11); //17		ok
		  ConfigHR2[thr][25]|=((tmask1&0x0000000000010000llu)>>13); //16		ok
		  ConfigHR2[thr][25]|=((tmask1&0x0000000000008000llu)>>15); //15	ok	
		  ConfigHR2[thr][26]|=((tmask1&0x0000000000004000llu)>>9); //14	ok
		  ConfigHR2[thr][26]|=((tmask1&0x0000000000002000llu)>>11); 	//13		ok
		  ConfigHR2[thr][27]|=((tmask1&0x0000000000001000llu)>>5); //12	ok
		  ConfigHR2[thr][27]|=((tmask1&0x0000000000000800llu)>>7); //11	ok
		  ConfigHR2[thr][27]|=((tmask1&0x0000000000000400llu)>>9); //10	ok
		  ConfigHR2[thr][28]|=((tmask1&0x0000000000000200llu)>>3); //9	ok
		  ConfigHR2[thr][28]|=((tmask1&0x0000000000000100llu)>>5); //8	ok
		  ConfigHR2[thr][28]|=((tmask1&0x0000000000000080llu)>>7); //7	ok
		  ConfigHR2[thr][29]|=((tmask1&0x0000000000000040llu)>>1); //6	ok
		  ConfigHR2[thr][29]|=((tmask1&0x0000000000000020llu)>>3); //5	ok
		  ConfigHR2[thr][30]|=((tmask1&0x0000000000000010llu)<<3); //4	 ok
		  ConfigHR2[thr][30]|=((tmask1&0x0000000000000008llu)<<1); //3	ok
		  ConfigHR2[thr][30]|=((tmask1&0x0000000000000004llu)>>1); //2	ok
		  ConfigHR2[thr][31]|=((tmask1&0x0000000000000002llu)<<5); //1	ok
		  ConfigHR2[thr][31]|=((tmask1&0x0000000000000001llu)<<3); //0	ok
	
		  ConfigHR2[thr][8]|= ((tmask0&0x8000000000000000llu)>>56); //63	ok
		  ConfigHR2[thr][8]|= ((tmask0&0x4000000000000000llu)>>58); //62	ok
		  ConfigHR2[thr][8]|= ((tmask0&0x2000000000000000llu)>>60); //61	ok
		  ConfigHR2[thr][9]|= ((tmask0&0x1000000000000000llu)>>54); //60	ok
		  ConfigHR2[thr][9]|= ((tmask0&0x0800000000000000llu)>>56); //59	ok
		  ConfigHR2[thr][9]|= ((tmask0&0x0400000000000000llu)>>58); //58ok	
		  ConfigHR2[thr][10]|=((tmask0&0x0200000000000000llu)>>52); //57	ok
		  ConfigHR2[thr][10]|=((tmask0&0x0100000000000000llu)>>54); //56	ok
		  ConfigHR2[thr][11]|=((tmask0&0x0080000000000000llu)>>48); //55	ok
		  ConfigHR2[thr][11]|=((tmask0&0x0040000000000000llu)>>50); //54	ok
		  ConfigHR2[thr][11]|=((tmask0&0x0020000000000000llu)>>52); //53	ok
		  ConfigHR2[thr][12]|=((tmask0&0x0010000000000000llu)>>46); //52	ok
		  ConfigHR2[thr][12]|=((tmask0&0x0008000000000000llu)>>48); //51	ok
		  ConfigHR2[thr][12]|=((tmask0&0x0004000000000000llu)>>50); //50	ok
		  ConfigHR2[thr][13]|=((tmask0&0x0002000000000000llu)>>44); //49	ok
		  ConfigHR2[thr][13]|=((tmask0&0x0001000000000000llu)>>46); //48	ok
		  ConfigHR2[thr][14]|=((tmask0&0x0000800000000000llu)>>40); //47	ok
		  ConfigHR2[thr][14]|=((tmask0&0x0000400000000000llu)>>42); //46	ok
		  ConfigHR2[thr][14]|=((tmask0&0x0000200000000000llu)>>44); //45	ok
		  ConfigHR2[thr][15]|=((tmask0&0x0000100000000000llu)>>38); //44	ok
		  ConfigHR2[thr][15]|=((tmask0&0x0000080000000000llu)>>40); //43	ok
		  ConfigHR2[thr][15]|=((tmask0&0x0000040000000000llu)>>42); //42	ok
		  ConfigHR2[thr][16]|=((tmask0&0x0000020000000000llu)>>36); //41	ok
		  ConfigHR2[thr][16]|=((tmask0&0x0000010000000000llu)>>38); //40	ok
		  ConfigHR2[thr][17]|=((tmask0&0x0000008000000000llu)>>32); //39	ok
		  ConfigHR2[thr][17]|=((tmask0&0x0000004000000000llu)>>34); //38	
		  ConfigHR2[thr][17]|=((tmask0&0x0000002000000000llu)>>36); //37	ok
		  ConfigHR2[thr][18]|=((tmask0&0x0000001000000000llu)>>30); //36	ok
		  ConfigHR2[thr][18]|=((tmask0&0x0000000800000000llu)>>32); //35	ok
		  ConfigHR2[thr][18]|=((tmask0&0x0000000400000000llu)>>34); //34ok	
		  ConfigHR2[thr][19]|=((tmask0&0x0000000200000000llu)>>28); //33	ok
		  ConfigHR2[thr][19]|=((tmask0&0x0000000100000000llu)>>30); //32	ok
		  ConfigHR2[thr][20]|=((tmask0&0x0000000080000000llu)>>24); //31	ok
		  ConfigHR2[thr][20]|=((tmask0&0x0000000040000000llu)>>26); //30	ok
		  ConfigHR2[thr][20]|=((tmask0&0x0000000020000000llu)>>28); //29	ok
		  ConfigHR2[thr][21]|=((tmask0&0x0000000010000000llu)>>22); //28	ok
		  ConfigHR2[thr][21]|=((tmask0&0x0000000008000000llu)>>24); //27	ok
		  ConfigHR2[thr][21]|=((tmask0&0x0000000004000000llu)>>26); //26	ok
		  ConfigHR2[thr][22]|=((tmask0&0x0000000002000000llu)>>20); //25	ok
		  ConfigHR2[thr][22]|=((tmask0&0x0000000001000000llu)>>22); //24	ok
		  ConfigHR2[thr][23]|=((tmask0&0x0000000000800000llu)>>16); //23	ok
		  ConfigHR2[thr][23]|=((tmask0&0x0000000000400000llu)>>18); //22	ok
		  ConfigHR2[thr][23]|=((tmask0&0x0000000000200000llu)>>20); //21	ok
		  ConfigHR2[thr][24]|=((tmask0&0x0000000000100000llu)>>14); //20	ok
		  ConfigHR2[thr][24]|=((tmask0&0x0000000000080000llu)>>16); //19	ok
		  ConfigHR2[thr][24]|=((tmask0&0x0000000000040000llu)>>18); //18	ok
		  ConfigHR2[thr][25]|=((tmask0&0x0000000000020000llu)>>12); //17	ok
		  ConfigHR2[thr][25]|=((tmask0&0x0000000000010000llu)>>14); //16	ok
		  ConfigHR2[thr][26]|=((tmask0&0x0000000000008000llu)>>8);//15ok
		  ConfigHR2[thr][26]|=((tmask0&0x0000000000004000llu)>>10); //14	ok
		  ConfigHR2[thr][26]|=((tmask0&0x0000000000002000llu)>>12); 		//13	ok
		  ConfigHR2[thr][27]|=((tmask0&0x0000000000001000llu)>>6);//12 ok
		  ConfigHR2[thr][27]|=((tmask0&0x0000000000000800llu)>>8);//11ok
		  ConfigHR2[thr][27]|=((tmask0&0x0000000000000400llu)>>10); //10ok
		  ConfigHR2[thr][28]|=((tmask0&0x0000000000000200llu)>>4);//9ok
		  ConfigHR2[thr][28]|=((tmask0&0x0000000000000100llu)>>6);//8ok
		  ConfigHR2[thr][29]|=((tmask0&0x0000000000000080llu)>>0);//7ok
		  ConfigHR2[thr][29]|=((tmask0&0x0000000000000040llu)>>2);//6ok
		  ConfigHR2[thr][29]|=((tmask0&0x0000000000000020llu)>>4);//5ok
		  ConfigHR2[thr][30]|=((tmask0&0x0000000000000010llu)<<2);//4 ok
		  ConfigHR2[thr][30]|=((tmask0&0x0000000000000008llu)<<0);//3ok
		  ConfigHR2[thr][30]|=((tmask0&0x0000000000000004llu)>>2);//2 ok
		  ConfigHR2[thr][31]|=((tmask0&0x0000000000000002llu)<<4);//1 ok
		  ConfigHR2[thr][31]|=((tmask0&0x0000000000000001llu)<<2);//0ok
		  ConfigHR2[thr][31]|=((itHR2->getInt("RS_OR_DISCRI")&0x01)<<1);
		  ConfigHR2[thr][31]|=((itHR2->getInt("DISCRI1")&0x01)<<0);
		  ConfigHR2[thr][32]|=((itHR2->getInt("DISCRI2")&0x01)<<7);
		  ConfigHR2[thr][32]|=((itHR2->getInt("DISCRI0")&0x01)<<6);
		  ConfigHR2[thr][32]|=((itHR2->getInt("OTAQ_PWRADC")&0x01)<<5);
		  ConfigHR2[thr][32]|=((itHR2->getInt("EN_OTAQ")&0x01)<<4);
		  ConfigHR2[thr][32]|=((itHR2->getInt("SW50F0")&0x01)<<3);
		  ConfigHR2[thr][32]|=((itHR2->getInt("SW100F0")&0x01)<<2);
		  ConfigHR2[thr][32]|=((itHR2->getInt("SW100K0")&0x01)<<1);
		  ConfigHR2[thr][32]|=((itHR2->getInt("SW50K0")&0x01)<<0);
		  ConfigHR2[thr][33]|=((itHR2->getInt("PWRONFSB1")&0x01)<<7);
		  ConfigHR2[thr][33]|=((itHR2->getInt("PWRONFSB2")&0x01)<<6);
		  ConfigHR2[thr][33]|=((itHR2->getInt("PWRONFSB0")&0x01)<<5);
		  ConfigHR2[thr][33]|=((itHR2->getInt("SEL1")&0x01)<<4);
		  ConfigHR2[thr][33]|=((itHR2->getInt("SEL0")&0x01)<<3);
		  ConfigHR2[thr][33]|=((itHR2->getInt("SW50F1")&0x01)<<2);
		  ConfigHR2[thr][33]|=((itHR2->getInt("SW100F1")&0x01)<<1);
		  ConfigHR2[thr][33]|=((itHR2->getInt("SW100K1")&0x01)<<0);
		  ConfigHR2[thr][34]|=((itHR2->getInt("SW50K1")&0x01)<<7);
		  ConfigHR2[thr][34]|=((itHR2->getInt("CMDB0FSB1")&0x01)<<6);
		  ConfigHR2[thr][34]|=((itHR2->getInt("CMDB1FSB1")&0x01)<<5);
		  ConfigHR2[thr][34]|=((itHR2->getInt("CMDB2FSB1")&0x01)<<4);
		  ConfigHR2[thr][34]|=((itHR2->getInt("CMDB3FSB1")&0x01)<<3);
		  ConfigHR2[thr][34]|=((itHR2->getInt("SW50F2")&0x01)<<2);
		  ConfigHR2[thr][34]|=((itHR2->getInt("SW100F2")&0x01)<<1);
		  ConfigHR2[thr][34]|=((itHR2->getInt("SW100K2")&0x01)<<0);
		  ConfigHR2[thr][35]|=((itHR2->getInt("SW50K2")&0x01)<<7);
		  ConfigHR2[thr][35]|=((itHR2->getInt("CMDB0FSB2")&0x01)<<6);
		  ConfigHR2[thr][35]|=((itHR2->getInt("CMDB1FSB2")&0x01)<<5);
		  ConfigHR2[thr][35]|=((itHR2->getInt("CMDB2FSB2")&0x01)<<4);
		  ConfigHR2[thr][35]|=((itHR2->getInt("CMDB3FSB2")&0x01)<<3);
		  ConfigHR2[thr][35]|=((itHR2->getInt("PWRONW")&0x01)<<2);
		  ConfigHR2[thr][35]|=((itHR2->getInt("PWRONSS")&0x01)<<1);
		  ConfigHR2[thr][35]|=((itHR2->getInt("PWRONBUFF")&0x01)<<0);
		  ConfigHR2[thr][36]|=((itHR2->getInt("SWSSC")&0x07)<<5);
		  ConfigHR2[thr][36]|=((itHR2->getInt("CMDB0SS")&0x01)<<4);
		  ConfigHR2[thr][36]|=((itHR2->getInt("CMDB1SS")&0x01)<<3);
		  ConfigHR2[thr][36]|=((itHR2->getInt("CMDB2SS")&0x01)<<2);
		  ConfigHR2[thr][36]|=((itHR2->getInt("CMDB3SS")&0x01)<<1);
		  ConfigHR2[thr][36]|=((itHR2->getInt("PWRONPA")&0x01)<<0);
		  std::vector<int> gain=itHR2->getIntVector("PAGAIN");
		  for (uint32_t ip=0;ip<64;ip++)
		    ConfigHR2[thr][100-ip]|=(gain[ip]&0xFF);
		  unsigned long long tctest;
		  sscanf(itHR2->getString("CTEST").c_str(),"%llx\n",&tctest);
		  tctest=0;

		  ConfigHR2[thr][101]=((tctest&0xFF00000000000000llu)>>56);
		  ConfigHR2[thr][102]=((tctest&0x00FF000000000000llu)>>48);
		  ConfigHR2[thr][103]=((tctest&0x0000FF0000000000llu)>>40);
		  ConfigHR2[thr][104]=((tctest&0x000000FF00000000llu)>>32);
		  ConfigHR2[thr][105]=((tctest&0x00000000FF000000llu)>>24);
		  ConfigHR2[thr][106]=((tctest&0x0000000000FF0000llu)>>16);
		  ConfigHR2[thr][107]=((tctest&0x000000000000FF00llu)>>8);
		  ConfigHR2[thr][108]=((tctest&0x00000000000000FFllu)>>0);
#undef DEBUG_DB
#ifdef DEBUG_DB
		  printf ("HR2 %d : ",thr);
		  for (int i=0;i<109;i++)
		    printf ("%02x",ConfigHR2[thr][i]);
		  printf ("\n");
#endif
			uint8_t tamponl=0;
		  thr++;
			if (thr%8==0) tamponl=1;
			else if (thr%8<5) tamponl=(thr%8);
			else tamponl=9-thr%8;
			if (tamponl==1) thrl1++;
			else if (tamponl==2) thrl2++;
			else if (tamponl==3) thrl3++;
			else if (tamponl==4) thrl4++;
			
			
		}//			if (	ASICHeaders[thr]==itHR2->getInt("ID"))
	    } catch (Exception e)
	    {
	      LOG4CXX_ERROR(_logOracle," Error in ASIC access"<<e.getMessage());
	      std::cout<<e.getMessage()<<std::endl;
	    }
	}		//			for (int thr=0;thr<NbOfASICs;thr++)
	
    }//  for (std::map<uint32_t,HR2DefCpp*>::iterator itHR2=dim.begin();itHR2!=dim.end();itHR2++)
 //   printf ("\nthr=%d\n",thr);
  return (thrl1&0xFF)+((thrl2&0xFF)<<8)+((thrl3&0xFF)<<16)+((thrl4&0xFF)<<24);
}		



uint32_t lydaq::OracleDIFDBInterface::LoadDIFMicrorocParameters(uint32_t difid, SingleHardrocV2ConfigurationFrame* ConfigMR)
{
  uint8_t tmr=0;  // thr
	uint8_t tmrl1=0;
	uint8_t tmrl2=0;
	uint8_t tmrl3=0;
	uint8_t tmrl4=0;
  std::vector<ConfigObject*> asic_vector_=_state->getAsicConfiguration()->getVector();
  for (int iasic=0;iasic<MAX_NB_OF_ASICS;iasic++)
    {

      for (std::vector<ConfigObject*>::iterator itMRp=asic_vector_.begin();itMRp!=asic_vector_.end();itMRp++)
	{
	  Asic* itMR=(Asic*) (*itMRp);
	  if (itMR->getInt("DIF_ID")!=difid) continue;
	  if (itMR->getInt("ENABLED")==0) continue;      
	  // xml conf file stores microroc headers in chain order
	  if (itMR->getInt("ENABLED")==0) continue;      
	  try 
	    {
	      if (	(iasic+1)==itMR->getInt("HEADER"))
		{
		  printf ("%d ",itMR->getInt("HEADER"));
		  unsigned long long mask0;
		  unsigned long long mask1;
		  unsigned long long mask2;
		  unsigned long long cTest;
//		  std::string tB0B3;
		  unsigned int tBB0;
		  unsigned int tBB1;
		  unsigned int tBB2;

		  sscanf(itMR->getString("MASK0").c_str(),"%llx\n",&mask0);				
		  sscanf(itMR->getString("MASK1").c_str(),"%llx\n",&mask1);				
		  sscanf(itMR->getString("MASK2").c_str(),"%llx\n",&mask2);		
		  sscanf(itMR->getString("CTEST").c_str(),"%llx\n",&cTest);		
//		  tB0B3 =itMR->getString("B0B3");
		  tBB0 =itMR->getInt("BB0");
		  tBB1 =itMR->getInt("BB1");
		  tBB2 =itMR->getInt("BB2");

		  tBB2= ~tBB2;							
		  tBB1= ~tBB1;							
		  tBB0= ~tBB0;							
		  int tch=0;
//		  int B0B3Gain[64];


		  std::vector<int> B0B3Gain=itMR->getIntVector("B0B3");
					
		  ConfigMR[tmr][0]=((itMR->getInt("ENOCDOUT1B")&0x01)<<7);
		  ConfigMR[tmr][0]|=((itMR->getInt("ENOCDOUT2B")&0x01)<<6);			
		  ConfigMR[tmr][0]|=((itMR->getInt("ENOCTRANSMITON1B")&0x01)<<5);			
		  ConfigMR[tmr][0]|=((itMR->getInt("ENOCTRANSMITON2B")&0x01)<<4);			
		  ConfigMR[tmr][0]|=((itMR->getInt("ENOCCHIPSATB")&0x01)<<3);			
		  ConfigMR[tmr][0]|=((itMR->getInt("SELSTARTREADOUT")&0x01)<<2);			
		  ConfigMR[tmr][0]|=((itMR->getInt("SELENDREADOUT")&0x01)<<1);	

		  ConfigMR[tmr][1] =((itMR->getInt("SELRAZ0")&0x01)<<6);		
		  ConfigMR[tmr][1]|=((itMR->getInt("SELRAZ1")&0x01)<<5);			
		  ConfigMR[tmr][1]|=((itMR->getInt("CKMUX")&0x01)<<4);			
		  ConfigMR[tmr][1]|=((itMR->getInt("SCON")&0x01)<<3);			
		  ConfigMR[tmr][1]|=((itMR->getInt("RAZCHNEXTVAL")&0x01)<<2);			
		  ConfigMR[tmr][1]|=((itMR->getInt("RAZCHNINTVAL")&0x01)<<1);			
		  ConfigMR[tmr][1]|=((itMR->getInt("TRIGEXTVAL")&0x01)<<0);		
							
		  ConfigMR[tmr][2] =((itMR->getInt("DISCOROR")&0x01)<<7);			
		  ConfigMR[tmr][2]|=((itMR->getInt("ENTRIGOUT")&0x01)<<6);			
		  ConfigMR[tmr][2]|=((itMR->getInt("TRIG0B")&0x01)<<5);			
		  ConfigMR[tmr][2]|=((itMR->getInt("TRIG1B")&0x01)<<4);			
		  ConfigMR[tmr][2]|=((itMR->getInt("TRIG2B")&0x01)<<3);											
		  ConfigMR[tmr][2]|=((tBB2&0x01)<<2);			//bit 0
		  ConfigMR[tmr][2]|=((tBB2&0x02)>>0);			// bit 1
		  ConfigMR[tmr][2]|=((tBB2&0x04)>>2);			// bit 2
								
		  ConfigMR[tmr][3] =((tBB2&0x08)<<4);  //bit 3
		  ConfigMR[tmr][3]|=((tBB2&0x10)<<2); // bit 4
		  ConfigMR[tmr][3]|=((tBB2&0x20)<<0);    // bit 5
		  ConfigMR[tmr][3]|=((tBB2&0x40)>>2);    // bit 6
		  ConfigMR[tmr][3]|=((tBB2&0x80)>>4);    // bit 7
		  ConfigMR[tmr][3]|=((tBB2&0x100)>>6);    // bit 8
		  ConfigMR[tmr][3]|=((tBB2&0x200)>>8);    // bit 9
		  ConfigMR[tmr][3]|=((tBB1&0x01)>>0);	  // bit 0		

		  ConfigMR[tmr][4]= ((tBB1&0x02)<<6);			// bit 1
		  ConfigMR[tmr][4]|=((tBB1&0x04)<<4);			// bit 2
		  ConfigMR[tmr][4]|=((tBB1&0x08)<<2);			// bit 3
		  ConfigMR[tmr][4]|=((tBB1&0x10)<<0);			// bit 4
		  ConfigMR[tmr][4]|=((tBB1&0x20)>>2);			// bit 5
		  ConfigMR[tmr][4]|=((tBB1&0x40)>>4);			// bit 6
		  ConfigMR[tmr][4]|=((tBB1&0x80)>>6);			// bit 7
		  ConfigMR[tmr][4]|=((tBB1&0x100)>>8);			// bit 8

		  ConfigMR[tmr][5]= ((tBB1&0x200)>>2);			// bit 9
		  ConfigMR[tmr][5]|=((tBB0&0x01)<<6);			// bit 0
		  ConfigMR[tmr][5]|=((tBB0&0x02)<<4);			// bit 1
		  ConfigMR[tmr][5]|=((tBB0&0x04)<<2);			// bit 2
		  ConfigMR[tmr][5]|=((tBB0&0x08)<<0);			// bit 3
		  ConfigMR[tmr][5]|=((tBB0&0x10)>>2);			// bit 4
		  ConfigMR[tmr][5]|=((tBB0&0x20)>>4);			// bit 5
		  ConfigMR[tmr][5]|=((tBB0&0x40)>>6);			// bit 6

		  ConfigMR[tmr][6]= ((tBB0&0x80)<<0);				// bit 7
		  ConfigMR[tmr][6]|=((tBB0&0x100)>>2);			// bit 8
		  ConfigMR[tmr][6]|=((tBB0&0x200)>>4);			// bit 9
		  ConfigMR[tmr][6]|=((itMR->getInt("PWRONDAC")&0x01)<<4);			
		  ConfigMR[tmr][6]|=((itMR->getInt("ENPPDAC")&0x01)<<3);			
		  ConfigMR[tmr][6]|=((itMR->getInt("PWRONBG")&0x01)<<2);			
		  ConfigMR[tmr][6]|=((itMR->getInt("ENPPBANDGAP")&0x01)<<1);			
		  ConfigMR[tmr][6]|=((itMR->getInt("HEADER")&0x01));			

		  ConfigMR[tmr][7]= ((itMR->getInt("HEADER")&0x02)<<6);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x04)<<4);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x08)<<2);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x10)<<0);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x20)>>2);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x40)>>4);			
		  ConfigMR[tmr][7]|=((itMR->getInt("HEADER")&0x80)>6);			
		  ConfigMR[tmr][7]|=((mask2&0x8000000000000000llu)>>63);			

		  ConfigMR[tmr][8]=  ((mask1&0x8000000000000000llu)>>56);			
		  ConfigMR[tmr][8]|= ((mask0&0x8000000000000000llu)>>57);			
		  ConfigMR[tmr][8]|= ((mask2&0x4000000000000000llu)>>57);			
		  ConfigMR[tmr][8]|= ((mask1&0x4000000000000000llu)>>58);			
		  ConfigMR[tmr][8]|= ((mask0&0x4000000000000000llu)>>59);			
		  ConfigMR[tmr][8]|= ((mask2&0x2000000000000000llu)>>59);			
		  ConfigMR[tmr][8]|= ((mask1&0x2000000000000000llu)>>60);			
		  ConfigMR[tmr][8]|= ((mask0&0x2000000000000000llu)>>61);			

		  ConfigMR[tmr][9]=  ((mask2&0x1000000000000000llu)>>53);			
		  ConfigMR[tmr][9]|= ((mask1&0x1000000000000000llu)>>54);			
		  ConfigMR[tmr][9]|= ((mask0&0x1000000000000000llu)>>55);			
		  ConfigMR[tmr][9]|= ((mask2&0x0800000000000000llu)>>55);			
		  ConfigMR[tmr][9]|= ((mask1&0x0800000000000000llu)>>56);			
		  ConfigMR[tmr][9]|= ((mask0&0x0800000000000000llu)>>57);			
		  ConfigMR[tmr][9]|= ((mask2&0x0400000000000000llu)>>57);			
		  ConfigMR[tmr][9]|= ((mask1&0x0400000000000000llu)>>58);			

		  ConfigMR[tmr][10] = ((mask0&0x0400000000000000llu)>>51);			
		  ConfigMR[tmr][10]|= ((mask2&0x0200000000000000llu)>>51);			
		  ConfigMR[tmr][10]|= ((mask1&0x0200000000000000llu)>>52);			
		  ConfigMR[tmr][10]|= ((mask0&0x0200000000000000llu)>>53);			
		  ConfigMR[tmr][10]|= ((mask2&0x0100000000000000llu)>>53);			
		  ConfigMR[tmr][10]|= ((mask1&0x0100000000000000llu)>>54);			
		  ConfigMR[tmr][10]|= ((mask0&0x0100000000000000llu)>>55);			
		  ConfigMR[tmr][10]|= ((mask2&0x0080000000000000llu)>>55);			

		  ConfigMR[tmr][11] = ((mask1&0x0080000000000000llu)>>48);			
		  ConfigMR[tmr][11]|= ((mask0&0x0080000000000000llu)>>49);			
		  ConfigMR[tmr][11]|= ((mask2&0x0040000000000000llu)>>49);			
		  ConfigMR[tmr][11]|= ((mask1&0x0040000000000000llu)>>50);			
		  ConfigMR[tmr][11]|= ((mask0&0x0040000000000000llu)>>51);			
		  ConfigMR[tmr][11]|= ((mask2&0x0020000000000000llu)>>51);			
		  ConfigMR[tmr][11]|= ((mask1&0x0020000000000000llu)>>52);			
		  ConfigMR[tmr][11]|= ((mask0&0x0020000000000000llu)>>53);			

		  ConfigMR[tmr][12] = ((mask2&0x0010000000000000llu)>>45);			
		  ConfigMR[tmr][12]|= ((mask1&0x0010000000000000llu)>>46);			
		  ConfigMR[tmr][12]|= ((mask0&0x0010000000000000llu)>>47);			
		  ConfigMR[tmr][12]|= ((mask2&0x0008000000000000llu)>>47);			
		  ConfigMR[tmr][12]|= ((mask1&0x0008000000000000llu)>>48);			
		  ConfigMR[tmr][12]|= ((mask0&0x0008000000000000llu)>>49);			
		  ConfigMR[tmr][12]|= ((mask2&0x0004000000000000llu)>>49);			
		  ConfigMR[tmr][12]|= ((mask1&0x0004000000000000llu)>>50);			

		  ConfigMR[tmr][13] = ((mask0&0x0004000000000000llu)>>43);			
		  ConfigMR[tmr][13]|= ((mask2&0x0002000000000000llu)>>43);			
		  ConfigMR[tmr][13]|= ((mask1&0x0002000000000000llu)>>44);			
		  ConfigMR[tmr][13]|= ((mask0&0x0002000000000000llu)>>45);			
		  ConfigMR[tmr][13]|= ((mask2&0x0001000000000000llu)>>45);			
		  ConfigMR[tmr][13]|= ((mask1&0x0001000000000000llu)>>46);			
		  ConfigMR[tmr][13]|= ((mask0&0x0001000000000000llu)>>47);			
		  ConfigMR[tmr][13]|= ((mask2&0x0000800000000000llu)>>47);			

		  ConfigMR[tmr][14] = ((mask1&0x0000800000000000llu)>>40);			
		  ConfigMR[tmr][14]|= ((mask0&0x0000800000000000llu)>>41);			
		  ConfigMR[tmr][14]|= ((mask2&0x0000400000000000llu)>>41);			
		  ConfigMR[tmr][14]|= ((mask1&0x0000400000000000llu)>>42);			
		  ConfigMR[tmr][14]|= ((mask0&0x0000400000000000llu)>>43);			
		  ConfigMR[tmr][14]|= ((mask2&0x0000200000000000llu)>>43);			
		  ConfigMR[tmr][14]|= ((mask1&0x0000200000000000llu)>>44);			
		  ConfigMR[tmr][14]|= ((mask0&0x0000200000000000llu)>>45);			

		  ConfigMR[tmr][15] = ((mask2&0x0000100000000000llu)>>37);			
		  ConfigMR[tmr][15]|= ((mask1&0x0000100000000000llu)>>38);			
		  ConfigMR[tmr][15]|= ((mask0&0x0000100000000000llu)>>39);			
		  ConfigMR[tmr][15]|= ((mask2&0x0000080000000000llu)>>39);			
		  ConfigMR[tmr][15]|= ((mask1&0x0000080000000000llu)>>40);			
		  ConfigMR[tmr][15]|= ((mask0&0x0000080000000000llu)>>41);			
		  ConfigMR[tmr][15]|= ((mask2&0x0000040000000000llu)>>41);			
		  ConfigMR[tmr][15]|= ((mask1&0x0000040000000000llu)>>42);			

		  ConfigMR[tmr][16] = ((mask0&0x0000040000000000llu)>>35);			
		  ConfigMR[tmr][16]|= ((mask2&0x0000020000000000llu)>>35);			
		  ConfigMR[tmr][16]|= ((mask1&0x0000020000000000llu)>>36);			
		  ConfigMR[tmr][16]|= ((mask0&0x0000020000000000llu)>>37);			
		  ConfigMR[tmr][16]|= ((mask2&0x0000010000000000llu)>>37);			
		  ConfigMR[tmr][16]|= ((mask1&0x0000010000000000llu)>>38);			
		  ConfigMR[tmr][16]|= ((mask0&0x0000010000000000llu)>>39);			
		  ConfigMR[tmr][16]|= ((mask2&0x0000008000000000llu)>>39);			

		  ConfigMR[tmr][17] = ((mask1&0x0000008000000000llu)>>32);			
		  ConfigMR[tmr][17]|= ((mask0&0x0000008000000000llu)>>33);			
		  ConfigMR[tmr][17]|= ((mask2&0x0000004000000000llu)>>33);			
		  ConfigMR[tmr][17]|= ((mask1&0x0000004000000000llu)>>34);			
		  ConfigMR[tmr][17]|= ((mask0&0x0000004000000000llu)>>35);			
		  ConfigMR[tmr][17]|= ((mask2&0x0000002000000000llu)>>35);			
		  ConfigMR[tmr][17]|= ((mask1&0x0000002000000000llu)>>36);			
		  ConfigMR[tmr][17]|= ((mask0&0x0000002000000000llu)>>37);			

		  ConfigMR[tmr][18] = ((mask2&0x0000001000000000llu)>>29);			
		  ConfigMR[tmr][18]|= ((mask1&0x0000001000000000llu)>>30);			
		  ConfigMR[tmr][18]|= ((mask0&0x0000001000000000llu)>>31);			
		  ConfigMR[tmr][18]|= ((mask2&0x0000000800000000llu)>>31);			
		  ConfigMR[tmr][18]|= ((mask1&0x0000000800000000llu)>>32);			
		  ConfigMR[tmr][18]|= ((mask0&0x0000000800000000llu)>>33);			
		  ConfigMR[tmr][18]|= ((mask2&0x0000000400000000llu)>>33);			
		  ConfigMR[tmr][18]|= ((mask1&0x0000000400000000llu)>>34);			

		  ConfigMR[tmr][19] = ((mask0&0x0000000400000000llu)>>27);			
		  ConfigMR[tmr][19]|= ((mask2&0x0000000200000000llu)>>27);			
		  ConfigMR[tmr][19]|= ((mask1&0x0000000200000000llu)>>28);			
		  ConfigMR[tmr][19]|= ((mask0&0x0000000200000000llu)>>29);			
		  ConfigMR[tmr][19]|= ((mask2&0x0000000100000000llu)>>29);			
		  ConfigMR[tmr][19]|= ((mask1&0x0000000100000000llu)>>30);			
		  ConfigMR[tmr][19]|= ((mask0&0x0000000100000000llu)>>31);			
		  ConfigMR[tmr][19]|= ((mask2&0x0000000080000000llu)>>31);			

		  ConfigMR[tmr][20] = ((mask1&0x0000000080000000llu)>>24);			
		  ConfigMR[tmr][20]|= ((mask0&0x0000000080000000llu)>>25);			
		  ConfigMR[tmr][20]|= ((mask2&0x0000000040000000llu)>>25);			
		  ConfigMR[tmr][20]|= ((mask1&0x0000000040000000llu)>>26);			
		  ConfigMR[tmr][20]|= ((mask0&0x0000000040000000llu)>>27);			
		  ConfigMR[tmr][20]|= ((mask2&0x0000000020000000llu)>>27);			
		  ConfigMR[tmr][20]|= ((mask1&0x0000000020000000llu)>>28);			
		  ConfigMR[tmr][20]|= ((mask0&0x0000000020000000llu)>>29);			

		  ConfigMR[tmr][21] = ((mask2&0x0000000010000000llu)>>21);			
		  ConfigMR[tmr][21]|= ((mask1&0x0000000010000000llu)>>22);			
		  ConfigMR[tmr][21]|= ((mask0&0x0000000010000000llu)>>23);			
		  ConfigMR[tmr][21]|= ((mask2&0x0000000008000000llu)>>23);			
		  ConfigMR[tmr][21]|= ((mask1&0x0000000008000000llu)>>24);			
		  ConfigMR[tmr][21]|= ((mask0&0x0000000008000000llu)>>25);			
		  ConfigMR[tmr][21]|= ((mask2&0x0000000004000000llu)>>25);			
		  ConfigMR[tmr][21]|= ((mask1&0x0000000004000000llu)>>26);			

		  ConfigMR[tmr][22] = ((mask0&0x0000000004000000llu)>>19);			
		  ConfigMR[tmr][22]|= ((mask2&0x0000000002000000llu)>>19);			
		  ConfigMR[tmr][22]|= ((mask1&0x0000000002000000llu)>>20);			
		  ConfigMR[tmr][22]|= ((mask0&0x0000000002000000llu)>>21);			
		  ConfigMR[tmr][22]|= ((mask2&0x0000000001000000llu)>>21);			
		  ConfigMR[tmr][22]|= ((mask1&0x0000000001000000llu)>>22);			
		  ConfigMR[tmr][22]|= ((mask0&0x0000000001000000llu)>>23);			
		  ConfigMR[tmr][22]|= ((mask2&0x0000000000800000llu)>>23);			

		  ConfigMR[tmr][23] = ((mask1&0x0000000000800000llu)>>16);			
		  ConfigMR[tmr][23]|= ((mask0&0x0000000000800000llu)>>17);			
		  ConfigMR[tmr][23]|= ((mask2&0x0000000000400000llu)>>17);			
		  ConfigMR[tmr][23]|= ((mask1&0x0000000000400000llu)>>18);			
		  ConfigMR[tmr][23]|= ((mask0&0x0000000000400000llu)>>19);			
		  ConfigMR[tmr][23]|= ((mask2&0x0000000000200000llu)>>19);			
		  ConfigMR[tmr][23]|= ((mask1&0x0000000000200000llu)>>20);			
		  ConfigMR[tmr][23]|= ((mask0&0x0000000000200000llu)>>21);			

		  ConfigMR[tmr][24] = ((mask2&0x0000000000100000llu)>>13);			
		  ConfigMR[tmr][24]|= ((mask1&0x0000000000100000llu)>>14);			
		  ConfigMR[tmr][24]|= ((mask0&0x0000000000100000llu)>>15);			
		  ConfigMR[tmr][24]|= ((mask2&0x0000000000080000llu)>>15);			
		  ConfigMR[tmr][24]|= ((mask1&0x0000000000080000llu)>>16);			
		  ConfigMR[tmr][24]|= ((mask0&0x0000000000080000llu)>>17);			
		  ConfigMR[tmr][24]|= ((mask2&0x0000000000040000llu)>>17);			
		  ConfigMR[tmr][24]|= ((mask1&0x0000000000040000llu)>>18);			
	
		  ConfigMR[tmr][25] = ((mask0&0x0000000000040000llu)>>11);			
		  ConfigMR[tmr][25]|= ((mask2&0x0000000000020000llu)>>11);			
		  ConfigMR[tmr][25]|= ((mask1&0x0000000000020000llu)>>12);			
		  ConfigMR[tmr][25]|= ((mask0&0x0000000000020000llu)>>13);			
		  ConfigMR[tmr][25]|= ((mask2&0x0000000000010000llu)>>13);			
		  ConfigMR[tmr][25]|= ((mask1&0x0000000000010000llu)>>14);			
		  ConfigMR[tmr][25]|= ((mask0&0x0000000000010000llu)>>15);			
		  ConfigMR[tmr][25]|= ((mask2&0x0000000000008000llu)>>15);			
	
		  ConfigMR[tmr][26] = ((mask1&0x0000000000008000llu)>>8);			
		  ConfigMR[tmr][26]|= ((mask0&0x0000000000008000llu)>>9);			
		  ConfigMR[tmr][26]|= ((mask2&0x0000000000004000llu)>>9);			
		  ConfigMR[tmr][26]|= ((mask1&0x0000000000004000llu)>>10);			
		  ConfigMR[tmr][26]|= ((mask0&0x0000000000004000llu)>>11);			
		  ConfigMR[tmr][26]|= ((mask2&0x0000000000002000llu)>>11);			
		  ConfigMR[tmr][26]|= ((mask1&0x0000000000002000llu)>>12);			
		  ConfigMR[tmr][26]|= ((mask0&0x0000000000002000llu)>>13);			
	
		  ConfigMR[tmr][27] = ((mask2&0x0000000000001000llu)>>5);			
		  ConfigMR[tmr][27]|= ((mask1&0x0000000000001000llu)>>6);			
		  ConfigMR[tmr][27]|= ((mask0&0x0000000000001000llu)>>7);			
		  ConfigMR[tmr][27]|= ((mask2&0x0000000000000800llu)>>7);			
		  ConfigMR[tmr][27]|= ((mask1&0x0000000000000800llu)>>8);			
		  ConfigMR[tmr][27]|= ((mask0&0x0000000000000800llu)>>9);			
		  ConfigMR[tmr][27]|= ((mask2&0x0000000000000400llu)>>9);			
		  ConfigMR[tmr][27]|= ((mask1&0x0000000000000400llu)>>10);			
	
		  ConfigMR[tmr][28] = ((mask0&0x0000000000000400llu)>>3);			
		  ConfigMR[tmr][28]|= ((mask2&0x0000000000000200llu)>>3);			
		  ConfigMR[tmr][28]|= ((mask1&0x0000000000000200llu)>>4);			
		  ConfigMR[tmr][28]|= ((mask0&0x0000000000000200llu)>>5);			
		  ConfigMR[tmr][28]|= ((mask2&0x0000000000000100llu)>>5);			
		  ConfigMR[tmr][28]|= ((mask1&0x0000000000000100llu)>>6);			
		  ConfigMR[tmr][28]|= ((mask0&0x0000000000000100llu)>>7);			
		  ConfigMR[tmr][28]|= ((mask2&0x0000000000000080llu)>>7);			
	
		  ConfigMR[tmr][29] = ((mask1&0x0000000000000080llu)>>0);			
		  ConfigMR[tmr][29]|= ((mask0&0x0000000000000080llu)>>1);			
		  ConfigMR[tmr][29]|= ((mask2&0x0000000000000040llu)>>1);			
		  ConfigMR[tmr][29]|= ((mask1&0x0000000000000040llu)>>2);			
		  ConfigMR[tmr][29]|= ((mask0&0x0000000000000040llu)>>3);			
		  ConfigMR[tmr][29]|= ((mask2&0x0000000000000020llu)>>3);			
		  ConfigMR[tmr][29]|= ((mask1&0x0000000000000020llu)>>4);			
		  ConfigMR[tmr][29]|= ((mask0&0x0000000000000020llu)>>5);			
	
		  ConfigMR[tmr][30] = ((mask2&0x0000000000000010llu)<<3);			
		  ConfigMR[tmr][30]|= ((mask1&0x0000000000000010llu)<<2);			
		  ConfigMR[tmr][30]|= ((mask0&0x0000000000000010llu)<<1);			
		  ConfigMR[tmr][30]|= ((mask2&0x0000000000000008llu)<<1);			
		  ConfigMR[tmr][30]|= ((mask1&0x0000000000000008llu)<<0);			
		  ConfigMR[tmr][30]|= ((mask0&0x0000000000000008llu)>>1);			
		  ConfigMR[tmr][30]|= ((mask2&0x0000000000000004llu)>>1);			
		  ConfigMR[tmr][30]|= ((mask1&0x0000000000000004llu)>>2);			
	
		  ConfigMR[tmr][31] = ((mask0&0x0000000000000004llu)<<5);			
		  ConfigMR[tmr][31]|= ((mask2&0x0000000000000002llu)<<5);			
		  ConfigMR[tmr][31]|= ((mask1&0x0000000000000002llu)<<4);			
		  ConfigMR[tmr][31]|= ((mask0&0x0000000000000002llu)<<3);			
		  ConfigMR[tmr][31]|= ((mask2&0x0000000000000001llu)<<3);			
		  ConfigMR[tmr][31]|= ((mask1&0x0000000000000001llu)<<2);			
		  ConfigMR[tmr][31]|= ((mask0&0x0000000000000001llu)<<1);			
		  ConfigMR[tmr][31]|= ((itMR->getInt("RS_OR_DISCRI")&0x01));			
	
		  ConfigMR[tmr][32]=  ((itMR->getInt("DISCRI1_PP")&0x01)<<7);			
		  ConfigMR[tmr][32]|= ((itMR->getInt("DISCRI2_PP")&0x01)<<6);			
		  ConfigMR[tmr][32]|= ((itMR->getInt("DISCRI0_PP")&0x01)<<5);			
		  ConfigMR[tmr][32]|= ((itMR->getInt("ENPPOTAQ")&0x01)<<4);			
		  ConfigMR[tmr][32]|= ((itMR->getInt("EN_OTAQ")&0x01)<<3);			
		  ConfigMR[tmr][32]|= ((itMR->getInt("ENPPDAC4BIT")&0x01)<<2);	
		  ConfigMR[tmr][32]|= ((B0B3Gain[63]&0x0f)>>2);

		  for (int ti=0;ti<31;ti++)
		    {							
		      ConfigMR[tmr][33+ti] = ((B0B3Gain[(31-ti)*2+1]&0x03)<<6);
		      ConfigMR[tmr][33+ti]|= ((B0B3Gain[(31-ti)*2]&0x0f)<<2);
		      ConfigMR[tmr][33+ti]|= ((B0B3Gain[(31-ti)*2-1]&0x0c)>>2);							
		    }

		  ConfigMR[tmr][64] = ((B0B3Gain[1]&0x03)<<6);
		  ConfigMR[tmr][64]|= ((B0B3Gain[0]&0x0f)<<2);
		  ConfigMR[tmr][64]|= ((itMR->getInt("SWHG")&0x01)<<1);			
		  ConfigMR[tmr][64]|= ((itMR->getInt("SWHG")&0x02)>>1);			

		  ConfigMR[tmr][65] = ((itMR->getInt("VALIDSH")&0x01)<<7);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("PWRONWIDLAR")&0x01)<<6);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("SWLG")&0x01)<<5);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("SWLG")&0x02)<<3);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("PWRONSHLG")&0x01)<<3);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("PWRONSHHG")&0x01)<<2);			
		  ConfigMR[tmr][65]|= ((itMR->getInt("ENGBST")&0x01)<<1);			// en_gbst
		  ConfigMR[tmr][65]|= ((itMR->getInt("PWRONPA")&0x01)<<0);			

					ConfigMR[tmr][66]=((cTest&0xFF00000000000000llu)>>56);			
					ConfigMR[tmr][67]=((cTest&0x00FF000000000000llu)>>48);			
					ConfigMR[tmr][68]=((cTest&0x0000FF0000000000llu)>>40);			
					ConfigMR[tmr][69]=((cTest&0x000000FF00000000llu)>>32);			
					ConfigMR[tmr][70]=((cTest&0x00000000FF000000llu)>>24);			
					ConfigMR[tmr][71]=((cTest&0x0000000000FF0000llu)>>16);			
					ConfigMR[tmr][72]=((cTest&0x000000000000FF00llu)>>8);			
					ConfigMR[tmr][73]=((cTest&0x00000000000000FFllu)>>0);						
			uint8_t tamponl=0;
		  tmr++;
			if (tmr%8==0) tamponl=1;
			else if (tmr%8<5) tamponl=(tmr%8);
			else tamponl=9-tmr%8;
			if (tamponl==1) tmrl1++;
			else if (tamponl==2) tmrl2++;
			else if (tamponl==3) tmrl3++;
			else if (tamponl==4) tmrl4++;
				}//if (	(iasic+1)==itMR->getInt("HEADER"))
    	}
			catch (Exception e)  {
			  LOG4CXX_ERROR(_logOracle," Error in ASIC access"<<e.getMessage());
			  std::cout<<e.getMessage()<<std::endl;  }
		} //for (int iasic=0;iasic<MAX_NB_OF_ASICS;iasic++)
	} //  for (std::vector<ConfigObject*>::iterator itMRp=asic_vector_.begin();itMRp!=asic_vector_.end();itMRp++)
  return (tmrl1&0xFF)+((tmrl2&0xFF)<<8)+((tmrl3&0xFF)<<16)+((tmrl4&0xFF)<<24);
}		


void lydaq::OracleDIFDBInterface::storeHardRoc2Configuration(Asic* itHR2,unsigned char* ConfigHR2)
{
  unsigned long long tmask0;
  unsigned long long tmask1;
  unsigned long long tmask2;
  sscanf(itHR2->getString("MASK0").c_str(),"%llx\n",&tmask0);	
  sscanf(itHR2->getString("MASK1").c_str(),"%llx\n",&tmask1);	
  sscanf(itHR2->getString("MASK2").c_str(),"%llx\n",&tmask2);

  //	      printf(" Header %d Mask0 %llx \n",itHR2->getInt("HEADER"),tmask0);
  for (int32_t ib=0;ib<HARDROCV2_SLC_FRAME_SIZE;ib++)
    ConfigHR2[ib]=0;

  ConfigHR2[0]|=((itHR2->getInt("ENOCDOUT1B")&0x01)<<7);
  ConfigHR2[0]|=((itHR2->getInt("ENOCDOUT2B")&0x01)<<6);
  ConfigHR2[0]|=((itHR2->getInt("ENOCTRANSMITON1B")&0x01)<<5);
  ConfigHR2[0]|=((itHR2->getInt("ENOCTRANSMITON2B")&0x01)<<4);
  ConfigHR2[0]|=((itHR2->getInt("ENOCCHIPSATB")&0x01)<<3);
  ConfigHR2[0]|=((itHR2->getInt("SELSTARTREADOUT")&0x01)<<2);
  ConfigHR2[0]|=((itHR2->getInt("SELENDREADOUT")&0x01)<<1);
  ConfigHR2[1]|=((itHR2->getInt("CLKMUX")&0x01)<<4);
  ConfigHR2[1]|=((itHR2->getInt("SCON")&0x01)<<3);
  ConfigHR2[1]|=((itHR2->getInt("RAZCHNEXTVAL")&0x01)<<2);
  ConfigHR2[1]|=((itHR2->getInt("RAZCHNINTVAL")&0x01)<<1);
  ConfigHR2[1]|=((itHR2->getInt("TRIGEXTVAL")&0x01)<<0);
  ConfigHR2[2]|=((itHR2->getInt("DISCROROR")&0x01)<<7);
  ConfigHR2[2]|=((itHR2->getInt("ENTRIGOUT")&0x01)<<6);
  ConfigHR2[2]|=((itHR2->getInt("TRIG0B")&0x01)<<5);
  ConfigHR2[2]|=((itHR2->getInt("TRIG1B")&0x01)<<4);
  ConfigHR2[2]|=((itHR2->getInt("TRIG2B")&0x01)<<3);
  ConfigHR2[2]|=((itHR2->getInt("OTABGSW")&0x01)<<2);
  ConfigHR2[2]|=((itHR2->getInt("DACSW")&0x01)<<1);
  ConfigHR2[2]|=((itHR2->getInt("SMALLDAC")&0x01)<<0);
  ConfigHR2[3]= ((itHR2->getInt("B2")>>2)&0xFF);
  ConfigHR2[4]|=((itHR2->getInt("B2")&0x03)<<6);
  ConfigHR2[4]|=((itHR2->getInt("B1")>>4)&0x3F);
  ConfigHR2[5]|=((itHR2->getInt("B1")&0x0F)<<4);
  ConfigHR2[5]|=((itHR2->getInt("B0")>>6)&0x0F);
  ConfigHR2[6]|=((itHR2->getInt("B0")&0x3F)<<2);
  ConfigHR2[6]|=(itHR2->getInt("HEADER")&0x01)<<1;
  ConfigHR2[6]|=(itHR2->getInt("HEADER")&0x02)>>1;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x04)<<5;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x08)<<3;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x10)<<1;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x20)>>1;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x40)>>3;
  ConfigHR2[7]|=(itHR2->getInt("HEADER")&0x80)>>5;

  ConfigHR2[7]|=((tmask2&0x8000000000000000llu)>>62); //63	ok
  ConfigHR2[8]|=((tmask2&0x4000000000000000llu)>>56); //62	ok
  ConfigHR2[8]|=((tmask2&0x2000000000000000llu)>>58); //61	ok
  ConfigHR2[8]|=((tmask2&0x1000000000000000llu)>>60); //60	ok
  ConfigHR2[9]|=((tmask2&0x0800000000000000llu)>>54); //59	ok
  ConfigHR2[9]|=((tmask2&0x0400000000000000llu)>>56); //58	ok
  ConfigHR2[10]|=((tmask2&0x0200000000000000llu)>>50); //57	ok
  ConfigHR2[10]|=((tmask2&0x0100000000000000llu)>>52); //56	ok
  ConfigHR2[10]|=((tmask2&0x0080000000000000llu)>>54); //55	ok
  ConfigHR2[11]|=((tmask2&0x0040000000000000llu)>>48); //54	ok
  ConfigHR2[11]|=((tmask2&0x0020000000000000llu)>>50); //53	ok
  ConfigHR2[11]|=((tmask2&0x0010000000000000llu)>>52); //52	ok
  ConfigHR2[12]|=((tmask2&0x0008000000000000llu)>>46); //51	ok
  ConfigHR2[12]|=((tmask2&0x0004000000000000llu)>>48); //50	ok
  ConfigHR2[13]|=((tmask2&0x0002000000000000llu)>>42); //49	ok
  ConfigHR2[13]|=((tmask2&0x0001000000000000llu)>>44); //48	ok
  ConfigHR2[13]|=((tmask2&0x0000800000000000llu)>>46); //47	ok
  ConfigHR2[14]|=((tmask2&0x0000400000000000llu)>>40); //46	ok
  ConfigHR2[14]|=((tmask2&0x0000200000000000llu)>>42); //45	ok
  ConfigHR2[14]|=((tmask2&0x0000100000000000llu)>>44); //44	ok
  ConfigHR2[15]|=((tmask2&0x0000080000000000llu)>>38); //43	ok
  ConfigHR2[15]|=((tmask2&0x0000040000000000llu)>>40); //42	ok
  ConfigHR2[16]|=((tmask2&0x0000020000000000llu)>>34); //41 ok	
  ConfigHR2[16]|=((tmask2&0x0000010000000000llu)>>36); //40	ok
  ConfigHR2[16]|=((tmask2&0x0000008000000000llu)>>38); //39	ok
  ConfigHR2[17]|=((tmask2&0x0000004000000000llu)>>32); //38	ok
  ConfigHR2[17]|=((tmask2&0x0000002000000000llu)>>34); //37	ok
  ConfigHR2[17]|=((tmask2&0x0000001000000000llu)>>36); //36	ok
  ConfigHR2[18]|=((tmask2&0x0000000800000000llu)>>30); //35	ok
  ConfigHR2[18]|=((tmask2&0x0000000400000000llu)>>32); //34	ok
  ConfigHR2[19]|=((tmask2&0x0000000200000000llu)>>26); //33	ok
  ConfigHR2[19]|=((tmask2&0x0000000100000000llu)>>28); //32	ok 
  ConfigHR2[19]|=((tmask2&0x0000000080000000llu)>>30); //31	ok
  ConfigHR2[20]|=((tmask2&0x0000000040000000llu)>>24); //30	ok
  ConfigHR2[20]|=((tmask2&0x0000000020000000llu)>>26); //29	ok
  ConfigHR2[20]|=((tmask2&0x0000000010000000llu)>>28); //28	ok
  ConfigHR2[21]|=((tmask2&0x0000000008000000llu)>>22); //27	ok
  ConfigHR2[21]|=((tmask2&0x0000000004000000llu)>>24); //26	ok
  ConfigHR2[22]|=((tmask2&0x0000000002000000llu)>>18); //25	ok
  ConfigHR2[22]|=((tmask2&0x0000000001000000llu)>>20); //24	ok
  ConfigHR2[22]|=((tmask2&0x0000000000800000llu)>>22); //23	ok
  ConfigHR2[23]|=((tmask2&0x0000000000400000llu)>>16); //22	ok
  ConfigHR2[23]|=((tmask2&0x0000000000200000llu)>>18); //21	ok 
  ConfigHR2[23]|=((tmask2&0x0000000000100000llu)>>20); //20	ok
  ConfigHR2[24]|=((tmask2&0x0000000000080000llu)>>14); //19	ok
  ConfigHR2[24]|=((tmask2&0x0000000000040000llu)>>16); //18	ok
  ConfigHR2[25]|=((tmask2&0x0000000000020000llu)>>10); //17	ok
  ConfigHR2[25]|=((tmask2&0x0000000000010000llu)>>12); //16	ok
  ConfigHR2[25]|=((tmask2&0x0000000000008000llu)>>14); //15 ok
  ConfigHR2[26]|=((tmask2&0x0000000000004000llu)>>8);//14 ok
  ConfigHR2[26]|=((tmask2&0x0000000000002000llu)>>10); //13 ok
  ConfigHR2[26]|=((tmask2&0x0000000000001000llu)>>12); //12 ok
  ConfigHR2[27]|=((tmask2&0x0000000000000800llu)>>6);//11 ok
  ConfigHR2[27]|=((tmask2&0x0000000000000400llu)>>8);//10 ok
  ConfigHR2[28]|=((tmask2&0x0000000000000200llu)>>2);//9 ok
  ConfigHR2[28]|=((tmask2&0x0000000000000100llu)>>4);//8 ok
  ConfigHR2[28]|=((tmask2&0x0000000000000080llu)>>6);//7 ok
  ConfigHR2[29]|=((tmask2&0x0000000000000040llu)>>0);//6 ok
  ConfigHR2[29]|=((tmask2&0x0000000000000020llu)>>2);//5 ok
  ConfigHR2[29]|=((tmask2&0x0000000000000010llu)>>4);//4 ok
  ConfigHR2[30]|=((tmask2&0x0000000000000008llu)<<2);//3 ok
  ConfigHR2[30]|=((tmask2&0x0000000000000004llu)<<0);//2 ok
  ConfigHR2[31]|=((tmask2&0x0000000000000002llu)<<6);//1 ok
  ConfigHR2[31]|=((tmask2&0x0000000000000001llu)<<4);//0 ok

  ConfigHR2[7]|= ((tmask1&0x8000000000000000llu)>>63); //63 ok		
  ConfigHR2[8]|= ((tmask1&0x4000000000000000llu)>>57); //62	ok	
  ConfigHR2[8]|= ((tmask1&0x2000000000000000llu)>>59); //61	ok	
  ConfigHR2[9]|= ((tmask1&0x1000000000000000llu)>>53); //60	ok	
  ConfigHR2[9]|= ((tmask1&0x0800000000000000llu)>>55); //59	ok	
  ConfigHR2[9]|= ((tmask1&0x0400000000000000llu)>>57); //58	ok	
  ConfigHR2[10]|=((tmask1&0x0200000000000000llu)>>51); //57	ok	
  ConfigHR2[10]|=((tmask1&0x0100000000000000llu)>>53); //56		ok
  ConfigHR2[10]|=((tmask1&0x0080000000000000llu)>>55); //55	ok	
  ConfigHR2[11]|=((tmask1&0x0040000000000000llu)>>49); //54		ok
  ConfigHR2[11]|=((tmask1&0x0020000000000000llu)>>51); //53	ok	
  ConfigHR2[12]|=((tmask1&0x0010000000000000llu)>>45); //52		ok
  ConfigHR2[12]|=((tmask1&0x0008000000000000llu)>>47); //51		ok
  ConfigHR2[12]|=((tmask1&0x0004000000000000llu)>>49); //50		ok
  ConfigHR2[13]|=((tmask1&0x0002000000000000llu)>>43); //49		ok
  ConfigHR2[13]|=((tmask1&0x0001000000000000llu)>>45); //48	ok	
  ConfigHR2[13]|=((tmask1&0x0000800000000000llu)>>47); //47	ok	
  ConfigHR2[14]|=((tmask1&0x0000400000000000llu)>>41); //46		ok
  ConfigHR2[14]|=((tmask1&0x0000200000000000llu)>>43); //45		ok
  ConfigHR2[15]|=((tmask1&0x0000100000000000llu)>>37); //44		ok
  ConfigHR2[15]|=((tmask1&0x0000080000000000llu)>>39); //43		ok
  ConfigHR2[15]|=((tmask1&0x0000040000000000llu)>>41); //42		ok
  ConfigHR2[16]|=((tmask1&0x0000020000000000llu)>>35); //41		ok
  ConfigHR2[16]|=((tmask1&0x0000010000000000llu)>>37); //40		ok
  ConfigHR2[16]|=((tmask1&0x0000008000000000llu)>>39);  //39	ok	
  ConfigHR2[17]|=((tmask1&0x0000004000000000llu)>>33); //38		ok
  ConfigHR2[17]|=((tmask1&0x0000002000000000llu)>>35); //37		ok
  ConfigHR2[18]|=((tmask1&0x0000001000000000llu)>>29); //36		ok
  ConfigHR2[18]|=((tmask1&0x0000000800000000llu)>>31); //35		ok
  ConfigHR2[18]|=((tmask1&0x0000000400000000llu)>>33); //34		ok
  ConfigHR2[19]|=((tmask1&0x0000000200000000llu)>>27); //33		ok
  ConfigHR2[19]|=((tmask1&0x0000000100000000llu)>>29); //32		ok
  ConfigHR2[19]|=((tmask1&0x0000000080000000llu)>>31); //31	ok	
  ConfigHR2[20]|=((tmask1&0x0000000040000000llu)>>25); //30		ok
  ConfigHR2[20]|=((tmask1&0x0000000020000000llu)>>27); //29		ok
  ConfigHR2[21]|=((tmask1&0x0000000010000000llu)>>21); //28		ok
  ConfigHR2[21]|=((tmask1&0x0000000008000000llu)>>23); //27		ok
  ConfigHR2[21]|=((tmask1&0x0000000004000000llu)>>25); //26		ok
  ConfigHR2[22]|=((tmask1&0x0000000002000000llu)>>19); //25		ok
  ConfigHR2[22]|=((tmask1&0x0000000001000000llu)>>21); //24		ok
  ConfigHR2[22]|=((tmask1&0x0000000000800000llu)>>23); //23	ok	
  ConfigHR2[23]|=((tmask1&0x0000000000400000llu)>>17); //22		ok
  ConfigHR2[23]|=((tmask1&0x0000000000200000llu)>>19); //21		ok
  ConfigHR2[24]|=((tmask1&0x0000000000100000llu)>>13); //20		ok
  ConfigHR2[24]|=((tmask1&0x0000000000080000llu)>>15); //19		ok
  ConfigHR2[24]|=((tmask1&0x0000000000040000llu)>>17); //18		ok
  ConfigHR2[25]|=((tmask1&0x0000000000020000llu)>>11); //17		ok
  ConfigHR2[25]|=((tmask1&0x0000000000010000llu)>>13); //16		ok
  ConfigHR2[25]|=((tmask1&0x0000000000008000llu)>>15); //15	ok	
  ConfigHR2[26]|=((tmask1&0x0000000000004000llu)>>9); //14	ok
  ConfigHR2[26]|=((tmask1&0x0000000000002000llu)>>11); 	//13		ok
  ConfigHR2[27]|=((tmask1&0x0000000000001000llu)>>5); //12	ok
  ConfigHR2[27]|=((tmask1&0x0000000000000800llu)>>7); //11	ok
  ConfigHR2[27]|=((tmask1&0x0000000000000400llu)>>9); //10	ok
  ConfigHR2[28]|=((tmask1&0x0000000000000200llu)>>3); //9	ok
  ConfigHR2[28]|=((tmask1&0x0000000000000100llu)>>5); //8	ok
  ConfigHR2[28]|=((tmask1&0x0000000000000080llu)>>7); //7	ok
  ConfigHR2[29]|=((tmask1&0x0000000000000040llu)>>1); //6	ok
  ConfigHR2[29]|=((tmask1&0x0000000000000020llu)>>3); //5	ok
  ConfigHR2[30]|=((tmask1&0x0000000000000010llu)<<3); //4	 ok
  ConfigHR2[30]|=((tmask1&0x0000000000000008llu)<<1); //3	ok
  ConfigHR2[30]|=((tmask1&0x0000000000000004llu)>>1); //2	ok
  ConfigHR2[31]|=((tmask1&0x0000000000000002llu)<<5); //1	ok
  ConfigHR2[31]|=((tmask1&0x0000000000000001llu)<<3); //0	ok
	
  ConfigHR2[8]|= ((tmask0&0x8000000000000000llu)>>56); //63	ok
  ConfigHR2[8]|= ((tmask0&0x4000000000000000llu)>>58); //62	ok
  ConfigHR2[8]|= ((tmask0&0x2000000000000000llu)>>60); //61	ok
  ConfigHR2[9]|= ((tmask0&0x1000000000000000llu)>>54); //60	ok
  ConfigHR2[9]|= ((tmask0&0x0800000000000000llu)>>56); //59	ok
  ConfigHR2[9]|= ((tmask0&0x0400000000000000llu)>>58); //58ok	
  ConfigHR2[10]|=((tmask0&0x0200000000000000llu)>>52); //57	ok
  ConfigHR2[10]|=((tmask0&0x0100000000000000llu)>>54); //56	ok
  ConfigHR2[11]|=((tmask0&0x0080000000000000llu)>>48); //55	ok
  ConfigHR2[11]|=((tmask0&0x0040000000000000llu)>>50); //54	ok
  ConfigHR2[11]|=((tmask0&0x0020000000000000llu)>>52); //53	ok
  ConfigHR2[12]|=((tmask0&0x0010000000000000llu)>>46); //52	ok
  ConfigHR2[12]|=((tmask0&0x0008000000000000llu)>>48); //51	ok
  ConfigHR2[12]|=((tmask0&0x0004000000000000llu)>>50); //50	ok
  ConfigHR2[13]|=((tmask0&0x0002000000000000llu)>>44); //49	ok
  ConfigHR2[13]|=((tmask0&0x0001000000000000llu)>>46); //48	ok
  ConfigHR2[14]|=((tmask0&0x0000800000000000llu)>>40); //47	ok
  ConfigHR2[14]|=((tmask0&0x0000400000000000llu)>>42); //46	ok
  ConfigHR2[14]|=((tmask0&0x0000200000000000llu)>>44); //45	ok
  ConfigHR2[15]|=((tmask0&0x0000100000000000llu)>>38); //44	ok
  ConfigHR2[15]|=((tmask0&0x0000080000000000llu)>>40); //43	ok
  ConfigHR2[15]|=((tmask0&0x0000040000000000llu)>>42); //42	ok
  ConfigHR2[16]|=((tmask0&0x0000020000000000llu)>>36); //41	ok
  ConfigHR2[16]|=((tmask0&0x0000010000000000llu)>>38); //40	ok
  ConfigHR2[17]|=((tmask0&0x0000008000000000llu)>>32); //39	ok
  ConfigHR2[17]|=((tmask0&0x0000004000000000llu)>>34); //38	
  ConfigHR2[17]|=((tmask0&0x0000002000000000llu)>>36); //37	ok
  ConfigHR2[18]|=((tmask0&0x0000001000000000llu)>>30); //36	ok
  ConfigHR2[18]|=((tmask0&0x0000000800000000llu)>>32); //35	ok
  ConfigHR2[18]|=((tmask0&0x0000000400000000llu)>>34); //34ok	
  ConfigHR2[19]|=((tmask0&0x0000000200000000llu)>>28); //33	ok
  ConfigHR2[19]|=((tmask0&0x0000000100000000llu)>>30); //32	ok
  ConfigHR2[20]|=((tmask0&0x0000000080000000llu)>>24); //31	ok
  ConfigHR2[20]|=((tmask0&0x0000000040000000llu)>>26); //30	ok
  ConfigHR2[20]|=((tmask0&0x0000000020000000llu)>>28); //29	ok
  ConfigHR2[21]|=((tmask0&0x0000000010000000llu)>>22); //28	ok
  ConfigHR2[21]|=((tmask0&0x0000000008000000llu)>>24); //27	ok
  ConfigHR2[21]|=((tmask0&0x0000000004000000llu)>>26); //26	ok
  ConfigHR2[22]|=((tmask0&0x0000000002000000llu)>>20); //25	ok
  ConfigHR2[22]|=((tmask0&0x0000000001000000llu)>>22); //24	ok
  ConfigHR2[23]|=((tmask0&0x0000000000800000llu)>>16); //23	ok
  ConfigHR2[23]|=((tmask0&0x0000000000400000llu)>>18); //22	ok
  ConfigHR2[23]|=((tmask0&0x0000000000200000llu)>>20); //21	ok
  ConfigHR2[24]|=((tmask0&0x0000000000100000llu)>>14); //20	ok
  ConfigHR2[24]|=((tmask0&0x0000000000080000llu)>>16); //19	ok
  ConfigHR2[24]|=((tmask0&0x0000000000040000llu)>>18); //18	ok
  ConfigHR2[25]|=((tmask0&0x0000000000020000llu)>>12); //17	ok
  ConfigHR2[25]|=((tmask0&0x0000000000010000llu)>>14); //16	ok
  ConfigHR2[26]|=((tmask0&0x0000000000008000llu)>>8);//15ok
  ConfigHR2[26]|=((tmask0&0x0000000000004000llu)>>10); //14	ok
  ConfigHR2[26]|=((tmask0&0x0000000000002000llu)>>12); 		//13	ok
  ConfigHR2[27]|=((tmask0&0x0000000000001000llu)>>6);//12 ok
  ConfigHR2[27]|=((tmask0&0x0000000000000800llu)>>8);//11ok
  ConfigHR2[27]|=((tmask0&0x0000000000000400llu)>>10); //10ok
  ConfigHR2[28]|=((tmask0&0x0000000000000200llu)>>4);//9ok
  ConfigHR2[28]|=((tmask0&0x0000000000000100llu)>>6);//8ok
  ConfigHR2[29]|=((tmask0&0x0000000000000080llu)>>0);//7ok
  ConfigHR2[29]|=((tmask0&0x0000000000000040llu)>>2);//6ok
  ConfigHR2[29]|=((tmask0&0x0000000000000020llu)>>4);//5ok
  ConfigHR2[30]|=((tmask0&0x0000000000000010llu)<<2);//4 ok
  ConfigHR2[30]|=((tmask0&0x0000000000000008llu)<<0);//3ok
  ConfigHR2[30]|=((tmask0&0x0000000000000004llu)>>2);//2 ok
  ConfigHR2[31]|=((tmask0&0x0000000000000002llu)<<4);//1 ok
  ConfigHR2[31]|=((tmask0&0x0000000000000001llu)<<2);//0ok
  ConfigHR2[31]|=((itHR2->getInt("RS_OR_DISCRI")&0x01)<<1);
  ConfigHR2[31]|=((itHR2->getInt("DISCRI1")&0x01)<<0);
  ConfigHR2[32]|=((itHR2->getInt("DISCRI2")&0x01)<<7);
  ConfigHR2[32]|=((itHR2->getInt("DISCRI0")&0x01)<<6);
  ConfigHR2[32]|=((itHR2->getInt("OTAQ_PWRADC")&0x01)<<5);
  ConfigHR2[32]|=((itHR2->getInt("EN_OTAQ")&0x01)<<4);
  ConfigHR2[32]|=((itHR2->getInt("SW50F0")&0x01)<<3);
  ConfigHR2[32]|=((itHR2->getInt("SW100F0")&0x01)<<2);
  ConfigHR2[32]|=((itHR2->getInt("SW100K0")&0x01)<<1);
  ConfigHR2[32]|=((itHR2->getInt("SW50K0")&0x01)<<0);
  ConfigHR2[33]|=((itHR2->getInt("PWRONFSB1")&0x01)<<7);
  ConfigHR2[33]|=((itHR2->getInt("PWRONFSB2")&0x01)<<6);
  ConfigHR2[33]|=((itHR2->getInt("PWRONFSB0")&0x01)<<5);
  ConfigHR2[33]|=((itHR2->getInt("SEL1")&0x01)<<4);
  ConfigHR2[33]|=((itHR2->getInt("SEL0")&0x01)<<3);
  ConfigHR2[33]|=((itHR2->getInt("SW50F1")&0x01)<<2);
  ConfigHR2[33]|=((itHR2->getInt("SW100F1")&0x01)<<1);
  ConfigHR2[33]|=((itHR2->getInt("SW100K1")&0x01)<<0);
  ConfigHR2[34]|=((itHR2->getInt("SW50K1")&0x01)<<7);
  ConfigHR2[34]|=((itHR2->getInt("CMDB0FSB1")&0x01)<<6);
  ConfigHR2[34]|=((itHR2->getInt("CMDB1FSB1")&0x01)<<5);
  ConfigHR2[34]|=((itHR2->getInt("CMDB2FSB1")&0x01)<<4);
  ConfigHR2[34]|=((itHR2->getInt("CMDB3FSB1")&0x01)<<3);
  ConfigHR2[34]|=((itHR2->getInt("SW50F2")&0x01)<<2);
  ConfigHR2[34]|=((itHR2->getInt("SW100F2")&0x01)<<1);
  ConfigHR2[34]|=((itHR2->getInt("SW100K2")&0x01)<<0);
  ConfigHR2[35]|=((itHR2->getInt("SW50K2")&0x01)<<7);
  ConfigHR2[35]|=((itHR2->getInt("CMDB0FSB2")&0x01)<<6);
  ConfigHR2[35]|=((itHR2->getInt("CMDB1FSB2")&0x01)<<5);
  ConfigHR2[35]|=((itHR2->getInt("CMDB2FSB2")&0x01)<<4);
  ConfigHR2[35]|=((itHR2->getInt("CMDB3FSB2")&0x01)<<3);
  ConfigHR2[35]|=((itHR2->getInt("PWRONW")&0x01)<<2);
  ConfigHR2[35]|=((itHR2->getInt("PWRONSS")&0x01)<<1);
  ConfigHR2[35]|=((itHR2->getInt("PWRONBUFF")&0x01)<<0);
  ConfigHR2[36]|=((itHR2->getInt("SWSSC")&0x07)<<5);
  ConfigHR2[36]|=((itHR2->getInt("CMDB0SS")&0x01)<<4);
  ConfigHR2[36]|=((itHR2->getInt("CMDB1SS")&0x01)<<3);
  ConfigHR2[36]|=((itHR2->getInt("CMDB2SS")&0x01)<<2);
  ConfigHR2[36]|=((itHR2->getInt("CMDB3SS")&0x01)<<1);
  ConfigHR2[36]|=((itHR2->getInt("PWRONPA")&0x01)<<0);
  std::vector<int> gain=itHR2->getIntVector("PAGAIN");
  for (uint32_t ip=0;ip<64;ip++)
    ConfigHR2[100-ip]|=(gain[ip]&0xFF);
  unsigned long long tctest;
  sscanf(itHR2->getString("CTEST").c_str(),"%llx\n",&tctest);
  tctest=0;

  ConfigHR2[101]=((tctest&0xFF00000000000000llu)>>56);
  ConfigHR2[102]=((tctest&0x00FF000000000000llu)>>48);
  ConfigHR2[103]=((tctest&0x0000FF0000000000llu)>>40);
  ConfigHR2[104]=((tctest&0x000000FF00000000llu)>>32);
  ConfigHR2[105]=((tctest&0x00000000FF000000llu)>>24);
  ConfigHR2[106]=((tctest&0x0000000000FF0000llu)>>16);
  ConfigHR2[107]=((tctest&0x000000000000FF00llu)>>8);
  ConfigHR2[108]=((tctest&0x00000000000000FFllu)>>0);

}

void lydaq::OracleDIFDBInterface::storeMicroRocConfiguration(Asic* itMR,unsigned char* ConfigMR)
{
  unsigned long long mask0;
  unsigned long long mask1;
  unsigned long long mask2;
  unsigned long long cTest;
//  std::string tB0B3;
  unsigned int tBB0;
  unsigned int tBB1;
  unsigned int tBB2;

  sscanf(itMR->getString("MASK0").c_str(),"%llx\n",&mask0);				
  sscanf(itMR->getString("MASK1").c_str(),"%llx\n",&mask1);				
  sscanf(itMR->getString("MASK2").c_str(),"%llx\n",&mask2);		
  sscanf(itMR->getString("CTEST").c_str(),"%llx\n",&cTest);		
//  tB0B3 =itMR->getString("B0B3");
  tBB0 =itMR->getInt("BB0");
  tBB1 =itMR->getInt("BB1");
  tBB2 =itMR->getInt("BB2");

  tBB2= ~tBB2;							
  tBB1= ~tBB1;							
  tBB0= ~tBB0;							
  int tch=0;
//  int B0B3Gain[64];


	std::vector<int> B0B3Gain=itMR->getIntVector("B0B3");

					
  ConfigMR[0]=((itMR->getInt("ENOCDOUT1B")&0x01)<<7);
  ConfigMR[0]|=((itMR->getInt("ENOCDOUT2B")&0x01)<<6);			
  ConfigMR[0]|=((itMR->getInt("ENOCTRANSMITON1B")&0x01)<<5);			
  ConfigMR[0]|=((itMR->getInt("ENOCTRANSMITON2B")&0x01)<<4);			
  ConfigMR[0]|=((itMR->getInt("ENOCCHIPSATB")&0x01)<<3);			
  ConfigMR[0]|=((itMR->getInt("SELSTARTREADOUT")&0x01)<<2);			
  ConfigMR[0]|=((itMR->getInt("SELENDREADOUT")&0x01)<<1);	

  ConfigMR[1] =((itMR->getInt("SELRAZ0")&0x01)<<6);		
  ConfigMR[1]|=((itMR->getInt("SELRAZ1")&0x01)<<5);			
  ConfigMR[1]|=((itMR->getInt("CKMUX")&0x01)<<4);			
  ConfigMR[1]|=((itMR->getInt("SCON")&0x01)<<3);			
  ConfigMR[1]|=((itMR->getInt("RAZCHNEXTVAL")&0x01)<<2);			
  ConfigMR[1]|=((itMR->getInt("RAZCHNINTVAL")&0x01)<<1);			
  ConfigMR[1]|=((itMR->getInt("TRIGEXTVAL")&0x01)<<0);		
							
  ConfigMR[2] =((itMR->getInt("DISCOROR")&0x01)<<7);			
  ConfigMR[2]|=((itMR->getInt("ENTRIGOUT")&0x01)<<6);			
  ConfigMR[2]|=((itMR->getInt("TRIG0B")&0x01)<<5);			
  ConfigMR[2]|=((itMR->getInt("TRIG1B")&0x01)<<4);			
  ConfigMR[2]|=((itMR->getInt("TRIG2B")&0x01)<<3);											
  ConfigMR[2]|=((tBB2&0x01)<<2);			//bit 0
  ConfigMR[2]|=((tBB2&0x02)>>0);			// bit 1
  ConfigMR[2]|=((tBB2&0x04)>>2);			// bit 2
								
  ConfigMR[3] =((tBB2&0x08)<<4);  //bit 3
  ConfigMR[3]|=((tBB2&0x10)<<2); // bit 4
  ConfigMR[3]|=((tBB2&0x20)<<0);    // bit 5
  ConfigMR[3]|=((tBB2&0x40)>>2);    // bit 6
  ConfigMR[3]|=((tBB2&0x80)>>4);    // bit 7
  ConfigMR[3]|=((tBB2&0x100)>>6);    // bit 8
  ConfigMR[3]|=((tBB2&0x200)>>8);    // bit 9
  ConfigMR[3]|=((tBB1&0x01)>>0);	  // bit 0		

  ConfigMR[4]= ((tBB1&0x02)<<6);			// bit 1
  ConfigMR[4]|=((tBB1&0x04)<<4);			// bit 2
  ConfigMR[4]|=((tBB1&0x08)<<2);			// bit 3
  ConfigMR[4]|=((tBB1&0x10)<<0);			// bit 4
  ConfigMR[4]|=((tBB1&0x20)>>2);			// bit 5
  ConfigMR[4]|=((tBB1&0x40)>>4);			// bit 6
  ConfigMR[4]|=((tBB1&0x80)>>6);			// bit 7
  ConfigMR[4]|=((tBB1&0x100)>>8);			// bit 8

  ConfigMR[5]= ((tBB1&0x200)>>2);			// bit 9
  ConfigMR[5]|=((tBB0&0x01)<<6);			// bit 0
  ConfigMR[5]|=((tBB0&0x02)<<4);			// bit 1
  ConfigMR[5]|=((tBB0&0x04)<<2);			// bit 2
  ConfigMR[5]|=((tBB0&0x08)<<0);			// bit 3
  ConfigMR[5]|=((tBB0&0x10)>>2);			// bit 4
  ConfigMR[5]|=((tBB0&0x20)>>4);			// bit 5
  ConfigMR[5]|=((tBB0&0x40)>>6);			// bit 6

  ConfigMR[6]= ((tBB0&0x80)<<0);				// bit 7
  ConfigMR[6]|=((tBB0&0x100)>>2);			// bit 8
  ConfigMR[6]|=((tBB0&0x200)>>4);			// bit 9
  ConfigMR[6]|=((itMR->getInt("PWRONDAC")&0x01)<<4);			
  ConfigMR[6]|=((itMR->getInt("ENPPDAC")&0x01)<<3);			
  ConfigMR[6]|=((itMR->getInt("PWRONBG")&0x01)<<2);			
  ConfigMR[6]|=((itMR->getInt("ENPPBANDGAP")&0x01)<<1);			
  ConfigMR[6]|=((itMR->getInt("HEADER")&0x01));			

  ConfigMR[7]= ((itMR->getInt("HEADER")&0x02)<<6);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x04)<<4);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x08)<<2);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x10)<<0);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x20)>>2);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x40)>>4);			
  ConfigMR[7]|=((itMR->getInt("HEADER")&0x80)>6);			
  ConfigMR[7]|=((mask2&0x8000000000000000llu)>>63);			

  ConfigMR[8]=  ((mask1&0x8000000000000000llu)>>56);			
  ConfigMR[8]|= ((mask0&0x8000000000000000llu)>>57);			
  ConfigMR[8]|= ((mask2&0x4000000000000000llu)>>57);			
  ConfigMR[8]|= ((mask1&0x4000000000000000llu)>>58);			
  ConfigMR[8]|= ((mask0&0x4000000000000000llu)>>59);			
  ConfigMR[8]|= ((mask2&0x2000000000000000llu)>>59);			
  ConfigMR[8]|= ((mask1&0x2000000000000000llu)>>60);			
  ConfigMR[8]|= ((mask0&0x2000000000000000llu)>>61);			

  ConfigMR[9]=  ((mask2&0x1000000000000000llu)>>53);			
  ConfigMR[9]|= ((mask1&0x1000000000000000llu)>>54);			
  ConfigMR[9]|= ((mask0&0x1000000000000000llu)>>55);			
  ConfigMR[9]|= ((mask2&0x0800000000000000llu)>>55);			
  ConfigMR[9]|= ((mask1&0x0800000000000000llu)>>56);			
  ConfigMR[9]|= ((mask0&0x0800000000000000llu)>>57);			
  ConfigMR[9]|= ((mask2&0x0400000000000000llu)>>57);			
  ConfigMR[9]|= ((mask1&0x0400000000000000llu)>>58);			

  ConfigMR[10] = ((mask0&0x0400000000000000llu)>>51);			
  ConfigMR[10]|= ((mask2&0x0200000000000000llu)>>51);			
  ConfigMR[10]|= ((mask1&0x0200000000000000llu)>>52);			
  ConfigMR[10]|= ((mask0&0x0200000000000000llu)>>53);			
  ConfigMR[10]|= ((mask2&0x0100000000000000llu)>>53);			
  ConfigMR[10]|= ((mask1&0x0100000000000000llu)>>54);			
  ConfigMR[10]|= ((mask0&0x0100000000000000llu)>>55);			
  ConfigMR[10]|= ((mask2&0x0080000000000000llu)>>55);			

  ConfigMR[11] = ((mask1&0x0080000000000000llu)>>48);			
  ConfigMR[11]|= ((mask0&0x0080000000000000llu)>>49);			
  ConfigMR[11]|= ((mask2&0x0040000000000000llu)>>49);			
  ConfigMR[11]|= ((mask1&0x0040000000000000llu)>>50);			
  ConfigMR[11]|= ((mask0&0x0040000000000000llu)>>51);			
  ConfigMR[11]|= ((mask2&0x0020000000000000llu)>>51);			
  ConfigMR[11]|= ((mask1&0x0020000000000000llu)>>52);			
  ConfigMR[11]|= ((mask0&0x0020000000000000llu)>>53);			

  ConfigMR[12] = ((mask2&0x0010000000000000llu)>>45);			
  ConfigMR[12]|= ((mask1&0x0010000000000000llu)>>46);			
  ConfigMR[12]|= ((mask0&0x0010000000000000llu)>>47);			
  ConfigMR[12]|= ((mask2&0x0008000000000000llu)>>47);			
  ConfigMR[12]|= ((mask1&0x0008000000000000llu)>>48);			
  ConfigMR[12]|= ((mask0&0x0008000000000000llu)>>49);			
  ConfigMR[12]|= ((mask2&0x0004000000000000llu)>>49);			
  ConfigMR[12]|= ((mask1&0x0004000000000000llu)>>50);			

  ConfigMR[13] = ((mask0&0x0004000000000000llu)>>43);			
  ConfigMR[13]|= ((mask2&0x0002000000000000llu)>>43);			
  ConfigMR[13]|= ((mask1&0x0002000000000000llu)>>44);			
  ConfigMR[13]|= ((mask0&0x0002000000000000llu)>>45);			
  ConfigMR[13]|= ((mask2&0x0001000000000000llu)>>45);			
  ConfigMR[13]|= ((mask1&0x0001000000000000llu)>>46);			
  ConfigMR[13]|= ((mask0&0x0001000000000000llu)>>47);			
  ConfigMR[13]|= ((mask2&0x0000800000000000llu)>>47);			

  ConfigMR[14] = ((mask1&0x0000800000000000llu)>>40);			
  ConfigMR[14]|= ((mask0&0x0000800000000000llu)>>41);			
  ConfigMR[14]|= ((mask2&0x0000400000000000llu)>>41);			
  ConfigMR[14]|= ((mask1&0x0000400000000000llu)>>42);			
  ConfigMR[14]|= ((mask0&0x0000400000000000llu)>>43);			
  ConfigMR[14]|= ((mask2&0x0000200000000000llu)>>43);			
  ConfigMR[14]|= ((mask1&0x0000200000000000llu)>>44);			
  ConfigMR[14]|= ((mask0&0x0000200000000000llu)>>45);			

  ConfigMR[15] = ((mask2&0x0000100000000000llu)>>37);			
  ConfigMR[15]|= ((mask1&0x0000100000000000llu)>>38);			
  ConfigMR[15]|= ((mask0&0x0000100000000000llu)>>39);			
  ConfigMR[15]|= ((mask2&0x0000080000000000llu)>>39);			
  ConfigMR[15]|= ((mask1&0x0000080000000000llu)>>40);			
  ConfigMR[15]|= ((mask0&0x0000080000000000llu)>>41);			
  ConfigMR[15]|= ((mask2&0x0000040000000000llu)>>41);			
  ConfigMR[15]|= ((mask1&0x0000040000000000llu)>>42);			

  ConfigMR[16] = ((mask0&0x0000040000000000llu)>>35);			
  ConfigMR[16]|= ((mask2&0x0000020000000000llu)>>35);			
  ConfigMR[16]|= ((mask1&0x0000020000000000llu)>>36);			
  ConfigMR[16]|= ((mask0&0x0000020000000000llu)>>37);			
  ConfigMR[16]|= ((mask2&0x0000010000000000llu)>>37);			
  ConfigMR[16]|= ((mask1&0x0000010000000000llu)>>38);			
  ConfigMR[16]|= ((mask0&0x0000010000000000llu)>>39);			
  ConfigMR[16]|= ((mask2&0x0000008000000000llu)>>39);			

  ConfigMR[17] = ((mask1&0x0000008000000000llu)>>32);			
  ConfigMR[17]|= ((mask0&0x0000008000000000llu)>>33);			
  ConfigMR[17]|= ((mask2&0x0000004000000000llu)>>33);			
  ConfigMR[17]|= ((mask1&0x0000004000000000llu)>>34);			
  ConfigMR[17]|= ((mask0&0x0000004000000000llu)>>35);			
  ConfigMR[17]|= ((mask2&0x0000002000000000llu)>>35);			
  ConfigMR[17]|= ((mask1&0x0000002000000000llu)>>36);			
  ConfigMR[17]|= ((mask0&0x0000002000000000llu)>>37);			

  ConfigMR[18] = ((mask2&0x0000001000000000llu)>>29);			
  ConfigMR[18]|= ((mask1&0x0000001000000000llu)>>30);			
  ConfigMR[18]|= ((mask0&0x0000001000000000llu)>>31);			
  ConfigMR[18]|= ((mask2&0x0000000800000000llu)>>31);			
  ConfigMR[18]|= ((mask1&0x0000000800000000llu)>>32);			
  ConfigMR[18]|= ((mask0&0x0000000800000000llu)>>33);			
  ConfigMR[18]|= ((mask2&0x0000000400000000llu)>>33);			
  ConfigMR[18]|= ((mask1&0x0000000400000000llu)>>34);			

  ConfigMR[19] = ((mask0&0x0000000400000000llu)>>27);			
  ConfigMR[19]|= ((mask2&0x0000000200000000llu)>>27);			
  ConfigMR[19]|= ((mask1&0x0000000200000000llu)>>28);			
  ConfigMR[19]|= ((mask0&0x0000000200000000llu)>>29);			
  ConfigMR[19]|= ((mask2&0x0000000100000000llu)>>29);			
  ConfigMR[19]|= ((mask1&0x0000000100000000llu)>>30);			
  ConfigMR[19]|= ((mask0&0x0000000100000000llu)>>31);			
  ConfigMR[19]|= ((mask2&0x0000000080000000llu)>>31);			

  ConfigMR[20] = ((mask1&0x0000000080000000llu)>>24);			
  ConfigMR[20]|= ((mask0&0x0000000080000000llu)>>25);			
  ConfigMR[20]|= ((mask2&0x0000000040000000llu)>>25);			
  ConfigMR[20]|= ((mask1&0x0000000040000000llu)>>26);			
  ConfigMR[20]|= ((mask0&0x0000000040000000llu)>>27);			
  ConfigMR[20]|= ((mask2&0x0000000020000000llu)>>27);			
  ConfigMR[20]|= ((mask1&0x0000000020000000llu)>>28);			
  ConfigMR[20]|= ((mask0&0x0000000020000000llu)>>29);			

  ConfigMR[21] = ((mask2&0x0000000010000000llu)>>21);			
  ConfigMR[21]|= ((mask1&0x0000000010000000llu)>>22);			
  ConfigMR[21]|= ((mask0&0x0000000010000000llu)>>23);			
  ConfigMR[21]|= ((mask2&0x0000000008000000llu)>>23);			
  ConfigMR[21]|= ((mask1&0x0000000008000000llu)>>24);			
  ConfigMR[21]|= ((mask0&0x0000000008000000llu)>>25);			
  ConfigMR[21]|= ((mask2&0x0000000004000000llu)>>25);			
  ConfigMR[21]|= ((mask1&0x0000000004000000llu)>>26);			

  ConfigMR[22] = ((mask0&0x0000000004000000llu)>>19);			
  ConfigMR[22]|= ((mask2&0x0000000002000000llu)>>19);			
  ConfigMR[22]|= ((mask1&0x0000000002000000llu)>>20);			
  ConfigMR[22]|= ((mask0&0x0000000002000000llu)>>21);			
  ConfigMR[22]|= ((mask2&0x0000000001000000llu)>>21);			
  ConfigMR[22]|= ((mask1&0x0000000001000000llu)>>22);			
  ConfigMR[22]|= ((mask0&0x0000000001000000llu)>>23);			
  ConfigMR[22]|= ((mask2&0x0000000000800000llu)>>23);			

  ConfigMR[23] = ((mask1&0x0000000000800000llu)>>16);			
  ConfigMR[23]|= ((mask0&0x0000000000800000llu)>>17);			
  ConfigMR[23]|= ((mask2&0x0000000000400000llu)>>17);			
  ConfigMR[23]|= ((mask1&0x0000000000400000llu)>>18);			
  ConfigMR[23]|= ((mask0&0x0000000000400000llu)>>19);			
  ConfigMR[23]|= ((mask2&0x0000000000200000llu)>>19);			
  ConfigMR[23]|= ((mask1&0x0000000000200000llu)>>20);			
  ConfigMR[23]|= ((mask0&0x0000000000200000llu)>>21);			

  ConfigMR[24] = ((mask2&0x0000000000100000llu)>>13);			
  ConfigMR[24]|= ((mask1&0x0000000000100000llu)>>14);			
  ConfigMR[24]|= ((mask0&0x0000000000100000llu)>>15);			
  ConfigMR[24]|= ((mask2&0x0000000000080000llu)>>15);			
  ConfigMR[24]|= ((mask1&0x0000000000080000llu)>>16);			
  ConfigMR[24]|= ((mask0&0x0000000000080000llu)>>17);			
  ConfigMR[24]|= ((mask2&0x0000000000040000llu)>>17);			
  ConfigMR[24]|= ((mask1&0x0000000000040000llu)>>18);			
	
  ConfigMR[25] = ((mask0&0x0000000000040000llu)>>11);			
  ConfigMR[25]|= ((mask2&0x0000000000020000llu)>>11);			
  ConfigMR[25]|= ((mask1&0x0000000000020000llu)>>12);			
  ConfigMR[25]|= ((mask0&0x0000000000020000llu)>>13);			
  ConfigMR[25]|= ((mask2&0x0000000000010000llu)>>13);			
  ConfigMR[25]|= ((mask1&0x0000000000010000llu)>>14);			
  ConfigMR[25]|= ((mask0&0x0000000000010000llu)>>15);			
  ConfigMR[25]|= ((mask2&0x0000000000008000llu)>>15);			
	
  ConfigMR[26] = ((mask1&0x0000000000008000llu)>>8);			
  ConfigMR[26]|= ((mask0&0x0000000000008000llu)>>9);			
  ConfigMR[26]|= ((mask2&0x0000000000004000llu)>>9);			
  ConfigMR[26]|= ((mask1&0x0000000000004000llu)>>10);			
  ConfigMR[26]|= ((mask0&0x0000000000004000llu)>>11);			
  ConfigMR[26]|= ((mask2&0x0000000000002000llu)>>11);			
  ConfigMR[26]|= ((mask1&0x0000000000002000llu)>>12);			
  ConfigMR[26]|= ((mask0&0x0000000000002000llu)>>13);			
	
  ConfigMR[27] = ((mask2&0x0000000000001000llu)>>5);			
  ConfigMR[27]|= ((mask1&0x0000000000001000llu)>>6);			
  ConfigMR[27]|= ((mask0&0x0000000000001000llu)>>7);			
  ConfigMR[27]|= ((mask2&0x0000000000000800llu)>>7);			
  ConfigMR[27]|= ((mask1&0x0000000000000800llu)>>8);			
  ConfigMR[27]|= ((mask0&0x0000000000000800llu)>>9);			
  ConfigMR[27]|= ((mask2&0x0000000000000400llu)>>9);			
  ConfigMR[27]|= ((mask1&0x0000000000000400llu)>>10);			
	
  ConfigMR[28] = ((mask0&0x0000000000000400llu)>>3);			
  ConfigMR[28]|= ((mask2&0x0000000000000200llu)>>3);			
  ConfigMR[28]|= ((mask1&0x0000000000000200llu)>>4);			
  ConfigMR[28]|= ((mask0&0x0000000000000200llu)>>5);			
  ConfigMR[28]|= ((mask2&0x0000000000000100llu)>>5);			
  ConfigMR[28]|= ((mask1&0x0000000000000100llu)>>6);			
  ConfigMR[28]|= ((mask0&0x0000000000000100llu)>>7);			
  ConfigMR[28]|= ((mask2&0x0000000000000080llu)>>7);			
	
  ConfigMR[29] = ((mask1&0x0000000000000080llu)>>0);			
  ConfigMR[29]|= ((mask0&0x0000000000000080llu)>>1);			
  ConfigMR[29]|= ((mask2&0x0000000000000040llu)>>1);			
  ConfigMR[29]|= ((mask1&0x0000000000000040llu)>>2);			
  ConfigMR[29]|= ((mask0&0x0000000000000040llu)>>3);			
  ConfigMR[29]|= ((mask2&0x0000000000000020llu)>>3);			
  ConfigMR[29]|= ((mask1&0x0000000000000020llu)>>4);			
  ConfigMR[29]|= ((mask0&0x0000000000000020llu)>>5);			
	
  ConfigMR[30] = ((mask2&0x0000000000000010llu)<<3);			
  ConfigMR[30]|= ((mask1&0x0000000000000010llu)<<2);			
  ConfigMR[30]|= ((mask0&0x0000000000000010llu)<<1);			
  ConfigMR[30]|= ((mask2&0x0000000000000008llu)<<1);			
  ConfigMR[30]|= ((mask1&0x0000000000000008llu)<<0);			
  ConfigMR[30]|= ((mask0&0x0000000000000008llu)>>1);			
  ConfigMR[30]|= ((mask2&0x0000000000000004llu)>>1);			
  ConfigMR[30]|= ((mask1&0x0000000000000004llu)>>2);			
	
  ConfigMR[31] = ((mask0&0x0000000000000004llu)<<5);			
  ConfigMR[31]|= ((mask2&0x0000000000000002llu)<<5);			
  ConfigMR[31]|= ((mask1&0x0000000000000002llu)<<4);			
  ConfigMR[31]|= ((mask0&0x0000000000000002llu)<<3);			
  ConfigMR[31]|= ((mask2&0x0000000000000001llu)<<3);			
  ConfigMR[31]|= ((mask1&0x0000000000000001llu)<<2);			
  ConfigMR[31]|= ((mask0&0x0000000000000001llu)<<1);			
  ConfigMR[31]|= ((itMR->getInt("RS_OR_DISCRI")&0x01));			
	
  ConfigMR[32]=  ((itMR->getInt("DISCRI1_PP")&0x01)<<7);			
  ConfigMR[32]|= ((itMR->getInt("DISCRI2_PP")&0x01)<<6);			
  ConfigMR[32]|= ((itMR->getInt("DISCRI0_PP")&0x01)<<5);			
  ConfigMR[32]|= ((itMR->getInt("ENPPOTAQ")&0x01)<<4);			
  ConfigMR[32]|= ((itMR->getInt("EN_OTAQ")&0x01)<<3);			
  ConfigMR[32]|= ((itMR->getInt("ENPPDAC4BIT")&0x01)<<2);	
  ConfigMR[32]|= ((B0B3Gain[63]&0x0f)>>2);

  for (int ti=0;ti<31;ti++)
    {							
      ConfigMR[33+ti] = ((B0B3Gain[(31-ti)*2+1]&0x03)<<6);
      ConfigMR[33+ti]|= ((B0B3Gain[(31-ti)*2]&0x0f)<<2);
      ConfigMR[33+ti]|= ((B0B3Gain[(31-ti)*2-1]&0x0c)>>2);							
    }

  ConfigMR[64] = ((B0B3Gain[1]&0x03)<<6);
  ConfigMR[64]|= ((B0B3Gain[0]&0x0f)<<2);
  ConfigMR[64]|= ((itMR->getInt("SWHG")&0x01)<<1);			
  ConfigMR[64]|= ((itMR->getInt("SWHG")&0x02)>>1);			

  ConfigMR[65] = ((itMR->getInt("VALIDSH")&0x01)<<7);			
  ConfigMR[65]|= ((itMR->getInt("PWRONWIDLAR")&0x01)<<6);			
  ConfigMR[65]|= ((itMR->getInt("SWLG")&0x01)<<5);			
  ConfigMR[65]|= ((itMR->getInt("SWLG")&0x02)<<3);			
  ConfigMR[65]|= ((itMR->getInt("PWRONSHLG")&0x01)<<3);			
  ConfigMR[65]|= ((itMR->getInt("PWRONSHHG")&0x01)<<2);			
  ConfigMR[65]|= ((itMR->getInt("ENGBST")&0x01)<<1);			// en_gbst
  ConfigMR[65]|= ((itMR->getInt("PWRONPA")&0x01)<<0);			

  ConfigMR[66]=((cTest&0xFF00000000000000llu)>>56);			
  ConfigMR[67]=((cTest&0x00FF000000000000llu)>>48);			
  ConfigMR[68]=((cTest&0x0000FF0000000000llu)>>40);			
  ConfigMR[69]=((cTest&0x000000FF00000000llu)>>32);			
  ConfigMR[70]=((cTest&0x00000000FF000000llu)>>24);			
  ConfigMR[71]=((cTest&0x0000000000FF0000llu)>>16);			
  ConfigMR[72]=((cTest&0x000000000000FF00llu)>>8);			
  ConfigMR[73]=((cTest&0x00000000000000FFllu)>>0);						

}
uint32_t lydaq::OracleDIFDBInterface::LoadAsicParameters()
{
  printf("Entering LoadAsicParameters \n");
  LOG4CXX_INFO(_logOracle,"Filling ASIC map ");
  std::vector<ConfigObject*> asic_vector_=_state->getAsicConfiguration()->getVector();
  for (std::vector<ConfigObject*>::iterator itMRp=asic_vector_.begin();itMRp!=asic_vector_.end();itMRp++)
    {
      Asic* itMR=(Asic*) (*itMRp);
      if (itMR->getInt("ENABLED")==0) continue;
      uint32_t key=(itMR->getInt("DIF_ID")<<8|itMR->getInt("HEADER"));

      if (theAsicKeyMap_.find(key)!=theAsicKeyMap_.end()) continue;
      std::string atype=itMR->getString("ASIC_TYPE");
      if (atype.compare("HR2")==0)
	{
	  unsigned char* buf= new unsigned char[110];
	  buf[0]=109;
	  storeHardRoc2Configuration(itMR,&buf[1]);
	  std::pair<uint32_t,unsigned char*> p(key,buf);
	  theAsicKeyMap_.insert(p);
	  continue;
	}
      if (atype.compare("MR")==0)
	{
	  unsigned char* buf= new unsigned char[75];
	  buf[0]=74;
	  storeMicroRocConfiguration(itMR,&buf[1]);
	  std::pair<uint32_t,unsigned char*> p(key,buf);
	  //printf("Adding MR asic key=%x \n",key);
	  theAsicKeyMap_.insert(p);
	  continue;
	}

    }
  return 0;
}
