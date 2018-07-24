/*CMSC 621 Advanced Operating Systems
Project 3
Rashmi Prava Patro - AK14498*/
/*This is front end Server which will accept the connection from client and as per the transaction details it would update
in the backend servers*/

#include <iostream>
#include <fcntl.h>
#include <thread>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define server1 4000
#define server2 5000
#define server3 6000

using namespace std;

//thread for each message from client
void *processing_thread(void *);

pthread_mutex_t mutex;

int main (int argc, char *argv[]){

	int sock_desc, cli_sock, cli_len;
	int *sock;
	struct sockaddr_in frontend, cli,server_main;
	if(argc < 3)
	{
		cout <<"Invalid arguments - Please provide your inputs as <output.o> <port> <localhost> " << endl;
		return 0;

	}

	
	//create front end TCP socket
	sock_desc = socket(AF_INET, SOCK_STREAM,0);
	if(sock_desc < 0){
		cout <<"Socket creation failed" << endl;
		return 1;

	}
	//cout<<"Socket created - Front end"<< endl;

	//Prepare sockaddr_in structure, get port from command line argument
	frontend.sin_family = AF_INET;
	frontend.sin_addr.s_addr = INADDR_ANY;
	frontend.sin_port = htons(atoi(argv[1]));

	//bind the front end  socket
	if(bind(sock_desc,(struct sockaddr *) &frontend,sizeof(frontend)) <0){
		cout <<"Failed while binding" << endl;
		return 1;
	}
	cout<<"Yay!! Front end server is up!!\n"<< endl;
	
	//listen
	listen(sock_desc,100);

	//Accept Connections
	cli_len = sizeof(struct sockaddr_in);

	//For each client, create a new thread and assigning a thread handle for each client
	while(cli_sock = accept(sock_desc,(struct sockaddr *) &cli,(socklen_t *)&cli_len)){
		cout<<"Connection accepted \n" << endl;
		sock = (int *)malloc(1);	
		*sock = cli_sock;
		pthread_t bankThread;
		if(pthread_create(&bankThread,NULL,processing_thread,(void*)sock) < 0){
			cout<<"No thread created"<< endl;
			return 1;
		}

		if(cli_sock<0){
			cout<<"Cannot accept connections from client"<< endl;
			return 1;
		}
		
	}
	
	return 0;
}

void *processing_thread(void *sockdesc){
	int client_sock = *(int *)sockdesc;
	int frontend_as_client[3];
	int array_for_port[3];	
	array_for_port[0] = htons(server1);
	array_for_port[1] = htons(server2);
	array_for_port[2] = htons(server3);
	//int i=0;
	int total_backend_servers=0;
	int backend_servers_connected[3];
	
	for (int i = 0; i < 3; i++){
		struct hostent *backend_server;
		struct sockaddr_in backend_server_addr;
		backend_server = gethostbyname("localhost");
		backend_server_addr.sin_family = AF_INET;
		bcopy((char *)backend_server->h_addr, (char *)&backend_server_addr.sin_addr.s_addr,backend_server->h_length);
		backend_server_addr.sin_port = ((array_for_port[i]));	

		//array of sockets to connect to multiple servers one by one 

		frontend_as_client[i] = socket(AF_INET, SOCK_STREAM,0);
		if(frontend_as_client[i] < 0){
			cout <<"Error from thread - Socket creation failed for backend server" << endl;
			return 0;

		}
	
		//Connect to servers one by one 
		if(connect(frontend_as_client[i], (struct sockaddr *) &backend_server_addr, sizeof(backend_server_addr)) < 0){
			backend_servers_connected[i] = 0;
		}
		else{
			backend_servers_connected[i] = 1;
			total_backend_servers++;
		}
	}
	while(true){
		
		char message[256]; 
		char transaction_status[256];
		
		//Read from client 
		int r1 = read(client_sock,message,sizeof(message));
		//cout<<"Transaction received: " << message << endl;

		//check and change the transaction details to uppercase
		for(int i = 0; i < sizeof(message); i++){
			message[i] = toupper(message[i]);
		}
		cout <<endl;
		cout<<"Transaction to be processed: " << message << endl;
		
		
		if(strcmp(message,"QUIT")==0){
			int s = send(client_sock,"OK",sizeof(2),0);	
			
			close(client_sock);
			break;
		}
		
		//ask the backend servers to provide their status
		for(int i = 0; i < 3; i++){
			if(backend_servers_connected[i]==1){
					int status= send(frontend_as_client[i],"VOTE",256,0);
				}
		}

		//Recieve status reply from the backend servers
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		backend_servers_connected[0] = 0;
		backend_servers_connected[1] = 0;
		backend_servers_connected[2] = 0;
		char buffer1[256];
		int count = 0;

		for(int i = 0; i<3;i++){
			if(setsockopt(frontend_as_client[i],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
			{
				cout << "Timeout error" << endl;
				return 0;
			
			}
			while(recv(frontend_as_client[i],buffer1,256,0)>0)
			{
				count++;
				backend_servers_connected[i] = 1;
			}
		}
		//cout << "Count: "<< count << endl;
		if(count==0){
			cout << "No active backend servers" << endl;
		}
		cout<<"Total active backend servers: "<< total_backend_servers;
		
		//sleep(3);

		if(count==0){
			//cout << "No active backend servers" << endl;
			sprintf(transaction_status,"ERROR: Please try after sometime");
			int s = send(client_sock,transaction_status,sizeof(transaction_status),0);
		}
		else{
			if(count == total_backend_servers){
				char buffer2[256];
				for ( int i = 0; i < 3; i++)
				{
					if(recv(frontend_as_client[i],buffer2,256,0)!=0)
					{
						if(backend_servers_connected[i]==1){
							int s3= send(frontend_as_client[i],"COMMIT",256,0);

							total_backend_servers++;
						}
					}
					else{
						backend_servers_connected[i] = 0;
					}

				}

				char buffer3[256];
				total_backend_servers = 0;
				for(int i = 0; i < 3; i++){
						
						
						if(recv(frontend_as_client[i],buffer3,256,0)!=0)
						{
							if(backend_servers_connected[i]==1){
								int s4 = send(frontend_as_client[i],message,sizeof(message),0);
								total_backend_servers++;
							}
						}
						else{
							backend_servers_connected[i] = 0;
						}
					}

				//Receive the updated transaction from the backend servers
				int y = 0;
				total_backend_servers = 0;
				//cout <<endl<<"Sleep" << endl;
				//sleep(5);
				
				char buffer4[256];
				bzero(transaction_status,256);
				for(int i = 0;i < 3;i++){
					

					if(recv(frontend_as_client[i],transaction_status,256,0)!=0)
					{
						if(backend_servers_connected[i]==1){
							
							total_backend_servers++;
						}
					}
					else{
						backend_servers_connected[i] = 0;
					}
					

				}
				cout << endl;
				//cout<<"Received update: " << transaction_status<< endl;
				//cout << "Backend servers: " << backend_servers_connected;
				int s3 = send(client_sock,transaction_status,sizeof(transaction_status),0);	


			}
			else{
				int sock_closed;
				
				total_backend_servers = 0;
				for(int i=0; i<3; i++){
					if(setsockopt(frontend_as_client[i],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
					{
						cout << "Timeout error" << endl;
						return 0;

					}
					if(backend_servers_connected[i]==1){
						send(frontend_as_client[i],"ABORT",256,0);		
						total_backend_servers++;	
					}
				}
				for(int i=0; i<3; i++){
					if(setsockopt(frontend_as_client[i],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
					{
						cout << "Timeout error" << endl;
						return 0;
					}
					if(backend_servers_connected[i]==1){
						int rd = read(frontend_as_client[i],transaction_status,sizeof(transaction_status));
					}
				}

				int s3 = send(client_sock,transaction_status,sizeof(transaction_status),0);
			}
		}	

		bzero(message,256);
		bzero(buffer1,256);
		

	}	
	
	close(client_sock);	
							
}

/*References:
1. https://stackoverflow.com/questions/4233598/about-setsockopt-and-getsockopt-function
2. ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
3. https://stackoverflow.com/questions/18050065/specifying-port-number-on-client-side
4. https://social.msdn.microsoft.com/Forumns/vstudio/en-US/da286445-72b3-4b0c-87f7-4703121f9974/how-to-put-int-values-to-char-array?forum=vcgeneral
5. https://msdn.microsoft.com/en-us/library/windows/desktop/ms738543(v=vs.85).aspx
6. https://www.cprogramming.com/tutorial/string.html
7. http://programmingknowledgeblog.blogspot.com/2013/05/c-program-to-display-current-date-and.html
8. http://www.cplusplus.com/reference/sstream/istringstream/istringstream/
9. https://solarianprogrammer.com/2012/02/27/cpp-11-thread-tutorial-part-2/
10.http://www.bogotobogo.com/cplusplus/sockets_server_client.php
11.https://stackoverflow.com/questions/1092631/get-current-time-in-seconds-since-the-epoch-on-linux-bash
12.http://www.cplusplus.com/reference/ctime/time/
13.https://solarianprogrammer.com/2011/12/16/cpp-11-thread-tutorial/
14.https://www.justsoftwaresolutions.co.uk/threading/managing_threads_with_a_vector.html
*/
