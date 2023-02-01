/*********************** SERIAL PORT COMMUNICATION CLIENT ***********************/
// Author: Vangelis Bakas //
// Last Edited: 1/2/2023 //
// Task: Create an AT-COMMAND based serial port communication system //

/* This program acts as the client for the system. The user directly interacts
with the client via the command line interface and its main two functions are:
	1. The 'help' command which prints all the available AT-COMMANDS.
	2. The AT-COMMAND request to the server. 

The client waits for a server response before the user can enter the next command. In case
of an insertion command, the help menu is updated with the new info as well. Finally, if
a quit command is entered, the client terminates the server as well.
*/

// Libraries
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "commonfunc.h"

// Preprocessor 
#define MAX_STRING 512
#define MAX_FILENAME 12
#define MAX_ENTRIES 30
#define INITIAL_REGS 2
#define MAX_RESPONSE_SIZE 20

// Global for storing info for menu in order to be updated after each insertion //
char *menu[MAX_STRING] = {"~ Available AT Commands:", 
"~ REG1: Read the 1st register's value -> Response: <int>", 
"~ REG1=?: Read the list of all allowed values for 1st register",
"~ REG1=<int>: Write the provided integer to the 1st register -> Response: OK|InvalidInput",
"~ REG2: Read the 2nd register's value -> Response: <int>",
"~ REG2=?: Read the list of all allowed values for 2nd register",
"~ REG2=<int>: Write the provided integer to the 2nd register -> Response: OK|InvalidInput"};
int last_entry = 7; // the last entry of the menu array, will be incremented in order to add more entries to the menu //
// when new registers are added //
int reg_count = INITIAL_REGS; // the number of registers; default value is the initial number of registers; //
// will be updated after each new register insertion //
char insertproperties[MAX_STRING]; // string to be used for the menu update to avoid segfaults //

// ********** print_help ********** //
// the function to print the menu with the available AT commands to the user //
void print_help()
{
  // print menu entries //
  for (int i = 0; i < MAX_ENTRIES; i++)
    {
      if (menu[i] != NULL)
        {
          printf("%s\n", menu[i]);
        }
    } 
}

// ********** send_request ********** //
// the function to send the request to the server for processing //
void send_request(int fd, char *request)
{
  int bytes_read; // 'read' system call result //
  char server_response[MAX_STRING] = {'\0'}; // server response to print //

  // send request to server and wait for response //
  my_write(fd, request, MAX_RESPONSE_SIZE);
  wait_for_response(fd, 0); 

  bytes_read = my_read(fd, server_response, sizeof(server_response));
  if (bytes_read)
    {
      printf("%s\n", server_response); // print response from server //
    }
}

// ********** update_menu ********** //
// function to update the menu after each register insertion //
void update_menu(char *request)
{
  char *token = NULL; // to break the request in order to get the separate info //
  char reg_bounds[MAX_STRING]; // new reg bounds //
  char reg_pos[2]; // new reg position on list //
  char reg_string[MAX_STRING] = {"REG"}; 
  char temp[MAX_STRING];

  // get the first token from the request - 'insert' //
  token = strtok(request,"+");

  // second token: new register value - no need to use it, since the server handles reg values //
  token = strtok(NULL, "+");

  // third token: new register bounds //
  token = strtok(NULL, "+");
  strcpy(reg_bounds, token);

  sprintf(reg_pos, "%d", reg_count); // convert the last reg position to a string //
  strcat(reg_string, reg_pos); // add position number to "REG" //

  // add the 3 new help lines //
  sprintf(temp, "~ %s: Read the value of register %d -> Response: <int>", reg_string, reg_count);
  menu[last_entry] = strdup(temp);

  if (menu[last_entry] == NULL)
    {
      fprintf(stderr, "ERROR: Not enough memory to update the menu\n");
    }

  sprintf(temp, "~ %s=?: Read the list of all allowed values for register %d", reg_string, reg_count);
  menu[last_entry + 1] = strdup(temp);

  if (menu[last_entry + 1] == NULL)
    {
      fprintf(stderr, "ERROR: Not enough memory to update the menu\n");
    }
  
  sprintf(temp, "~ %s=<int>: Write the provided integer to register %d -> Response: OK|InvalidInput", reg_string, reg_count);
  menu[last_entry + 2] = strdup(temp);

  if (menu[last_entry + 2] == NULL)
    {
      fprintf(stderr, "ERROR: Not enough memory to update the menu\n");
    }
  // change the last entry index to the next empty menu position //
  last_entry = last_entry + 3; 
} 

// ********** main program ********** //
int main(int argc, char *argv[])
{
	int fd; // file descriptor for the serial port //
  char filename[MAX_FILENAME]; // filename of the serial port //
  char request[MAX_STRING]; // request to send to the server //
  char response[MAX_STRING] = {'\0'}; // response from the server //

	// check argument count //
	if (argc < 2)
		{
			fprintf(stderr, "ERROR: You must specify a serial port name\n");
			return 1;
		}

  // get serial port name //
  strcpy(filename, argv[1]);

  // test printf for debugging //
  printf("Client port is: %s\n", filename);

	fd = my_open(filename, O_RDWR | O_NOCTTY);
  if (fd < 0)
    {
      fprintf(stderr, "ERROR: Open syscall failed from client\n");
      return 1;
    }

  // set the serial port attributes, i.e baud rate and parity //
  set_interface_attributes(fd, B115200, 0);
  
  // main loop to wait user interactions //
  printf("Enter AT-Command, 'insert+<value>+<bounds>', 'help' or 'quit': \n");
  while (1)
    {
      printf("~ ");
      scanf("%s", request);
      if (strcmp(request, "help") == 0) // if 'help' is entered, print the help menu //
        {
          print_help();
        }
      else if (strcmp(request, "quit") == 0)
        {
          send_request(fd, request); // kill the server //
          break; // if 'quit' is entered, close the client //
        }
      else if(strncmp(request, "insert", 6) == 0)
        {
          // 'insert' requests are set in this statement, because we need to update //
          // the help menu after the insertion //
          send_request(fd, request);
          reg_count++; // update reg count since a new register was inserted //
          strcpy(insertproperties, request); // copy the request to a temp string, to avoid segfault //
          update_menu(insertproperties);
          printf("~ Register inserted, help menu updated\n");
        }
      else
        {
          // normal AT-Command to send to server //
          send_request(fd, request);
        }
    }

  // close the serial port //
  my_close(fd);

	return 0;
}
