#A server application that receives messages from multi clients. 

from socket import *
import threading

class ThreadedServer():

    def listenToClient(self, client, addr):
        
            while True:

                    message= client.recv(1024)
                    if message=="exit":
                        print addr , " is closed"
                        client.close()
                        exit(0)
                    else:
                        print addr , " says: ", message


    def __init__(self,serverPort):

        try:
            serverSocket=socket(AF_INET,SOCK_STREAM)

        except:
    
            print "Socket cannot be created!!!"
            exit(1)
            
        print "Socket is created..."

        try:
            serverSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        except:
    
            print "Socket cannot be used!!!"
            exit(1)

        print "Socket is being used..."

        try:
            serverSocket.bind(('',serverPort))
        except:
        
            print "Binding cannot de done!!!"
            exit(1)

        print "Binding is done..."

        try:
            serverSocket.listen(45)
        except:
    
            print "Server cannot listen!!!"
            exit(1)

        print "The server is ready to receive"


        while True:

            connectionSocket,addr=serverSocket.accept()
            
            threading.Thread(target = self.listenToClient,args = (connectionSocket,addr)).start()
            

if __name__=="__main__":
    serverPort=12000
    ThreadedServer(serverPort)
	
