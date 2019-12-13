Server: Server.c
	gcc Server.c -o Server

Client: Client.c
	gcc Client.c -o Client

run_Server: Server
	./Server

run_Client: Client
	./Client

clean:
	rm Server Client
	/etc/init.d/networking restart
