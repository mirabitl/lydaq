#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
import bson
from bson.objectid import ObjectId
import time





def IP2Int(ip):
    """
    convert IP adress string to int

    :param IP: the IP address
    :return: the encoded integer 
    """
    o = list(map(int, ip.split('.')))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class MongoSlow:
    """
    Main class to access the Mongo DB 
    """

    def __init__(self, host,port,dbname,username,pwd):
        """
        connect Mongodb database 

        :param host: Hostanme of the PC running the mongo DB

        :param port: Port to access the base

        :param dbname: Data base name

        :param username: Remote access user

        :param pwd: Remote access password

        """
      
        self.connection=MongoClient(host,port)
        self.db=self.connection[dbname]
        self.db.authenticate(username,pwd)

    def reset(self):
        """
        Reset connection to download another configuration
        """
        self.bson_id=[] 
    def items(self,path,depth=50000,from_time=0):
        """
        List all the run informations stored
        """
        device=path.split(",")[2]
        mintime=0
        if (from_time>0):
            mintime=time.time()-from_time
            
        res=self.db.MONITORED_ITEMS.find({"path":{'$regex':path},"ctime":{'$gt':mintime}},{"_id":0}).limit(depth).sort("ctime",pymongo.DESCENDING)
        for x in res:
            #print(x)
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            if (device=="BMP" and x["status"]["name"]==device):
                print("%s P=%.2f mbar T=%.2f K " % (sti,x["status"]["pressure"],x["status"]["temperature"]+273.15))
            if (device=="HIH" and x["status"]["name"]==device):
                print("%s H0=%.2f %% T0=%.2f K H1=%.2f %% T1=%.2f K " % (sti,x["status"]["humidity0"],x["status"]["temperature0"],x["status"]["humidity1"],x["status"]["temperature1"]))
            if (device=="ISEG" and x["status"]["name"]==device):
                print(sti)
                for y in x["status"]["channels"]:
                    sstat=y["status"].split("=")[1]
                    
                    #print("ch%.3d %12.2f %12.2f %12.2f %s" %(y["id"],y["vset"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                    print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
            if (device=="GENESYS" and x["status"]["name"]==device):
                print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (sti,x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["status"]))
            if (device=="SYX27" and x["status"]["name"]==device):
                print(sti)
                for y in x["status"]["channels"]:
                    print("%12s %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["name"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))                

                
def instance():
    """
    Create a MongoJob Object
    
    The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set

    :return: The MongoJob Object
    """
    # create the default access
    login=os.getenv("MGDBMON","NONE")
    if (login == "NONE"):
        print("The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set")
        exit(0)
    userinfo=login.split("@")[0]
    hostinfo=login.split("@")[1]
    dbname=login.split("@")[2]
    user=userinfo.split("/")[0]
    pwd=userinfo.split("/")[1]
    host=hostinfo.split(":")[0]
    port=int(hostinfo.split(":")[1])
    #print(host,port,dbname,user,pwd)
    _wdd=MongoSlow(host,port,dbname,user,pwd)
    #print("apres")
    return _wdd
