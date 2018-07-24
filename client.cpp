/*CMSC 621 Advanced Operating Systems
Project 3
Rashmi Prava Patro - AK14498*/
/*Client which will connect to front-end server */
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
int main(int argc, char *argv[])
{

	string tran;
	char input_tran[256];
	char output_tran[256];
	if(argc < 3)
	{
		cout <<"Invalid arguments - Please provide your inputs as <output.o> <port> <localhost>" << endl;
		return 1;

	}

	struct hostent *servername;
	int portno;
	portno = atoi(argv[1]);
	servername = gethostbyname(argv[2]);
	
	if(servername == NULL)
	{
		cout <<"Invalid servername" << endl;
		return 1;
	}

	struct sockaddr_in frontend_server_addr;

	frontend_server_addr.sin_family = AF_INET;
	//copy the server address obtained from gethostbyname to our server_addr structure
	bcopy((char *)servername->h_addr, (char *)&frontend_server_addr.sin_addr.s_addr,servername->h_length);
	frontend_server_addr.sin_port = htons(portno);	


	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		cout <<"Failed to create client socket" << endl;
		return 1;
	}

	if(connect(sockfd, (struct sockaddr *) &frontend_server_addr, sizeof(frontend_server_addr)) < 0)
	{
		cout <<"Failed to connect to front end server" << endl;
		return 1;
	}
	else{
		cout <<"OK" << endl;
	}
	//char input_transaction[256], output_transaction[256];
	while(1){
		//accepts the transactions details from user
		cout << endl;
		cout<<"Please provide your input"<< endl;
		getline(cin,tran);
		
		strcpy(input_tran, tran.c_str());
		//Send transaction to front end server
		int send_tran = send(sockfd, input_tran, sizeof(input_tran),0);
		//cout<<"Sent details: "<<input_tran << endl;

		//Recieve transaction status
		int read_tran = read(sockfd, output_tran, sizeof(output_tran));

		if(strcmp(output_tran,"OK")==0){
			cout << output_tran << endl;
			cout << "Connection closed by foreign host" << endl;	
			break;
		}
		else{

			cout << output_tran<< endl;;
			bzero(output_tran,256);
			bzero(input_tran,256);
		}
	}

	close(sockfd);
	return 0;



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
