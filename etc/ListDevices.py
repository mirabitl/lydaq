#!/usr/bin/python
import os
os.environ["PATH"] = os.environ["PATH"] +":/sbin"
f=os.popen("sudo lsusb -v -d 0x403: | sed 's/^[ \t]*//;s/[ \t]*$//'")
lines=f.readlines()
devices=[]
info=[]
for x in lines:
 ff=x.split()
 if (len(ff) == 0) :
  continue
 if (ff[0] == 'Bus'):
   if (len(info)!=0):
       print info
       devices.append(info)
   info=[]
   info.append(ff[1])
   info.append(ff[3])
   info.append(ff[5])
 if ((ff[0] == 'iSerial') and (len(ff)>2)):  
   info.append(ff[2])
   print "info ",info
if (len(info)!=0):
    devices.append(info)

f=os.open("/var/log/pi/ftdi_devices",os.O_RDWR|os.O_CREAT)
for y in devices:
    print "Device found =>"
    print y
    print "len", len(y)
    if (len(y)>3):
      print y[0]
      print y[1]
      print y[2]   
      print y[3]
      bus=y[0]
      dev=y[1].split(":")[0]
      vend="0x"+y[2].split(":")[0]
      prod="0x"+y[2].split(":")[1]
      ser=y[3]
      command="sudo chmod 666 /dev/bus/usb/"+bus+"/"+dev
      print command
      print vend,prod,ser
      os.system(command)
    
      os.write(f,"%s %s %s\n" % (vend,prod,ser) )

os.close(f)
