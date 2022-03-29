# Web streaming example
# Source code from the official PiCamera package
# http://picamera.readthedocs.io/en/latest/recipes2.html#web-streaming

import logging
import socketserver
from http import server
import socket
import struct

PAGE="""\
<html>
<head>
<title>Raspberry Pi - Surveillance Camera</title>
</head>
<body>
<center><h1>Raspberry Pi - Surveillance Camera</h1></center>
<center><img src="stream.mjpg" width="640" height="480"></center>
</body>
</html>
"""

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.bind(("", 8100))
serverSocket.listen()


print("waiting for connection")
(clientConnected, clientAddress) = serverSocket.accept()
print("Accepted a connection request from %s:%s"%(clientAddress[0], clientAddress[1]))

def receiveMessage(connection):
    result = None
    # Message: [4 byte message length][variable message]
    lengthOfStruct = internalReceiveMessage(connection, 4)
    
    if lengthOfStruct:
        messageLength = struct.unpack('>I', lengthOfStruct)[0]
        result = internalReceiveMessage(connection, messageLength)

    return result

def internalReceiveMessage(connection, length):
    # Internal function to receive a specific length number of bytes.
    # Returns the received data or returns None if EOF is encountered.
    result = bytearray()

    while len(result) < length:
        packet = connection.recv(length - len(result))

        if not packet:
            result = None
            break

        result.extend(packet)

    return result

class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            content = PAGE.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    frame = receiveMessage(clientConnected)
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        else:
            self.send_error(404)
            self.end_headers()

class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

try:
    address = ('', 8000)
    server = StreamingServer(address, StreamingHandler)
    server.serve_forever()
    print("starting")
finally:
    print("shutdown")