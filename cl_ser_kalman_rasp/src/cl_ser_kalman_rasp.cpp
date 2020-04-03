//============================================================================
// Name        : server1.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <eigen3/Eigen/Dense>

#include "./kalman-filter.hpp"

using namespace std;

#define Gc_CPUs_PORT 4243
#define CPUc_Gs_PORT 4244
#define MAXLINE 50

//Server
//This Server collects the data from a client and also perform Kalman filter

void *serverfunc(void*){

	//---------------Variable declaration for socket Server-----------------
	int server_sockfd, recv_bytes;
	char buffer[MAXLINE];
	struct sockaddr_in serv_addr,client_addr;
	socklen_t len = sizeof(client_addr);  //len is value/result

	//Creating the Server socket
	if((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0))==0){

		cout << "Socket Creation Error" << endl;
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_family = AF_INET;;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(Gc_CPUs_PORT);

	//Binding the socket to the ipaddress and the port number
	if((bind(server_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))<0){

		cout <<"binding failed" << endl;
	}
	//Server Created
	cout << "Raspberrypi Server: waiting for client to connect " << endl;


	// ------------------Variables for kalman filter ----------------------------
	char * pch;
	float x_meas, y_meas;

	int n = 2; // Number of states
	int m = 2; // Number of measurements
	int c = 2; // Number of control inputs

	Eigen::MatrixXd A(n, n); // System dynamics matrix
	Eigen::MatrixXd B(n, c); // Input control matrix
	Eigen::MatrixXd C(m, n); // Output matrix
	Eigen::MatrixXd Q(n, n); // Process noise covariance
	Eigen::MatrixXd R(m, m); // Measurement noise covariance
	Eigen::MatrixXd P(n, n); // Estimate error covariance

	Eigen::VectorXd u(c); //Input matrix
	Eigen::VectorXd y(m); //Measurement matrix

	//System Matrices
	A << 1, 0, 0, 1;
	B << 10, 0, 0, 0;
	C << 1, 0, 0, 1;

	//Covariance matrices
	Q << 10, 0, 0, 10;
	R << 0.8307, 0, 0, 0.8307;
	P << 10000, 0, 0, 10000;

//	cout << "System Matrices : " << endl;
//	std::cout << "A: \n" << A << std::endl;
//	std::cout << "B: \n" << B << std::endl;
//	std::cout << "C: \n" << C << std::endl;
//	cout << "Covariance Matrices : " << endl;
//	std::cout << "Q: \n" << Q << std::endl;
//	std::cout << "R: \n" << R << std::endl;
//	std::cout << "P: \n" << P << std::endl;

	//Initializing Kalman filter
	KalmanFilter kf(A, B, C, Q, R, P);

	//Setting the initial values of X0.
	//This X0 represents the initial states of the system.(i.e.Initial conditions)
	Eigen::VectorXd X0(n); // Vector, Initial condition for the system states
	X0 << 10, 0;
	kf.init(X0);
	cout << "Kalman filter constructor has been initialized" << endl;

	//Recieveing data from Remote client
	while(1){

		//Recieving Data from client in bytes
		recv_bytes= recvfrom(server_sockfd, (char *)buffer,
					MAXLINE, MSG_WAITALL,
					(struct sockaddr*) &client_addr, &len);

		buffer[recv_bytes] = '\0';

		//Splitting the string obtained using strtok
		pch = strtok (buffer,":");
		pch = strtok (NULL, ":");
		x_meas = atof(pch);
		pch = strtok (NULL, ":");
		y_meas = atof(pch);

		//Setting the measurement matrix
		y << x_meas, y_meas;

		cout << "\nMeas:      " << x_meas << " "<< y_meas << endl;

		//kalman filter predict
		kf.predict(u);
		cout <<"Xhat_pred: " << kf.state().transpose()<< endl;

		//kalman filter update
		kf.update(y);
		cout <<"Xhat_upd:  " << kf.state().transpose()<< endl;

		//Clearing the buffer
		bzero(buffer, MAXLINE);
	}


}



//Client
//Client Send the filtered data to a remote server (i.e. In this case GoDot)
void *clientfunc(void*){

	//---------------Variable declaration for client --------------------
	int sockfd;
	char *correction = "$KALMAN1:0.0:-0.0";
	struct sockaddr_in	servaddr;


	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CPUc_Gs_PORT);
	servaddr.sin_addr.s_addr = inet_addr("169.254.102.146");;



	cout << "\nRaspberrypi Client: sending data to server" << endl;

	while(1){
		sleep(0.5);
		sendto(sockfd, (const char *)correction,
				strlen(correction),MSG_CONFIRM,
				(const struct sockaddr *)&servaddr,
				sizeof(servaddr));
	}

}



int main(int argc, char *argv[]){

	//Initializing threads
	pthread_t server_thread,client_thread;

	pthread_create( &server_thread, NULL, serverfunc, NULL);
	pthread_create( &client_thread, NULL, clientfunc, NULL);
	pthread_setname_np(server_thread,"SERVER_Thread");
	pthread_setname_np(client_thread,"CLIENT_Thread");

	//Waiting for threads to join
	pthread_join( server_thread, NULL);
	pthread_join( client_thread, NULL);


return 0;
}

