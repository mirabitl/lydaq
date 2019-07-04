#!/usr/bin/python
import time
import serial
import os,sys


class genesys:
    def __init__(self,device,number):
        self.ser=serial.Serial(device)
        self.board=number
    
        # configure the serial connections (the parameters differs on the device you are connecting to)


        print self.ser.portstr       # check which port was really used
        self.ser.baudrate = 9600

        self.ser.bytesize = serial.EIGHTBITS #number of bits per bytes
        
        self.ser.parity = serial.PARITY_NONE #set parity check: no parity
        
        self.ser.stopbits = serial.STOPBITS_ONE #number of stop bits

        #ser.timeout = None          #block read

        self.ser.timeout = 1            #non-block read

        self.ser.write("ADR %d\r" % self.board)

        self.ser.write("RMT %d\r" % 1)  

        print self.ser.readline()
        
    def setOn(self,ch):
        self.ser.write("OUT 1\r")
        print self.ser.readline()
    def setOff(self,ch):
        self.ser.write("OUT 0\r")
        print self.ser.readline()

    def status(self,ch):
        self.ser.write("STT?\r")
        
        rc=self.ser.readline().split(",")
        print rc
