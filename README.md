# OperatingSystems-TwoPhaseCommitControl
Design and implementation of a simple, distributed banking service that permits multiple concurrent operations and exhibits simple fault tolerance.

There are 3 .cpp programs as below:
•	client.cpp: 
o	Compile: g++ -std=c++11 –o client client.cpp
o	Run the executable: ./client <portnumber> localhost
o	After running this executable, it will connect to the front end server and ask the user to provide the required inputs to carry out the required transactions (CREATE, QUERY, UPDATE or QUIT).

•	frontServer.cpp:
o	Compile: g++ -std=c++11 –o -pthread frontendserver frontServer.cpp
o	Run the executable: ./frontendserver <portnumber> localhost
o	After running this executable, it will be able to receive all the transaction details from the client and pass those transaction details to the backend servers.

•	backendServer.cpp:
o	Compile: g++ -std=c++11 –o –pthread backendserver backendServer.cpp
o	Run the executable: ./backendserver <portnumber>
o	After running this executable, it will get connected to the frontend server and accept all the transaction details, perform required actions and send back the updated status to the front end server.
First of all, backend servers are started in different terminals. All the 3 backend servers will be connecting to the port numbers which are defined in the frontend server .cpp file. 
o	./backendserver 4000
o	./backendserver 5000
o	./backendserver 6000
Above servers are waiting for the connection from the frontend server. Now, in different terminal frontend server can be run as below:
o	./frontendserver <portnumber> localhost
This will be waiting for the connections from the client.
Finally, in another terminal client can be started as below:
o	./client <portnumber> localhost
Once, this is started it will ask the user to provide the transaction details. Each transaction in turn will be processed and the final output will be displayed at the client side.
Makefile contains all the commands that are required to compile and run these programs.

