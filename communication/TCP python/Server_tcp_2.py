#A server application that receives a message from client and print it on screen
from socket import *

class Server():


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


        connectionSocket,addr=serverSocket.accept() 
        while True:


                message= connectionSocket.recv(1024)
                if message=="exit":
                        print addr , " is closed"
                        connectionSocket.close()
                        exit(0)
                else:
                    print addr , " says: ", message
            

if __name__=="__main__":
    serverPort=12000
    Server(serverPort)
	
