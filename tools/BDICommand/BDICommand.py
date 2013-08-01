import time
import telnetlib
import sys


tn = telnetlib.Telnet( )
tn.open(sys.argv[1], sys.argv[2])
tn.read_until("Core#0>")


command = sys.argv[3]

if command == "go":
    print "go"
    tn.write("go\r")

if command == "reset":
    print "resetting..."
    tn.write("reset\r")
    tn.read_until("target startup passed")
    tn.read_until("Core#0>")
    
if command == "load":  
    argc = len(sys.argv)
   
    for i in range(4, argc):
        arg = sys.argv[i]
        #print arg
        tn.write(arg +"\r")
        answer = tn.read_until("Loading program file passed")
        print answer.replace('\r', '') 
        answer = tn.read_until("Core#0>")
        #print answer.replace('\r', '') 
        time.sleep(0.5)

print ""




