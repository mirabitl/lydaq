
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMServer.h"
using namespace std;
#include <sstream>
class DIFHardware
{
  virtual void addDIF(uint32_t num)=0;
  virtual uint32_t writeSlowControl(uint32_t num,uint32_t nbasic,uint32_t framesize,unsigned char* cbuf)=0;
  virtual void writeSlowControlRegister(uint32_t num,uint32_t screg)=0;
  virtual uint32_t readdata(uint32_t num,unsigned char* cbuf)=0;
};

class UsbDIFHardware
{
  virtual void addDIF(uint32_t num)
  {
    std::map<uint32_t,UsbDIFDriver*>::iterator it = mDriver_.find(num);
    if (it==mDriver_.end())
      {
	char name[24];
	memset(name,0,24);
	sprintf(name,"FT101%.3d",num);
	UsbDIFDriver* d= new UsbDIFDriver(name);
	std::pair<uint32_t,UsbDIFDriver*> p(num,d);
	mDriver_.insert(p)
	  }
  }
  virtual void writeSlowControlRegister(uint32_t num,uint32_t screg)
  {
    std::map<uint32_t,UsbDIFDriver*>::iterator it = mDriver_.find(num);
    if (it==mDriver_.end()) return;
    it->second->->SetControlRegister(screg);
  }
  virtual uint32_t writeSlowControl(uint32_t num,uint32_t nbasic,uint32_t framesize,unsigned char* cbuf)
  {
    std::map<uint32_t,UsbDIFDriver*>::iterator it = mDriver_.find(num);
    if (it==mDriver_.end()) return 0xFFFFFFFF;
    unsigned char vframe[HARDROCV2_SLC_FRAME_SIZE]; 
    unsigned char** cslow = (unsigned char**) cbuf;
    // send configure command
    unsigned short tCRC;
    unsigned char CurrentCRC[2];   
    // set default register values
    tCRC=0xFFFF;							// initial value of the CRC
    try 
      {
	it->second->HardrocCommandSLCWrite();	
      }
    catch (HAL::HardwareAccessException& e)
      {
	std::cout<< num<<": Unable to send start SLC command to DIF"<<std::endl;
	throw;
      }

    for (int tAsic=nbasic;tAsic>0;tAsic--)
      {
	//      std::cout<<"Configuring "<<tAsic<<std::endl;
	for (int tbyte=0;tbyte<framesize;tbyte++)
	  {
	    printf("%02x",cslow[tAsic-1][tbyte] );
	    vframe[tbyte]=cslow[tAsic-1][tbyte];
	  }
	printf("\n");

	for (int tbyte=0;tbyte<framesize;tbyte++) 
	  {
	    tCRC=((tCRC&0xFF)<<8) ^ ( UsbDIFDriver::CrcTable[ (tCRC>>8)^theSlowBuffer_[tAsic-1][tbyte] ] );
	  } 
	try 
	  {

	    it->second->CommandSLCWriteSingleSLCFrame(vframe,framesize);
	    
	    
	  }
	catch (HAL::HardwareAccessException& e)
	  {
	    std::cout<<num<<":Unable to send SLC frame to DIF"<<std::endl;	
	    throw;
	  }

      }	//for (int tAsic=NbOfASICs;tAsic>0;tAsic--)
		
    uint32_t CurrentSLCStatus=0;
    int32_t tretry=0;
    while ((CurrentSLCStatus==0) && (tretry<5))
      {
	try {		it->second->HardrocSLCStatusRead(&CurrentSLCStatus);}
	catch (HAL::HardwareAccessException& e)	{	std::cout<<__PRETTY_FUNCTION__<<" "<<num<<"==>"<< "DCCDIFDataHandler : Unable to send command to DIF : "+(std::string)e.what()<<std::endl;	}
	tretry++;
	//      std::cout<<__PRETTY_FUNCTION__<<" "<<num<<"==>"<<toolbox::toString("****** 	CurrentSLCStatus = %ld",CurrentSLCStatus)<<std::endl;	
      }
    return CurrentSLCStatus;
  }
virtual uint32_t readdata(uint32_t num,unsigned char* CurrentDIFDigitalData)
  {
    std::map<uint32_t,UsbDIFDriver*>::iterator it = mDriver_.find(num);
    if (it==mDriver_.end()) return 0xFFFFFFFF;
    UsbDIFDriver* theDriver_=it->second;
    unsigned int ComputedCRC=0xFFFF;
    unsigned int ReceivedCRC=0xFFFF;
    unsigned char tdata;
    unsigned char tdata32[42];
    unsigned char tdataana[MAX_ANALOG_DATA_SIZE];
    int tHardrocIndex=0;
    int tMemoryIndex=0;
    int tBunchIndex=0;
    int tindex=0;
    double tefficacity=0;
    int tDIFIDIndex=0;	
    char tdisplay[600];			
    unsigned  tCurrentAbsoluteBCIDMSB;
    uint32_t tCurrentAbsoluteBCIDLSB;


    uint16_t theadersize;
    // on calcule le CRC a la volee			
    try	{	 
      theDriver_->HardrocReadByte(&tdata); 
    }  // Global header
    catch (HAL::HardwareAccessException& e)	  
      {	 printf("DIF : no data, exiting %s",e.what());  return 0;}	
    uint32_t CurrentNbOfEventsPerTrigger=0;

    //  printf ("%02x",tdata);

    if ((tdata != 0xB0) && (tdata != 0xBB))		// global header
      {	
	printf("DIF : Bad global header(0x%02x instead of 0xb0 or 0xbb), exiting",tdata);	
	theDriver_->HardrocFastFlushDigitalFIFO();
	return 0;
      }	
    tindex=0;
  
    CurrentDIFDigitalData[tindex++]=tdata;		// global header
    ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );			// global header
  
    tDIFIDIndex=tindex;
    // DIF ID  			
    //DIF trigger counter  (32 bits)	
    //acq trigger counter  (32 bits)		
    //global trigger counter  (32 bits)
    //absolute bcid counter (48bits, natural binary, MSB first) 
    //timedif counter (24bits, natural binary, MSB first)
    if ( tdata ==0xb0) theadersize=23; 
    else theadersize = 32;

    try	{	    theDriver_->HardrocReadnBytes(tdata32,theadersize); 	  }  
    catch (HAL::HardwareAccessException& e)
      {
	printf("DIF %d :  There should be a DIF ID",num);	
	//DoFlushDigitalFIFOs();
		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
      }
	
    for (int ti=0;ti<theadersize;ti++)
      {
	//printf ("%02x",tdata32[ti]);
	CurrentDIFDigitalData[tindex++]=tdata32[ti];
	ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(tdata32[ti]&0xFF))&0xFF ] );		// dif_id
      }
    //printf ("\n");
    uint32_t CurrentDifId = tdata32[0];
    // printf ("DIF = %d\n",CurrentDifId);
    if (CurrentDifId!=theDIFId_)
      {
	printf("DIF %d :  Invalid  DIF ID %d",num,CurrentDifId);	
	//DoFlushDigitalFIFOs();
		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
      }
    CurrentDIFDigitalData[tDIFIDIndex]=CurrentDifId;

    uint32_t CurrentDIFTriggerCounter=tdata32[1]<<24;
    CurrentDIFTriggerCounter+=tdata32[2]<<16;
    CurrentDIFTriggerCounter+=tdata32[3]<<8;
    CurrentDIFTriggerCounter+=tdata32[4];
    //  std::cout<<CurrentDIFTriggerCounter<<" is read"<<std::endl;
    uint32_t CurrentAcqTriggerCounter=tdata32[5]<<24;
    CurrentAcqTriggerCounter+=tdata32[6]<<16;
    CurrentAcqTriggerCounter+=tdata32[7]<<8;
    CurrentAcqTriggerCounter+=tdata32[8];
		 		
    uint32_t CurrentGlobalTriggerCounter=tdata32[9]<<24;
    CurrentGlobalTriggerCounter+=tdata32[10]<<16;
    CurrentGlobalTriggerCounter+=tdata32[11]<<8;
    CurrentGlobalTriggerCounter+=tdata32[12];

    tCurrentAbsoluteBCIDMSB=tdata32[13]<<8;
    tCurrentAbsoluteBCIDMSB+=tdata32[14];

    tCurrentAbsoluteBCIDLSB=tdata32[15]<<24;
    tCurrentAbsoluteBCIDLSB+=tdata32[16]<<16;
    tCurrentAbsoluteBCIDLSB+=tdata32[17]<<8;
    tCurrentAbsoluteBCIDLSB+=tdata32[18];

    uint32_t CurrentTimeDif=tdata32[19]<<16;
    CurrentTimeDif+=tdata32[20]<<8;
    CurrentTimeDif+=tdata32[21];
	  
    uint32_t CurrentNbLines=tdata32[22]>>4;
    //std::cout<<"CurrentNbLines="<<CurrentNbLines<<std::endl;
    uint32_t CurrentTemperatureASU1;
    uint32_t CurrentTemperatureASU2;
    uint32_t CurrentTemperatureDIF;
    if ( tdata ==0xbb) 
      {
	CurrentTemperatureASU1=tdata32[23]<<24;
	CurrentTemperatureASU1+=tdata32[24]<<16;
	CurrentTemperatureASU1+=tdata32[25]<<8;
	CurrentTemperatureASU1+=tdata32[26];

	CurrentTemperatureASU2=tdata32[27]<<24;
	CurrentTemperatureASU2+=tdata32[28]<<16;
	CurrentTemperatureASU2+=tdata32[29]<<8;
	CurrentTemperatureASU2+=tdata32[30];

	CurrentTemperatureDIF=tdata32[31];
		
	//		printf ("Temps = %d %d %d \n",CurrentTemperatureDIF,CurrentTemperatureASU1,CurrentTemperatureASU2);
      }

    uint32_t CurrentHardrocHeader=0;
    int tcontinue=1;
    while (tcontinue)
      {	
  	try	
	  {	
	    theDriver_->HardrocReadByte(&tdata);	
	  }//frame hearder or Global trailer 
	catch (HAL::HardwareAccessException& e)
	  {
	    printf("DIF %d :  There should be a frame header/global trailer",num);	
	    //
	    	theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
	  }	
	CurrentDIFDigitalData[tindex++]=tdata;
	uint32_t theader=tdata;

	if (theader==0xC4) 				  // analog frame header
	  {	
	    for (uint32_t tl=0;tl<CurrentNbLines;tl++)	
	      {
		try {  theDriver_->HardrocReadByte(&tdata);	}//nb of chips on line tl 
		catch (HAL::HardwareAccessException& e){printf("DIF %d :  There should be number of chip on line %d",num,tl);			theDriver_->HardrocFastFlushDigitalFIFO();return 0;;}	
				
		CurrentDIFDigitalData[tindex++]=tdata;		
		uint32_t tanasize = tdata*64*2;
		if (tdata>12) printf ("erreur taille data analogiques = %d\n",tdata);
		try	{	    theDriver_->HardrocReadnBytes(tdataana,tanasize); 	  }  
		catch (HAL::HardwareAccessException& e){printf("DIF %d :  There should analog data",num);		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;}
		for (uint32_t ti=0;ti<tanasize;ti++)
		  {
		    CurrentDIFDigitalData[tindex++]=tdataana[ti];
		  }
	      } //for (uint32_t tl=0;tl<	CurrentNbLines;tl++)				
	    try {	 theDriver_->HardrocReadByte(&tdata);	}//0xD4 
	    catch (HAL::HardwareAccessException& e){printf("DIF %d :  There should be a 0xD4",num);		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;}	
	    //			printf ("%02x\n",tdata);
	    CurrentDIFDigitalData[tindex++]=tdata;		//0xD4
	  }
	else if (theader==0xB4) 				  // frame header
	  {
	    ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(theader&0xFF))&0xFF ] );			// B4 in crc but not C4
	    while (1)
	      {	
		try {	theDriver_->HardrocReadByte(&tdata);	}
		catch (HAL::HardwareAccessException& e){printf("DIF %d :  There should be a valid frame trailer/hardroc header",num); 	theDriver_->HardrocFastFlushDigitalFIFO();return 0;;}	
		if ((tdata != 0xA3)&&(tdata != 0xC3))		//not a frame trailer, so a hardroc header
		  {	
		    if (tMemoryIndex>ASIC_MEM_DEPTH)
		      {
		      	printf("DIF %d :  unable to read more than 128 Memory indexes",num);	
			
				theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
		      }		
		    ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );			// hardroc header
		    CurrentDIFDigitalData[tindex++]=tdata;
		    CurrentHardrocHeader=tdata;
		    CurrentNbOfEventsPerTrigger++;
		    if (theMonitoring_)  hasics->Fill(CurrentHardrocHeader*1.);
		    try {	 theDriver_->HardrocReadnBytes(tdata32,19);	}  // BCID
		    catch (HAL::HardwareAccessException& e){printf("DIF %d :  There should be a hardroc frame",num);			      	theDriver_->HardrocFastFlushDigitalFIFO();return 0;;}	
		    // bcid (3 bytes)
		    // data (16bytes)								
		    for (int ti=0;ti<19;ti++)
		      {
			CurrentDIFDigitalData[tindex++]=tdata32[ti];
		      	ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(tdata32[ti]&0xFF))&0xFF ] );		
		      }

		    tMemoryIndex++;
		    if( tMemoryIndex>ASIC_MEM_DEPTH) 
		      {
			printf("tMemoryIndex > ASIC_MEM_DEPTH");			      
				theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
		      }
		  }
	    	else //if ((tdata != 0xA3)&&(tdata != 0xC3))
		  {			//frame trailer	
		    if (tdata==0xC3)	
		      {
			printf(" %s Incomplete frame received (0xC3)",num);			      
		      	
		      		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
		      }	
		    ComputedCRC=((ComputedCRC&0xFF)<<8) ^ ( USBDRIVER::CrcTable[ ((ComputedCRC>>8)^(tdata&0xFF))&0xFF ] );		// frame trailer
		    CurrentDIFDigitalData[tindex++]=tdata;
		    tMemoryIndex=0;			// next hardroc, so mem index should be reseted
		    tHardrocIndex++;
		    if( tHardrocIndex>MAX_NB_OF_ASICS) 
		      {
		      	printf("tHardrocIndex > MAX_NB_OF_ASICS");	
		     
			
		      		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
		      }
		    break;
		  } //if ((tdata != 0xA3)&&(tdata != 0xC3))
	      }	//while (1)
	  }	//if (tdata==0xB4)
  	else if (tdata == 0xA0) 		// global trailer	
	  {	
	    try	{	  theDriver_->HardrocRead2Bytes(&ReceivedCRC);	}		// CRC
	    catch (HAL::HardwareAccessException& e)
	      {
	    	printf("DIF %d :  There should be a valid CRC",num);		     
	    	
	    		theDriver_->HardrocFastFlushDigitalFIFO();return 0;;
	      }	
	    CurrentDIFDigitalData[tindex++]=(ReceivedCRC>>8)&0xFF;
	    CurrentDIFDigitalData[tindex++]=(ReceivedCRC>>0)&0xFF;
	  	
	    if (ComputedCRC == ReceivedCRC)
	      {	
	    	tcontinue =0;
	      }	
	    else
	      {			
#ifdef DISPLAY_CRC
		printf("- CRC mismatch ( received 0x%04x instead of 0x%04x)",ReceivedCRC,ComputedCRC);	
#endif
	    	tcontinue =0;
	      }
	  }	//else if (tdata == 0xA0)
      }	//while (tcontinue) 
	
  }
private:
  std::map<uint32_t,UsbDIFDriver*> mDriver_;
};
class RPIServer: public NMServer
{
public:
  RPIServer(std::string host,uint32_t port,DIFHardware* dh,uint32_t dif1,uint32_t dif2,uint32_t dif3) : NMServer(host,port),difHardware_(dh)
  {
    
    this->registerCommand("START",boost::bind(&RPIServer::commandHandler,this,_1));
    this->registerCommand("STOP",boost::bind(&RPIServer::commandHandler,this,_1));

    if (dif1!=0) this->registerDIF(dif1);
    if (dif2!=0) this->registerDIF(dif2);
    if (dif3!=0) this->registerDIF(dif3);
    
    this->start(); // On demarre le serveur

    running_=false;
  }
  void registerDIF(uint32_t num)
  {
    std::stringstream sc("");
    sc<<"SC"<<num;
    this->registerCommand(sc.str(),boost::bind(&RPIServer::SCHandler,this,_1)); // StoreSlowControl
    std::stringstream scr("");
    sc<<"SCR"<<num;
    this->registerCommand(sc.str(),boost::bind(&RPIServer::SCRHandler,this,_1)); // StoreSlowControl

    std::stringstream data("");
    data<<"DIF"<<num;
    this->registerService(data.str());
    difList_.push_back(num);
    difHardware_->addDIF(num);
  }

  NetMessage* SCHandler(NetMessage* m)
  {

    // On fait le SC et on renvoie le code
    uint32_t* ibuf=(uint32_t*) m->getPayload();
    unsigned char* cbuf=m->getPayload();
    uint32_t nbAsic=ibuf[0];
    uint32_t frameSize=ibuf[1];
    unsigned char* slowBuffer=&cbuf[8];
    uint32_t num;
    sscanf(m->getName().c_str(),"SC%d",&num);
    std::vector<uint32_t>::iterator i = find(difList_.begin(),difList_.end(), num);
    if (i==difList_.end()) difList_.push_back(num);

    uint32_t rc=difHardware_->writeSlowControl(num,nbAsic,frameSize,slowBuffer);
    NetMessage* mrep=new NetMessage(m->getName,NetMessage::COMMAND_ACKNOWLEDGE,4);
    uint32_t* ibufr=(uint32_t*) mrep->getPayload();
    ibufr[0]=rc;
    return mrep;
  }

  NetMessage* SCRHandler(NetMessage* m)
  {

    // On fait le SC et on renvoie le code
    uint32_t* ibuf=(uint32_t*) m->getPayload();
    uint32_t slowcontrolregister=ibuf[0];
    uint32_t num;
    sscanf(m->getName().c_str(),"SC%d",&num);

    difHardware_->writeSlowControlRegister(num,slowControlRegister);
    return NULL;
  }

  NetMessage* commandHandler(NetMessage* m)
  {
    std::string start="START";
    std::string stop="STOP";
    
    printf(" J'ai recu %s COMMAND %s %s  \n",m->getName().c_str(),start.c_str(),stop.c_str());
    if (m->getName().compare(start)==0)
      {
	running_=true;
	this->startServices();
      }
    if (m->getName().compare(stop)==0)
      {
	running_=false;
	this->joinServices();
      }

    return NULL;
  }
  void startServices()
  {
    // On envoie le start au DIF ?
    // On demarre la lecture
    m_Thread_s = boost::thread(&RPIServer::readServices, this);  
  }
  void joinServices()
  {
    // On attend la fin des threads
    m_Thread_s.join();  
  }
  void readServices()
  {
    unsigned char CurrentDIFDigitalData[MAX_EVENT_SIZE];

    while (true)
      {
	usleep((uint32_t) 100);
	if (!running_) break;
	for (std::vector<uint32_t>::iterator i=difList_.begin();i!=difList_.end();i++)
	  {
	    uint32_t len =difHardware_->readData(i,CurrentDIFDigitalData);
	    DEBUG(" J'update UNESSAI \n");

	    if (len>0)
	      {
		std::stringstream s();
		s<<"DIF"<<i;
		NetMessage m(s.str(),NetMessage::SERVICE,len);
		//uint32_t* ibuf=(uint32_t*) m.getPayload();
		memcpy(m.getPayload(),CurrentDIFDigitalData,len);
		this->updateService(s.str(),&m);
	      }
	  }
      }
  }
private:
  bool running_;
  boost::thread    m_Thread_s;
  std::vector<uint32_t> difList_;
  DIFHardware* difHardware_;
};

int main(int argc, char **argv) {
  RPIServer s("lyopc252",atoi(argv[1]));
  //s.start();
	// NL::init();

	// cout << "\nStarting Server...";
	// cout.flush();

	// NL::Socket socketServer(SERVER_PORT);

	// NL::SocketGroup group;

	// OnAccept onAccept;
	// OnRead onRead;
	// OnDisconnect onDisconnect;

	// group.setCmdOnAccept(&onAccept);
	// group.setCmdOnRead(&onRead);
	// group.setCmdOnDisconnect(&onDisconnect);

	// group.add(&socketServer);

	// while(true) {

	// 	if(!group.listen(2000))
	// 		cout << "\nNo msg recieved during the last 2 seconds";
	// }
  while (true)
    { sleep((unsigned int) 100);}
}
