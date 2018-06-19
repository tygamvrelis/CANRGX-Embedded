
import sys
import socket
import struct
from datetime import datetime
import time

def makeData(zAccel):
    ''' Generates plausible data, parametrized by the Z-acceleration.
    '''
    
    # Big endian because that is consistent with network byte order
    data = bytes(''.encode())
    data = data + struct.pack('>d', 1234567890)
    data = data + struct.pack('>i', 9)
    data = data + struct.pack('>i', 1)
    data = data + struct.pack('>f', 1.1)
    data = data + struct.pack('>f', 1.2)
    data = data + struct.pack('>f', 3.14)
    data = data + struct.pack('>f', 0.019)
    data = data + struct.pack('>f', 0.24)
    data = data + struct.pack('>f', 0.044)
    data = data + struct.pack('>d', 1.902839)
    data = data + struct.pack('>d', 34.4456987645)
    data = data + struct.pack('>d', 42313.987638450)
    data = data + struct.pack('>f', 414.1279872)
    data = data + struct.pack('>f', 78.53744)
    data = data + struct.pack('>f', -22.49668)
    data = data + struct.pack('>f', -0.213456673)
    data = data + struct.pack('>f', 0.05566777)
    data = data + struct.pack('>f', zAccel)
    
    return data
    
def main():
    print("Starting PC-side application (sender-side)")
        
    # The aircraft data is broadcast as a UDP packet on port 5124.
    # The host computer must have an IP address as follows: 132.246.192.[25..50]
    host = "134.246.192.255" # Ending in 255
    UDP_PORT = 5124

    # Create a socket for sending
    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    print("Socket created")             
    
    # Configure socket for broadcast
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)     
    print("Socket broadcasing on {0}:{1}".format(host, UDP_PORT)) 

    # Send dummy data forever
    numPackets = 0
    zeroG = False
    while(True):
        if(not zeroG):
             data = makeData(9.81)
        else:
             data = makeData(0.213)
        
        # Send a new message every 10 ms
        sock.sendto(data, (host, UDP_PORT))
        numPackets = numPackets + 1

        # Every 100 transmits, toggle zeroG event
        if(numPackets % 100 == 0):
            zeroG = not zeroG
            print(str(datetime.now()) + " Sent {0} packets (with zeroG {1})".format(numPackets, zeroG))
            
        time.sleep(0.010) # 10 ms
            
if __name__ == "__main__":
    main()