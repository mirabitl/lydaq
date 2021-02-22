#!/usr/bin/env python3
from __future__ import absolute_import
from __future__ import print_function
import os
import socks
import socket
import six.moves.http_client
import six.moves.urllib.request, six.moves.urllib.parse, six.moves.urllib.error
import six.moves.urllib.request, six.moves.urllib.error, six.moves.urllib.parse
from six.moves.urllib.error import URLError, HTTPError
import json
from copy import deepcopy
import base64
import time
import requests
import six

try:
    from urllib.parse import urlparse
except ImportError:
     from six.moves.urllib.parse import urlparse
# Sock support
sockport = None
sp = os.getenv("SOCKPORT", "Not Found")
if (sp != "Not Found"):
    sockport = int(sp)
if (sockport != None):
    # print "Using SOCKPORT ",sockport
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", sockport)
    socket.socket = socks.socksocket
    # socks.wrapmodule(urllib2)


def executeRequest(url):
    req = six.moves.urllib.request.Request(url)
    try:
        r1 = six.moves.urllib.request.urlopen(req,timeout=0.1)
    except URLError as e:
        p_rep = {}
        p_rep["STATE"] = "DEAD"
        return False

    return True

hostlist=[]
for i in range(1,25):
    url="http://lyoilcrpi%.2d:888" % i
    if (executeRequest(url)):
        hostlist.append("lyoilcrpi%.2d" % i)
    
for i in range(1,9):
    url="http://lyorpipul%.2d:888" % i
    if (executeRequest(url)):
        hostlist.append("lyorpipul%.2d" % i)
for i in range(7,15):
    url="http://lyosdhcal%d:888" % i
    if (executeRequest(url)):
        hostlist.append("lyosdhcal%d" % i)
    
for i in range(1,5):
    url="http://lyocmsmu%.2d:888" % i
    if (executeRequest(url)):
        hostlist.append("lyocmsmu%.2d" % i)

#print("living=",hostlist)
s="living="+json.dumps(hostlist)


fo = open("/opt/dhcal/script/lyon_slow_alive.py", "wb")

fo.write(s.encode())
fo.close()
#scm="--host "
#for x in hostlist:
#  scm=scm+"%s," %x

#print(scm)
