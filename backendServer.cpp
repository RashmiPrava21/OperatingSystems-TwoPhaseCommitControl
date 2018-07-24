/*CMSC 621 Advanced Operating Systems
Project 3
Rashmi Prava Patro - AK14498*/
/*This is a backend server which will accept the connections from front end server and perform required actions*/

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

using namespace std;

//function to create a new thread for each connection
void *transaction_thread(void *);

//function to carry out "CREATE" transactions
int create_transaction(float);

//function to carry out "QUERY" transactions
float query_transaction(int);

//function to carry out "UPDATE" transactions
float update_transaction(int, float);

int number = 100;

struct account_details{
	int account_number;
	int amount; 
};
struct account_details records[1000];

pthread_mutex_t mutex;
int main (int argc, char *argv[]){
	//int number = 100;
	for (int i = 0; i < 1000; i++)
	{
		records[i].account_number = 0;
		records[i].amount = 0;
		//number++;
	}
	//int sock_desc, client_sock,clilen,read_size;
	int sock_desc, cli_sock, cli_len;
	int *sock1;
	//struct sockaddr_in server, client;
	struct sockaddr_in backend_server, cli;
	
	//create socket for backend server
	sock_desc = socket(AF_INET, SOCK_STREAM,0);
	if(sock_desc == -1){
		cout <<"Invalid arguments - Please provide your inputs as <output.o> <port>" << endl;
		return 1;

	}
	cout <<"Socket created successfully for backend server" << endl;

	//create sockaddr_in structure
	backend_server.sin_family = AF_INET;
	backend_server.sin_addr.s_addr = INADDR_ANY;
	backend_server.sin_port = htons(atoi(argv[1]));

	//bind the socket
	if(bind(sock_desc,(struct sockaddr *) &backend_server,sizeof(backend_server)) <0){
		cout <<"Error while binding backend server" << endl;
		return 1;
	}
	//cout <<"Error while binding backend server" << endl;
	
	//listen to accept connections from front end server
	listen(sock_desc,50);
	cout <<"Waiting for incoming connections: " << endl;

	//Accept Connections

	cli_len = sizeof(struct sockaddr_in);
	while(cli_sock = accept(sock_desc,(struct sockaddr *) &cli,(socklen_t *)&cli_len)){
		//printf("Connection accepted \n");
		sock1 = (int *)malloc(1);	
		*sock1 = cli_sock;
		pthread_t backendThread;
		if(pthread_create(&backendThread,NULL,transaction_thread,(void*)sock1) < 0)	{
			cout <<"Failed to create thread in backend server" << endl;
			return 1;
		}

		if(cli_sock<0){
			cout <<"Cannot accept connections to backend server" << endl;
			return 1;
		}
		
	}
	return 0;
}

void *transaction_thread(void *sockdesc)
{
	int client_sock = *(int *)sockdesc;
	while(true){

		char message1[256];	
		bzero(message1,256);
		int receive1 = read(client_sock,message1,sizeof(message1));
		//cout << "First message from front end server: " << message1 << endl;
		cout <<endl<<"*****" <<message1 <<"*****" <<endl;

		if(receive1 != 0){
		int s1 = send(client_sock,"ACTIVE",256,0);
		}
	
		char commit_status[256];
		bzero(commit_status,256);
		int receive2 = read(client_sock,commit_status,sizeof(commit_status));
		
		cout << endl<<"*****" << commit_status << "*****" << endl;
		
		char message2[256];	
		bzero(message2,256);
		
		char transaction_after_updation[256];
		bzero(transaction_after_updation,256);

		//Read transaction
		//If message is not equal to global commit
		if((strcmp(commit_status,"ABORT"))!=0){
			int receive3 = read(client_sock,message2,sizeof(message2));
			//cout << endl;
			cout<<endl <<"Transaction details received: "<< message2 << endl;
		}
		else{
		//If message is equal to global abort	
			
		 
			sprintf(transaction_after_updation,"******There seems to be some problem while connecting to backend servers - Please try after sometime******");
			int s1 = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
			cout << endl;
			bzero(message1,256);
			bzero(commit_status,256);
			bzero(transaction_after_updation,256);
			bzero(message2,256);
			

		}
		if(((strcmp(commit_status,"COMMIT")) == 0) && ((strcmp(commit_status,"ABORT"))!=0)){
			char *split_transaction;
			float balance;
			split_transaction = strtok (message2, " ");
			//balance = strtok(NULL, " ");
			if(split_transaction!= NULL)
			{
				
				//performs required operations when a transaction contains "CREATE"
				if(strcmp(split_transaction,"CREATE")==0)
				{
				
					pthread_mutex_lock(&mutex);
						
					balance = atof(strtok(NULL, " "));
					//cout <<"Value in current transaction: " << balance << endl;
					int account_created = create_transaction(balance);
					
					sprintf(transaction_after_updation,"OK %d",account_created);
					int s1 = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
					pthread_mutex_unlock(&mutex);
					//cout << "Sent: "<< transaction_after_updation<< endl;
					cout <<endl<<"*****Transaction completed successfully*****" << endl;
					
					
				}
				//performs required operations when a transaction contains "QUERY"
				else if(strcmp(split_transaction,"QUERY")==0){
					
					pthread_mutex_lock(&mutex);					
					
					int account_tobechecked = atoi(strtok(NULL, " "));//101
					float amount_received = query_transaction(account_tobechecked);//2 or 101
					//cout << "Amount received: " << amount_received << endl;
										
					if(amount_received == 1000){
						//cout << "Testing in query........................." << endl;
						sprintf(transaction_after_updation,"ERR: Account %d does not exist",account_tobechecked);
						
						int s1 = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
						cout <<endl<<"*****Transaction unsuccessful*****" << endl;
					}
					else{
						sprintf(transaction_after_updation,"OK %.2f",amount_received);
						
						int s1 = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
						cout <<endl<<"*****Transaction completed successfully*****" << endl;
					}
					pthread_mutex_unlock(&mutex);
				}
							

				//performs required operations when a transaction contains "UPDATE"
				else if(strcmp(split_transaction,"UPDATE")==0){
					
					pthread_mutex_lock(&mutex);
					
					int accountNumber = atoi(strtok (NULL, " "));
					float amountReceived = atof(strtok (NULL, " "));
					
					
					float amountUpdated = update_transaction(accountNumber, amountReceived);//2 or 101
					if(amountUpdated == 1000){
						//cout << "Testing in update........................." << endl;
						sprintf(transaction_after_updation,"Err Account %d does not exist",accountNumber);
						
						int s = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
						cout <<endl<<"*****Transaction unsuccessful because could not find the account number*****" << endl;
					}

					else{
						
						sprintf(transaction_after_updation,"OK %.2f",amountUpdated);	
						int s = send(client_sock,transaction_after_updation,sizeof(transaction_after_updation),0);
						cout <<endl<<"**********Transaction completed successfully**********" << endl;

					}
					pthread_mutex_unlock(&mutex);
				}
				
				else{
					sprintf(transaction_after_updation,"Error: This transaction is not valid");
					int s1= send(client_sock, transaction_after_updation, sizeof(transaction_after_updation),0);
					cout <<"**********Transaction is not valid**********" << endl;

				}
			}

		}

		bzero(transaction_after_updation,256);
		
	}	
	
	close(client_sock);	
}

int create_transaction(float bal){
	//int amount_received = bal;
	int account_tobesend;
	int i = 0;
	for (i = 0; i < 1000; i++){
		if (records[i].account_number == 0){
			records[i].account_number = number;
			number = number + 1;
			records[i].amount = bal;
			//account_tobesend = records[i].account_number;
			break;
			
		}
		else{
			continue;
		}
	}
	return records[i].account_number;	
}

float query_transaction(int acc){//acc = 101
	//cout << "Received input: " << acc;
	//int value = 2;
	int i = 0;
	for (i = 0; i < 1000; i++){
		if(records[i].account_number == acc){
			cout << "Return value: " << records[i].amount;
			return records[i].amount;
		}
		else{
			continue;
		}
	}
	//cout << "Return value: " << (float)i;
	return i;
	
}

float update_transaction(int acct, float amt)
{
	int i = 0;
	for (i = 0; i < 1000; i++){
		if(records[i].account_number == acct){
			//cout << "Return value: " << records[i].amount;
			//return records[i].amount;
			records[i].amount = amt;
			return records[i].amount;
		}
		else{
			continue;
		}
	}
	//cout << "Return value: " << (float)i;
	return i;
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
