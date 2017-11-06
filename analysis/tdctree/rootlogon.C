{
  gSystem->AddIncludePath(" -I/usr/include/jsoncpp/");
  gSystem->AddLinkedLibs("-L/usr/lib -ljsoncpp -lpthread ");
  gSystem->Load("./jsonGeo_C.so");
  gSystem->Load("./DCHistogramHandler_C.so");
  //gSystem->Load("./TdcAnalyzer_C.so");
  gROOT->ProcessLine(".L TdcAnalyzer.C+");
  gROOT->ProcessLine(".L evt.C+");

}
