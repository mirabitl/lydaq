import sqlite3 as sqlite
import string
#import MySQLdb

def info(object, spacing=10, collapse=1):   
    """Print methods and doc strings.
    
    Takes module, class, list, dictionary, or string."""
    methodList = [method for method in dir(object) if callable(getattr(object, method))]
    argList = [method for method in dir(object) if not callable(getattr(object, method))]
    processFunc = collapse and (lambda s: " ".join(s.split())) or (lambda s: s)
    print "\n".join(["%s %s" %
                      (method.ljust(spacing),
                       processFunc(str(getattr(object, method).__doc__)))
                     for method in methodList])
    print argList

def buildClassInclude(obj,className,tableName):
    str_include =[]
    str_impl =[]


    lnames=[]
    lvals=[]
#    print obj.__dict__
    for name,val in sorted(obj.__dict__.iteritems()):
            lnames.append(name)
            lvals.append(val)



    str_include.append("#ifndef _%s_H_" % className)
    str_include.append("#define _%s_H_" % className)
    str_include.append("#include <string>")
    str_include.append("#include <stdint.h>")
    str_include.append("#include <dbi/dbi.h>")
    str_impl.append("#include \"%s.hh\"" % className)
    str_include.append("class %s {" % className)
    str_impl.append("%s::%s() {" % (className,className))
    str_include.append("  private:")

    for  i in range(len(lnames)):
#        print type(lvals[i]).__name__

        if (type(lvals[i]).__name__ == 'int'):
            str_include.append("  int32_t %s;" % lnames[i])
            str_impl.append("  %s = %s; " % (lnames[i],lvals[i]))
        if (type(lvals[i]).__name__ == 'long'):
            str_include.append("  int64_t %s;" % lnames[i])
            str_impl.append("  %s = %s; " % (lnames[i],lvals[i]))
        if (type(lvals[i]).__name__ == 'str'):
            str_include.append("  std::string %s;" % lnames[i])
            str_impl.append("  %s = \"%s\"; " % (lnames[i],lvals[i]))
        if (type(lvals[i]).__name__ == 'unicode'):
            str_include.append("  std::string %s;" % lnames[i])
            str_impl.append("  %s = \"%s\"; " % (lnames[i],lvals[i]))
        if (type(lvals[i]).__name__ == 'float'):
            str_include.append("  float %s;" % lnames[i])
            str_impl.append("  %s = %s; " % (lnames[i],lvals[i]))

    str_include.append("  public:")
    str_include.append(" %s();" % className)
    for  i in range(len(lnames)):
#        print type(lvals[i]).__name__
        if (type(lvals[i]).__name__ == 'int'):
            str_include.append("  inline int32_t get%s(){return %s;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'long'):
            str_include.append("  inline int64_t get%s(){return %s;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'str'):
            str_include.append("  inline std::string get%s(){return %s;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'unicode'):
            str_include.append("  inline std::string get%s(){return %s;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'float'):
            str_include.append("  inline float get%s(){return %s;}" % (lnames[i],lnames[i]))

    for  i in range(len(lnames)):
#        print type(lvals[i]).__name__
        if (type(lvals[i]).__name__ == 'int'):
            str_include.append("  inline void set%s( int32_t t){%s=t;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'long'):
            str_include.append("  inline void set%s( int64_t t){%s=t;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'str'):
            str_include.append("  inline void set%s( std::string t){%s=t;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'unicode'):
            str_include.append("  inline void set%s( std::string t){%s=t;}" % (lnames[i],lnames[i]))
        if (type(lvals[i]).__name__ == 'float'):
            str_include.append("  inline void set%s( float t){%s=t;}" % (lnames[i],lnames[i]))


    str_impl.append("}")
    # Build of a select command
    str_include.append(" std::string selectCommand();")
    str_impl.append(" std::string %s::selectCommand(){ return \"%s\";} " %(className,buildSelect(obj,tableName,"")))
    str_include.append(" void parseDbiRow(dbi_result &result);")
    str_impl.append(" void %s::parseDbiRow(dbi_result &result){" % className )
    for  i in range(len(lnames)):
        if (lnames[i] == "ID" or lnames[i] == "ID"):
            str_impl.append("%s =dbi_result_get_longlong(result,\"%s\");" % (lnames[i],lnames[i]))
        else:
            if (type(lvals[i]).__name__ == 'int'):
                str_impl.append("%s =dbi_result_get_int(result,\"%s\");" % (lnames[i],lnames[i]))   
            if (type(lvals[i]).__name__ == 'long'):
                str_impl.append("%s =dbi_result_get_int(result,\"%s\");" % (lnames[i],lnames[i]))   
            if (type(lvals[i]).__name__ == 'str'):
                str_impl.append(" const char *c_%s;c_%s =dbi_result_get_string(result,\"%s\"); %s.assign(c_%s);" % (lnames[i],lnames[i],lnames[i],lnames[i],lnames[i]))   
            if (type(lvals[i]).__name__ == 'unicode'):
                str_impl.append(" const char *c_%s;c_%s =dbi_result_get_string(result,\"%s\"); %s.assign(c_%s);" % (lnames[i],lnames[i],lnames[i],lnames[i],lnames[i]))   
            if (type(lvals[i]).__name__ == 'float'):
                str_impl.append("%s =dbi_result_get_float(result,\"%s\");" % (lnames[i],lnames[i]))

    str_impl.append("}")

    str_include.append("};")
    str_include.append("#endif") 

    finc= open( "%s.hh" % className,"w+")
    fimpl= open( "%s.cc" % className,"w+")
    for x in str_include:
        finc.write(x+"\n")
    for x in str_impl:
        fimpl.write(x+"\n")


def PrintDict(obj):
    for name,val in sorted(obj.__dict__.iteritems()):
        if (type(val).__name__ == 'int'):
            print '%s = %d' % (name,val)
        if (type(val).__name__ == 'long'):
            print '%s = %d' % (name,val)
        if (type(val).__name__ == 'str'):
            print '%s = %s' % (name,val)
        if (type(val).__name__ == 'float'):
            print '%s = %f' % (name,val)

def buildInsert(obj,table_name):
    str_cmd=" insert into "+table_name+" ("
    str_format="( "
    str_list ="( "
    lnames=[]
    lvals=[]
#    print obj.__dict__
    for name,val in sorted(obj.__dict__.iteritems()):
        if (name != 'ID'):
            lnames.append(name)
            lvals.append(val)
    for  i in range(len(lnames)):
#       print i,lnames[i],lvals[i]
        if (i == len(lnames)-1):
            nsuffix=")"
            vsuffix=")"
        else:
            nsuffix=","
            vsuffix=","
        str_cmd =str_cmd+lnames[i]+nsuffix
#        print type(lvals[i]).__name__
        if (type(lvals[i]).__name__ == 'int'):
            str_list=str_list+("\"%d\"" % lvals[i])+vsuffix
        if (type(lvals[i]).__name__ == 'long'):
            str_list=str_list+("\"%d\"" % lvals[i])+vsuffix
        if (type(lvals[i]).__name__ == 'str'):
            str_list=str_list+("\"%s\"" % lvals[i])+vsuffix
        if (type(lvals[i]).__name__ == 'unicode'):
            str_list=str_list+("\"%s\"" % lvals[i])+vsuffix
        if (type(lvals[i]).__name__ == 'float'):
            str_list=str_list+("\"%f\"" % lvals[i])+vsuffix
    return (str_cmd+" values"+str_list)
    
def buildSelect(obj,table_name,condition):
    str_cmd="select "
    lnames=[]
    for name,val in sorted(obj.__dict__.iteritems()):
        lnames.append(name)
    for  i in range(len(lnames)):
        if (i == len(lnames)-1):
            nsuffix=" "
        else:
            nsuffix=","
        str_cmd =str_cmd+lnames[i]+nsuffix
            
    sqlcmd= str_cmd+" from "+table_name+" "+condition
#    print sqlcmd
    return sqlcmd




class HR2Def:
    def __init__(self,num):
        self.ID = 0
        self.SETUP_ID =0
        self.DIF_NUM=0
        self.Header=num
        self.MASK=0
        self.QScSrOutSc = 1	
        self.EnOCDout1b = 1		
        self.EnOCDout2b = 0			
        self.EnOCTransmitOn1b = 1			
        self.EnOCTransmitOn2b = 0			
        self.EnOCchipSatb = 1			
        self.SelEndReadout = 1			
        self.SelStartReadout = 1 			
        self.ClkMux = 1			
        self.ScOn = 0			
        self.RazChnExtVal = 0			
        self.RazChnIntVal = 1			
        self.TrigExtVal = 0			
        self.DiscrOrOr = 1			
        self.EnTrigOut = 1		
        self.Trig0b = 1			
        self.Trig1b = 0			
        self.Trig2b = 0			
        self.OtaBgSw = 0			
        self.DacSw = 0			
        self.SmallDac = 0			
        self.B2 = 220		
        self.B1 = 220			
        self.B0 = 220			
        self.Mask2 = '0xFFFFFFFFFFFFFFFF'
        self.Mask1 = '0xFFFFFFFFFFFFFFFF'
        self.Mask0 = '0xFFFFFFFFFFFFFFFF'
        self.RS_or_Discri = 1			
        self.Discri1 = 0 			
        self.Discri2 = 0			
        self.Discri0 = 0			
        self.OtaQ_PwrADC = 0			
        self.En_OtaQ = 1			
        self.Sw50f0 = 1			
        self.Sw100f0 = 1			
        self.Sw100k0 = 1			
        self.Sw50k0 = 1			
        self.PwrOnFsb1 = 0			
        self.PwrOnFsb2 = 0			
        self.PwrOnFsb0 = 0			
        self.Sel1 = 0			
        self.Sel0 = 1			
        self.Sw50f1 = 1			
        self.Sw100f1 = 1			
        self.Sw100k1 = 1			
        self.Sw50k1 = 1		
        self.Cmdb0Fsb1 = 1			
        self.Cmdb1Fsb1 = 1			
        self.Cmdb2Fsb1 = 0			
        self.Cmdb3Fsb1 = 1			
        self.Sw50f2 = 1			
        self.Sw100f2 = 1			
        self.Sw100k2 = 1			
        self.Sw50k2 = 1			
        self.Cmdb0Fsb2 = 1			
        self.Cmdb1Fsb2 = 1			
        self.Cmdb2Fsb2 = 0			
        self.Cmdb3Fsb2 = 1			
        self.PwrOnW = 0			
        self.PwrOnSS = 0			
        self.PwrOnBuff = 0			
        self.SwSsc = 7			
        self.CmdB0SS = 0			
        self.CmdB1SS = 0			
        self.CmdB2SS = 0			
        self.CmdB3SS = 0			
        self.PwrOnPA = 0			
        self.PaGain0 = 128			
        self.PaGain1 = 128			
        self.PaGain2 = 128			
        self.PaGain3 = 128			
        self.PaGain4 = 128			
        self.PaGain5 = 128			
        self.PaGain6 = 128			
        self.PaGain7 = 128			
        self.PaGain8 = 128			
        self.PaGain9 = 128			
        self.PaGain10 = 128			
        self.PaGain11 = 128			
        self.PaGain12 = 128			
        self.PaGain13 = 128			
        self.PaGain14 = 128			
        self.PaGain15 = 128			
        self.PaGain16 = 128			
        self.PaGain17 = 128			
        self.PaGain18 = 128			
        self.PaGain19 = 128			
        self.PaGain20 = 128			
        self.PaGain21 = 128			
        self.PaGain22 = 128			
        self.PaGain23 = 128			
        self.PaGain24 = 128			
        self.PaGain25 = 128			
        self.PaGain26 = 128			
        self.PaGain27 = 128			
        self.PaGain28 = 128			
        self.PaGain29 = 128			
        self.PaGain30 = 128			
        self.PaGain31 = 128			
        self.PaGain32 = 128			
        self.PaGain33 = 128			
        self.PaGain34 = 128			
        self.PaGain35 = 128			
        self.PaGain36 = 128			
        self.PaGain37 = 128			
        self.PaGain38 = 128			
        self.PaGain39 = 128			
        self.PaGain40 = 128			
        self.PaGain41 = 128			
        self.PaGain42 = 128			
        self.PaGain43 = 128			
        self.PaGain44 = 128			
        self.PaGain45 = 128			
        self.PaGain46 = 128			
        self.PaGain47 = 128			
        self.PaGain48 = 128			
        self.PaGain49 = 128			
        self.PaGain50 = 128			
        self.PaGain51 = 128			
        self.PaGain52 = 128			
        self.PaGain53 = 128			
        self.PaGain54 = 128			
        self.PaGain55 = 128			
        self.PaGain56 = 128			
        self.PaGain57 = 128			
        self.PaGain58 = 128			
        self.PaGain59 = 128			
        self.PaGain60 = 128			
        self.PaGain61 = 128			
        self.PaGain62 = 128			
        self.PaGain63 = 128			
        self.cTest = 0		
    def Print(self):
        print 'QScSrOutSc = %d' % self.QScSrOutSc
        print 'EnOCDout1b = %d' % self.EnOCDout1b
        print 'EnOCDout2b = %d' % self.EnOCDout2b
        print 'EnOCTransmitOn1b = %d' % self.EnOCTransmitOn1b
        print 'EnOCTransmitOn2b = %d' % self.EnOCTransmitOn2b
        print 'EnOCchipSatb = %d' % self.EnOCchipSatb
        print 'SelEndReadout = %d' % self.SelEndReadout
        print 'SelStartReadout = %d' % self.SelStartReadout
        print 'ClkMux = %d' % self.ClkMux
        print 'ScOn = %d' % self.ScOn
        print 'RazChnExtVal = %d' % self.RazChnExtVal
        print 'RazChnIntVal = %d' % self.RazChnIntVal
        print 'TrigExtVal = %d' % self.TrigExtVal
        print 'DiscrOrOr = %d' % self.DiscrOrOr
        print 'EnTrigOut = %d' % self.EnTrigOut
        print 'Trig0b = %d' % self.Trig0b
        print 'Trig1b = %d' % self.Trig1b
        print 'Trig2b = %d' % self.Trig2b
        print 'OtaBgSw = %d' % self.OtaBgSw
        print 'DacSw = %d' % self.DacSw
        print 'SmallDac = %d' % self.SmallDac
        print 'B2 = %d' % self.B2
        print 'B1 = %d' % self.B1
        print 'B0 = %d' % self.B0
        print 'Header = %d' % self.Header
        print 'Mask2 = %d' % string.atoi(self.Mask2,16)
        print 'Mask1 = %d' % string.atoi(self.Mask1,16)
        print 'Mask0 = %d' % string.atoi(self.Mask0,16)
        print 'RS_or_Discri = %d' % self.RS_or_Discri
        print 'Discri1 = %d' % self.Discri1
        print 'Discri2 = %d' % self.Discri2
        print 'Discri0 = %d' % self.Discri0
        print 'OtaQ_PwrADC = %d' % self.OtaQ_PwrADC
        print 'En_OtaQ = %d' % self.En_OtaQ
        print 'Sw50f0 = %d' % self.Sw50f0
        print 'Sw100f0 = %d' % self.Sw100f0
        print 'Sw100k0 = %d' % self.Sw100k0
        print 'Sw50k0 = %d' % self.Sw50k0
        print 'PwrOnFsb1 = %d' % self.PwrOnFsb1
        print 'PwrOnFsb2 = %d' % self.PwrOnFsb2
        print 'PwrOnFsb0 = %d' % self.PwrOnFsb0
        print 'Sel1 = %d' % self.Sel1
        print 'Sel0 = %d' % self.Sel0
        print 'Sw50f1 = %d' % self.Sw50f1
        print 'Sw100f1 = %d' % self.Sw100f1
        print 'Sw100k1 = %d' % self.Sw100k1
        print 'Sw50k1 = %d' % self.Sw50k1
        print 'Cmdb0Fsb1 = %d' % self.Cmdb0Fsb1
        print 'Cmdb1Fsb1 = %d' % self.Cmdb1Fsb1
        print 'Cmdb2Fsb1 = %d' % self.Cmdb2Fsb1
        print 'Cmdb3Fsb1 = %d' % self.Cmdb3Fsb1
        print 'Sw50f2 = %d' % self.Sw50f2
        print 'Sw100f2 = %d' % self.Sw100f2
        print 'Sw100k2 = %d' % self.Sw100k2
        print 'Sw50k2 = %d' % self.Sw50k2
        print 'Cmdb0Fsb2 = %d' % self.Cmdb0Fsb2
        print 'Cmdb1Fsb2 = %d' % self.Cmdb1Fsb2
        print 'Cmdb2Fsb2 = %d' % self.Cmdb2Fsb2
        print 'Cmdb3Fsb2 = %d' % self.Cmdb3Fsb2
        print 'PwrOnW = %d' % self.PwrOnW
        print 'PwrOnSS = %d' % self.PwrOnSS
        print 'PwrOnBuff = %d' % self.PwrOnBuff
        print 'SwSsc = %d' % self.SwSsc
        print 'CmdB0SS = %d' % self.CmdB0SS
        print 'CmdB1SS = %d' % self.CmdB1SS
        print 'CmdB2SS = %d' % self.CmdB2SS
        print 'CmdB3SS = %d' % self.CmdB3SS
        print 'PwrOnPA = %d' % self.PwrOnPA
        print 'PaGain0 = %d' % self.PaGain0
        print 'PaGain1 = %d' % self.PaGain1
        print 'PaGain2 = %d' % self.PaGain2
        print 'PaGain3 = %d' % self.PaGain3
        print 'PaGain4 = %d' % self.PaGain4
        print 'PaGain5 = %d' % self.PaGain5
        print 'PaGain6 = %d' % self.PaGain6
        print 'PaGain7 = %d' % self.PaGain7
        print 'PaGain8 = %d' % self.PaGain8
        print 'PaGain9 = %d' % self.PaGain9
        print 'PaGain10 = %d' % self.PaGain10
        print 'PaGain11 = %d' % self.PaGain11
        print 'PaGain12 = %d' % self.PaGain12
        print 'PaGain13 = %d' % self.PaGain13
        print 'PaGain14 = %d' % self.PaGain14
        print 'PaGain15 = %d' % self.PaGain15
        print 'PaGain16 = %d' % self.PaGain16
        print 'PaGain17 = %d' % self.PaGain17
        print 'PaGain18 = %d' % self.PaGain18
        print 'PaGain19 = %d' % self.PaGain19
        print 'PaGain20 = %d' % self.PaGain20
        print 'PaGain21 = %d' % self.PaGain21
        print 'PaGain22 = %d' % self.PaGain22
        print 'PaGain23 = %d' % self.PaGain23
        print 'PaGain24 = %d' % self.PaGain24
        print 'PaGain25 = %d' % self.PaGain25
        print 'PaGain26 = %d' % self.PaGain26
        print 'PaGain27 = %d' % self.PaGain27
        print 'PaGain28 = %d' % self.PaGain28
        print 'PaGain29 = %d' % self.PaGain29
        print 'PaGain30 = %d' % self.PaGain30
        print 'PaGain31 = %d' % self.PaGain31
        print 'PaGain32 = %d' % self.PaGain32
        print 'PaGain33 = %d' % self.PaGain33
        print 'PaGain34 = %d' % self.PaGain34
        print 'PaGain35 = %d' % self.PaGain35
        print 'PaGain36 = %d' % self.PaGain36
        print 'PaGain37 = %d' % self.PaGain37
        print 'PaGain38 = %d' % self.PaGain38
        print 'PaGain39 = %d' % self.PaGain39
        print 'PaGain40 = %d' % self.PaGain40
        print 'PaGain41 = %d' % self.PaGain41
        print 'PaGain42 = %d' % self.PaGain42
        print 'PaGain43 = %d' % self.PaGain43
        print 'PaGain44 = %d' % self.PaGain44
        print 'PaGain45 = %d' % self.PaGain45
        print 'PaGain46 = %d' % self.PaGain46
        print 'PaGain47 = %d' % self.PaGain47
        print 'PaGain48 = %d' % self.PaGain48
        print 'PaGain49 = %d' % self.PaGain49
        print 'PaGain50 = %d' % self.PaGain50
        print 'PaGain51 = %d' % self.PaGain51
        print 'PaGain52 = %d' % self.PaGain52
        print 'PaGain53 = %d' % self.PaGain53
        print 'PaGain54 = %d' % self.PaGain54
        print 'PaGain55 = %d' % self.PaGain55
        print 'PaGain56 = %d' % self.PaGain56
        print 'PaGain57 = %d' % self.PaGain57
        print 'PaGain58 = %d' % self.PaGain58
        print 'PaGain59 = %d' % self.PaGain59
        print 'PaGain60 = %d' % self.PaGain60
        print 'PaGain61 = %d' % self.PaGain61
        print 'PaGain62 = %d' % self.PaGain62
        print 'PaGain63 = %d' % self.PaGain63
        print 'cTest = %d' % self.cTest

    def Write(self,fout):
        fout.write(('QScSrOutSc = %d\n' % self.QScSrOutSc))
        fout.write(('EnOCDout1b = %d\n' % self.EnOCDout1b))
        fout.write(('EnOCDout2b = %d\n' % self.EnOCDout2b))
        fout.write(('EnOCTransmitOn1b = %d\n' % self.EnOCTransmitOn1b))
        fout.write(('EnOCTransmitOn2b = %d\n' % self.EnOCTransmitOn2b))
        fout.write(('EnOCchipSatb = %d\n' % self.EnOCchipSatb))
        fout.write(('SelEndReadout = %d\n' % self.SelEndReadout))
        fout.write(('SelStartReadout = %d\n' % self.SelStartReadout))
        fout.write(('ClkMux = %d\n' % self.ClkMux))
        fout.write(('ScOn = %d\n' % self.ScOn))
        fout.write(('RazChnExtVal = %d\n' % self.RazChnExtVal))
        fout.write(('RazChnIntVal = %d\n' % self.RazChnIntVal))
        fout.write(('TrigExtVal = %d\n' % self.TrigExtVal))
        fout.write(('DiscrOrOr = %d\n' % self.DiscrOrOr))
        fout.write(('EnTrigOut = %d\n' % self.EnTrigOut))
        fout.write(('Trig0b = %d\n' % self.Trig0b))
        fout.write(('Trig1b = %d\n' % self.Trig1b))
        fout.write(('Trig2b = %d\n' % self.Trig2b))
        fout.write(('OtaBgSw = %d\n' % self.OtaBgSw))
        fout.write(('DacSw = %d\n' % self.DacSw))
        fout.write(('SmallDac = %d\n' % self.SmallDac))
        fout.write(('B2 = %d\n' % self.B2))
        fout.write(('B1 = %d\n' % self.B1))
        fout.write(('B0 = %d\n' % self.B0))
        fout.write(('Header = %d\n' % self.Header))
        fout.write(('Mask2 = %d\n' % string.atoi(self.Mask2,16)))
        fout.write(('Mask1 = %d\n' % string.atoi(self.Mask1,16)))
        fout.write(('Mask0 = %d\n' % string.atoi(self.Mask0,16)))
        fout.write(('RS_or_Discri = %d\n' % self.RS_or_Discri))
        fout.write(('Discri1 = %d\n' % self.Discri1))
        fout.write(('Discri2 = %d\n' % self.Discri2))
        fout.write(('Discri0 = %d\n' % self.Discri0))
        fout.write(('OtaQ_PwrADC = %d\n' % self.OtaQ_PwrADC))
        fout.write(('En_OtaQ = %d\n' % self.En_OtaQ))
        fout.write(('Sw50f0 = %d\n' % self.Sw50f0))
        fout.write(('Sw100f0 = %d\n' % self.Sw100f0))
        fout.write(('Sw100k0 = %d\n' % self.Sw100k0))
        fout.write(('Sw50k0 = %d\n' % self.Sw50k0))
        fout.write(('PwrOnFsb1 = %d\n' % self.PwrOnFsb1))
        fout.write(('PwrOnFsb2 = %d\n' % self.PwrOnFsb2))
        fout.write(('PwrOnFsb0 = %d\n' % self.PwrOnFsb0))
        fout.write(('Sel1 = %d\n' % self.Sel1))
        fout.write(('Sel0 = %d\n' % self.Sel0))
        fout.write(('Sw50f1 = %d\n' % self.Sw50f1))
        fout.write(('Sw100f1 = %d\n' % self.Sw100f1))
        fout.write(('Sw100k1 = %d\n' % self.Sw100k1))
        fout.write(('Sw50k1 = %d\n' % self.Sw50k1))
        fout.write(('Cmdb0Fsb1 = %d\n' % self.Cmdb0Fsb1))
        fout.write(('Cmdb1Fsb1 = %d\n' % self.Cmdb1Fsb1))
        fout.write(('Cmdb2Fsb1 = %d\n' % self.Cmdb2Fsb1))
        fout.write(('Cmdb3Fsb1 = %d\n' % self.Cmdb3Fsb1))
        fout.write(('Sw50f2 = %d\n' % self.Sw50f2))
        fout.write(('Sw100f2 = %d\n' % self.Sw100f2))
        fout.write(('Sw100k2 = %d\n' % self.Sw100k2))
        fout.write(('Sw50k2 = %d\n' % self.Sw50k2))
        fout.write(('Cmdb0Fsb2 = %d\n' % self.Cmdb0Fsb2))
        fout.write(('Cmdb1Fsb2 = %d\n' % self.Cmdb1Fsb2))
        fout.write(('Cmdb2Fsb2 = %d\n' % self.Cmdb2Fsb2))
        fout.write(('Cmdb3Fsb2 = %d\n' % self.Cmdb3Fsb2))
        fout.write(('PwrOnW = %d\n' % self.PwrOnW))
        fout.write(('PwrOnSS = %d\n' % self.PwrOnSS))
        fout.write(('PwrOnBuff = %d\n' % self.PwrOnBuff))
        fout.write(('SwSsc = %d\n' % self.SwSsc))
        fout.write(('CmdB0SS = %d\n' % self.CmdB0SS))
        fout.write(('CmdB1SS = %d\n' % self.CmdB1SS))
        fout.write(('CmdB2SS = %d\n' % self.CmdB2SS))
        fout.write(('CmdB3SS = %d\n' % self.CmdB3SS))
        fout.write(('PwrOnPA = %d\n' % self.PwrOnPA))
        fout.write(('PaGain0 = %d\n' % self.PaGain0))
        fout.write(('PaGain1 = %d\n' % self.PaGain1))
        fout.write(('PaGain2 = %d\n' % self.PaGain2))
        fout.write(('PaGain3 = %d\n' % self.PaGain3))
        fout.write(('PaGain4 = %d\n' % self.PaGain4))
        fout.write(('PaGain5 = %d\n' % self.PaGain5))
        fout.write(('PaGain6 = %d\n' % self.PaGain6))
        fout.write(('PaGain7 = %d\n' % self.PaGain7))
        fout.write(('PaGain8 = %d\n' % self.PaGain8))
        fout.write(('PaGain9 = %d\n' % self.PaGain9))
        fout.write(('PaGain10 = %d\n' % self.PaGain10))
        fout.write(('PaGain11 = %d\n' % self.PaGain11))
        fout.write(('PaGain12 = %d\n' % self.PaGain12))
        fout.write(('PaGain13 = %d\n' % self.PaGain13))
        fout.write(('PaGain14 = %d\n' % self.PaGain14))
        fout.write(('PaGain15 = %d\n' % self.PaGain15))
        fout.write(('PaGain16 = %d\n' % self.PaGain16))
        fout.write(('PaGain17 = %d\n' % self.PaGain17))
        fout.write(('PaGain18 = %d\n' % self.PaGain18))
        fout.write(('PaGain19 = %d\n' % self.PaGain19))
        fout.write(('PaGain20 = %d\n' % self.PaGain20))
        fout.write(('PaGain21 = %d\n' % self.PaGain21))
        fout.write(('PaGain22 = %d\n' % self.PaGain22))
        fout.write(('PaGain23 = %d\n' % self.PaGain23))
        fout.write(('PaGain24 = %d\n' % self.PaGain24))
        fout.write(('PaGain25 = %d\n' % self.PaGain25))
        fout.write(('PaGain26 = %d\n' % self.PaGain26))
        fout.write(('PaGain27 = %d\n' % self.PaGain27))
        fout.write(('PaGain28 = %d\n' % self.PaGain28))
        fout.write(('PaGain29 = %d\n' % self.PaGain29))
        fout.write(('PaGain30 = %d\n' % self.PaGain30))
        fout.write(('PaGain31 = %d\n' % self.PaGain31))
        fout.write(('PaGain32 = %d\n' % self.PaGain32))
        fout.write(('PaGain33 = %d\n' % self.PaGain33))
        fout.write(('PaGain34 = %d\n' % self.PaGain34))
        fout.write(('PaGain35 = %d\n' % self.PaGain35))
        fout.write(('PaGain36 = %d\n' % self.PaGain36))
        fout.write(('PaGain37 = %d\n' % self.PaGain37))
        fout.write(('PaGain38 = %d\n' % self.PaGain38))
        fout.write(('PaGain39 = %d\n' % self.PaGain39))
        fout.write(('PaGain40 = %d\n' % self.PaGain40))
        fout.write(('PaGain41 = %d\n' % self.PaGain41))
        fout.write(('PaGain42 = %d\n' % self.PaGain42))
        fout.write(('PaGain43 = %d\n' % self.PaGain43))
        fout.write(('PaGain44 = %d\n' % self.PaGain44))
        fout.write(('PaGain45 = %d\n' % self.PaGain45))
        fout.write(('PaGain46 = %d\n' % self.PaGain46))
        fout.write(('PaGain47 = %d\n' % self.PaGain47))
        fout.write(('PaGain48 = %d\n' % self.PaGain48))
        fout.write(('PaGain49 = %d\n' % self.PaGain49))
        fout.write(('PaGain50 = %d\n' % self.PaGain50))
        fout.write(('PaGain51 = %d\n' % self.PaGain51))
        fout.write(('PaGain52 = %d\n' % self.PaGain52))
        fout.write(('PaGain53 = %d\n' % self.PaGain53))
        fout.write(('PaGain54 = %d\n' % self.PaGain54))
        fout.write(('PaGain55 = %d\n' % self.PaGain55))
        fout.write(('PaGain56 = %d\n' % self.PaGain56))
        fout.write(('PaGain57 = %d\n' % self.PaGain57))
        fout.write(('PaGain58 = %d\n' % self.PaGain58))
        fout.write(('PaGain59 = %d\n' % self.PaGain59))
        fout.write(('PaGain60 = %d\n' % self.PaGain60))
        fout.write(('PaGain61 = %d\n' % self.PaGain61))
        fout.write(('PaGain62 = %d\n' % self.PaGain62))
        fout.write(('PaGain63 = %d\n' % self.PaGain63))
        fout.write(('cTest = %d\n' % self.cTest))
# Changemenet de seuil
    def SetSeuil(self,id,val):
        if (id == 0) : self.B0 = val
        elif (id == 1) : self.B1 = val
        elif ( id== 2 ) : self.B2=val
# Set mask
    def SetMasks(self,id,val):
        if(id == 0) : 
            self.Mask0 = '0x%x' % val
        elif(id == 1) : 
            self.Mask1 = '0x%x' % val
        elif(id == 2) : 
            self.Mask2 = '0x%x' % val

#do calibration
    def SetCalibration(self,channel):
        self.cTest=channel
    def UnSetCalibration(self):
        self.cTest=0
# do PowerPusling
    def SetPowerPulsing(self):
        self.ClkMux = 0
        self.ScOn = 0
        self.OtaQ_PwrADC = 0
        self.PwrOnW = 0
        self.PwrOnSS = 0
        self.PwrOnBuff = 0
        self.PwrOnPA = 0
        self.Discri0 = 0
        self.Discri1 = 0
        self.Discri2 = 0
        self.OtaBgSw = 0
        self.DacSw = 0
        self.PwrOnFsb0 = 0
        self.PwrOnFsb1 = 0
        self.PwrOnFsb2 = 0
# undo PowerPusling
    def UnSetPowerPulsing(self):
        self.ClkMux = 1
        self.ScOn = 1
        self.OtaQ_PwrADC = 1
        self.PwrOnW = 1
        self.PwrOnSS = 1
        self.PwrOnBuff = 1
        self.PwrOnPA = 1
        self.Discri0 = 1
        self.Discri1 = 1
        self.Discri2 = 1
        self.OtaBgSw = 1
        self.DacSw = 1
        self.PwrOnFsb0 = 1
        self.PwrOnFsb1 = 1
        self.PwrOnFsb2 = 1
# set gain
    def SetGains(self,gain):
        self.PaGain0 = gain
        self.PaGain1 = gain
        self.PaGain2 = gain
        self.PaGain3 = gain
        self.PaGain4 = gain
        self.PaGain5 = gain
        self.PaGain6 = gain
        self.PaGain7 = gain
        self.PaGain8 = gain
        self.PaGain9 = gain
        self.PaGain10 = gain
        self.PaGain11 = gain
        self.PaGain12 = gain
        self.PaGain13 = gain
        self.PaGain14 = gain
        self.PaGain15 = gain
        self.PaGain16 = gain
        self.PaGain17 = gain
        self.PaGain18 = gain
        self.PaGain19 = gain
        self.PaGain20 = gain
        self.PaGain21 = gain
        self.PaGain22 = gain
        self.PaGain23 = gain
        self.PaGain24 = gain
        self.PaGain25 = gain
        self.PaGain26 = gain
        self.PaGain27 = gain
        self.PaGain28 = gain
        self.PaGain29 = gain
        self.PaGain30 = gain
        self.PaGain31 = gain
        self.PaGain32 = gain
        self.PaGain33 = gain
        self.PaGain34 = gain
        self.PaGain35 = gain
        self.PaGain36 = gain
        self.PaGain37 = gain
        self.PaGain38 = gain
        self.PaGain39 = gain
        self.PaGain40 = gain
        self.PaGain41 = gain
        self.PaGain42 = gain
        self.PaGain43 = gain
        self.PaGain44 = gain
        self.PaGain45 = gain
        self.PaGain46 = gain
        self.PaGain47 = gain
        self.PaGain48 = gain
        self.PaGain49 = gain
        self.PaGain50 = gain
        self.PaGain51 = gain
        self.PaGain52 = gain
        self.PaGain53 = gain
        self.PaGain54 = gain
        self.PaGain55 = gain
        self.PaGain56 = gain
        self.PaGain57 = gain
        self.PaGain58 = gain
        self.PaGain59 = gain
        self.PaGain60 = gain
        self.PaGain61 = gain
        self.PaGain62 = gain
        self.PaGain63 = gain

    def insert(self,connector):
        """
        Insert the HR2 in the sqlite DB
        """
        c= connector.cursor()
 #       print str_cmd
        snew = buildInsert(self,"HR2")
#        print snew
        #      print type(snew)
        #print type(str_cmd)
        c.execute(snew)
        c.execute("select MAX(ID) from HR2")
        for row in c:
            lastid=row[0]
            break
        self.ID=lastid
        connector.commit()
        return lastid

    def download(self,connector,condition):
        """
        download the HR2 from the sqlite DB
        """
        c= connector.cursor()
#        condition = " WHERE DIF_ID=%d AND NUM=%d" % (difid,num)
        snew = buildSelect(self,'HR2',condition)
#        print snew
        c.execute(snew)
        lnames=[]
        for name,val in sorted(self.__dict__.iteritems()):
            lnames.append(name)

        vobj=[]
        for row in c:
           # print row
            hr2=HR2Def(0)
            for i in range(len(lnames)):
                #print lnames[i],row[i]
                hr2.__dict__[lnames[i]]=row[i]
            vobj.append(hr2)
        return vobj


def generateDIF(x,id,first,last,fdir="/data/online/config"):
    for i in range(first,last+1):
        x.Header=i
        #x.Print()
        name='%s/HR2_FT101%.3d_%d.cfg' % (fdir,id,i)
        print 'generating %s' % name
        fout=open(name,'w+')
        x.Write(fout)
        fout.close()

class DCCDef:
    def __init__(self):
        self.ID=0
        self.NAME="NONE"
        self.FIRMWARE="Rev0"
        self.HARDWARE="Rev0"
        self.SETUP_ID=0
        self.LDA_CHANNEL=0
    def insert(self,connector):
        """
        Insert the DCC in the sqlite DB
        """
        c= connector.cursor()
 #       print str_cmd
        snew = buildInsert(self,"DCC")
#        print snew
        #      print type(snew)
        #print type(str_cmd)
        c.execute(snew)
        c.execute("select MAX(ID) from DCC")
        for row in c:
            lastid=row[0]
            break
        connector.commit()
        return lastid


    def download(self,connector,condition):
        """
        download the DCC from the sqlite DB
        """
        c= connector.cursor()


        snew = buildSelect(self,'DCC',condition)
#        print snew
        c.execute(snew)
        lnames=[]
        for name,val in sorted(self.__dict__.iteritems()):
            lnames.append(name)

        vobj=[]
        for row in c:
#            print row
            obj=DCCDef()
            for i in range(len(lnames)):
                obj.__dict__[lnames[i]]=row[i]
            vobj.append(obj)
        return vobj



class LDADef:
    def __init__(self):
        self.ID=0
        self.NAME="NONE"
        self.FIRMWARE="Rev0"
        self.HARDWARE="Rev0"
        self.SETUP_ID=0
        self.MACADDRESS="ff:ff:ff:ff:ff:ff"
    def insert(self,connector):
        """
        Insert the LDA in the sqlite DB
        """
        c= connector.cursor()
 #       print str_cmd
        snew = buildInsert(self,"LDA")
#        print snew
        #      print type(snew)
        #print type(str_cmd)
        c.execute(snew)
        c.execute("select MAX(ID) from LDA")
        for row in c:
            lastid=row[0]
            break
        self.ID=lastid
        connector.commit()
        return lastid


    def download(self,connector,condition):
        """
        download the LDA from the sqlite DB
        """
        c= connector.cursor()


        snew = buildSelect(self,'LDA',condition)
#        print snew
        c.execute(snew)
        lnames=[]
        for name,val in sorted(self.__dict__.iteritems()):
            lnames.append(name)
        vobj=[]
        for row in c:
#            print row
            obj=LDADef()
            for i in range(len(lnames)):
                obj.__dict__[lnames[i]]=row[i]
            vobj.append(obj)
        return vobj


class SETUPDef:
    def __init__(self):
        self.ID=0
        self.NAME="NONE"
        self.FIRST_RUN=0
        self.LAST_RUN=99999999

    def insert(self,connector):
        """
        Insert the SETUP in the sqlite DB
        """
        c= connector.cursor()
 #       print str_cmd
        snew = buildInsert(self,"SETUP")
#        print snew
        #      print type(snew)
        #print type(str_cmd)
        c.execute(snew)
        c.execute("select MAX(ID) from SETUP")
        for row in c:
            lastid=row[0]
            break
        self.ID=lastid
        connector.commit()
        return lastid
    def download(self,connector,condition):
        """
        download the SETUP from the sqlite DB
        """
        c= connector.cursor()


        snew = buildSelect(self,'SETUP',condition)
#        print snew
        c.execute(snew)
        lnames=[]
        for name,val in sorted(self.__dict__.iteritems()):
            lnames.append(name)
        vobj=[]
        for row in c:
#            print row
            obj=SETUPDef()
            for i in range(len(lnames)):
                obj.__dict__[lnames[i]]=row[i]
            vobj.append(obj)
        return vobj





class DIFDef:
    def __init__(self):
        self.ID =0
        self.NAME="NONE"
        self.FIRMWARE="Rev0"
        self.HARDWARE="Rev0"
        self.DCC_CHANNEL=0
        self.LDA_ADDRESS="ff:ff:ff:ff:ff:ff"
        self.LDA_CHANNEL=0
        self.CHAMBER_ID=0
        self.SETUP_ID=0
        self.MASK=0
        self.NUM=0
        self.HW_TYPE=0
        self.MonitorSequencer=1
        self.NumericalReadoutMode=1
        self.NumericalReadoutStartMode=0
        self.AVDDShdn=1
        self.DVDDShdn=1
        self.DIFIMonGain=50
        self.SlabIMonGain=50
        self.MonitoredChannel=3
        self.PowerAnalog=1
        self.PowerDigital=1
        self.PowerDAC=1
        self.PowerADC=1
        self.PowerSS=1
        self.TimerHoldRegister=5
        self.EnableMonitoring=0
        self.MemoryDisplayLimitMax=10
        self.MemoryEfficiencyLimitMax=10
        self.MemoryDisplayLimitMin=0
        self.MemoryEfficiencyLimitMin=0			

    def insert(self,connector):
        """
        Insert the DIF in the sqlite DB
        """
        c= connector.cursor()
 #       print str_cmd
        snew = buildInsert(self,"DIF")
#        print snew
        #      print type(snew)
        #print type(str_cmd)
        c.execute(snew)
        c.execute("select MAX(ID) from DIF")
        for row in c:
            lastid=row[0]
            break
        connector.commit()
        return lastid


    def download(self,connector,condition):
        """
        download the DIF from the sqlite DB
        """
        c= connector.cursor()


        snew = buildSelect(self,'DIF',condition)
#        print snew
        c.execute(snew)
        lnames=[]
        for name,val in sorted(self.__dict__.iteritems()):
            lnames.append(name)

        vobj=[]
        for row in c:
#            print row
            obj=DIFDef()
            for i in range(len(lnames)):
                obj.__dict__[lnames[i]]=row[i]
            vobj.append(obj)

        
        return vobj

    def Print(self):
          print "MonitorSequencer = %d\n" % self.MonitorSequencer
          print "NumericalReadoutMode = %d\n" % self.NumericalReadoutMode
          print "NumericalReadoutStartMode = %d\n" % self.NumericalReadoutStartMode
          print "AVDDShdn = %d\n" % self.AVDDShdn
          print "DVDDShdn = %d\n" % self.DVDDShdn
          print "DIFIMonGain = %d\n" % self.DIFIMonGain
          print "SlabIMonGain = %d\n" % self.SlabIMonGain
          print "MonitoredChannel = %d\n" % self.MonitoredChannel
          print "PowerAnalog = %d\n" % self.PowerAnalog
          print "PowerDigital = %d\n" % self.PowerDigital
          print "PowerDAC = %d\n" % self.PowerDAC
          print "PowerADC = %d\n" % self.PowerADC
          print "PowerSS = %d\n" % self.PowerSS
          print "TimerHoldRegister = %d\n" % self.TimerHoldRegister
          print "EnableMonitoring = %d\n" % self.EnableMonitoring
          print "MemoryDisplayLimitMax = %d\n" % self.MemoryDisplayLimitMax
          print "MemoryEfficiencyLimitMax = %d\n" % self.MemoryEfficiencyLimitMax
          print "MemoryDisplayLimitMin = %d\n" % self.MemoryDisplayLimitMin
          print "MemoryEfficiencyLimitMin = %d\n" % self.MemoryEfficiencyLimitMin
    def Write(self,fout):
          fout.write(("Monitor Sequencer = %d\n" % self.MonitorSequencer))
          fout.write(("Numerical Readout Mode = %d\n" % self.NumericalReadoutMode))
          fout.write(("Numerical Readout Start Mode = %d\n" % self.NumericalReadoutStartMode))
          fout.write(("AVDD Shdn = %d\n" % self.AVDDShdn))
          fout.write(("DVDD Shdn = %d\n" % self.DVDDShdn))
          fout.write(("DIF IMon Gain = %d\n" % self.DIFIMonGain))
          fout.write(("Slab IMon Gain = %d\n" % self.SlabIMonGain))
          fout.write(("Monitored Channel = %d\n" % self.MonitoredChannel))
          fout.write(("Power Analog = %d\n" % self.PowerAnalog))
          fout.write(("Power Digital = %d\n" % self.PowerDigital))
          fout.write(("Power DAC = %d\n" % self.PowerDAC))
          fout.write(("Power ADC = %d\n" % self.PowerADC))
          fout.write(("Power SS = %d\n" % self.PowerSS))
          fout.write(("Timer Hold Register = %d\n" % self.TimerHoldRegister))
          fout.write(("Enable Monitoring = %d\n" % self.EnableMonitoring))
          fout.write(("Memory Display Limit Max = %d\n" % self.MemoryDisplayLimitMax))
          fout.write(("Memory Efficiency Limit Max = %d\n" % self.MemoryEfficiencyLimitMax))    
	  fout.write(("Memory Display Limit Min = %d\n" % self.MemoryDisplayLimitMin))
          fout.write(("Memory Efficiency Limit Min = %d\n" % self.MemoryEfficiencyLimitMin))
def generateDIFBoard(x,id,fdir="/data/online/config"):
        name='%s/DIF_FT101%.3d.cfg' % (fdir,id)
        print 'generating %s' % name
        fout=open(name,'w+')
        x.Write(fout)
        fout.close()


class Setup:
    def __init__(self,name):
        self.LDAs={}
        self.DIFs={}
        self.HR2s={}
        self.dbdef = SETUPDef()
        self.dbdef.NAME=name
        self.conn=None
    def openDb(self,fileName):
        self.conn = sqlite.connect(fileName)
    def openMysqlDb(self,host,username,password,baseName):
        self.conn = MySQLdb.connect(host,username,password,baseName)
    def closeDb(self):
        self.conn.close()
    def create(self):
        if (self.conn!=None):
            self.dbdef.ID = self.dbdef.insert(self.conn)
    
    def addLDA(self,ld):
        if (self.dbdef.ID==0):
            print "Please create or download first"
            return
        ld.SETUP_ID=self.dbdef.ID
        ld.insert(self.conn)
        self.LDAs[ld.MACADDRESS]=ld
        return ld.ID
    def getLDA(self,mac):
        return self.LDAs[mac]
    def addDIF(self,di):
        if (self.dbdef.ID==0):
            print "Please create or download first"
            return
        di.SETUP_ID=self.dbdef.ID
        di.insert(self.conn)
        self.DIFs[di.NUM]=di
        return di.ID
    def getDIF(self,num):
        return self.DIFs[num]

    def connectDIF(self,dif,lda,ldachannel,dccchannel=0):
        dif.LDA_ADDRESS=lda.MACADDRESS
        dif.LDA_CHANNEL=ldachannel
        dif.DCC_CHANNEL=dccchannel
    def addHR2(self,hr):
        if (self.dbdef.ID==0):
            print "Please create or download first"
            return
        hr.SETUP_ID=self.dbdef.ID
        hr.insert(self.conn)
        self.HR2s[(hr.DIF_NUM<<8)| hr.Header ]=hr
        return hr.ID
    def getHR2(self,difnum,num):
        return self.DIFs[(difnum<<8)|num]

    def update(self,inc_setup):
        self.dbdef = inc_setup.dbdef
        self.create()
        # Store LDAs
        for mac,ld in inc_setup.LDAs.iteritems():
  #          print mac
  #          print ld
            self.addLDA(ld)
        # Store DIFs
        for num,di in inc_setup.DIFs.iteritems():
            self.addDIF(di)
        #Store HR21
        for num,hr in inc_setup.HR2s.iteritems():
            self.addHR2(hr)

    def download(self,setup_id):
        s = SETUPDef()
        self.dbdef = s.download(self.conn," WHERE ID=%d" % setup_id)[0]
        l=LDADef()
        vlda = l.download(self.conn," WHERE SETUP_ID=%d" % setup_id)
        for x in vlda:
            self.LDAs[x.MACADDRESS]= x
        d=DIFDef()
        vdif = d.download(self.conn," WHERE SETUP_ID=%d" % setup_id)
        for x in vdif:
 #           print x
            self.DIFs[x.NUM]= x
        h=HR2Def(0)
        vhr = h.download(self.conn," WHERE SETUP_ID=%d" % setup_id)
        for x in vhr:
#            print "%d" % string.atoi(x.Mask1,16)
            self.HR2s[(x.DIF_NUM<<8)|x.Header]= x


        
