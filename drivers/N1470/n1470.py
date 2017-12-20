#!/usr/bin/python
import time
import serial
import os,sys


class mod1470:
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

        self.ser.write("$BD:%d,CMD:MON,PAR:BDCTR\r\n" % self.board)  

        print self.ser.readline()
        
    def setVoltage(self,ch,val):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:VSET,VAL:%f\r\n" % (self.board,ch,val))
        print self.ser.readline()
    def setCurrent(self,ch,val):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:ISET,VAL:%f\r\n" % (self.board,ch,val))
        print self.ser.readline()
    def setRampUp(self,ch,val):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:RUP,VAL:%d\r\n" % (self.board,ch,val))
        print self.ser.readline()
    def setRampDown(self,ch,val):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:RDW,VAL:%d\r\n" % (self.board,ch,val))
        print self.ser.readline()
    def setOn(self,ch):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:ON\r\n" % (self.board,ch))
        print self.ser.readline()
    def setOff(self,ch):
        self.ser.write("$BD:%d,CMD:SET,CH:%d,PAR:OFF\r\n" % (self.board,ch))
        print self.ser.readline()

    def status(self,ch):
        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:POL\r\n" % (self.board,ch))
        
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Polarity: ",rc[2].split(":")[1]
        else:
            print rc
            
        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:STAT\r\n" % (self.board,ch))
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Status: ",rc[2].split(":")[1]
        else:
            print rc


        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:VSET\r\n" % (self.board,ch))
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Vset: ",rc[2].split(":")[1]
        else:
            print rc


        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:ISET\r\n" % (self.board,ch))
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Iset: ",rc[2].split(":")[1]
        else:
            print rc

        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:VMON\r\n" % (self.board,ch))
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Vmon: ",rc[2].split(":")[1]
        else:
            print rc

        self.ser.write("$BD:%d,CMD:MON,CH:%d,PAR:IMON\r\n" % (self.board,ch))
        rc=self.ser.readline().split(",")
        if (len(rc)==3):
            print "Imon: ",rc[2].split(":")[1]
        else:
            print rc

