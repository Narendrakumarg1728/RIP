/*
*   File:      proxy_server.c 
*
*   Purpose:    This is a skeleton file for a server.
*/
#include "unp.h"
//#include "myunp.h"
#include <time.h>

int main(int argc, char **argv)
{
  int listenfd,connfd,portno, clilen, num_f_sites, k, n, forbidden_flag=0, sockfd_cli;
  struct sockaddr_in servaddr, cli_addr;
  char buff[MAXLINE], temp_buff[MAXLINE], cmd_browser[MAXLINE];
  char *forbidden_response= 
                            "HTTP/1.1 403 Forbidden\n"
                            "Connection: close\n"
                             "\n"
                             "Dont Try to access forbidden sites";
  char f_sites[100] [100]; 
  char site_name[100];
  char temp_comp[100];
  int port_num =15608;
  char port_num_string [10] = sprintf(port_num_string, "%d", port_num);

  time_t mytime;
  mytime = time(NULL);
  printf("Current time: %s",ctime(&mytime)); 

 
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
      printf("Debug: Forbidden sites: %s\n", f_sites[num_f_sites]);
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

//  for( ; ; )
//  { 
  printf("Debug: Start of Accept...\n");
  connfd = Accept(listenfd, (SA *) &cli_addr, &clilen);
  printf("Debug: Accept completed... with connfd of %d\n", connfd);
  


//  for ( ; ;)
//  {   
  bzero(buff, MAXLINE);
  FILE *f_browser =fopen("browser_file", "w");   //Get the command from the broswer and put it to the file
  if (f_browser == NULL)
      err_sys("fopen error");
  
  FILE *f_logger = fopen("logger_file", "w");
  if (f_logger == NULL)
      err_sys("fopen error");

  recv(connfd,buff, MAXLINE,0);
 
//  if (n = (recv(connfd,buff, MAXLINE,0)) < 0);
//      err_sys("read error \n");
  printf("Debug: Recv completed... \n");

  mytime = time(NULL);
  printf("Current time: %s:",ctime(&mytime));
  
  fprintf(f_logger, "%s", ctime(&mytime));
  fputs(": From browser: ", f_logger); 
  fprintf(f_logger, "%s", buff);

  printf("Debug: Logged to logger... \n");
  printf("Command received from the client: %s\n", buff);
  
  strcpy (cmd_browser,buff);
  printf("Debug: After copying to cmd_browser: %s\n", cmd_browser);
  fprintf(f_browser, "%s", buff);
  printf("Number of bytes written %zu \n", strlen(buff));
  fclose(f_browser);
  
  FILE *f_browser_read = fopen("browser_file", "r");    //To read the Host: part of the command 
  if (f_browser_read == NULL)
      err_sys("fopen error");
  
  while(!feof(f_browser_read))             
  {   
      fgets(buff, MAXLINE, f_browser_read);
      strcpy(temp_buff, buff);
      printf("Debug: http Command header lines are: %s\n", temp_buff);
      if(strncmp(temp_buff, "Host:", 5) == 0)
      {
          printf("Value of the Host is: %s", temp_buff);
          strcpy (site_name, &temp_buff[6]);
          site_name[strcspn(site_name, "\n")] = '\0';  
          site_name[strcspn(site_name, "\r")] = '\0';  
          printf("Value of the site is: %s", site_name);
          break;
      }
  }


  for (k = 0; k<num_f_sites; k++)    //Looping through forbidden sites stored in the array...
  {
      printf("Inside for...and checking forbidden sites array %d:%swith site name:%s", k, f_sites[k], site_name);
      strcpy (temp_comp, f_sites[k]);
      temp_comp[strcspn(temp_comp, "\n")] = '\0';
      printf("Value of the temp_comp site is %s", temp_comp);
    
      if (strcmp(temp_comp,site_name) == 0)
      {
          printf("trying to connect to forbidden site...\n");
          printf("Will send forbidden response to the browser which is:%s",forbidden_response);
          Write(connfd,forbidden_response, strlen(forbidden_response));
          mytime = time(NULL);
          printf("Current time: %s", ctime(&mytime));

          fprintf(f_logger, "%s", ctime(&mytime));
          fputs(": From Proxy server: ", f_logger);
          fprintf(f_logger, "%s", forbidden_response);

          close(connfd);
          forbidden_flag =1;
          break;
                  
      } 
  }

  if (forbidden_flag == 0)           //Not a forbidden site send it to client to handle the site connection...
  {
     sockfd_cli = tcp_connect(site_name, port_num_string);   
     printf("Value of the sockfd from proxy to web server is:%d", sockfd_cli); 
  }

  return 0;
} 


/* int
tcp_connect(const char *host, const char *serv)
{
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

//
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
// 

int
Tcp_connect(const char *host, const char *serv)
{
	return(tcp_connect(host, serv));
} */


