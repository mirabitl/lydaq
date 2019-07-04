from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import lyondb as idty

connection = MongoClient(idty.MONGO_HOST, idty.MONGO_PORT)
db = connection[idty.MONGO_DB]
db.authenticate(idty.MONGO_USER, idty.MONGO_PASS)

slc={}
n_asics=[]
for res in db.states.find({}):
    print res
for res in db.asics.find({'_id':ObjectId('5d1492ee4167474cb097da35')}):
    s={}
    s["slc"]=res["slc"]
    s["num"]=res["num"]
    s["dif"]=res["dif"]
    #print res["dif"]
    
    n_asics.append(s)
    #if (res["dif"] == 202025152):
    #    for i in range(13,157):
    #        a=s.copy()
    #        a["dif"]=((i&0xFF)<<24)|(10<<16)|(168<<8)|192
    #        print i,a
    #        #v=raw_input()
    #        n_asics.append(a)


#print n_asics
#results=db.asics.insert_many(n_asics)
#print results.inserted_ids
slc["asics"]=n_asics
f= open("/dev/shm/asics","w+")
f.write(json.dumps(slc))
f.close()
