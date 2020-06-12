//============================================================================
// Name        : client_server communication
// Author      :
// Version     :
// Copyright   : Completly free
// Description : PID, Kalman filter for Godot Ship control
//============================================================================

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
#include "../eigen3/Eigen/Dense"

#include "./kalman-filter.hpp"
#include "./pid.h"

using namespace std;

#define Gc_CPUs_PORT 4243
#define CPUc_Gs_PORT 4244
#define PLT_PORT     4245
#define MAXLINE 50

//Set the reference position for the final destination
const int x_ref = 7500;
const int y_ref = 0;

//Global variable (Shared Data between threads)
float global_x_meas = 0;
float global_y_meas = 0;
double global_x_u = 0;
double global_y_u = 0;
float CPU_ticks = 0; // Time received as 'OS' ticks
float delta_time_ms = 0; // Represents the time difference in which data is received

//variables for thread synchronization
int data_ready;
pthread_mutex_t mutex;
pthread_cond_t condvar;

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



	//Variables for string split
	char * pch;
	float x_meas, y_meas;

	//---------------------Variables for PID controller--------------------------

	PID x_pid = PID(0.1, 50, -50, 0.02, 0.0001, 0.0001); //PID( double dt, double max, double min, double Kp, double Kd, double Ki )
	PID y_pid = PID(0.1, 50, -50, 0.01, 0, 0.0005); //PID( double dt, double max, double min, double Kp, double Kd, double Ki )
	double x_u = 0 ; //Manipulation variable for x (i.e. 'u' input for x)
	double y_u = 0 ; //Manipulation variable for y (i.e. 'u' input for y)

	// ------------------Variables for kalman filter ----------------------------

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
	B << 1, 0, 0, 1;
	C << 1, 0, 0, 1;

	//Covariance matrices
	Q << 10, 0, 0, 10;
	R << 1.30, 0, 0, 1.30;
	P << 10000, 0, 0, 10000;

	//Initializing Kalman filter
	KalmanFilter kf(A, B, C, Q, R, P);

	//Setting the initial values of X0.
	//This X0 represents the initial states of the system.(i.e.Initial conditions)
	Eigen::VectorXd x0(n); // Vector, Initial condition for the system states
	x0 << 0, 0;
	kf.init(x0);
	cout << "Kalman filter constructor has been initialized" << endl;


	// x_hat_updated
	Eigen::VectorXd xhat_updated(n); //Matrix to store the updated values of the kalman filter

	//-------------------Recieveing data from Remote client-----------------------
	while(1){

		pthread_mutex_lock(&mutex);
		while (data_ready) {
			pthread_cond_wait (&condvar, &mutex);
		}
		recv_bytes = recvfrom(server_sockfd, (char *)buffer,MAXLINE, MSG_WAITALL,(struct sockaddr*) &client_addr, &len);
		buffer[recv_bytes] = '\0';

		data_ready = 1; // Setting var for mutex

		//Splitting the string obtained using strtok
		pch = strtok (buffer,":");
		pch = strtok (NULL, ":");
		x_meas = atof(pch);
		global_x_meas = x_meas;
		pch = strtok (NULL, ":");
		y_meas = atof(pch);
		global_y_meas = y_meas;
		pch = strtok (NULL, ":");
		delta_time_ms = atof(pch);
		pch = strtok (NULL, ":");
		CPU_ticks = atof(pch);

		// -----------------Kalman filter execution ---------------------

		//Setting the measurement matrix
		y << x_meas, y_meas;
		cout << "\nMeas:      " << x_meas << " "<< y_meas << endl;

		//kalman filter predict
		u << global_x_u, global_y_u; // input matrix x_u and y_u is provided as an input to kalman filter
		kf.predict(u);
		cout <<"Xhat_pred: " << kf.state().transpose()<< endl;

		//kalman filter update
		kf.update(y);
		cout <<"Xhat_upd:  " << kf.state().transpose()<< endl;

		// -----------------PID Controller execution ---------------------

		xhat_updated = kf.state();

		x_u = x_pid.calculate(x_ref, xhat_updated(0,0)); //Calculating x_u (input variable)
		global_x_u = x_u; // updating shared global variable
		y_u = y_pid.calculate(y_ref, xhat_updated(1,0)); //Calculating y_u (input variable)
		global_y_u = y_u; // updating shared global variable

		//Clearing the buffer
		bzero(buffer, MAXLINE);
		pthread_cond_signal (&condvar);
		pthread_mutex_unlock(&mutex);
	}


}



//Client
//Client Send the filtered data to a remote server (i.e. In this case GoDot)
void *clientfunc(void*){

	//---------------Variable declaration for client --------------------
	int sockfd;
	char * correction_data, *plotting_data;
	struct sockaddr_in	servaddr1, servaddr2;
	string correction_str, plotting_str;


	// -----------Creating client socket file descriptor--------
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	//----------Sever1 details---------------
	memset(&servaddr1, 0, sizeof(servaddr1));
	servaddr1.sin_family = AF_INET;
	servaddr1.sin_port = htons(CPUc_Gs_PORT);
	servaddr1.sin_addr.s_addr = inet_addr("127.0.0.1");

	//----------Sever2 details---------------
	memset(&servaddr2, 0, sizeof(servaddr2));
	// Filling server information
	servaddr2.sin_family = AF_INET;
	servaddr2.sin_port = htons(PLT_PORT);
	servaddr2.sin_addr.s_addr = inet_addr("127.0.0.1");

	cout << "\nRaspberrypi Client: sending data to server" << endl;

	while(1){
		pthread_mutex_lock(&mutex);
		while (!data_ready) {
			pthread_cond_wait(&condvar, &mutex); //wait for the condition
		}

		// Sending data to Server1
		correction_str = "$KALMAN1:" + to_string(global_x_u) + ":" + to_string(global_y_u);
		correction_data = const_cast<char*>(correction_str.c_str());
		sendto(sockfd, (const char *)correction_data, strlen(correction_data),MSG_CONFIRM, (const struct sockaddr *)&servaddr1, sizeof(servaddr1));

		//Sending data to Sever2
		plotting_str = "plot:" + to_string(global_x_meas) + ":" + to_string(global_y_meas) + ":" + to_string(CPU_ticks);
		plotting_data = const_cast<char*>(plotting_str.c_str());
		sendto(sockfd, (const char *)plotting_data, strlen(plotting_data),MSG_CONFIRM, (const struct sockaddr *)&servaddr2, sizeof(servaddr2));

		data_ready = 0; // Setting var for mutex
		pthread_cond_signal (&condvar);
		pthread_mutex_unlock(&mutex);
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

