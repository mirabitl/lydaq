import lydaqrc

class febRC(lydaqrc.lydaqControl):
    
    def Connect(self):
        self.parseMongo()
        self.discover()
        
    def BuilderStatus(self):
        rep={}
        for ( k,v in self.appMap.items):
            if (k != "BUILDER"):
                continue
            for (s in v):
                r={}
                r['run']=-1;r['event']=-1;r['url']=s.host
                mr=json.loads(s.sendCommand("STATUS",None))
                if (mr['status']!="FAILED"):
                    r["run"] = mr["answer"]["answer"]["run"]
                    r["event"] = mr["answer"]["answer"]["event"]
                    r["builder"] = mr["answer"]["answer"]["difs"]
                    r["built"] = mr["answer"]["answer"]["build"]
                    r["total"] = mr["answer"]["answer"]["total"]
                    r["compressed"] = mr["answer"]["answer"]["compressed"]
                    r["time"] = DateTime.now().millisecondsSinceEpoch / 1000;
