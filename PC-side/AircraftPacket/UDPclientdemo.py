import asyncio,socket
import numpy as np


class EchoClientProtocol(asyncio.DatagramProtocol):
    def __init__(self, message, loop):
        self.message = message
        self.loop = loop
        self.transport = None
        self.count=0

    def connection_made(self, transport):
        self.transport = transport
        self.count = 0
        print('Connection Made', self.message)

    def datagram_received(self, data, addr):
        print("Received from: ", addr)

        double_type = np.dtype('>d')
        float_type = np.dtype('>f')
        int_type = np.dtype('>i')

        GPSTime = np.frombuffer(data, double_type, 1, 0)
        ModeInfo = np.frombuffer(data, int_type, 2, 8)
        AttitudeInfo = np.frombuffer(data, float_type, 6, 16)
        EarthInfo = np.frombuffer(data, double_type, 3, 40)
        LinearMotion = np.frombuffer(data, float_type, 6, 64)

        print("GPSTime",GPSTime)
        #print("ModeInfo",ModeInfo)
        #print("AttitudeInfo",AttitudeInfo)
        #print("EarthInfo", EarthInfo)
        print("LinearMotion", LinearMotion)

        self.count+=1
        print("count", self.count)

        if self.count>500:
            self.connection_lost(None)
        #print("Close the socket")
        #self.transport.close()

    def error_received(self, exc):
        print('Error received:', exc)

    def connection_lost(self, exc):
        print("Socket closed, stop the event loop")
        #loop = asyncio.get_event_loop()
        print(exc)
        self.loop.stop()


bind_ip = '127.0.0.1' # All addresses
bind_port = 5124
# Create a socket for receiving
#sock = socket.socket(socket.AF_INET, # Internet
#                    socket.SOCK_DGRAM) # UDP
#sock.bind((bind_ip, bind_port))
#print (sock)
loop = asyncio.get_event_loop()
message = "Hello World!"
connect = loop.create_datagram_endpoint(lambda: EchoClientProtocol(message, loop),  local_addr=(bind_ip,bind_port))
transport, protocol = loop.run_until_complete(connect)
loop.run_forever()
transport.close()
loop.close()
