/*
*   File:      rip.c 
*
*   Purpose:    This is a skeleton file for a server.
*/
#include "unp.h"
//#include "myunp.h"
#include <time.h>
//Global Declarations

pthread_mutex_t routing_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in servaddr;
struct m_out
{
	char phy_ip[20];
	int phy_port;
};
struct m_out mapper(char *switch_name);
struct route1 
{
	char dest[20];
	int cost;
	char hop[20];
};
char routing_table[MAXLINE];
char direct_connect_nodes[16][MAXLINE];
int portno,  direct_nodes;
int all_nodes_count, direct_nodes, sockfd;
char source_switch[20];

int route_send () ;
void * route_recv (struct sockaddr_in cliaddr);

char *route_compare( char c_buff[]);
char *route_modifier (char node[], int value);
struct route string_to_struct_route(char r_table[MAXLINE]);
char * reverse_mapper(char message[MAXLINE]);
void * timer_180(char v_ip[20]);
char * r_mapper(char switch_name[MAXLINE]);

int main(int argc, char **argv)
{
  int num_nodes=0, i, j, t;
  int flag_node_present, flag_direct_node_present;
  char buff[MAXLINE], temp_node[20], *node_holder1, *node_holder2, node1[MAXLINE], node2[MAXLINE], comment_checker[10], all_nodes[16][MAXLINE], *context, temp_msg[MAXLINE], r_buff[MAXLINE];
//  char completion_string[20] = "~0000COMPLETED0000~";
  time_t mytime;
  mytime = time(NULL);
  printf("\nCurrent time: %s\n",ctime(&mytime));
  pid_t childpid;

/*Declaring  threadids... */
  pthread_t tid[direct_nodes], tid100;

  int length, sockfd_r;
  struct sockaddr_in cliaddr;

  struct route 
  {
	char dest[20];
	int cost;
	char hop[20];
  };
  struct route route16[16];
  struct m_out m; 

  if (argc!=2)
      err_quit("\n\nusage: rip <switch_ip>\n");

  /*Handling neighbour.config file to get the node topology to create the data structure */

  FILE *f_nodes =fopen("neighbor.config", "r");                                            //Get the neighbor configurations
  if (f_nodes == NULL)
      err_sys("fopen error");

  strcpy (source_switch, argv[1]);

  all_nodes_count =0;
  direct_nodes=0;
 
  struct m_out mapp; 
  while (!feof(f_nodes) && num_nodes !=160)                                             //Considering there will be max of 160 different nodes
  {
        fgets(buff,MAXLINE, f_nodes);
//        printf("Value of buff is: %c\n",buff[0]);
        comment_checker[0]= buff[0];
        comment_checker[1] = '\0';
  	if  (strcmp(comment_checker,"#") == 0 || strcmp(comment_checker, "1"))
	{
//		printf("Debug: Dropping commented line...\n%s",buff);
		continue;
	}
        
        node_holder1 = strtok_r(buff, " ", &context);
        strcpy(node1,node_holder1);
        ++num_nodes;
        node_holder2 = strtok_r(NULL, " ", &context);
        if (node_holder2 == NULL)
		continue;
        ++num_nodes;
//	printf("Debug: Number of nodes is: %d\n", num_nodes);
//        printf("\n\n\n\nValue of node_holder2 value is: %s\n\n\n\n", node_holder2);
        strcpy(node2, node_holder2);

 //       printf("Value of switches in node.config file are %s     %s\n", node1, node2);
	if (all_nodes_count == 0)
	{
		//printf("Initializing the all_nodes array with node1 and node 2 values\n");
		strcpy(all_nodes[all_nodes_count],node1);
		++all_nodes_count;
		strcpy(all_nodes[all_nodes_count],node2);
		++all_nodes_count;
	}

        for (i = 0 ; i < 16; i++)             //Comparing node1 with array of all nodes if it is not there appending to the array list...
	{
		//printf("Debug: Comparing node1:%s with all_ndes_array[%d]:%s\n",node1, i, all_nodes[i]);
		if (strcmp (node1, all_nodes[i]) != 0)
		{
		//	printf("Debug Not found in array at value %d\n", i);
			flag_node_present = 0;
			continue;
		}
//		printf("Compare passed...\n");
		flag_node_present = 1;
		break;
	}
	if (flag_node_present == 0)
	{	
//		printf("Appending the node to tha all-nodes_array");
		strcpy (all_nodes[all_nodes_count], node1);
               	++all_nodes_count;
	}
        flag_node_present = 0;
	


	for (i = 0 ; i <= 16; i++)           //Comparing node1 with array of all nodes if it is not there appending to the array list...
        {
		//printf("Debug: Comparing node2:%s with all_ndes_array[%d]:%s\n",node2, i, all_nodes[i]);
		
                if (strcmp (node2, all_nodes[i]) != 0)
                {
//                        printf("Debug Not found in array at value %d\n", i);
                        flag_node_present = 0;
                        continue;
                }
//		printf("Compare passed...\n");
                flag_node_present = 1;
		break;
	}

	if (flag_node_present == 0)
	{
//		printf("Appending the node to tha all-nodes_array at all_nodes_count %d", all_nodes_count);
		strcpy(all_nodes[all_nodes_count], node2);
		++all_nodes_count;
	}


        if ((strcmp(source_switch, node1) == 0) || (strcmp(source_switch, node2) == 0))
	{ 
//		printf("**************************Debug:Switch %s or %s  is directly connected to  source switch %s... Check which one is it...********************\n", node1, node2, source_switch);
		if(strcmp(source_switch, node1) == 0)
		{
//			printf("Other node is node2: %s\n",node2);
			strcpy(direct_connect_nodes[direct_nodes] , node2);
			++direct_nodes;
		}
		else
		{
//			printf("Other node is node1: %s\n", node1);
                        strcpy(direct_connect_nodes[direct_nodes], node1);
			++direct_nodes;
		}
		
	} 
//	printf("Nodes are: %s and %s\n", node1, node2);

	
  }

  printf("Printing all nodes array...\n");

  for (i = 0; i < all_nodes_count; i++)
  { 
  	printf("Value of nodes: %s\n", all_nodes[i]);
  }

  printf("Printing direct attached nodes array...\n");	

  for (i = 0; i < direct_nodes; i++)
  {
        printf("Value of nodes: %s\n", direct_connect_nodes[i]);
  }
//  printf("Done with node config data collection...\n");
  
  printf("Number of nodes in the network are:%d",all_nodes_count );
//  struct threads_param th_direct[direct_nodes];


//  routing table creation

  bzero(routing_table,MAXLINE);
  flag_direct_node_present = 0;

  for (i =0; i< all_nodes_count; i++)
  {
	flag_direct_node_present = 0;
 //       printf("Value of i is %d value of all_nodes[i] is %s", i, all_nodes[i]);
	strcpy(route16[i].dest, all_nodes[i]);
//	printf("Debug: Value of route16[i].dest is %s\n", route16[i].dest);
	strcat(routing_table,route16[i].dest);
	strcat(routing_table,"\t");
	if (strcmp(all_nodes[i], source_switch) == 0)
	{
		route16[i].cost = 0;
		bzero(temp_msg,MAXLINE);
 		sprintf(temp_msg, "%d", route16[i].cost);
		strcat(routing_table,temp_msg);
		strcat(routing_table,"	");
		strcpy (route16[i].hop, "self");
		strcat(routing_table,route16[i].hop);
		strcat(routing_table,"\n");
 
		continue;
	}

	for (j = 0; j < direct_nodes; j++)
	{
		if( strcmp (all_nodes[i], direct_connect_nodes[j]) == 0)
		{
			flag_direct_node_present = 1;
			route16[i].cost = 1;
			bzero(temp_msg,MAXLINE);
 			sprintf(temp_msg, "%d", route16[i].cost);
			strcat(routing_table,temp_msg);
			strcat(routing_table,"	");
			strcpy (route16[i].hop, source_switch);
			strcat(routing_table,route16[i].hop);
			strcat(routing_table,"\n");
			break;
		}
	}
	if ( flag_direct_node_present == 1)
		continue; 
	
	route16[i].cost = 16;
	bzero(temp_msg,MAXLINE);
 	sprintf(temp_msg, "%d", route16[i].cost);
	strcat(routing_table,temp_msg);
	strcat(routing_table,"\t");
	strcpy (route16[i].hop, "-");
	strcat(routing_table,route16[i].hop);
	strcat(routing_table,"\n");
  }
  mytime = time(NULL);
  printf("\n**********************************\nCurrent time: %s\n**********************************\n",ctime(&mytime));
  printf("Routing table value for initial setup before start of route table sharing with the neighbours is: \n %s \n",routing_table); 
  printf("**********************************\n");

//Creating a directly connected nodes with physical IP 





//Handling node.config file
  m= mapper(source_switch);

  printf("\n*****Source switch Ip and port address from the node.config file is: %s and %d\n*****\n", m.phy_ip, m.phy_port); 


  portno = m.phy_port;                                                               //Using asci to integer to get the port number
  printf("The port number on which the switch will be running is: %d\n", portno);


/*  if ( (childpid = fork()) != 0 )           				//Parent will call function which will run in loop to send data every 30 seconds...
  {									//And child process will handle threads which will receive data from neighbors, 180secs timeout will send infinity value...

  	route_send(routing_table, direct_connect_nodes, direct_nodes);
  }*/
	

  char route_table[MAXLINE] ;
  length = sizeof(servaddr);
//  m= mapper(source_switch);
  bzero(&servaddr,sizeof(servaddr));
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)  				//creating DGRAM socket
	err_sys("socket error\n");
  printf ("Debug: Verifying the sockfd value for debugging:%d \n ",sockfd);
  servaddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, m.phy_ip, &servaddr.sin_addr) <= 0)           		/*Getting server address for the first server */
      err_quit("inet_pton error for %s\n", m.phy_ip);
  servaddr.sin_port = htons(m.phy_port);

  Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

  if (pthread_create(&tid100, NULL, &route_send, NULL))
  {
 	 err_sys("Unable to craate pthread for func route send \n");
                        exit(1);
  }


 
//Initial socket creation for receiving

//  length= sizeof(servaddr);
//  if ( (sockfd_r = socket(AF_INET, SOCK_DGRAM,0)) < 0)                                //creating DGRAM socket
//       	err_sys("socket error\n");
//  printf ("Debug: Verifying the sockfd_r value for debugging:%d \n ",sockfd_r);
//  bzero(&servaddr,sizeof(servaddr));
//  servaddr.sin_family = AF_INET;
//  cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//  printf("Will use the port numbber mapped for this switch to be listning at port no: %d", portno);
//  servaddr.sin_port = htons(portno);
//  Bind(sockfd_r, (SA *) &servaddr, sizeof(servaddr));

	

//  for (t=0 ; t < direct_nodes; t++)
//  {
	
  for (i = 0; i < direct_nodes; i++)
  {
//			strcpy(route_table, routing_table);
	printf("Value of node to which the route table to be sent is: %s\n", direct_connect_nodes[i]);
	m= mapper(direct_connect_nodes[i]);
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, m.phy_ip, &cliaddr.sin_addr) <= 0)           		/*Getting client neighbor address for the first server */
	      err_quit("inet_pton error for %s\n", m.phy_ip);
	cliaddr.sin_port = htons(m.phy_port); 
//	  	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("Debug: map from which route will be received is virtual_IP: %s to m.phy_ip is: %sand m.phy_port is %d:\n", direct_connect_nodes[i], m.phy_ip, m.phy_port);
	printf("Same neighbor details from cliaddr struct:physical_ip:%s and physical port:%d",inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohl(cliaddr.sin_port));

//			connect(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
//			printf("\nTime to send routing update to the neighbors...\n");
//		sendto(sockfd,route_table,strlen(route_table), 0, (SA *) &cliaddr, length);
//			printf("Sent route table from route_send function to switch with route table info:\n %s\n", route_table);
// 		 	printf("***********Will wait for RECVFROM********\n");	
//	  		recvfrom(sockfd, r_buff, MAXLINE, 0, (SA *) &cliaddr, &length);
//			printf("ROUTE RECEIVED is:%s\n",routing_table);

  
//  for (t=0 ; t < direct_nodes; t++)
//  {
//	strcpy(th_direct[t].dest,direct_connect_nodes[t]);
//	strcpy(th_direct[t].r_table, routing_table);
	printf("Calling threads to receive route update from neighbor...\n");
//	strcpy(temp_node,direct_connect_nodes[t]);
//	printf("Direct connect node is:%s\n",temp_node);
	
	
  	if (pthread_create(&tid[t], NULL, &route_recv, &cliaddr))
       	{
               	fprintf(stderr, "Unable to craate thread_%d thread \n",t);
               	exit(1);
       	}
                     
//  }

  }

  printf("Debug: Back to main func...\n");
 
  for (t= 0; t < direct_nodes; t++)
  {
	printf("Inside loop to perform pthread join for the threads...\n");
	pthread_join(tid[t], NULL);
  }

  pthread_join(tid100, NULL);

  return(0);
} 


  struct m_out mapper(char switch_name[MAXLINE])
  {
	char name_copy[20], *n_holder1, *n_holder2, *n_holder3, *n_holder4, *context, node1[20], vi_port[20], p_port[20], m_buff[MAXLINE], comment_checker[10];
	struct m_out mout;

	strcpy(name_copy,switch_name);
	printf("Inside Mapper, value of the Virtual IP which should be mapped is: %s\n",name_copy);
	FILE *f_map =fopen("node.config", "r");                                            //Get the node configurations
	if (f_map == NULL)
	err_sys("fopen error");

	while (!feof(f_map))                                             //Considering there will be max of 16 different nodes
	{
		fgets(m_buff,MAXLINE, f_map);
		//        printf("Value of buff is: %c\n",buff[0]);
		comment_checker[0]= m_buff[0];
		comment_checker[1] = '\0';
		if  (strcmp(comment_checker,"#") == 0)                                             // || strcmp(comment_checker, "520")) Will see if I can drop 520 too
		{
//			printf("Debug: Dropping commented line...\n%s",m_buff);
			continue;
		}

		n_holder1 = strtok_r(m_buff, " ", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(node1,n_holder1);
//		printf("Value of the virtual_IP is: %s\n",node1);
		n_holder2 = strtok_r(NULL, " ", &context);
		if (n_holder2 == NULL)
			continue;
		strcpy(vi_port,n_holder2);
//		printf("Value of the virtual_port is: %s\n",vi_port);
		n_holder3 = strtok_r(NULL, " ", &context);
		if (n_holder3 == NULL)
			continue;
		strcpy(mout.phy_ip,n_holder3);
//		printf("Value of the phy_ip is: %s\n",mout.phy_ip);
		n_holder4 = strtok_r(NULL, " ", &context);
		if (n_holder4 == NULL)
			continue;
		strcpy(p_port,n_holder4);
//		printf("Value of the phy_port is: %s\n",p_port);
		mout.phy_port = atoi(p_port);

		if (strcmp(node1, name_copy) == 0)
		{
			printf("Value of the map is virtual_IP:%s phy_ip: %s, phy_port: %d\n", node1, mout.phy_ip, mout.phy_port); 
			return(mout);
		}
	}
  	exit(-1);
  }
	
  int route_send ( )
  {
  	time_t mytime;

	int  i, length;
	struct sockaddr_in cliaddr,allcliaddr;
	struct m_out m; 
	char route_table[MAXLINE] ;
	m= mapper(source_switch);
	length = sizeof(cliaddr);
	printf("Debug: map to which route will be sent from is virtual_IP which is self: %s to m.phy_ip is: %sand m.phy_port is %d:\n", source_switch, m.phy_ip, m.phy_port);
	for ( ; ; )
	{
		for (i = 0; i < direct_nodes; i++)
		{	
			Pthread_mutex_lock(&routing_mutex);
                        printf("Updating ROUTING TABLE...\n");
			strcpy(route_table, routing_table);
                        printf("Verifying routing table...\n%s\n", routing_table);
                        Pthread_mutex_unlock(&routing_mutex);

//			printf("Value of node to which the route table to be sent is: %s\n", d_connect_nodes[i]);
			m= mapper(direct_connect_nodes[i]);
			bzero(&cliaddr,sizeof(cliaddr));
			bzero(&allcliaddr,sizeof(cliaddr));
			cliaddr.sin_family = AF_INET;
//			allcliaddr.sin_family = AF_INET;
//		  	allcliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			if (inet_pton(AF_INET, m.phy_ip, &cliaddr.sin_addr) <= 0)           		/*Getting client neighbor address for the first server */
			      err_quit("inet_pton error for %s\n", m.phy_ip);
			cliaddr.sin_port = htons(m.phy_port); 
			printf("Debug: map to which route will be sent is virtual_IP: %s to m.phy_ip is: %sand m.phy_port is %d:\n", direct_connect_nodes[i], m.phy_ip, m.phy_port);
//			printf("\nTime to send routing update to the neighbors...\n");
//			sendto(sockfd,route_table,strlen(route_table), 0, (SA *) &allcliaddr, length);
			sendto(sockfd,route_table,strlen(route_table), 0, (SA *) &cliaddr, length);
			printf("Sent route table from route_send function to switch with route table info:\n %s\n", route_table);
		}
		mytime = time(NULL);
		printf("\n**********************************\nCurrent time: %s\n**********************************\n",ctime(&mytime));
		struct timeval tv = {30, 0};   // sleep for 30 seconds!
		if (select(0, NULL, NULL, NULL, &tv) < 0) 
			err_sys("select error\n");
	}
	return(-1);
  }


 void * route_recv (struct sockaddr_in cliaddr)
  {	
	printf("\n\n\n\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Inside thread route_recv~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n\n\n\n\n");
  	pthread_t tid_timer;
	int  length, max_fd, rv, cli_port, s, count_received=0;
	long node_ip;
	struct m_out m_recv;
//	struct sockaddr_in servaddr; //, cliaddr;
        struct timeval tv = {180, 0};   // sleep for 180 seconds!
//	fd_set rset;
	char r_buff[MAXLINE], cli_address[MAXLINE], temp_buff[MAXLINE], buff[MAXLINE], *up_route_table, *neighbor_virtual_ip, n_v_ip[20], neighbor_mess[MAXLINE];
	length = sizeof(cliaddr);
	node_ip = ntohl(cliaddr.sin_addr.s_addr);
	bzero(r_buff, MAXLINE);
	Pthread_mutex_lock(&routing_mutex);
        printf("Updating ROUTING TABLE...\n");
	strcpy(r_buff,routing_table);
        printf("Verifying routing table...\n%s\n", routing_table);
        Pthread_mutex_unlock(&routing_mutex);
	connect(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

	for ( ; ; )
	{
		//printf("\n\n\n\n\n\n\n\n\nSTART of for-LOOP\n\n\n\n\n\n\n");
		//printf("\n\n\nValue of the direct connect node from where we will start to recv is:%ld\n\n\n",cliaddr.sin_addr.s_addr );
		strcpy(cli_address, inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)));
		cli_port = ntohs(cliaddr.sin_port);
		printf("Debug: Received data from the neighbor:%s with port: %d\n And route table is: %s\n\n\n", cli_address, cli_port, temp_buff);
//		m_recv = mapper(d_connect_node);
//		max_fd=sockfd_r+1;
//		FD_ZERO(&rset);
//                if (inet_pton(AF_INET, m_recv.phy_ip, &servaddr.sin_addr) <= 0)                      /*Getting server address for the first server */
//                	err_quit("inet_pton error for %s\n", m_recv.phy_ip);

//                printf("Debug: THREAD_where RECVFROM... Value of virtual_ip %s to m_recv.phy_ip is: %s and m_recv.phy_port is %d\n", d_connect_node, m_recv.phy_ip, m_recv.phy_port);
		strcpy(neighbor_mess,r_buff); 
		++count_received;
		printf("Total Receive from:%d",count_received);
		if(count_received >10)
		{
			neighbor_virtual_ip = r_mapper(cli_address);
			strcpy(n_v_ip,neighbor_virtual_ip);
		}
		else
		{
			neighbor_virtual_ip = reverse_mapper(neighbor_mess);
			strcpy(n_v_ip,neighbor_virtual_ip);
		}
		printf("Value of virtiual IP is:%s  for the cliaddr:%s\n",n_v_ip,cli_address);
                tv.tv_sec = 180;
		tv.tv_usec = 0;
//		FD_SET(sockfd_r, &rset);

//		printf("Waiting on select...\n");
//		if ( (rv =select(max_fd, &rset , NULL, NULL, &tv)) < 0)
//              	err_sys("select error\n");
//		printf("Return value of select is: %d", rv);

//		if (FD_ISSET (sockfd_r, &rset))
//		{
//			printf("\n\n\n\nBefore recv\n\n\n\n\n"); 
			printf("Thread receiving from NEIGHBOR:%s", n_v_ip);
			printf("Debug... in Thread function. Will create one more thread:timer_180 recvfrom function...\n");
			if (pthread_create(&tid_timer, NULL, &timer_180, &n_v_ip))
			{
				err_sys("Unable to craate tid_timer thread \n");
				exit(1);
			}	
			recvfrom(sockfd, r_buff, MAXLINE, 0, (SA *) &cliaddr, &length);  //If we wont receive from the neighbor we will be in this state for ever
			printf("Will cancle TIMER_180 timer...\n");			//if we receive data we will kill th etimer thread...
			 pthread_cancel(tid_timer);	
		        //if (s != 0)
              	//		err_sys("pthread_cancel ERROR");
			printf("Have executed the cancle thread...\n");

			strcpy(temp_buff,r_buff);
			strcpy(cli_address, inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)));
			cli_port = ntohs(cliaddr.sin_port);
			printf("Debug: Received data from the neighbor:%s with port: %d\n And route table is: %s\n\n\n", cli_address, cli_port, temp_buff);
			printf("Calling route compare function...\n");
			up_route_table  = route_compare(temp_buff);
			printf("Retuen from route_compare is:\n%s\n", up_route_table);
			Pthread_mutex_lock(&routing_mutex);
			printf("Updating ROUTING TABLE...\n");
			strcpy(routing_table,up_route_table);
			printf("Verifying routing table...\n%s\n", routing_table);
			Pthread_mutex_unlock(&routing_mutex);
			
//			printf("Retuen from route_compare is:%s\n", route_compare(temp_buff));
//			printf("Return value of route_compare is:%d\n",r);
			printf("Perform pthread join for the threads...\n");
			pthread_join(tid_timer, NULL);
	}
  }

void * timer_180(char v_ip[20])
{	
	printf("Inside THREAD timer_80...\n");
	int  length, s;
        long node_ip;
        struct sockaddr_in servaddr; //, cliaddr;
//        length = sizeof(cliaddr);
	char  buff[MAXLINE], *up_route_table;
	struct timeval tv = {80, 0};
        tv.tv_sec = 80;
	tv.tv_usec = 0;
	printf("START OF 80 seconds TIMER...\n");
	s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	printf("Will be waiting on select function for 80 sec time out...\n");
        if (s != 0)
               err_sys("pthread_setcancelstate ERROR");
	if (select(0, NULL, NULL, NULL, &tv) < 0)		
		err_sys("select error\n");				//Will be in this select state for 180 seconds...

	printf("\n\n\n\n80 sec TIMEOUT\n\n\n\n\n");
//	strcpy(cli_address, inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)));
//	cli_port = ntohs(cliaddr.sin_port);
	
	printf("Not received data from the neighbor:%s\nModify the routing table to reflect infinit (16)\n",v_ip);
	up_route_table = route_modifier(v_ip,16);
	printf("Retuen from route_modifier is:\n%s\n", up_route_table);
	Pthread_mutex_lock(&routing_mutex);
	printf("Updating ROUTING TABLE...\n");
	strcpy(routing_table,up_route_table);
	printf("Verifying routing table...\n%s\n", routing_table);
	Pthread_mutex_unlock(&routing_mutex);
}

  char * route_modifier (char node[20], int value)
  {
	char route_update[MAXLINE], *return_table = malloc(MAXLINE+1), updated_routing_table[MAXLINE], cost[20], node_copy[20];
	char  *n_holder1, *n_holder2, *n_holder3, *context;
	int i;
	struct route1 route17[all_nodes_count];

	printf("Inside route MODIFIER... Node which is not responding is:%s Its cost wilol be set to 16\n\n\n", node);
	strcpy(node_copy,node);
	printf("Value of the node_copy:%sand the node:%s",node_copy,node);
        Pthread_mutex_lock(&routing_mutex);
        printf("Updating ROUTING TABLE...\n");
	strcpy(route_update,routing_table);
        printf("Verifying routing table...\n%s\n", routing_table);
        Pthread_mutex_unlock(&routing_mutex);


	n_holder1 = strtok_r(route_update, "\t", &context);                             //Handling strtok to run in loop...
        if (n_holder1 != NULL)
                strcpy(route17[0].dest, n_holder1);
//        printf("dest:%s\t",route17[0].dest);

        for (i = 0; i < all_nodes_count; i++)
        {
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route17[i].cost = atoi(cost);
 //               printf("cost:%d\t",route17[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route17[i].hop,n_holder3);
   //             printf("hop:%s\n",route17[i].hop);
                n_holder1 = strtok_r(NULL, "\t", &context);
                if (n_holder1 == NULL)
                        continue;
                strcpy(route17[i+1].dest, n_holder1);
     //           printf("dest:%s\t",route17[i].dest);

        }
	//printf("\n\nValue of routing table from struct...\n");
        //printf("Dest\t\t\tCost\t\t\thop\n");
        for (i = 0; i < all_nodes_count; i++)
        {
                //printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
        }
	printf("\n\n\n Debug: WILL COMPARE value of route17[i].cost:%swith the node%s",route17[i].dest, node);
	for (i = 0; i < all_nodes_count; i++)
        {

		if(strcmp (route17[i].dest, node) == 0)
		{
			printf("Updating the neighbor %s cost  which is not reachable from past 180 secs to infinity which is 16...\n", route17[i].dest);
			route17[i].cost = 16;	
		}

	}
	
	
	for (i = 0; i < all_nodes_count; i++)
        {

		if(strcmp (route17[i].hop, node) == 0)
		{
			printf("Updating %s cost which has the hop has %s which is not reachable from past 180 secs to infinity which is 16...\n", route17[i].dest,route17[i].hop);
			route17[i].cost = 16;	
		}

	}

//Struct to string copy
	bzero(updated_routing_table, MAXLINE);
        for (i = 0; i < all_nodes_count; i++)
        {
//                printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
                strcat(updated_routing_table, route17[i].dest);
                strcat(updated_routing_table, "\t");
                sprintf(cost,"%d",route17[i].cost);
                strcat(updated_routing_table,cost);
                strcat(updated_routing_table, "\t");
                strcat(updated_routing_table, route17[i].hop);
                strcat(updated_routing_table, "\n");
        }
        printf("Value of the routing table in string format is:%s",updated_routing_table);
        strcpy(return_table,updated_routing_table);
	
	printf("Completed route update to infinity...\n");
	return(return_table);
  }

  char * route_compare( char rec_r_table[MAXLINE])
  {
	char temp_r_table[MAXLINE], temp_rec_r_table[MAXLINE];
	char  *n_holder1, *n_holder2, *n_holder3, *context, cost[20], updated_routing_table[MAXLINE], *return_table = malloc(MAXLINE+1);
	struct route1 route17[all_nodes_count], route18[all_nodes_count], routecopy_17[all_nodes_count];
	int i, j, k;

	printf("Inside route_compare, Print CURRENT route table:%s\n",routing_table);
//	printf("Inside route compare...\n");
//	printf("Before first copy\n");
	Pthread_mutex_lock(&routing_mutex);
        printf("Updating ROUTING TABLE...\n");
	strcpy(temp_r_table, routing_table);
        printf("Verifying routing table...\n%s\n", routing_table);
        Pthread_mutex_unlock(&routing_mutex);

//	printf("Current Routing table...:\n%s\n",temp_r_table);
//	printf("Before 2nd strcpy...\n");
	strcpy(temp_rec_r_table, rec_r_table);
	printf("Routing table from neighbor...:\n%s\n",temp_rec_r_table);
	printf("All nodes count is:%d\n\n",all_nodes_count);

	n_holder1 = strtok_r(temp_r_table, "\t", &context);				//Handling strtok to run in loop...
	if (n_holder1 != NULL)
		strcpy(route17[0].dest, n_holder1);
//	printf("dest:%s\t",route17[0].dest);

//String to struct
        for (i = 0; i < all_nodes_count; i++)
        {
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route17[i].cost = atoi(cost);
//		printf("cost:%d\t",route17[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route17[i].hop,n_holder3);
//		printf("hop:%s\n",route17[i].hop);
		n_holder1 = strtok_r(NULL, "\t", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(route17[i+1].dest, n_holder1);
//		printf("dest:%s\t",route17[i].dest);
		
        }

	n_holder1 = strtok_r(temp_rec_r_table, "\t", &context);				//Handling strtok to run in loop...
	if (n_holder1 != NULL)
		strcpy(route18[0].dest, n_holder1);
//	printf("dest:%s\t",route18[0].dest);

        for (i = 0; i < all_nodes_count; i++)
        {

//		printf("Inside 2nd for\n");
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route18[i].cost = atoi(cost);
//		printf("cost:%d\t",route18[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route18[i].hop,n_holder3);
//		printf("hop:%s\n",route18[i].hop);
		n_holder1 = strtok_r(NULL, "\t", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(route18[i+1].dest, n_holder1);
//		printf("dest:%s\t",route18[i].dest);
        }
	

	//printf("\n\nValue of Current routing table from struct...\n");
	//printf("Dest\t\t\tCost\t\t\thop\n");
	for (i = 0; i < all_nodes_count; i++)
        {
		//printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
	}
	//printf("\n\nValue of newly received routing table from struct...\n");
	//printf("Dest\t\t\tCost\t\t\thop\t\n");
	for (i = 0; i < all_nodes_count; i++)
        {
		//printf("%s\t%d\t%s\t\n",route18[i].dest, route18[i].cost, route18[i].hop);
	}
//Comparing route table with the received data...
	memcpy(&routecopy_17, &route17, sizeof(route17));

//Bellman Ford Algorithm calculation...
	printf("\n\nBellman Ford Algorithm calculation....\n");
        for (i= 0; i< all_nodes_count; i++)
        {
                printf("Check %d: switch: %s having cost:%d and hop:%s with each dest value of received route node:", i, routecopy_17[i].dest, routecopy_17[i].cost, routecopy_17[i].hop);
                for (j=0; j< all_nodes_count ; j++)
                {
                        printf("%s\n",route18[j].dest);
		
	
                        if((strcmp (routecopy_17[i].dest, route18[j].dest) == 0) && routecopy_17[i].cost !=0 && route18[j].cost ==0)  //Checking neighbor node
				routecopy_17[i].cost =1; 
                        if((strcmp (routecopy_17[i].dest, route18[j].dest) == 0) && routecopy_17[i].cost !=0 && route18[j].cost !=0)  //Checking each node of original route to received route
                        {
                                printf("Debug: Node match... Verify cost...\n");
                                if(routecopy_17[i].cost <= route18[j].cost +1)    //Verifying cost of the original is less than than the received
                                        break;
				for(k=0; k< all_nodes_count; k++)
				{
					if(route18[k].cost == 0)
					{
			//		 	if(routecopy_17[i].cost > (route18[j].cost + 1))
			//			{
                                       			printf("Debug: Updating route... of %s with cost %d and hop %s\n", route17[i].dest, (route18[j].cost+1), route18[k].dest);
							route17[i].cost = route18[j].cost+1;
							strcpy(route17[i].hop, route18[k].dest);
			//			}
					}
							  
//                                        route17[i].cost = route18[j].cost + ;
//					route17[i].hop = route18[j].dest;
//                                        strcpy(route17[i].hop,route18[j].hop);
                                }
                        }
                }

        }
        printf("\n\nValue of routing table from struct... after route compare...\n");
        printf("Dest\tCost\thop\n");
        bzero(updated_routing_table, MAXLINE);
        for (i = 0; i < all_nodes_count; i++)
        {
//                printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
                strcat(updated_routing_table, route17[i].dest);
                strcat(updated_routing_table, "\t");
		sprintf(cost,"%d",route17[i].cost);
                strcat(updated_routing_table,cost);
                strcat(updated_routing_table, "\t");
                strcat(updated_routing_table, route17[i].hop);
                strcat(updated_routing_table, "\n");
        }

        //printf("Value of the routing table in string format is:%s",updated_routing_table);   
//	strcpy(routing_table,updated_routing_table);
	strcpy(return_table,updated_routing_table);

        printf("Value of the routing table in string format is:%s",return_table);   
	return(return_table);	
  }


	
  char * reverse_mapper(char message[MAXLINE])
  {
	printf("Inside Reverse Mapper, value of the Virtual IP which should be mapped is: %s\n", "self");

	char temp_message[MAXLINE], *temp_return = malloc(MAXLINE+1);
	char  *n_holder1, *n_holder2, *n_holder3, *context, cost[20];
	struct route1 route17[all_nodes_count];
	int i; 

	printf("Print route received table:%s\n",message);
//	printf("Before first copy\n");
	strcpy(temp_message, message);
//	printf("Before 2nd strcpy...\n");
	printf("All nodes count is:%d\n\n",all_nodes_count);

	n_holder1 = strtok_r(temp_message, "\t", &context);				//Handling strtok to run in loop...
	if (n_holder1 != NULL)
		strcpy(route17[0].dest, n_holder1);
	printf("dest:%s\t",route17[0].dest);

//String to struct
        for (i = 0; i < all_nodes_count; i++)
        {
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route17[i].cost = atoi(cost);
//		printf("cost:%d\t",route17[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route17[i].hop,n_holder3);
//		printf("hop:%s\n",route17[i].hop);
		n_holder1 = strtok_r(NULL, "\t", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(route17[i+1].dest, n_holder1);
//		printf("dest:%s\t",route17[i].dest);
		
        }
	for (i = 0; i< all_nodes_count; i++)
	{
		if (strcmp("self", route17[i].hop) == 0)
		{
			printf("Value of the map is physical_IP:%s\n",route17[i].dest);
			strcpy(temp_return,route17[i].dest);

		}
	}
	
	return(temp_return);

  }


  char * r_mapper(char switch_name[MAXLINE])
  {
	char name_copy[20], *n_holder1, *n_holder2, *n_holder3, *n_holder4, *context, node1[20], vi_port[20], p_port[20], m_buff[MAXLINE], comment_checker[10], *temp_return = malloc(MAXLINE+1);
	struct m_out mout;

	strcpy(name_copy,switch_name);
	printf("Inside r_Mapper, value of the Virtual IP which should be mapped is: %s\n",name_copy);
	FILE *f_map =fopen("node.config", "r");                                            //Get the node configurations
	if (f_map == NULL)
	err_sys("fopen error");

	while (!feof(f_map))                                             //Considering there will be max of 16 different nodes
	{
		fgets(m_buff,MAXLINE, f_map);
		//        printf("Value of buff is: %c\n",buff[0]);
		comment_checker[0]= m_buff[0];
		comment_checker[1] = '\0';
		if  (strcmp(comment_checker,"#") == 0)                                             // || strcmp(comment_checker, "520")) Will see if I can drop 520 too
		{
//			printf("Debug: Dropping commented line...\n%s",m_buff);
			continue;
		}

		n_holder1 = strtok_r(m_buff, " ", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(node1,n_holder1);
//		printf("Value of the virtual_IP is: %s\n",node1);
		n_holder2 = strtok_r(NULL, " ", &context);
		if (n_holder2 == NULL)
			continue;
		strcpy(vi_port,n_holder2);
//		printf("Value of the virtual_port is: %s\n",vi_port);
		n_holder3 = strtok_r(NULL, " ", &context);
		if (n_holder3 == NULL)
			continue;
		strcpy(mout.phy_ip,n_holder3);
//		printf("Value of the phy_ip is: %s\n",mout.phy_ip);
		n_holder4 = strtok_r(NULL, " ", &context);
		if (n_holder4 == NULL)
			continue;
		strcpy(p_port,n_holder4);
//		printf("Value of the phy_port is: %s\n",p_port);
		mout.phy_port = atoi(p_port);

		if (strcmp(mout.phy_ip, name_copy) == 0)
		{
			printf("Value of the r_map is virtual_IP:%s phy_ip: %s, phy_port: %d\n", node1, mout.phy_ip, mout.phy_port); 
			strcpy(temp_return,node1);
			return(temp_return);
		}
	}
  	exit(-1);
  }
