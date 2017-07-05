#ifndef _DIFDBManager_H
#define _DIFDBManager_H
#include <stdint.h>
#include "DIFFirmwareFormat.h"
//#include "DHCALDefs.h"
#include <string>
#include <vector>
#include <map>
#include "DbLogger.h"
#include "UsbDIFSettings.h"
typedef unsigned char SingleHardrocV2ConfigurationFrame[HARDROCV2_SLC_FRAME_SIZE];

class DIFDBManager
{
public:
  DIFDBManager(){;}
  virtual ~DIFDBManager(){;}
  DIFDBManager(std::string sdifs);
  DIFDBManager(std::vector<uint32_t> &vdifs);
  
  void clearMaps();
  void download(std::string asicType="HR2");
  std::vector<uint32_t>& getDIFVector(){return theDIFs_;}
  uint32_t getNumberOfDIFs(){return theDIFs_.size();}
  uint32_t getDIFId(uint8_t idx){return theDIFs_[idx];}
  UsbDIFSettings* getDIFSettings(uint32_t difid) { return theDifMap_[difid];}
  SingleHardrocV2ConfigurationFrame* getAsicsConfiguration(uint32_t difid){return theAsicsMap_[difid];}
  inline std::map<uint32_t,unsigned char*>& getAsicKeyMap(){return theAsicKeyMap_;}
  void dumpAsicMap();
	void dumpToTree(std::string rootPath,std::string setupName);
  uint8_t getAsicsNumber(uint32_t difid);
	uint8_t getAsicsNumberl1(uint32_t difid);
	uint8_t getAsicsNumberl2(uint32_t difid);
	uint8_t getAsicsNumberl3(uint32_t difid);
	uint8_t getAsicsNumberl4(uint32_t difid);

  virtual void initialize()=0;
  virtual uint32_t LoadDIFHardrocV2Parameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v)=0;
  virtual uint32_t LoadDIFMicrorocParameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v)=0;
  virtual void LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v)=0;
  virtual uint32_t LoadAsicParameters()=0;

protected:
  std::string theDIFList_;
  std::vector<uint32_t> theDIFs_;
  std::map<uint32_t,UsbDIFSettings*> theDifMap_;
  std::map<uint32_t,SingleHardrocV2ConfigurationFrame*> theAsicsMap_;
  std::map<uint32_t,uint32_t> theNumberOfAsicsMap_;
  std::map<uint32_t,unsigned char*> theAsicKeyMap_;
};

#endif
