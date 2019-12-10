/************************************************************************/
/* ILC test beam daq							*/	
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/************************************************************************/
#include "FtdiDIFDriver.hh"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>

#define MY_DEBUG
//#define DEBUG_LOWLEVEL

//#define FLUSH_TO_FILE
//#define FAST_FLUSH_TO_FILE
using namespace lydaq;
int32_t CHardrocRegisterWrite(struct ftdi_context *ftdic,uint32_t address, uint32_t data)
{
	printf(" FTDI %p \n",ftdic);
	int32_t ret=0;
	uint32_t taddress;
	unsigned char  ttampon[7];

	taddress=address&0x3FFF;						// keep only 14 LSB, write, so bit 14=0,register mode, so bit 15=0
	ttampon[0] = (taddress>>8)&0xFF;
	ttampon[1] = taddress&0xFF;
	ttampon[2] = (data>>24)&0xFF;
	ttampon[3] = (data>>16)&0xFF;
	ttampon[4] = (data>>8)&0xFF;
	ttampon[5] = data&0xFF;

	ret=ftdi_write_data(ftdic, ttampon,6);

	return ret;
}	

lydaq::FtdiDIFDriver::FtdiDIFDriver(char * deviceIdentifier,uint32_t productid )    : lydaq::FtdiUsbDriver(deviceIdentifier,productid) 
{


}


int32_t FtdiDIFDriver :: NbAsicsWrite(uint32_t tnumber,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4 )    //throw (LocalHardwareException)
{
	uint32_t taddress=0x05;
	//printf ("nb of asics = %d\n",tnumber);
	
	tnumber += (l1<<8) + (l2<<14) + (l3<<20)+ (l4<<26);
	//	printf ("tnumber = %d\n",tnumber);
	try	{		UsbRegisterWrite2(taddress,tnumber);			}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2; }
	return 0;
}	

int32_t FtdiDIFDriver :: UsbSetDIFID(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x01;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2;}
	return 0;
}	

int32_t FtdiDIFDriver :: GetDIFID(uint32_t *tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x01;
	try	{		UsbRegisterRead(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocSetGeneratorDivision(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x200;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: NbAsicsRead(uint32_t *tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x05;

	try{	UsbRegisterRead(taddress,tnumber);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	



int32_t FtdiDIFDriver :: HardrocPwonDacDelayRead(uint32_t *tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x41;

	try{	UsbRegisterRead(taddress,tnumber);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocPwonDacDelayWrite(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x41;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocPwonAEndDelayRead(uint32_t *tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x40;

	try{	UsbRegisterRead(taddress,tnumber);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocPwonAEndDelayWrite(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x40;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocSLCStatusRead(uint32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x06;
	try	
	{		
		UsbRegisterRead(taddress,tstatus);
	}
	catch (LocalHardwareException& e)
	{
	  std::cout<<" HardrocSLCStatusRead "<< e.message()<<std::endl;throw (e);
		return -2;
	}
	return 0;

}	

int32_t FtdiDIFDriver :: HardrocSLCCRCStatusRead(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x06;
	uint32_t tdata;
	try	{		UsbRegisterRead(taddress,&tdata);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	if ((tdata&0x03)==0x01) // OK
	return 0;
	else
	return -1;	
}	

int32_t FtdiDIFDriver :: HardrocSLCLoadStatusRead(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x06;
	uint32_t tdata;

	try	{		UsbRegisterRead(taddress,&tdata);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	if ((tdata&0x0C)==0x04) // OK
	return 0;
	else
	return -1;	
}	

int32_t FtdiDIFDriver :: DIFMonitoringEnable(int32_t status)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFFE) +(status&0x01);
	try	{		UsbRegisterWrite(taddress,tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetDIFGain (int32_t gain)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFFD) +((gain&0x01)<<1);
	try{	UsbRegisterWrite(taddress,tstatus);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetSlabGain(int32_t gain)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFFB) +((gain&0x01)<<2);
	try	{		UsbRegisterWrite(taddress,tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetSequencer(int32_t status)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFF7) +((status&0x01)<<3);
	try	{		UsbRegisterWrite(taddress,tstatus);		}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetAVDDshdn (int32_t status)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFEF) +((status&0x01)<<4);
	try{UsbRegisterWrite(taddress,tstatus);		}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetDVDDshdn (int32_t status)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFFDF) +((status&0x01)<<5);
	try	{		UsbRegisterWrite(taddress,tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringSetConvertedChannels (int32_t channel)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	uint32_t tstatus;
	
	try	{		UsbRegisterRead(taddress,&tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tstatus =(tstatus&0xFF3F) +((channel&0x03)<<6);
	try	{		UsbRegisterWrite(taddress,tstatus);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringGetConfigRegister(uint32_t *status)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x10;					
	try	{		UsbRegisterRead(taddress,status);			}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringGetTemperature(uint32_t *Temperature)
//throw (LocalHardwareException)
{
	uint32_t taddress;

	taddress=0x11;
	try {	UsbRegisterRead(taddress,Temperature);}
	catch (LocalHardwareException& e)
	{
		*Temperature = 0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringGetDIFCurrent(uint32_t *DIFCurrent)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x12;

	try	{		UsbRegisterRead(taddress,DIFCurrent);			}
	catch (LocalHardwareException& e)
	{
		*DIFCurrent = 0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringGetSlabCurrent(uint32_t *SlabCurrent)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x13;

	try	{	UsbRegisterRead(taddress,SlabCurrent);			}
	catch (LocalHardwareException& e)
	{
		*SlabCurrent = 0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFMonitoringGetChannel4Monitoring(uint32_t *Ch4Value)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x14;

	try	{	UsbRegisterRead(taddress,Ch4Value);			}
	catch (LocalHardwareException& e)
	{
		*Ch4Value = 0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocCommandSLCWrite(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x01;
	
	try	{		UsbCommandWrite	(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: HardrocCommandSLCWriteLocal(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x11;
	
	try	{		UsbCommandWrite	(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: HardrocCommandSLCWriteByte(unsigned char  tbyte)    //throw (LocalHardwareException)
{
	try	{		write(tbyte);		}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	


int32_t FtdiDIFDriver :: HardrocCommandSLCWriteCRC(unsigned char  *tbyte)    //throw (LocalHardwareException)
{
	try	{		MonWritenAmoi(tbyte,2);		}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: CommandSLCWriteSingleSLCFrame(unsigned char  *tbyte,uint32_t n)
//throw (LocalHardwareException)
{
	try	{		MonWritenAmoi(tbyte,n);		}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}



int32_t FtdiDIFDriver :: HardrocCommandLemoPulse(void)
//throw (LocalHardwareException)
{
	uint32_t taddress= 0x06;
	
	try	{		UsbCommandWrite	(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	


int32_t FtdiDIFDriver :: FT245Reset(void)
//throw (LocalHardwareException)
{
	try
	{
		resetBus();
	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

// OK HR2 et MR
int32_t FtdiDIFDriver :: FPGAReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x01; 
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFFE;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

// OK
int32_t FtdiDIFDriver :: HardrocReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x02; 
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFD;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: BCIDReset(void)   //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;

	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x04; 
	try	{	UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFFB;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: SCReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x08; 
	try{UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFF7;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

// met select = 0 puis sc_sr_rest = 0
// puis sc_sr_reset = 1
// puis select = 1
int32_t FtdiDIFDriver :: SCSRReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	/*	
	for (;;)
	{
	for (int32_t i=0;i<32;i++)
	{
	printf ("i=%d\n",i);
	getchar();
	tdata =1<<i;
	try{UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
	LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
	return -2;
	}
	tdata =0;
	try{UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
	LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
	return -2;
	}
	}	
	}
*/	
	tdata =tdata & 0xFFFEFFFF; // select = 0
	try{UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata | 0x08;	// *sc_resetn=0
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFF7;	 // *sc_resten = 1
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata | 0x10000;	 //select = 1
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: SRReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x10; 
	try{	UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFEF;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: SCReportReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata | 0x20; 
	try	{	UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	tdata =tdata & 0xFFFFFFDF;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: DIFCptReset(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	tdata =tdata | 0x2000; 
	try	{	UsbRegisterWrite(taddress,tdata);}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2; }
	tdata =tdata & 0xFFFFDFFF;	
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2; }
	return 0;
}	

int32_t FtdiDIFDriver :: SetPowerAnalog(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata = ((tdata&0xFFFFFEFF) |(tstatus<<8));
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetPowerAnalog(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	*tstatus= (tdata >>8)&0x01;
	return 0;
}	


int32_t FtdiDIFDriver :: SetPowerADC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFFFFDFF) |(tstatus<<9));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	return 0;
}	

int32_t FtdiDIFDriver :: GetPowerADC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	*tstatus= (tdata >>9)&0x01;
	return 0;
}	
/*cc 3011
int32_t FtdiDIFDriver :: SetPowerSS(int32_t tstatus)    //throw (LocalHardwareException)
{
uint32_t taddress=0x03;
uint32_t tdata;
	
try	{		UsbRegisterRead(taddress,&tdata);	}
catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
tdata =((tdata&0xFFFFFBFF) |(tstatus<<10));
try	{		UsbRegisterWrite(taddress,tdata);	}
catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
return 0;
}	

int32_t FtdiDIFDriver :: GetPowerSS(int32_t *tstatus)    //throw (LocalHardwareException)
{
uint32_t taddress=0x03;
uint32_t tdata;
	
try	{		UsbRegisterRead(taddress,&tdata);	}
catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
*tstatus= (tdata >>10)&0x01;
return 0;
}	
*/
int32_t FtdiDIFDriver :: SetPowerDigital(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}

	tdata =((tdata&0xFFFFF7FF) |(tstatus<<11));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	return 0;
}	

int32_t FtdiDIFDriver :: GetPowerDigital(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	*tstatus= (tdata >>11)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetPowerDAC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFFFEFFF) |(tstatus<<12));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetPowerDAC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>12)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: ResetCounter(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata |=(1<<13);
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	tdata &=0xFFFFDFFF;
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: ClearAnalogSR(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata |=(1<<15);
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	tdata &=0xFFFF7FFF;
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: SetSCChoice(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFFDFFFF) |(tstatus<<17));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetSCChoice(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>17)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetCalibrationMode(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFFBFFFF) |(tstatus<<18));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetCalibrationMode(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>18)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetSetupWithCCC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFF7FFFF) |(tstatus<<19));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetSetupWithCCC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>19)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetSetupWithDCC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFEFFFFF) |(tstatus<<20));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetSetupWithDCC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>20)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetAcqTest(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFDFFFFF) |(tstatus<<21));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetAcqTest(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>21)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: Set4VforSC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFFBFFFFF) |(tstatus<<22));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: Get4VforSC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>22)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetMode4VforSC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFF7FFFFF) |(tstatus<<23));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetMode4VforSC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>23)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetModeDCCCCC(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFEFFFFFF) |(tstatus<<24));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetModeDCCCCC(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>24)&0x01;
	return 0;
}	





int32_t FtdiDIFDriver :: SetHold(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFDFFFFFF) |(tstatus<<25));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetHold(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>25)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetTimeoutDigitalReadout(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xFBFFFFFF) |(tstatus<<26));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetTimeoutDigitalReadout(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>26)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetPowerPulsing(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xF7FFFFFF) |(tstatus<<27));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetPowerPulsing(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>27)&0x01;
	return 0;
}

int32_t FtdiDIFDriver :: SetRealPowerPulsing(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xEFFFFFFF) |(tstatus<<28));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetRealPowerPulsing(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>28)&0x01;
	return 0;
}

int32_t FtdiDIFDriver :: SetDIFCommandsONOFF(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xDFFFFFFF) |(tstatus<<29));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetDIFCommandsONOFF(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>29)&0x01;
	return 0;
}

int32_t FtdiDIFDriver :: SetDROBtMode(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0xBFFFFFFF) |(tstatus<<30));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetDROBtMode(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>30)&0x01;
	return 0;
}

int32_t FtdiDIFDriver :: SetClockFrequency(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1;}
	tdata =((tdata&0x7FFFFFFF) |(tstatus<<31));
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)  { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t FtdiDIFDriver :: GetClockFrequency(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus= (tdata >>31)&0x01;
	return 0;
}




int32_t lydaq::FtdiDIFDriver::SetControlRegister(int32_t tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	
	try{	UsbRegisterWrite(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::GetControlRegister(uint32_t *tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	
	try	{		UsbRegisterRead(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		*tvalue=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	return 0;
}	




int32_t FtdiDIFDriver :: HardrocSetPowerPulsing(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	tstatus=tstatus&0x01;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	if (tstatus ==1)
	tdata =tdata |(tstatus<<27);
	else
	tdata = tdata&(~((!tstatus)<<27)) ;
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocGetPowerPulsing(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tstatus= (tdata >>27)&0x01;
	return 0;
}	

int32_t FtdiDIFDriver :: SetSCClockFrequency(int32_t tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	if (tstatus ==1)
	tdata =tdata |(tstatus<<31);
	else
	tdata = tdata&(~((!tstatus)<<31)) ;
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: GetSCClockFrequency(int32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tstatus= (tdata >>31)&0x01;
	return 0;
}	


// ************ digital readout ************
int32_t FtdiDIFDriver ::HardrocFlushDigitalFIFO(void)
{
  return 0;
	unsigned char  tdata;
	
#ifdef FLUSH_TO_FILE
	char dateStr [20];
	time_t tm= time(NULL);
	strftime(dateStr,20,"%d%m%y_%H%M%S",localtime(&tm));
	
	sprintf (FlushFileName,"Results/flush_%s.dat",	dateStr);			// format is "flush_date_time_index_runnumber.dat" stored in results/
	std::cout<<"flushFileName= "<<FlushFileName<<std::endl;
	FlushFile.open(FlushFileName,std::ios_base::out);				

	// get timestamp at global header
	struct timeb tTimestamp;
	char tchaine[30];
	ftime(&tTimestamp);
	sprintf (tchaine, "%08lx%08x",(tTimestamp.time), tTimestamp.millitm);
	FlushFile <<tchaine;

#endif

	uint32_t tbytesread;
	uint32_t tbytestoread=1;

	std::cout<<"Flushing FIFO : "<<std::endl;

	while  (read(&tdata)!=0)
	{
		std::cout.width(2);
		std::cout.fill('0');
		std::cout<<std::hex<<"0x"<<(int)tdata<<" "<<std::flush;
#ifdef FLUSH_TO_FILE
		if (FlushFile.is_open())
		{		
			sprintf (tchaine, "%02x",tdata);
			FlushFile <<tchaine;
		}
#endif
	}
#ifdef FLUSH_TO_FILE
	FlushFile.flush();
	FlushFile.close();
#endif
	std::cout<<std::dec<<std::endl<<"Memory flushed"<<std::endl;
	return 0;
}	

int32_t FtdiDIFDriver ::HardrocFastFlushDigitalFIFO(void)
{
	return 0; //Not implemented
	unsigned char  tdata;
	
#ifdef FAST_FLUSH_TO_FILE
	char dateStr [20];
	time_t tm= time(NULL);
	strftime(dateStr,20,"%d%m%y_%H%M%S",localtime(&tm));
	sprintf (FlushFileName,"Results/flush_%s.dat",	dateStr);			// format is "flush_date_time_index_runnumber.dat" stored in results/
	std::cout<<"flushFileName= "<<FlushFileName<<std::endl;
	FlushFile.open(FlushFileName,std::ios_base::out);				
	struct timeb tTimestamp;
	char tchaine[30];
	ftime(&tTimestamp);
	sprintf (tchaine, "%08lx%08x",(tTimestamp.time), tTimestamp.millitm);
	FlushFile <<tchaine;
#endif

	int32_t RXQueue;
	int32_t TXQueue;
	int32_t Event;
	uint32_t tbytesread;
	uint32_t tbytestoread=1;

	//	std::cout<<"Flushing FIFO : "<<std::endl;

	FT245GetStatus(&RXQueue,&TXQueue,&Event);
	for (int32_t i=0;i<RXQueue;i++)
	{
		read(&tdata);

#ifdef FAST_FLUSH_TO_FILE
		if (FlushFile.is_open())
		{		
			sprintf (tchaine, "%02x",tdata);
			FlushFile <<tchaine;
		}
#endif
	}
#ifdef FAST_FLUSH_TO_FILE
	FlushFile.flush();
	FlushFile.close();
#endif
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocStartDigitalAcquisitionCommand(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x02;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocStopDigitalAcquisitionCommand(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x23;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	


int32_t FtdiDIFDriver :: HardrocStartDigitalReadoutCommand(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocSendRamfullExtCommand(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x21;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocSendExternalTriggerCommand(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x22;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocSendMezzanine11Command(void)
//throw (LocalHardwareException)
{
	uint32_t taddress=0x50;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

// *********** analog readout ************

int32_t  FtdiDIFDriver :: HardrocSetTimerHoldRegister(int32_t thold)
//throw (LocalHardwareException)
{
	int32_t taddress=0x20;
	uint32_t tdata;

	tdata=thold&0x0F;
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}

int32_t  lydaq::FtdiDIFDriver::HardrocGetTimerHoldRegister(uint32_t *thold)    //throw (LocalHardwareException)
{
	int32_t taddress=0x20;

	try	{		UsbRegisterRead(taddress,thold);	}
	catch (LocalHardwareException& e)
	{
		*thold=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	(*thold)=(*thold)&0x0F;
	return 0;
}

int32_t  FtdiDIFDriver :: HardrocStartAnalogAcq(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x04;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}

int32_t  FtdiDIFDriver :: HardrocSoftwareTriggerAnalogAcq(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x41;
	
	try	{		UsbCommandWrite(taddress);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}

int32_t lydaq::FtdiDIFDriver::HardrocSetNumericalReadoutMode(int32_t tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	if (tmode ==1)
	tdata =tdata |((tmode&0x01)<<6);
	else
	tdata = tdata&(~((!(tmode&0x01))<<6)) ;
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetNumericalReadoutMode(int32_t *tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tmode=(tdata>>6)&0x01;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetNumericalReadoutStartMode(int32_t tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	if (tmode ==1)
	tdata =tdata |((tmode&0x01)<<7);
	else
	tdata = tdata&(~((!(tmode&0x01))<<7)) ;
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetNumericalReadoutStartMode(int32_t *tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		*tmode=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetSCOverVoltageDefault(void)
//throw (LocalHardwareException)
{
	uint32_t taddress;
	uint32_t tdata;
	
	taddress=0x03;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata = tdata&(~((!0x01)<<22)) ;
	tdata =tdata |((0x01)<<23);
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetSCOverVoltage(int32_t *tmode)
//throw (LocalHardwareException)
{
	uint32_t taddress;
	uint32_t tdata;
	
	taddress=0x03;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tmode=(tdata>>22)&0x02;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetTestAllAsicsDefault(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata |((0x01)<<25);
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetTestAllAsics(int32_t *tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress =0x03;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tmode=(tdata>>25)&0x01;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetEnablePowerPulsing(int32_t tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata |((tmode&0x01)<<27);
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetEnablePowerPulsing(int32_t *tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tmode=(tdata>>27)&0x01;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetEnableTimeoutDigitalReadout(int32_t tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	tdata =tdata |((tmode&0x01)<<26);
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetEnableTimeoutDigitalReadout(int32_t *tmode)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x03;
	uint32_t tdata;
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tmode=(tdata>>22)&0x01;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetStatusRegister(uint32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x04;
	try	{		UsbRegisterRead(taddress,tstatus);	}
	catch (LocalHardwareException& e) { LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetMemFull(uint32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress;
	
	taddress=0x04;
	try	{		UsbRegisterRead(taddress,tstatus);	}
	catch (LocalHardwareException& e) { *tstatus=0; LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }
	*tstatus=(*tstatus)&0x01;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetRamFullCpt(uint32_t *tstatus)    //throw (LocalHardwareException)
{
	uint32_t taddress = 0x09;
	
	try	{		UsbRegisterRead(taddress,tstatus);	}
	catch (LocalHardwareException& e)
	{
		*tstatus=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tstatus=(*tstatus)&0xFFFFFFFF;
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocSetSCDebugRegister(int32_t tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x19;
	
	try{	UsbRegisterWrite(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::HardrocGetSCDebugRegister(uint32_t *tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x19;
	
	try	{		UsbRegisterRead(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		*tvalue=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tvalue=(*tvalue)&0xFF;
	return 0;
}	



int32_t lydaq::FtdiDIFDriver::SetChipTypeRegister(int32_t tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x0;
	
	try{	UsbRegisterWrite(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::GetChipTypeRegister(uint32_t *tvalue)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x0;
	
	try	{		UsbRegisterRead(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		*tvalue=0;
		LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
		return -1;
	}
	*tvalue=(*tvalue);
	return 0;
}	
/*cc 3011
int32_t lydaq::FtdiDIFDriver::SetResetCounter(int32_t tvalue)    //throw (LocalHardwareException)
{
uint32_t taddress=0x03;
uint32_t tampon;
	
try	{		UsbRegisterRead(taddress,&tampon);	}
catch (LocalHardwareException& e) {LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -1; }

tampon&=0xDFFF;
tampon |=tvalue<<13;
try{	UsbRegisterWrite(taddress,tampon);	}
catch (LocalHardwareException& e) {LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e); return -2; }
return 0;
}	

int32_t lydaq::FtdiDIFDriver::GetResetCounter(uint32_t *tvalue)    //throw (LocalHardwareException)
{
uint32_t taddress=0x03;
	
try	{		UsbRegisterRead(taddress,tvalue);	}
catch (LocalHardwareException& e)
{
*tvalue=0;
LOG4CXX_ERROR(_logDIF," "<<e.message());throw (e);
return -1;
}
return 0;
}	
*/


int32_t FtdiDIFDriver :: SetPwrToPwrARegister(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x40;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: SetPwrAToPwrDRegister(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x41;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: SetPwrDToDAQRegister(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x42;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: SetDAQToPwrDRegister(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x43;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: SetPwrDToPwrARegister(uint32_t tnumber)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x44;
	try	{		UsbRegisterWrite(taddress,tnumber);			}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: HardrocCommandAskDifTemperature(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x09;
	
	try	{		UsbCommandWrite	(taddress);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	
int32_t FtdiDIFDriver :: HardrocCommandAskAsuTemperature(void)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x08;
	
	try	{		UsbCommandWrite	(taddress);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

int32_t FtdiDIFDriver :: GetDIFTemperature(uint32_t *tvalue)    				 	//throw (LocalHardwareException)
{
	uint32_t taddress=0x11;
	*tvalue=0;	
	
	try	{		UsbRegisterRead(taddress,tvalue);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;
	}
	return 0;	
}

int32_t FtdiDIFDriver :: SetTemperatureReadoutToAuto(uint32_t tvalue)    				 	//throw (LocalHardwareException)
{
	uint32_t taddress=0x10;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;		
	}
	tdata &=~1;
	tdata |=(tvalue&0x01);	
	tdata |=(tvalue&0x08);	
	
	tdata = 0xF9; // test selon guillamue 02/04/12
	try	{		UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;		
	}
	return 0;	
}
int32_t FtdiDIFDriver :: GetTemperatureReadoutAutoStatus(uint32_t *tvalue)    				 	//throw (LocalHardwareException)
{
	uint32_t taddress=0x10;
	uint32_t tdata;
	
	try	{		UsbRegisterRead(taddress,&tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;		
	}
	*tvalue=(tdata &0x01);
	return 0;	
}

int32_t FtdiDIFDriver :: GetASUTemperature(uint32_t *tvalue1,uint32_t *tvalue2)    						//throw (LocalHardwareException)
{
	uint32_t taddress=0x52;
	*tvalue1=0;	
	*tvalue2=0;	
	try	{		UsbRegisterRead(taddress,tvalue1);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;
	}
	taddress=0x53;
	try	{		UsbRegisterRead(taddress,tvalue2);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -1;
	}	
}

int32_t lydaq::FtdiDIFDriver::SetEventsBetweenTemperatureReadout(uint32_t tdata)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x55;
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::GetEventsBetweenTemperatureReadout(uint32_t *tdata)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x55;
	try{	UsbRegisterRead(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

int32_t lydaq::FtdiDIFDriver::SetAnalogConfigureRegister(uint32_t tdata)    //throw (LocalHardwareException)
{
	uint32_t taddress=0x60;
	try{	UsbRegisterWrite(taddress,tdata);	}
	catch (LocalHardwareException& e)
	{
		throw (e);
		return -2;
	}
	return 0;
}	

