import io
import picamera
import socket
from threading import Condition
import argparse
import time
import struct
import signal

parser = argparse.ArgumentParser(description="stream video.")
parser.add_argument("ip", type=str)
parser.add_argument("port", type=int)
parser.add_argument("res", type=str, default='640x480')
parser.add_argument("fr", type=int, default=24)
args = parser.parse_args()

connected = False

address = (args.ip, args.port)
clientSocket = None
res = args.res
fr = args.fr

while not connected:
    try:
        clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        clientSocket.connect(address)
        connected = True
        break
    except socket.error:
        print("Connection Failed, Retrying..")
        time.sleep(1)

print("connected")

try:
    def sendMessage(connection, message):
        # Message: [4 byte message length][variable message]
        lmessage = struct.pack('>I', len(message)) + message
        connection.sendall(lmessage)

    class StreamingOutput(object):
        def __init__(self):
            self.buffer = io.BytesIO()

        def write(self, buf):
            if buf.startswith(b'\xff\xd8'):
                self.buffer.truncate()
                sendMessage(clientSocket, self.buffer.getvalue())
                self.buffer.seek(0)
            return self.buffer.write(buf)

    with picamera.PiCamera(resolution=res, framerate=fr) as camera:
        output = StreamingOutput()
        camera.start_recording(output, format='mjpeg')
        print("camera")

        try:
            #camera.wait_recording(6)
            signal.pause()
            print("done")
        finally:
            camera.stop_recording()
            clientSocket.close()

except socket.error:
    connected = False