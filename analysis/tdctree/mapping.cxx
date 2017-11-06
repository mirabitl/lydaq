#include "TdcMapping.hh"

#include <stdio.h>
int main()
{
  printf(" TDC2PR %d PR2LEMO %d \n",TDC2PR[7],PR2LEMO[TDC2PR[7]]);
  printf(" TDC2PR %d PR2LEMO %d \n",LEMO2PR[18],PR2TDC[LEMO2PR[18]]);
  
}
