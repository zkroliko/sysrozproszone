#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <limits.h>
#include <errno.h>
#include <math.h>

#define IN_BUFSIZE 1024

#define REC_BUFSIZE 2

// Could do with 8 but there is memcpy problem
// two consecutive addresses even thought they
// are not overlapping will fail memcpy (bug?)
#define MAX_QUERY_SIZE 9

#define SEPARATOR 10

static int querySize;

static int parameterCountGood(int length);

static int makeQuery(char* input, char* query);

static int parameterCountGood(int length) {
	if (length < 2) {
		return -1;
	} else {
		return 0;
	}
}

static int makeQuery(char* in, char* query) {
	long long number;
     
	/* Trying to parse the input as a number */


	/* To distinguish success/failure after call */


	errno = 0;

	number = strtoll(in, NULL, 10);


	if (errno != 0 || number <= 0) {
		fprintf(stderr, "Failure to parse input as postive number\n");
		return -1;
	}

	fprintf(stderr, "Number %lld recognized\n", number);

     
	/* Will try all the sizes sequentially */

	unsigned int sizes [] = {8,4,2,1};

	int i = 0;
	int size = -1;
	while (i < 4 ) {
		long long lowerbound = -powl((long long)((2)),((long long)(sizes[i]*8-1))); 
		long long upperbound = -lowerbound-1;
		
		fprintf(stderr, "%lld <? %lld <? %lld\n", lowerbound, number, upperbound);

		if (number >= lowerbound && number <= upperbound) {
			size = sizes[i];
			i++;
		} else {
			break;
		}
	}


	if (size > 0) {
		fprintf(stderr, "Query size of %d bytes has been selected\n", size);
	} else {		
		fprintf(stderr, "Unspecified error in parsing input number. Cannot find a correct size.\n");
		return -1;				
	}        

	/* Finally formatting(copying) the number to(of) the correct size */
	// potential hacking !!!
	if (memcpy(query,&number, size) < 0 ) {
		fprintf(stderr, "Problem with placing the number in the buffer.\n");
		return -1;
	}		

	/* Returning the size */

	return size;
}

static void printNumber(FILE* file, char* number, int size) {

	if (size == 8) {
		fprintf(file, "%lld", *(long long *)number);
	} else if (size == 4) {
		fprintf(file, "%d", *(int *)number);
	} else if (size == 2) {
		fprintf(file, "%hd", *(short *)number);
	} else {
		fprintf(file, "%hhd", *number);
	}
}

int main (int argc, char* args[]) {

   	struct sockaddr_in serv_addr;
	int fd, len;

	/* Checking the parameter size */

	if (parameterCountGood(argc) < 0) {
		fprintf(stderr, "Usage: client <postive number>\n");
        return 0;

	}

	/* Choosing a correct number formar(validating input) and making a query */

	char query [MAX_QUERY_SIZE];
	bzero((char*)&query, sizeof(query));

	if ((querySize = (makeQuery(args[1], query))) < 1) {
		fprintf(stderr, "Invalid number. Please enter a positive integer which fits in 8 bytes.\n");
		return -1;
	}


	/* Creating the socket */

	fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero((char*)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr= inet_addr("192.168.88.1");
	serv_addr.sin_port = htons(atoi("5000"));

    	fprintf(stderr, "Connecting at port %d\n", serv_addr.sin_port);

	connect(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

    	fprintf(stderr, "Socket open\n");


	/* Sending message and a separator */

    	fprintf(stderr, "Sending %d byte message: ", querySize);
	printNumber(stderr,query, querySize);
	fprintf(stderr, "\n");

	query[querySize]=SEPARATOR;

	len = send(fd, query, querySize+1, 0);

	fprintf(stderr, "Sent %d bytes of message\n", len);

	if (len != querySize+1) {		
		fprintf(stderr, "Invalid number of sent bytes.\n");
		return -1;
	}

	/* Getting a response */

	
	char recvline[REC_BUFSIZE];

	bzero((char*)&recvline, sizeof(recvline));

    	fprintf(stderr, "Waiting for response\n");

	len = recv(fd, recvline, 1, 0);

	fprintf(stderr, "Recv returned %d\n", len);

	if (len != 1) {		
		fprintf(stderr, "Recv malfunction.\n");
		return -1;
	}


    	fprintf(stderr,"Received %d byte message: %s\n", len, recvline);


	/* Printing the result*/

	if (recvline[0] == '-') {
		fprintf(stderr, "Not positive number sent to the server. Unexpected error.\n");
		printf("Service unaviable. Please report the problem to development team.\n");
		return -1;
	} else if (recvline[0] == '?') {
		fprintf(stderr, "Server declined the request. The number might be too big.\n");
		printf("Request is too big. Please contact administrator in order to raise the limit.\n");
		return -1;
	} else {
		printf("The "); 
		printNumber(stdout,query, querySize);
		printf(" decimal digit of pi is %s\n", recvline); 
	}

	int res;
	if ((res = close(fd)) < 0) {
		fprintf(stderr, "Closing the socket failed. Returned:%d\n", res);
	}

	return 0;
}


