
import os
import sys
import socket
import struct
from datetime import datetime
import time

def NavPacketDecoder(packet):
    ''' Decodes the navigational packet, interpreted as per the Excel document
        given by the NRC.
    '''
    
    # Big endian because that is consistent with network byte order
    d = dict()
    d['GPSTime'] = struct.unpack('>d', packet[0:8])[0]
    d['INSMode'] = struct.unpack('>i', packet[8:12])[0]
    d['GPSMode'] = struct.unpack('>i', packet[12:16])[0]
    d['Roll'] = struct.unpack('>f', packet[16:20])[0]
    d['Pitch'] = struct.unpack('>f', packet[20:24])[0]
    d['TrueHeading'] = struct.unpack('>f', packet[24:28])[0]
    d['AngularRateX'] = struct.unpack('>f', packet[28:32])[0]
    d['AngularRateY'] = struct.unpack('>f', packet[32:36])[0]
    d['AngularRateZ'] = struct.unpack('>f', packet[36:40])[0]
    d['Latitude'] = struct.unpack('>d', packet[40:48])[0]
    d['Longitude'] = struct.unpack('>d', packet[48:56])[0]
    d['Altitude'] = struct.unpack('>d', packet[56:64])[0]
    d['VelocityNorth'] = struct.unpack('>f', packet[64:68])[0]
    d['VelocityEast'] = struct.unpack('>f', packet[68:72])[0]
    d['VelocityDown'] = struct.unpack('>f', packet[72:76])[0]
    d['AccelerationX'] = struct.unpack('>f', packet[76:80])[0]
    d['AccelerationY'] = struct.unpack('>f', packet[80:84])[0]
    d['AccelerationZ'] = struct.unpack('>f', packet[84:88])[0]
    
    return d
    
def logString(userMsg, file):
    theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg
    print(theString)
    file.write(theString)
    file.flush()

def main():
    print("Starting PC-side application")
    
    data_root = 'CANRGX_data\\' + "packet\\" + time.strftime('%Y_%m_%d_%H_%M_%S')+'\\'
    if not os.path.exists(data_root):
        os.makedirs(data_root)
        
    with open(data_root + time.strftime('%Y_%m_%d_%H_%M_%S') + ".txt", 'w') as f:
        logString("Log created at " + str(os.getcwd()) + '\\' + data_root, f)
        # The aircraft data is broadcast as a UDP packet on port 5124.
        # The host computer must have an IP address as follows: 132.246.192.[25..50]
        bind_ip = '127.0.0.1' #'134.246.192.255'
        bind_port = 5124
    
        # Create a socket for receiving
        sock = socket.socket(socket.AF_INET, # Internet
                            socket.SOCK_DGRAM) # UDP
        print("Socket created")                   
        
        # Bind the socket to the IP address and port
        sock.bind((bind_ip, bind_port))
        print("Socket connected to {0}:{1}".format(bind_ip, bind_port))
    
        numReceptions = 0
        while True:
            # Recive data
            rawNavPacket, addr = sock.recvfrom(88)
            numReceptions += 1
    
            # Unpack data
            parsedNavPacket = NavPacketDecoder(rawNavPacket)
                
            # Log data
            f.write(datetime.now().strftime('%H.%M.%S.%f') + '\n')
            for key in parsedNavPacket.keys():
                f.write(str(key) + ": " + str(parsedNavPacket[key]) + '\n\n')
            
            # Print data every so often
            if(numReceptions % 100 == 0):
                for key in parsedNavPacket.keys():
                    print(str(key) + ": " + str(parsedNavPacket[key]))

if __name__ == "__main__":
    main()