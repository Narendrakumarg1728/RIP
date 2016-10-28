/*
*   File:      proxy_server.c 
*
*   Purpose:    This is a skeleton file for a server.
*/
#include "unp.h"
//#include "myunp.h"
#include <time.h>
//Global Declarations
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
int portno, sockfd_r;
int all_nodes_count;

int route_send (char route_table[MAXLINE], char d_connect_nodes[16][MAXLINE], int direct_nodes) ;
void * route_recv (char d_connect_node[MAXLINE]);
int route_compare( char c_buff[]);
int route_modifier (char node[], int value);
struct route string_to_struct_route(char r_table[MAXLINE]);
void sig_chld(int);

int main(int argc, char **argv)
{
  int num_nodes=0, direct_nodes, i, j, t;
  int flag_node_present, flag_direct_node_present;
  char buff[MAXLINE], source_switch[20], *node_holder1, *node_holder2, node1[MAXLINE], node2[MAXLINE], comment_checker[10], all_nodes[16][MAXLINE], direct_connect_nodes[16][MAXLINE] , *context, temp_msg[MAXLINE];
//  char completion_string[20] = "~0000COMPLETED0000~";
  time_t mytime;
  mytime = time(NULL);
  printf("\nCurrent time: %s\n",ctime(&mytime));
  pid_t childpid;

/*Declaring  threadids... */
  pthread_t tid[direct_nodes];

  int length ;
  struct sockaddr_in servaddr;

  struct route 
  {
	char dest[20];
	int cost;
	char hop[20];
  };
  struct route route16[16];


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
  while (!feof(f_nodes) && num_nodes !=160)                                             //Considering there will be max of 16 different nodes
  {
        fgets(buff,MAXLINE, f_nodes);
//        printf("Value of buff is: %c\n",buff[0]);
        comment_checker[0]= buff[0];
        comment_checker[1] = '\0';
  	if  (strcmp(comment_checker,"#") == 0 || strcmp(comment_checker, "1"))
	{
		printf("Debug: Dropping commented line...\n%s",buff);
		continue;
	}
        
//        fscanf(f_nodes,"%s", node);
        node_holder1 = strtok_r(buff, " ", &context);
        strcpy(node1,node_holder1);
        ++num_nodes;
        node_holder2 = strtok_r(NULL, " ", &context);
        if (node_holder2 == NULL)
		continue;
        ++num_nodes;
	printf("Debug: Number of nodes is: %d\n", num_nodes);
//        printf("\n\n\n\nValue of node_holder2 value is: %s\n\n\n\n", node_holder2);
        strcpy(node2, node_holder2);

        printf("Value of switches in node.config file are %s     %s\n", node1, node2);
	if (all_nodes_count == 0)
	{
		printf("Initializing the all_nodes array with node1 and node 2 values\n");
		strcpy(all_nodes[all_nodes_count],node1);
		++all_nodes_count;
		strcpy(all_nodes[all_nodes_count],node2);
		++all_nodes_count;
	}

        for (i = 0 ; i < 16; i++)             //Comparing node1 with array of all nodes if it is not there appending to the array list...
	{
		printf("Debug: Comparing node1:%s with all_ndes_array[%d]:%s\n",node1, i, all_nodes[i]);
		if (strcmp (node1, all_nodes[i]) != 0)
		{
			printf("Debug Not found in array at value %d\n", i);
			flag_node_present = 0;
			continue;
		}
		printf("Compare passed...\n");
		flag_node_present = 1;
		break;
	}
	if (flag_node_present == 0)
	{	
		printf("Appending the node to tha all-nodes_array");
		strcpy (all_nodes[all_nodes_count], node1);
               	++all_nodes_count;
	}
        flag_node_present = 0;
	for (i = 0 ; i <= 16; i++)           //Comparing node1 with array of all nodes if it is not there appending to the array list...
        {
 		printf("Debug: Comparing node2:%s with all_ndes_array[%d]:%s\n",node2, i, all_nodes[i]);
		
                if (strcmp (node2, all_nodes[i]) != 0)
                {
                        printf("Debug Not found in array at value %d\n", i);
                        flag_node_present = 0;
                        continue;
                }
		printf("Compare passed...\n");
                flag_node_present = 1;
		break;
	}

	if (flag_node_present == 0)
	{
		printf("Appending the node to tha all-nodes_array at all_nodes_count %d", all_nodes_count);
		strcpy(all_nodes[all_nodes_count], node2);
		++all_nodes_count;
	}


        if ((strcmp(source_switch, node1) == 0) || (strcmp(source_switch, node2) == 0))
	{ 
		printf("**************************Debug:Switch %s or %s  is directly connected to  source switch %s... Check which one is it...********************\n", node1, node2, source_switch);
		if(strcmp(source_switch, node1) == 0)
		{
			printf("Other node is node2: %s\n",node2);
			strcpy(direct_connect_nodes[direct_nodes] , node2);
			++direct_nodes;
		}
		else
		{
			printf("Other node is node1: %s\n", node1);
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
        printf("Value of i is %d value of all_nodes[i] is %s", i, all_nodes[i]);
	strcpy(route16[i].dest, all_nodes[i]);
	printf("Debug: Value of route16[i].dest is %s\n", route16[i].dest);
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

//Handling node.config file
  mapp= mapper(source_switch);

  printf("\n*****Source switch Ip and port address from the node.config file is: %s and %d\n*****\n", mapp.phy_ip, mapp.phy_port); 


  portno = mapp.phy_port;                                                               //Using asci to integer to get the port number
  printf("The port number on which the switch will be running is: %d\n", portno);
  Signal(SIGCHLD, sig_chld);      							//SIGCHLD to handel zambie child


  if ( (childpid = fork()) != 0 )           				//Parent will call function which will run in loop to send data every 30 seconds...
  {									//And child process will handle threads which will receive data from neighbors, 180secs timeout will send infinity value...

  	route_send(routing_table, direct_connect_nodes, direct_nodes);
  }
 
//Initial socket creation for receiving

  length= sizeof(servaddr);
  if ( (sockfd_r = socket(AF_INET, SOCK_DGRAM,0)) < 0)                                //creating DGRAM socket
       	err_sys("socket error\n");
  printf ("Debug: Verifying the sockfd_r value for debugging:%d \n ",sockfd_r);
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Will use the port numbber mapped for this switch to be listning at port no: %d", portno);
  servaddr.sin_port = htons(portno);
  Bind(sockfd_r, (SA *) &servaddr, sizeof(servaddr));


  
  for (t=0 ; t < direct_nodes; t++)
  {
//	strcpy(th_direct[t].dest,direct_connect_nodes[t]);
//	strcpy(th_direct[t].r_table, routing_table);
	printf("Calling threads to receive route update from neighbor:%s", direct_connect_nodes[t]);	
  	if (pthread_create(&tid[t], NULL, &route_recv, (void *) source_switch))
        	{
                	fprintf(stderr, "Unable to craate thread_%d thread \n",t);
                	exit(1);
        	}
                     
  }


  printf("Debug: Back to main func...\n");
 
  for (t= 0; t < direct_nodes; t++)
  {
	printf("Inside loop to perform pthread join for the threads...\n");
	pthread_join(tid[t], NULL);
  }

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
	
  int route_send (char route_table[MAXLINE], char d_connect_nodes[16][MAXLINE], int d_nodes)
  {
  	time_t mytime;
	for ( ; ; )
	{

//	char node1[MAXLINE],
		int sockfd, i, length;
		struct sockaddr_in servaddr;
		struct m_out m; 
		length = sizeof(servaddr);
		for (i = 0; i < d_nodes; i++)
		{
//			printf("Value of node to which the route table to be sent is: %s\n", d_connect_nodes[i]);
			m= mapper(d_connect_nodes[i]);
			if ( (sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)  				//creating DGRAM socket
				err_sys("socket error\n");
//			printf ("Debug: Verifying the sockfd value for debugging:%d \n ",sockfd);
			bzero(&servaddr,sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			if (inet_pton(AF_INET, m.phy_ip, &servaddr.sin_addr) <= 0)           		/*Getting server address for the first server */
			      err_quit("inet_pton error for %s\n", m.phy_ip);
			servaddr.sin_port = htons(m.phy_port);
//			printf("Debug: map to which route will be sent is virtual_IP: %s to m.phy_ip is: %sand m.phy_port is %d:\n", d_connect_nodes[i], m.phy_ip, m.phy_port);
//			printf("\nTime to send routing update to the neighbors...\n");
			sendto(sockfd,route_table,strlen(route_table), 0, (SA *) &servaddr, length);
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


  void
  sig_chld(int signo)
  {
        pid_t   pid;
        int             stat;

        while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                printf("child %d terminated\n", pid);
        }
        return;
  }



 void * route_recv (char d_connect_node[])
  {	
	printf("\n\n\n\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Inside thread~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n\n\n\n\n");
	int  length, max_fd, rv, cli_port;
	struct m_out m_recv;
	struct sockaddr_in servaddr, cliaddr;
        struct timeval tv = {180, 0};   // sleep for 180 seconds!
	fd_set rset;
	char r_buff[MAXLINE], cli_address[MAXLINE], temp_buff[MAXLINE], buff[MAXLINE];
	FD_ZERO(&rset);
	length = sizeof(cliaddr);
	for ( ; ; )
	{
		printf("\n\n\n\n\n\n\n\n\nSTART of for-LOOP\n\n\n\n\n\n\n");
		printf("\n\n\nValue of the direct connect node is:%s\n\n\n", d_connect_node);
		m_recv = mapper(d_connect_node);
		max_fd=sockfd_r+1;
//                if (inet_pton(AF_INET, m_recv.phy_ip, &servaddr.sin_addr) <= 0)                      /*Getting server address for the first server */
//                	err_quit("inet_pton error for %s\n", m_recv.phy_ip);

                printf("Debug: THREAD_where RECVFROM... Value of virtual_ip %s to m_recv.phy_ip is: %s and m_recv.phy_port is %d\n", d_connect_node, m_recv.phy_ip, m_recv.phy_port);

                tv.tv_sec = 180;
		tv.tv_usec = 0;
		FD_SET(sockfd_r, &rset);

		printf("Waiting on select...\n");
		if ( (rv =select(max_fd, &rset , NULL, NULL, &tv)) < 0)
                	err_sys("select error\n");
		printf("Return value of select is: %d", rv);

		if (FD_ISSET (sockfd_r, &rset))
		{
			printf("\n\n\n\nBefore recv\n\n\n\n\n"); 
			printf("Debug... in Thread function. Will execute recvfrom function...\n");
			recvfrom(sockfd_r, r_buff, MAXLINE, 0, (SA *) &cliaddr, &length);
			strcpy(temp_buff,r_buff);
			strcpy(cli_address, inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)));
			cli_port = ntohs(cliaddr.sin_port);
			printf("Debug: Received data from the neighbor:%s with port: %d\n And route table is: %s\n\n\n", cli_address, cli_port, temp_buff);
			printf("Calling route compare function...\n");
			int r = route_compare(temp_buff);
			printf("Return value of route_compare is:%d\n",r);
		}
		else
		{
			printf("\n\n\n\n180 sec TIMEOUT\n\n\n\n\n");
			printf("Not received data from the neighbor:%s with in 180 secs \nModify the routing table to reflect infinit (16)\n",d_connect_node);
			route_modifier(d_connect_node,16);
		}
	}
  }

  int route_modifier (char node[MAXLINE], int value)
  {
	char route_update[MAXLINE];
	

	strcpy(route_update,routing_table);

	printf("Inside route modifier...\n");
	return(0);
  }

  int route_compare( char rec_r_table[MAXLINE])
  {
	char temp_r_table[MAXLINE], temp_rec_r_table[MAXLINE], updated_routing_table[MAXLINE];
	char  *n_holder1, *n_holder2, *n_holder3, *context, cost[20];
	struct route1 route17[all_nodes_count], route18[all_nodes_count];
	int i,j;
	printf("Print route table:%s\n",routing_table);
	printf("Inside route compare...\n");
	printf("Before first copy\n");
	strcpy(temp_r_table, routing_table);
	printf("Current Routing table...:\n%s\n",temp_r_table);
	printf("Before 2nd strcpy...\n");
	strcpy(temp_rec_r_table, rec_r_table);
	printf("Routing table from neighbor...:\n%s\n",temp_rec_r_table);
	printf("All nodes count is:%d\n\n",all_nodes_count);

	n_holder1 = strtok_r(temp_r_table, "\t", &context);				//Handling strtok to run in loop...
	if (n_holder1 != NULL)
		strcpy(route17[0].dest, n_holder1);
	printf("dest:%s\t",route17[0].dest);

        for (i = 0; i < all_nodes_count; i++)
        {
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route17[i].cost = atoi(cost);
		printf("cost:%d\t",route17[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route17[i].hop,n_holder3);
		printf("hop:%s\n",route17[i].hop);
		n_holder1 = strtok_r(NULL, "\t", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(route17[i+1].dest, n_holder1);
		printf("dest:%s\t",route17[i].dest);
		
        }

	n_holder1 = strtok_r(temp_rec_r_table, "\t", &context);				//Handling strtok to run in loop...
	if (n_holder1 != NULL)
		strcpy(route18[0].dest, n_holder1);
	printf("dest:%s\t",route18[0].dest);

        for (i = 0; i < all_nodes_count; i++)
        {

		printf("Inside 2nd for\n");
                n_holder2 = strtok_r(NULL, "\t", &context);
                if (n_holder2 == NULL)
                        continue;
                strcpy(cost, n_holder2);
                route18[i].cost = atoi(cost);
		printf("cost:%d\t",route18[i].cost);

                n_holder3 = strtok_r(NULL, "\n", &context);
                if (n_holder3 == NULL)
                        continue;
                strcpy(route18[i].hop,n_holder3);
		printf("hop:%s\n",route18[i].hop);
		n_holder1 = strtok_r(NULL, "\t", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(route18[i+1].dest, n_holder1);
		printf("dest:%s\t",route18[i].dest);
        }
	

	printf("\n\nValue of routing table from struct...\n");
	printf("Dest\tCost\thop\n");
	for (i = 0; i < all_nodes_count; i++)
        {
		printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
	}
	printf("\n\nValue of newly received routing table from struct...\n");
	printf("Dest\t\t\tCost\t\t\thop\t\n");
	for (i = 0; i < all_nodes_count; i++)
        {
		printf("%s\t%d\t%s\t\n",route18[i].dest, route18[i].cost, route18[i].hop);
	}

//Comparing route table with the received data...

  	for (i= 0; i< all_nodes_count; i++)
  	{
		printf("Check %d: switch: %s having cost:%d and hop:%s with each dest value of received route node:", i, route17[i].dest, route17[i].cost, route17[i].hop);
		for (j=0; j< all_nodes_count ; j++)
		{
			printf("%s\n",route18[j].dest);
			if((strcmp (route17[i].dest, route18[j].dest) == 0) && route17[i].cost !=0 && route18[j].cost !=0)
			{
				printf("Debug: Node match... Verify cost...\n");
				if(route17[i].cost <= route18[j].cost)
					break;
				if(route17[i].cost > route18[j].cost)
				{
					printf("Debug: Updating route... of %s with cost %d and hop %s from node %s\n",route17[i].dest, route18[j].cost, route18[j].hop);
					route17[i].cost = route18[j].cost;
					strcpy(route17[i].hop,route18[j].hop);
				}
			}
		}

 	}
	printf("\n\nValue of routing table from struct... after route compare...\n");
	printf("Dest\tCost\thop\n");
	bzero(updated_routing_table, MAXLINE);
	for (i = 0; i < all_nodes_count; i++)
        {
		printf("%s\t%d\t%s\n",route17[i].dest, route17[i].cost, route17[i].hop);
		strcat(updated_routing_table, route17[i].dest);
		strcat(updated_routing_table, "\t");
		strcat(updated_routing_table, route17[i].cost);
		strcat(updated_routing_table, "\t");
		strcat(updated_routing_table, route17[i].hop);
		strcat(updated_routing_table, "\n");
	}

	printf("Value of the routing table in string format is:%s",updated_routing_table); 
	return(updated_routing_table);	
  }

	
