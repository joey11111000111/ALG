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

int create_and_configure_socket() {
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
		kill("failed to create client socket");
	int option = 1;
	setsockopt(client_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char * ) & option, sizeof(option));
	
	struct sockaddr_in clientSettings;
	memset(&clientSettings, '0', sizeof(clientSettings));
	clientSettings.sin_family = AF_INET;
	clientSettings.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientSettings.sin_port = htons(4040);
	
	int bind_result = bind(client_socket, (struct sockaddr *) & clientSettings, sizeof(clientSettings));
	if (bind_result < 0)
		kill("failed to bind client socket");
	
	return client_socket;
}

void connect_to_server(int client_socket) {
	struct sockaddr_in serverSettings;
	memset(&serverSettings, '0', sizeof(serverSettings));
	serverSettings.sin_family = AF_INET;
	serverSettings.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverSettings.sin_port = htons(4000);
	
	int connection_result = connect(client_socket, (struct sockaddr *) & serverSettings, sizeof(serverSettings));
	if (connection_result < 0)
		kill("failed to connect to server");
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

bool receive_message(int client_socket) {
	memset(&buffer, '\0', BUFFERSIZE);
	int received = recv(client_socket, buffer, BUFFERSIZE, 0);
	if (received < 0)
		kill("failed to receive message from client");
	if (received == 0)
		return true;
	printf("\tMessage from server: %s", buffer);
	return false;
}


int main(int argc, char * argv[]) {
	
	int client_socket = create_and_configure_socket();
	connect_to_server(client_socket);
	
	// send message
	while (1) {
		if (read_and_check_message())
			break;
			
		send_message(client_socket);
		if (receive_message(client_socket))
			break;
	}//while
	
	
	close(client_socket);

	return 0;
}//main


