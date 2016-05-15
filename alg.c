#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFERSIZE 1024
void kill(const char * message) {perror(message); exit(1);}
char buffer[BUFFERSIZE];


int prepare_own_server_socket() {
	struct sockaddr_in ownServerSettings;
	memset(&ownServerSettings, '0', sizeof(ownServerSettings));
	ownServerSettings.sin_family = AF_INET;
	ownServerSettings.sin_addr.s_addr = htonl(INADDR_ANY);
	ownServerSettings.sin_port = htons(3000);
	
	int own_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (own_server_socket < 0)
		kill("failed to create own server socket");

	int option = 1;
	setsockopt(own_server_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *) & option, sizeof(option));
	
	int bind_result = bind(own_server_socket, (struct sockaddr *) & ownServerSettings, sizeof(ownServerSettings));
	if (bind_result < 0)
		kill("failed to bind own server socket");
	
	int listen_result = listen(own_server_socket, 5);
	if (listen_result < 0)
		kill("failed to listen to own server socket");
		
	return own_server_socket;
}

int prepare_own_client_socket() {
	struct sockaddr_in ownClientSettings;
	memset(&ownClientSettings, '0', sizeof(ownClientSettings));
	ownClientSettings.sin_family = AF_INET;
	ownClientSettings.sin_addr.s_addr = htonl(INADDR_ANY);
	ownClientSettings.sin_port = htons(3030);
	
	int own_client_socket = socket(AF_INET, SOCK_STREAM, 0);

	int option = 1;
	setsockopt(own_client_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *) & option, sizeof(option));
	
	int bind_result = bind(own_client_socket, (struct sockaddr *) & ownClientSettings, sizeof(ownClientSettings));
	if (bind_result < 0)
		kill("failed to bind own client socket");
	
	return own_client_socket;
}

void connect_to_real_server(int own_client_socket) {
	struct sockaddr_in realServerSettings;
	memset(&realServerSettings, '0', sizeof(realServerSettings));
	realServerSettings.sin_family = AF_INET;
	realServerSettings.sin_addr.s_addr = htonl(INADDR_ANY);
	realServerSettings.sin_port = htons(4000);
	
	int connection_result = connect(own_client_socket, (struct sockaddr *) & realServerSettings, sizeof(realServerSettings));
	if (connection_result < 0)
		kill("failed to connect to real server");
}

int accept_real_client(int own_server_socket) {
	struct sockaddr_in realClientSettings;
	memset(&realClientSettings, '0', sizeof(realClientSettings));
	unsigned int clientlen = sizeof(realClientSettings);
	int real_client_socket = accept(own_server_socket, (struct sockaddr *) & realClientSettings, & clientlen);
	if (real_client_socket < 0)
		kill("failed to accept real client connection");
	
	return real_client_socket;
}


int receive_message(int sockfd) {
	memset(&buffer, '\0', BUFFERSIZE);
	int received = recv(sockfd, buffer, BUFFERSIZE, 0);
	if (received < 0)
		kill("failed to receive message from client");

	return received;
}


void send_message_to(int sockfd) {
	int msg_length = strlen(buffer);
	int sent_bytes = send(sockfd, buffer, msg_length, 0);
	if (sent_bytes != msg_length)
		kill("mismatch in sent bytes");
}



int main() {
	
	int own_server_socket = prepare_own_server_socket();
	
 
	while (1) {
		int real_client_socket = accept_real_client(own_server_socket);
		printf("------- real client connected --------\n");
		
		int own_client_socket = prepare_own_client_socket();
		connect_to_real_server(own_client_socket);
		printf("------ connected to real server ------\n");

		while (1) {
			int received = receive_message(real_client_socket);
			if (received == 0)
				break;
			printf("client: %s", buffer);
			send_message_to(own_client_socket);
			
			received = receive_message(own_client_socket);
			if (received == 0)
				break;
			printf("server: %s", buffer);
			send_message_to(real_client_socket);
		}//inner while

		close(real_client_socket);
		close(own_client_socket);
		printf("--------- connections closed ---------\n");
	}//while
	
}//main

