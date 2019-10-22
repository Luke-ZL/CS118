#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netdb.h>
#include <signal.h>
#include <strings.h>
#include <dirent.h>

#define MYPORT 5000/* Avoid reserved ports */
#define BACKLOG 10 /* pending connections queue size */

char* fileType(char* file) {
	char *ext = strrchr(file, '.');
	if (ext == NULL) {
		return "Content-Type: text/plain\r\n";
	} else {
		if (!strcasecmp(ext, ".html"))
			return "Content-Type: text/html\r\n";
		else if (!strcasecmp(ext, ".htm"))
			return "Content-Type: text/htm\r\n";
		else if (!strcasecmp(ext, ".txt"))
			return "Content-Type: text/plain\r\n";
		else if (!strcasecmp(ext, ".jpg"))
			return "Content-Type: image/jpg\r\n";
		else if (!strcasecmp(ext, ".jpeg"))
			return "Content-Type: image/jpeg\r\n";
		else if (!strcasecmp(ext, ".png"))
			return "Content-Type: image/png\r\n";
		else if (!strcasecmp(ext, ".gif"))
			return "Content-Type: image/gif\r\n";
		else
			return "Content-Type: text/plain\r\n";
	}
}

void processRequest(char* fileName, int new_sockfd) {
	int isValid = 1;
	char* file_name = fileName;

	struct dirent *entry;
	DIR *dir = opendir(".");
	if (dir == NULL) {
		printf("Could not open current directory");
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL) {
		if (!strcasecmp(entry->d_name, fileName)) {
			file_name = entry->d_name;
			break;
		}
	}

	FILE *fileFD = fopen(file_name, "r");
	if (fileFD == NULL) {
		isValid = 0;
		fprintf(stderr, "Opening file failed.\n");
	}
	char* extension = fileType(file_name);

	//if (extension == "Error")
	//	isValid = 0;

	char* found = "HTTP/1.1 200 OK\r\n\0";
	char* E404 = "HTTP/1.1 404 Not Found\r\n\0";
	char* E404HTML = "<html> <body> 404 Not Found. </body> </html>";

	char *content;
	int contentSize;
	char length[2048];
	//starts here
	if (isValid) {
		fseek(fileFD, 0, SEEK_END);
		int filesize = ftell(fileFD);
		fseek(fileFD, 0, SEEK_SET);
		content = malloc(sizeof(char) * filesize + 1);

		//append 0 at end
		contentSize = fread(content, 1, filesize, fileFD);
		content[contentSize] = '\0';
       		write(new_sockfd, found, strlen(found));
		sprintf(length,"Content-Length: %d\r\n", contentSize );
		write(new_sockfd, length, strlen(length));
		//fprintf(stdout, "Content-Length: %d\r\n", contentSize);
		write(new_sockfd, extension, strlen(extension));
		write(new_sockfd, "\r\n\0", strlen("\r\n\0"));
		write(new_sockfd, content, contentSize);
	} else {
		write(new_sockfd, E404, strlen(E404));
		sprintf(length,"Content-Length: %d\r\n", strlen(E404HTML));
		write(new_sockfd, length, strlen(length));
	     	write(new_sockfd, "Content-Type: text/html\r\n",
				strlen("Content-Type: text/html\r\n"));
		write(new_sockfd, "\r\n\0", strlen("\r\n\0"));
		write(new_sockfd, E404HTML, strlen(E404HTML));
	}

	fclose(fileFD);
	closedir(dir);
}

char* get_filename(char *buffer) {

	char* token;
	//process the request message
	token = strtok(buffer, " ");
	token = strtok(NULL, " ");
	//splited into token
	char* frag;
	while ((frag = strstr(token, "%20"))) {
		*frag = ' ';
		frag++;
		while (*(frag + 1) != '\0') {
			*frag = *(frag + 2);
			frag++;
		}
	}
	token++;
	fprintf(stdout, "file requested: %s\n", token);
	return (token);
}

void handle_file(int fd) {
	char buf[2048];

	bzero(buf, 2048);

	if ((read(fd, buf, 2048)) < 0) {
		fprintf(stderr, "read function call error.\n");
		exit(1);
	}
	printf("%s", buf);
	processRequest(get_filename(buf), fd);
}

int main() {
	int pid;

	struct sockaddr_in serv_addr, cli_addr;
	int sockfd, new_sockfd, clilen;
	//listen on sockfd, new connection on new_sockfd

	char part = 'B'; //change this to change part

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket function call error.\n");
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr)); // Clear structure, from bind manpage
	//set address info
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(MYPORT); //short network byte order
	//added htonl(), maybe doesnt matter
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//added this, ying gai he shang bian de yi ge yong tu
	memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));

	//changed sizeof(serv_addr) to sizeof(soxkaddr)
	if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr))
			< 0) {
		fprintf(stderr, "ERROR calling bind function.\n");
		exit(1);
	}

	//deleted:
	//listen(sockfd, 50); //50 connections at most according to bind manpage

	if (listen(sockfd, 1) == -1) {
		fprintf(stderr, "ERROR calling listen function.\n");
		exit(1);
	}

	clilen = sizeof(cli_addr);

	if (part == 'A') {
		if ((new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))
				< 0) {
			fprintf(stderr, "accept function call error.\n");
			exit(1);
		}

		char buf[1024];

		bzero(buf, 1024);  //reset buffer

		if ((read(new_sockfd, buf, 1024)) < 0) {
			fprintf(stderr, "read function call error.\n");
			exit(1);
		} //read from client

		//buf is the request
		printf("%s", buf);

		if ((write(new_sockfd, "message received", 16)) < 0) {
			fprintf(stderr, "write function call error.\n");
			exit(1);
		} //successfully read

		close(new_sockfd);
		close(sockfd); //close sockets

		exit(0);

	} else if (part == 'B') { //obtained from http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/server.c, able to handle multiple connections

		while (1) {
			//sin_size = sizeof(struct sockaddr_in);
			//changed clilen to sin_size
			if ((new_sockfd = accept(sockfd, (struct sockaddr*) &cli_addr,
					&clilen)) < 0) {
				fprintf(stderr, "accept function call error.\n");
				//changed exit(1) to continue
				continue;
			}

			if ((pid = fork()) < 0) {
				fprintf(stderr, "fork function call error.\n");
				exit(1);
			} else if (pid == 0) {
				//char* fileNaame = get_filename(buffer1);
				handle_file(new_sockfd);
				//processRequest(fileNaame,new_sockfd);
				close(new_sockfd);
				//close(sockfd);
				exit(0);
			} else {
				fprintf(stdout, "server: got connection from %s\n",
						inet_ntoa(cli_addr.sin_addr));
				close(new_sockfd);
				waitpid(-1, NULL, WNOHANG);
			}

		}
	}
}
