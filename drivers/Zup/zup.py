#!/usr/bin/python
import time
import serial
import os,sys


class zup:
    def __init__(self,device,number):
        self.ser=serial.Serial(device)
        self.board=number
    
        # configure the serial connections (the parameters differs on the device you are connecting to)


        print self.ser.portstr       # check which port was really used
        self.ser.baudrate = 9600

        self.ser.bytesize = serial.EIGHTBITS #number of bits per bytes
        
        self.ser.parity = serial.PARITY_NONE #set parity check: no parity
        
        self.ser.stopbits = serial.STOPBITS_ONE #number of stop bits

        #block read
        #self.ser.timeout = None          

        self.ser.timeout = 1            #non-block read

        self.ser.write(":ADR%.2d" % self.board)

        #self.ser.write("RMT %d\r" % 1)  

        #print self.ser.readline()
        
    def setOn(self,ch):
        self.ser.write(":OUT1;")
        print self.ser.readline()
    def setOff(self,ch):
        self.ser.write(":OUT0;")
        print self.ser.readline()

    def status(self,ch):
        self.ser.write(":MDL?;")
        
        rc=self.ser.readline().split(",")
        print rc
        self.ser.write(":VOL!;")
        
        rc=self.ser.readline().split(",")
        print rc
        self.ser.write(":VOL?;")
        
        rc=self.ser.readline().split(",")
        print rc
        self.ser.write(":CUR?;")
        
        rc=self.ser.readline().split(",")
        print rc
