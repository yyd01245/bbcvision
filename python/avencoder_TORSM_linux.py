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
import subprocess

from subprocess import Popen, PIPE


fileLogName = "resid.log"
RSM_ServerIP = "192.168.200.183"
RSM_ServerPort = 25000
URL = "http://218.108.50.246/test6/test6/test6.htm"
TDP_Path="./"
TDP_Name="TDP"
global TTStreamSuccess
global TTStreamFailed
TTStreamSuccess = 0
TTStreamFailed = 0

file_Returnlog = "reslut.log"

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
    '''recvdict = eval(data[0:-4]);'''
    recvdict = eval(data[0:-4]);
   ''' print "recvdict ",recvdict'''

    global TTStreamSuccess
    global TTStreamFailed
   ''' print "code %s " % (recvdict["retcode"])'''
    if(cmp(recvdict["retcode"],"0") == 0):
        TTStreamSuccess += 1
    else:
        TTStreamFailed += 1
        
    print "current code %s, total failed num=%d,success num=%d" % (recvdict["retcode"],TTStreamSuccess,TTStreamFailed)
    return recvdict["retcode"]
        
    
    
    

def startOneStream(strSID,dstip,iport,freturnlog):
    """ connect to rsm create resid  and send logvnc to rsm
    
    """
    strSendBuff = {"cmd":"vnclogin","resid":"db97d8f424da87b2468317","iip":"192.168.60.104","iport":"51448",\
                    "rate":"3072","url":"http://vod.appaccess.wasu.cn/template_html/tv4/cstv4/index_recommend/index_recommend.html",\
                    "serialno":"db974e9d8f424d2da87b246831442867"}

    '''begin tdp recv'''
    exepath = TDP_Path + TDP_Name+" port="+iport

    """"
    os.system(exepath)
    """
    p = Popen(exepath, stdin=subprocess.PIPE, stdout=subprocess.PIPE,shell=True)
   ''' p.communicate('Line number %d.\n' % )'''

    strSendBuff["iport"] = iport
    strSendBuff["iip"] = dstip
    strSendBuff["resid"] = strSID
    strSendBuff["url"] = URL
    print "BUFF:",repr(strSendBuff)
    data_string = json.dumps(strSendBuff)
    print "JSON:",data_string
    data_string = data_string + "XXEE"
    hostIP = RSM_ServerIP
    hostport = RSM_ServerPort
    iret = SendJsonData(hostIP,hostport,data_string)
    print "return %s " % (iret)
    logtxt = "SID="+strSID+" dstip="+dstip+" iport="+iport+"  retcode:"+iret
    freturnlog.writelines(logtxt)
    freturnlog.write("\n")

def EndOneStream(strSID):
    
    strSendBuff = {"cmd":"vnclogout","resid":"db97d8f424da87b2468317",\
                    "serialno":"db974e9d8f424d2da87b246831442867"}


    strSendBuff["resid"] = strSID
    print "BUFF:",repr(strSendBuff)
    data_string = json.dumps(strSendBuff)
    print "JSON:",data_string

    data_string = data_string + "XXEE"
    hostIP = RSM_ServerIP
    hostport = RSM_ServerPort
    SendJsonData(hostIP,hostport,data_string)

def SaveStreamIDToFile(fobj,strSID):
    """ open file write sid to file"""
    
    fobj.writelines(strSID)
    fobj.write("\n")

def FreeStreamIDFromFile():
    if not os.path.exists(fileLogName):
        print "cann't find file "
        return 
    
    fobj = open(fileLogName,"r")
    for eachline in fobj:
        print eachline
        eachline = eachline.strip('\n')  
        EndOneStream(eachline)
        time.sleep(2)
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

    if not os.path.exists(file_Returnlog):
        print "create return logfile "
        freturnlog = open(file_Returnlog,"w")
    else:
        print "append return logfile"
        freturnlog = open(file_Returnlog,"a")
    
    port = beginPort
    for index in range(totalnum):
        resid = uuid.uuid1()
        print resid
         
        strid = "%s" % resid
        SaveStreamIDToFile(fobj,strid)
        tem='%d' % port
        port += 2
        startOneStream(strid,dstip,tem,freturnlog)
        time.sleep(2)
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
