#A server application that receives a message from client, creates uppercase version and sends back to client.

from socket import *

serverPort=12000
serverSocket=socket(AF_INET,SOCK_STREAM)

serverSocket.bind(('',serverPort))

serverSocket.listen(1)
print "The server is ready to receive"

while True:
            connectionSocket,addr=serverSocket.accept()
            print "accepted"
            message= connectionSocket.recv(1024)
            
            modifiedMessage=message.upper()
            
            connectionSocket.send(modifiedMessage)

            connectionSocket.close()
	
