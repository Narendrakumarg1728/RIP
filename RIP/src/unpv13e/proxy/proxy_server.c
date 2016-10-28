/*
*   File:      proxy_server.c 
*
*   Purpose:    This is a skeleton file for a server.
*/
#include "unp.h"
//#include "myunp.h"
#include <time.h>
#define MAXLIEN 87300

void debug_buff(char *p, int num_bytes);
int main(int argc, char **argv)
{
  int listenfd,connfd,portno, clilen, num_f_sites, k, n, forbidden_flag=0, connfd_cli;
  struct sockaddr_in servaddr, cli_addr;
  char buff[MAXLINE], temp_buff[MAXLINE], cmd_browser[MAXLINE], buff_web_server[MAXLINE];
  char *forbidden_response= 
                            "HTTP/1.1 403 Forbidden\n"
                            "Connection: close\n"
                             "\n"
                             "Dont Try to access forbidden sites";
  char *not_imple_response= 
                            "HTTP/1.1 405 Method Not Allowed\n"
                            "Connection: close\n"
                             "\n"
                             "Only Get and Head are allowed";
  char f_sites[100] [100]; 
  char site_name[100];
  char temp_comp[100];
  int port_num =80;
  char port_num_string [10];
  char *http_request_check;
  char http_request_check_temp[MAXLINE];
  char h_request[MAXLINE];
  int get_or_head=0;      //Default not true     
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;

  pid_t childpid;
  sprintf(port_num_string, "%d", port_num);
  printf("\n\n\n*************************Proxy Server********************************\n\n\n");
  printf("Value of MAXLINE is:%d\n",MAXLINE);
  time_t mytime;
  mytime = time(NULL);
  printf("\nCurrent time: %s\n",ctime(&mytime)); 
  
  void sig_chld(int);
 
//  char camp[3];
//  char file_name[MAXLINE];

  if (argc!=3)
      err_quit("\n\nusage: myserver <port_number> forbidden-sites\n");

  FILE *forbidden = fopen(argv[2], "r");
  bzero(buff, MAXLINE);
  num_f_sites=0;
  while(!feof(forbidden) && num_f_sites <100)
  {   
      fgets(buff, MAXLINE, forbidden);
      strcpy(f_sites[num_f_sites], buff);
      printf("Debug: Forbidden sites are: %s", f_sites[num_f_sites]);
      ++num_f_sites;
  }
  fclose(forbidden);
  printf("Number of forbidden sites are: %d\n", num_f_sites);

  portno = atoi(argv[1]);
  printf("Debug: The port number entered is: %d\n", portno);

  if (( listenfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
      err_sys("socket error \n");
  printf("socket Creation completed...\n");
  bzero(&servaddr, sizeof(servaddr));
  printf("Bzero completed...\n");

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(portno);
  printf("Bind started...\n");
  Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
  printf("bind completd...\n");

  printf("Now will be listening...\n");
  Listen(listenfd, LISTENQ);

  printf("Listen command completed...\n");

  clilen = sizeof(cli_addr);
 
  Signal(SIGCHLD, sig_chld);    //SIGCHLD to handel zambie child

  for( ; ; )
  { 
  	printf("Debug: Start of Accept...\n");
  	connfd = Accept(listenfd, (SA *) &cli_addr, &clilen);
  	printf("Debug: Accept completed... with connfd of %d\n", connfd);
  
        if ( (childpid = fork()) == 0 )                       //Creating child process to handle processing of the sites and parent will go back waiting for accept...
        {
        	close(listenfd); 

  		bzero(buff, MAXLINE);
		FILE *f_browser =fopen("browser_file", "w");   //Get the command from the broswer and put it to the file
		if (f_browser == NULL)
			err_sys("fopen error");
	  
		FILE *f_logger = fopen("logger_file", "a");
		if (f_logger == NULL)
			err_sys("fopen error");
                printf("Reading from browser request...\n");
		recv(connfd,buff, MAXLINE,0);
		printf("Number of bytes written %zu \n", strlen(buff));
                bcopy(buff, http_request_check_temp, MAXLINE);	 
	//  if (n = (recv(connfd,buff, MAXLINE,0)) < 0);
	//      err_sys("read error \n");
// 		printf("Debug: Recv completed... \n");

                http_request_check = strtok(http_request_check_temp," ");    //Getting the HTTP request type
                printf("Request type is: %s\n\n\n\n",http_request_check);
		mytime = time(NULL);
	  
		fprintf(f_logger, "%s", ctime(&mytime));
		fputs(": From browser: ", f_logger); 
		fprintf(f_logger, "%s", buff);

		printf("Command received from the client: %s\n", buff);
                fputs("HTTP request is: ", f_logger); 
                fprintf(f_logger, "%s\n",http_request_check);
	  
		bcopy (buff, cmd_browser, MAXLINE);
                printf("Command received from the client after copying it to cmd_browser: %s\n",cmd_browser);
//                strcpy(http_request_check,buff);
//                printf("Verifying value of the http_request_check: %s\n\n\n\n",http_request_check);
//		printf("Debug: After copying to cmd_browser: %s\n", cmd_browser);
		fprintf(f_browser, "%s", buff);
		fclose(f_browser);
	  
		FILE *f_browser_read = fopen("browser_file", "r");    //To read the Host: part of the command 
		if (f_browser_read == NULL)
			err_sys("fopen error");
	  
		FILE *f_browser_write = fopen("browser_file", "r");    //To block the Accept-Encoding and  part of the command 
		if (f_browser_write == NULL)
			err_sys("fopen error");

		while(!feof(f_browser_read))             
		{	bzero(buff,MAXLINE);
                        bzero(temp_buff,MAXLINE);   
			fgets(buff, MAXLINE, f_browser_read);
			bcopy(buff, temp_buff, MAXLINE);
			printf("Debug: http Command header lines are: %s\n", temp_buff);
			if(strncmp(temp_buff, "Host:", 5) == 0)
			{
				printf("Value of the Host is: %s", temp_buff);
                		fputs("Trying to contact host: ", f_logger); 
                		fprintf(f_logger, "%s\n",temp_buff);
				strcpy (site_name, &temp_buff[6]);
				site_name[strcspn(site_name, "\n")] = '\0';  
				site_name[strcspn(site_name, "\r")] = '\0';  
				printf("Value of the site is: %s\n", site_name);
				break;
                                fclose(f_browser_read);
			}
		}
                bzero(buff,MAXLINE);

                bzero(h_request,MAXLINE);
                while(!feof(f_browser_write))                                      //Removing accept-encoding and connection
                {       bzero(buff,MAXLINE);
                        bzero(temp_buff,MAXLINE);
                        fgets(buff, MAXLINE, f_browser_write);
                        bcopy(buff, temp_buff, MAXLINE);
                        printf("Debug: Writing to buff h_request %s\n", temp_buff);
                        if((strncmp(temp_buff, "Accept-Encoding:", 16) == 0) || (strncmp(temp_buff, "Connection:", 11)) == 0)
                        {
                                printf("Value of the Accept-Encoding: or Connection: is: %s", temp_buff);
                                fputs("Trying to Remove Accept-Encoding: or Connection: is: ", f_logger);
                                fprintf(f_logger, "%s\n",temp_buff);
                                printf("Value of the Accept-Encoding which is:%s will not put it to HTTP request", temp_buff);
                                continue;
                        }
                       // fprintf(f_browse_write, "%s", temp_buff);
                        strcat(h_request,temp_buff);
                        
                }
               
                printf("\n\n\n\n\nValue of the h_request:%s\n", h_request);
                bcopy (h_request, cmd_browser, MAXLINE);
                printf("\n\n\n\n\nValue of the cmd_browse which is the HTTP request which will be fired at the web server is:%s\n", cmd_browser);

                







		for (k = 0; k<num_f_sites; k++)    //Looping through forbidden sites stored in the array...
		{
			printf("Inside for...and checking forbidden sites array %d: %s with site name:%s\n", k, f_sites[k], site_name);
			strcpy (temp_comp, f_sites[k]);
			temp_comp[strcspn(temp_comp, "\n")] = '\0';
			printf("Comparing site %s\n", temp_comp);
	    
			if (strcmp(temp_comp,site_name) == 0)
			{
				printf("\n\n\n********Trying to connect to forbidden site*********\n\n\n");
				printf("Will send forbidden response to the browser which is:%s\n\n\n",forbidden_response);
				Write(connfd,forbidden_response, strlen(forbidden_response));
				mytime = time(NULL);

				fprintf(f_logger, "%s", ctime(&mytime));
				fputs(": From Proxy server: ", f_logger);
				fprintf(f_logger, "%s", forbidden_response);

				close(connfd);
				forbidden_flag =1;
//				break;
 				exit(0);
			  
			} 
		}
 
                 	
	        if ((strcmp(http_request_check, "GET") ==0) || (strcmp(http_request_check, "HEAD") == 0))
                {
                	printf("Filtered the HTTP request to have only GET or Head request\n");
                        get_or_head= 1; 
                }
                                           


                if (get_or_head == 0)
                {
                	printf("Trying other than GET or HEAD...\n");
                        printf("Will send Not implemented response to the browser which is:%s",not_imple_response);
                        Write(connfd,not_imple_response, strlen(not_imple_response));                             //Sending not implemented to the web browser...
                        mytime = time(NULL);
                        fprintf(f_logger, "%s", ctime(&mytime));
                        fputs("Trying other than GET or HEAD... Will send 'Not implemented' response to the browser which is:", f_logger);
                        fputs(": From Proxy server: ", f_logger);
                        fprintf(f_logger, "%s", not_imple_response);

                        close(connfd);

                }
       
	  

		if (forbidden_flag == 0 && get_or_head == 1)           //Not a forbidden site and its GET or HEAD request, send it to client to handle the site connection...
		{
			printf("Not a forbidden site and its request is GET or HEAD... Trying to connect to web server: %s\n", site_name);
			mytime = time(NULL);
			printf("Current time: %s\n", ctime(&mytime));

			fprintf(f_logger, "%s", ctime(&mytime));
			fputs(": From Proxy server: Trying to connect to the web server:  ", f_logger);
			fprintf(f_logger, "%s\n", site_name);
			fputs(": \n\n NOT a Forbidden site...\n\n", f_logger);
			connfd_cli = tcp_connect(site_name, port_num_string);                //TCP connection from proxy client to the web server

                        Setsockopt(connfd_cli, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));    //Setting socket options of recv time out
                        Setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			if (connfd_cli < 0)
				err_sys("tcp connect error to the web server\n");
			printf("Value of the sockfd from proxy to web server is:%d\n", connfd_cli); 
			mytime = time(NULL);
			printf("\nCurrent time: %s\n", ctime(&mytime));

			fprintf(f_logger, "%s", ctime(&mytime));
			fputs(": From Proxy server: Connection successed and connection file discriptor is: ", f_logger);
			fprintf(f_logger, "%d\n", connfd_cli);

			mytime = time(NULL);
			printf("\nCurrent time: %s\n", ctime(&mytime));
			printf("From Proxy server: Will initiate HTTP request to the server: %s\n", site_name);
			fprintf(f_logger, "%s", ctime(&mytime));
			fputs(": From Proxy server: Will initiate HTTP request to the server: ", f_logger);
			fprintf(f_logger, "%s\n", site_name);
	   
			fputs("With command: ", f_logger);
			fprintf(f_logger, "%s\n", cmd_browser);
 
                        for ( ; ;)
                        {
	     
				printf("\n\n\nWill send the request received from the client to the web server which is:\n %s\n\n\n",cmd_browser);

				Write(connfd_cli,cmd_browser,strlen(cmd_browser));                                        //Sending HTTP request to the web server which was captured from the browser

				mytime = time(NULL);
				printf("\nCurrent time: %s\n", ctime(&mytime));
				printf("From Proxy server: Waiting for  HTTP response from the server: %s\n", site_name);
				fprintf(f_logger, "%s", ctime(&mytime));
				fputs("From Proxy server: Waiting for HTTP response from the server: ", f_logger);
				fprintf(f_logger, "%s\n", site_name);
				bzero(buff_web_server,MAXLINE);
				if ((n = recv(connfd_cli,buff_web_server, MAXLINE,0)) <= 0)                                //Receiving HTTP response from the web server to th the proxy client
                                {
					if (errno == EWOULDBLOCK)
                                        {
                                        	mytime = time(NULL);
                                		printf("\nCurrent time: %s\n", ctime(&mytime));
                                		printf("From Proxy server: Receive from web sererver completed...\n");
                                                fprintf(f_logger, "%s", ctime(&mytime));
                                		fputs("From Proxy server: Receive from web sererver completed...\n", f_logger);
                                                break;
                                        }
					else 
						err_sys("Receive from the web server completed...\n");
 				}
					
				printf("\nNum of bytes read is: %d\n", n);   
				printf("Response from the web server is: %s\n", buff_web_server);   
                                printf("\n\n\n************Debug***********\n\n\n");
                                debug_buff(buff_web_server,n);
				mytime = time(NULL);
				printf("\nCurrent time: %s\n", ctime(&mytime));
				fprintf(f_logger, "%s", ctime(&mytime));
				fputs("Response from the web server is:", f_logger);
				fprintf(f_logger, "%s", buff_web_server);

				mytime = time(NULL);
				printf("\nCurrent time: %s\n", ctime(&mytime));
				fprintf(f_logger, "%s", ctime(&mytime));
				fputs("Sending web server's response to the browser with response:\n", f_logger);
//				fprintf(f_logger, "%s", buff_web_server);

                                printf("Sending web server's response to the browser with response:\n%s\n", buff_web_server);

				Write(connfd,buff_web_server, strlen(buff_web_server));                                    //Sending the HTTP response back to the browser
                                printf("Write to Browser completed...\n");
                                printf("Will read from Browser now...\n");
                                bzero(buff,MAXLINE);
				if ( (n = recv(connfd,buff, MAXLINE,0)) <= 0)                          //Again getting response from the client
                                {
					printf("Inside if statement of recv...\n");
					if (errno == EWOULDBLOCK)
                                        { 
                                                bcopy(cmd_browser, buff, MAXLINE);                         //Making sure that we will execute old request again if we wont receive anything from the browser
                                        	mytime = time(NULL);
                                		printf("\nCurrent time: %s\n", ctime(&mytime));
                                		printf("From Proxy server: Receive from web browser completed as time out occured...\n");
                                                fprintf(f_logger, "%s", ctime(&mytime));
                                		fputs("From Proxy server: Receive from web browser completed as time out occured...", f_logger);
//                                                break;                                       //Not breaking to check if we receive anything from the browser
                                        }
					else 
						err_sys("Receive from the web server failire...\n");
 				}
                                FILE *f_browser2 =fopen("browser_file2", "w");   //Get the command from the broswer and put it to the file
		                if (f_browser2 == NULL)
                		        err_sys("fopen error");

		                fprintf(f_browser2, "%s", buff);
                                fputs("Narendra here...", f_browser2);
		                fclose(f_browser2);
			bzero(buff,MAXLINE);

			bzero(h_request,MAXLINE);
                        FILE *f_browser_write2 = fopen("browser_file2", "r");    //To block the Accept-Encoding and  part of the command 
	                if (f_browser_write2 == NULL)
        	                err_sys("fopen error");

			while(!feof(f_browser_write2))                                      //Removing accept-encoding and connection
			{       bzero(buff,MAXLINE);
				bzero(temp_buff,MAXLINE);
				fgets(buff, MAXLINE, f_browser_write2);
				bcopy(buff, temp_buff, MAXLINE);
				printf("Debug: Writing to buff h_request %s\n", temp_buff);
				if((strncmp(temp_buff, "Accept-Encoding:", 16) == 0) || (strncmp(temp_buff, "Connection:", 11)) == 0)
				{
					printf("Value of the Accept-Encoding: or Connection: is: %s", temp_buff);
					fputs("Trying to Remove Accept-Encoding: or Connection: is: ", f_logger);
					fprintf(f_logger, "%s\n",temp_buff);
					printf("Value of the Accept-Encoding which is:%s will not put it to HTTP request", temp_buff);
					continue;
				}
			       // fprintf(f_browse_write, "%s", temp_buff);
				strcat(h_request,temp_buff);
				
			}
                        fclose(f_browser_write2);
		       
			printf("\n\n\n\n\nValue of the h_request:%s\n", h_request);
			bcopy (h_request, cmd_browser, MAXLINE);
			printf("\n\n\n\n\nValue of the cmd_browse which is the HTTP request which will be fired at the web server is:%s\n", cmd_browser);
//			bcopy(buff, cmd_browser, MAXLINE);

			printf("Written to the browser...\n");

/*			bzero(buff_web_server,MAXLINE);
			if ((n = recv(connfd_cli,buff_web_server, MAXLINE,0)) < 0)
				err_sys("Receive from the web server failire...\n");
			printf("Num of bytes read is: %d", n);
			printf("Response from the web server is: %s", buff_web_server);
			mytime = time(NULL);
			printf("\nCurrent time: %s\n", ctime(&mytime));
			fprintf(f_logger, "%s", ctime(&mytime));
			fputs("Response from the web server is:", f_logger);
			fprintf(f_logger, "%s", buff_web_server);

			mytime = time(NULL);
			printf("\nCurrent time: %s\n", ctime(&mytime));
			fprintf(f_logger, "%s", ctime(&mytime));
			fputs("Sending web server's response to the browser with response:", f_logger);
			fprintf(f_logger, "%s", buff_web_server);

			Write(connfd,buff_web_server, strlen(buff_web_server));  */

		        	printf("Written to the browser...\n");
		        }
			exit(0);
		}

  	}
        close(connfd);
  }
  return 0;
} 


 int
tcp_connect(const char *host, const char *serv)
{
	printf("\n\n\nNarendra's TCP...\n\n\n");
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;    //	 ignore this one 

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;		// success

		Close(sockfd);	// ignore this one 
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	// errno set from final connect()
		err_sys("tcp_connect error for %s, %s", host, serv);

	freeaddrinfo(ressave);

	return(sockfd);
}
// end tcp_connect 

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
*/ 

/*int
Tcp_connect(const char *host, const char *serv)
{
	return(tcp_connect(host, serv));
} */


void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		printf("child %d terminated\n", pid);
	}
	return;
}

void debug_buff(char *p, int num_bytes)
{
	while (num_bytes != 0)
        {
		
 		printf(" %d",*p);
                --num_bytes;
                ++p;
        }
} 
