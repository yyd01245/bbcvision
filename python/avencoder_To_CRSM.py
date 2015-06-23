#!/usr/bin/env python

"""
    avencoder Control
"""

import os
import os.path
import json
import socket
import time
import uuid
import sys
import string

fileLogName = "crsmresid.log"
RSM_ServerIP = "192.168.200.88"
RSM_ServerPort = 15174
URL = "http://192.168.70.106/cisco_test11/tv2.0352.html"

def startVNC(vncPath,index):
    print "start vnc index %d \n" % index
    """get env xvnc """
    strPathEnv = os.getenv('PATH')
    print strPathEnv


def ExecSystem():
    pid = os.fork()
    if pid == 0:
        """ child process """
        vncPath 
        os.system(vncPath)
    else:
        """parent process"""
        print "child PID is %d " % pid

def SaveStreamIDToFile(fobj,strSID):
    """ open file write sid to file"""
    
    fobj.writelines(strSID)
    fobj.write("\n")

def SendJsonData(ServerIP,ServerPort,strBuff):
    address = (ServerIP,ServerPort)
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    while True:
        try:
            sock.connect(address)
            break
        except Exception,e:
            time.sleep(0.1)
            continue
    lenght = len(strBuff)
    print "Send:",strBuff
    sdlen = sock.send(strBuff)

    data = sock.recv(1476)
    print "Recv:",data
    sock.close()

    

def startOneStream(strSID,dstip,iport):
    """ connect to rsm create resid  and send logvnc to rsm
    
    """
    strSendBuff = {"cmd":"login","sessionid":"1","sid":"1001","operid":"1","authname":"cscs",\
                   "authcode":"123456","vnctype":"PORTAL","videotype":"HD","vendor":"BLUELINK",\
                   "rate":"3072","url":"http://192.168.70.106/cisco_test11/tv2.0352.html",\
                   "iip":"192.168.60.104","iport":"50448","sip":"192.168.200.88","sport":"20909",\
                   "areaid":"3301","serialno":"1dfawefwew","msg": ""}
    strSendBuff["iport"] = iport
    strSendBuff["iip"] = dstip
    strSendBuff["url"] = URL
    strSendBuff["sessionid"] = strSID
    strSendBuff["serialno"] = strSID
    print "BUFF:",repr(strSendBuff)
    data_string = json.dumps(strSendBuff)
    print "JSON:",data_string

    data_string = data_string + "XXEE"
    hostIP = RSM_ServerIP
    hostport = RSM_ServerPort
    SendJsonData(hostIP,hostport,data_string)

def EndOneStream(strSID):
    
    strSendBuff = {"cmd":"logout","sessionid":"1","sid":"1001","authname":"cscs","authcode":"123456",\
     "resid":"667588c422b941eb9ea21dc9db519798","operid":"d8062d110cbf4fda8e39881509ac7dce",\
     "serialno":"1","msg":""}

    strSendBuff["sessionid"] = strSID
    strSendBuff["serialno"] = strSID
    print "BUFF:",repr(strSendBuff)
    data_string = json.dumps(strSendBuff)
    print "JSON:",data_string

    data_string = data_string + "XXEE"
    hostIP = RSM_ServerIP
    hostport = RSM_ServerPort
    SendJsonData(hostIP,hostport,data_string)



def FreeStreamIDFromFile():
    if not os.path.exists(fileLogName):
        print "cann't find file "
        return 
    
    fobj = open(fileLogName,"r")
    for eachline in fobj:
        print eachline
        eachline = eachline.strip('\n')  
        EndOneStream(eachline)
        time.sleep(4)
    Close(fobj)
    os.remove(fileLogName)

def Close(fobj):
    fobj.close()


def LoopStart(totalnum,dstip,beginPort):
    if not os.path.exists(fileLogName):
        print "create file "
        fobj = open(fileLogName,"w")
    else:
        print "append file"
        fobj = open(fileLogName,"a")
    port = beginPort
    for index in range(totalnum):
        resid = uuid.uuid1()
        print resid
         
        strid = "%s" % resid
        SaveStreamIDToFile(fobj,strid)
        tem='%d' % port
        port += 2
        startOneStream(strid,dstip,tem)
        time.sleep(4)
    Close(fobj)


if __name__ == "__main__":
     print '-'*10,'show dir','-'*10
  
     paramlen = len(sys.argv)
     if cmp(sys.argv[1],"login")==0:
         num = string.atoi(sys.argv[2])
         dstip = sys.argv[3]
         beginport = string.atoi(sys.argv[4])
         print "login num :%d dst ip %s begin port %d" % (num,dstip,beginport)
         LoopStart(num,dstip,beginport)
     elif cmp(sys.argv[1],"logout")==0:
         FreeStreamIDFromFile()
     elif cmp(sys.argv[1],"?")==0:
         print "use login num dstportbeign"
         print "use logout"

     
print 'DONE!'   
