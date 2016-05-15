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

/* Creates the server socket, binds it and listens to it */
int prepare_server_socket(short port) {
	struct sockaddr_in serverSettings;
	memset(&serverSettings, '0', sizeof(serverSettings));
	serverSettings.sin_family = AF_INET;
	serverSettings.sin_addr.s_addr = htonl(INADDR_ANY);
	serverSettings.sin_port = htons(port);
	
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
		kill("failed to create server socket");
	
	int option = 1;
	setsockopt(server_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *) & option, sizeof(option));
	
	int bind_result = bind(server_socket, (struct sockaddr *) & serverSettings, sizeof(serverSettings));
	if (bind_result < 0)
		kill("failed to bind server socket");
		
	int listen_result = listen(server_socket, 5);
	if (listen_result < 0)
		kill("failed to listen on server socket");
		
	return server_socket;
}

int accept_connection(int server_socket) {
	struct sockaddr_in clientSettings;
	unsigned int clientlen = sizeof(clientSettings);
	int client_socket = accept(server_socket, (struct sockaddr *) & clientSettings, & clientlen);
	if (client_socket < 0)
		kill("failed to accept connection");
		
	return client_socket;
}

int receive_message(int client_socket) {
	memset(&buffer, '\0', BUFFERSIZE);
	int received = recv(client_socket, buffer, BUFFERSIZE, 0);
	if (received < 0)
		kill("failed to receive message from client");

	return received;
}

bool read_and_check_message() {
	printf("Enter your message: ");
	fgets(buffer, BUFFERSIZE - 1, stdin);
	if ( strlen(buffer) >= 2 && buffer[0] == ':' && buffer[1] == 'q' )
		return true;
	return false;
}

void send_message(int client_socket) {
	int msg_length = strlen(buffer);
	int sent_bytes = send(client_socket, buffer, msg_length, 0);
	if (sent_bytes != msg_length)
		kill("mismatch in sent bytes");
}


int main(int argc, char * argv[]) {
	
	int server_socket = prepare_server_socket(4000);
	int client_socket = accept_connection(server_socket);
	
	while (1) {
		int received = receive_message(client_socket);
		if (received == 0)
			break;

		printf("\tMessage from client: %s", buffer);
		
		if (read_and_check_message())
			break;
		
		send_message(client_socket);
	}//while
	
	close(server_socket);
	
	return 0;
}//main
