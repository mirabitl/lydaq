#ifndef _UTILDEFS_H
#define _UTILDEFS_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <vector>
//typedef std::vector<RecoHit*>::iterator recit;


#define IMIN(X,Y) ( ((X) < (Y)) ? (X) : (Y))
#define IMAX(X,Y) ((X) > (Y) ? : (X) : (Y))
#define PI 3.1415926535897931


// #define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#define INFO_PRINT_ENABLED 1
#if DEBUG_PRINT_ENABLED
#define INFO_PRINT_ENABLED 1
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(format, args...) ((void)0)
#endif
#if INFO_PRINT_ENABLED
#define INFO_PRINT printf
#else
#define INFO_PRINT(format, args...) ((void)0)
#endif


#endif
