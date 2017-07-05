%module LSDHCALcommon
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"

 %{
#include "DIFWritterInterface.h"
#include "BasicWritterInterface.h"
#include "ShmProxy.h"

 %}


%include "DIFWritterInterface.h"
%include "BasicWritterInterface.h"
%include "ShmProxy.h"

