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
  int sockfd, portno, n, clilen, offset_value, num_bytes, bytes_read, num_nodes=0, all_nodes_count, direct_nodes, i,j;
  long file_size;
  int flag_node_present, flag_direct_node_present;
  struct sockaddr_in servaddr, cliaddr;
  char buff[MAXLINE], file_name[MAXLINE], source_switch[20], *node_holder1, *node_holder2, node1[MAXLINE], node2[MAXLINE], comment_checker[10], all_nodes[16][MAXLINE], direct_connect_nodes[16][MAXLINE] , *context, routing_table[MAXLINE], temp_msg[MAXLINE];
  char completion_string[20] = "~0000COMPLETED0000~";
  struct stat stat_buff;

  time_t mytime;
  mytime = time(NULL);
  printf("\nCurrent time: %s\n",ctime(&mytime));


  if (argc!=3)
      err_quit("\n\nusage: rip <switch_ip>  <port_number>\n");
  portno = atoi(argv[2]);                                                               //Using asci to integer to get the port number
  printf("Debug: The port number entered is: %d\n", portno);

  /*Handling neighbour.config file to get the node topology to create the data structure */

  FILE *f_nodes =fopen("neighbor.config", "r");                                            //Get the node configurations
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



  sockfd = Socket(AF_INET, SOCK_DGRAM,0);
  bzero( &servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(portno);

  Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

  clilen = sizeof(cliaddr);   

  return(0);
} 
