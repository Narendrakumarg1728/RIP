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
char routing_table[MAXLINE];

int route_send (char route_table[MAXLINE], char d_connect_nodes[16][MAXLINE], int direct_nodes);

int main(int argc, char **argv)
{
  int portno, num_nodes=0, all_nodes_count, direct_nodes, i,j;
  int flag_node_present, flag_direct_node_present;
//  struct sockaddr_in servaddr;
  char buff[MAXLINE], source_switch[20], *node_holder1, *node_holder2, node1[MAXLINE], node2[MAXLINE], comment_checker[10], all_nodes[16][MAXLINE], direct_connect_nodes[16][MAXLINE] , *context, routing_table[MAXLINE], temp_msg[MAXLINE];
//  char completion_string[20] = "~0000COMPLETED0000~";
  time_t mytime;
  mytime = time(NULL);
  printf("\nCurrent time: %s\n",ctime(&mytime));
  pid_t childpid;

  if (argc!=2)
      err_quit("\n\nusage: rip <switch_ip>\n");

  /*Handling neighbour.config file to get the node topology to create the data structure */

  FILE *f_nodes =fopen("neighbor.config", "r");                                            //Get the neighbor configurations
  if (f_nodes == NULL)
      err_sys("fopen error");

  strcpy (source_switch, argv[1]);

  all_nodes_count =0;
  direct_nodes=0;
  struct route 
  {
	char dest[20];
	int cost;
	char hop[20];
  };
  struct threads_param
  {
	char dest[20];
	char r_table[MAXLINE];
  };
 
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
  printf("Done with node config data collection...\n");
  
  printf("Number of nodes in the network are:%d",all_nodes_count );
  struct route route16[all_nodes_count];
  struct threads_param th_direct[direct_nodes];


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
	strcat(routing_table,"	");
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
	strcat(routing_table,"	");
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
  printf("Debug: The port number entered is: %d\n", portno);

  if ( (childpid = fork()) == 0 )           						//Child process to send data every 30 seconds...
  {

  	route_send(routing_table, direct_connect_nodes, direct_nodes);
  }
  
  
/*  for (t=0 ; t < direct_nodes; t++)
  {
//	strcpy(th_direct[t].dest,direct_connect_nodes[t]);
//	strcpy(th_direct[t].r_table, routing_table);
	
	pthread_create(&tid[t],NULL, route_send,                     */


  printf("Debug: Back to main func...\n");

  return(0);
} 
















  struct m_out mapper(char switch_name[MAXLINE])
  {
	char name_copy[20], *n_holder1, *n_holder2, *n_holder3, *n_holder4, *context, node1[20], vi_port[20], p_port[20], m_buff[MAXLINE], comment_checker[10];
	struct m_out mout;

	strcpy(name_copy,switch_name);
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
			printf("Debug: Dropping commented line...\n%s",m_buff);
			continue;
		}

		//        fscanf(f_nodes,"%s", node);
		n_holder1 = strtok_r(m_buff, " ", &context);
		if (n_holder1 == NULL)
			continue;
		strcpy(node1,n_holder1);
		n_holder2 = strtok_r(NULL, " ", &context);
		if (n_holder2 == NULL)
			continue;
		strcpy(vi_port,n_holder2);
		n_holder3 = strtok_r(NULL, " ", &context);
		if (n_holder3 == NULL)
			continue;
		strcpy(mout.phy_ip,n_holder3);
		n_holder4 = strtok_r(NULL, " ", &context);
		if (n_holder4 == NULL)
			continue;
		strcpy(p_port,n_holder4);
		mout.phy_port = atoi(p_port);

		if (strcmp(node1, name_copy) == 0)
		{
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
			printf("Value of node to which the route table to be sent is: %s\n", d_connect_nodes[i]);
			m= mapper(d_connect_nodes[1]);
			if ( (sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)  				//creating DGRAM socket
				err_sys("socket error\n");
			printf ("Debug: Verifying the sockfd value for debugging:%d \n ",sockfd);
			bzero(&servaddr,sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			if (inet_pton(AF_INET, m.phy_ip, &servaddr.sin_addr) <= 0)           		/*Getting server address for the first server */
			      err_quit("inet_pton error for %s\n", m.phy_ip);
			servaddr.sin_port = htons(m.phy_port);
			printf("Debug: Value of m.phy_ip is: %sand htons(m.phy_port)) is %d:\n", m.phy_ip, (htons(m.phy_port)));
			sendto(sockfd,route_table,strlen(route_table), 0, (SA *) &servaddr, length);
			printf("Sent route table from route_send function to switch %s with route table info:\n %s\n",m.phy_ip, route_table);
		}
  	mytime = time(NULL);
  	printf("\n**********************************\nCurrent time: %s\n**********************************\n",ctime(&mytime));
	struct timeval tv = {30, 0};   // sleep for 30 seconds!
	if (select(0, NULL, NULL, NULL, &tv) < 0) 
		err_sys("select error\n");
	printf("\nTime to send routing update to the neighbors...\n");
	}
	return(-1);
  }



