/*********************** SERIAL PORT COMMUNICATION SERVER ***********************/
// Author: Vangelis Bakas //
// Last Edited: 1/2/2023 //
// Task: Create an AT-COMMAND based serial port communication system //

/* This program acts as the server for the AT-Command communication system.
When a request from the client is received, it performs the following actions:
  1. Checks if the request is valid.
  2. If valid, performs one of the following actions:
    a. If it is an insertion request, adds a new register to the list.
    b. If it is an AT+REG command, it prints the requested register's value.
    c. If it is an AT+REG=? command, it prints the requested register's bounds for accepted values.
    d. If it is an AT+REG=<int> command, it changes the desired register's value with the one entered.

In all the above cases, the server checks for the following errors:
  1. If the AT-Command received is valid.
  2. If the register given exist on the list.
  3. If the desired value is allowed within the bounds of the selected register.

After the server processes each requests, it sends the appropriate answer to the client, or an error
message in case of a failure. 

The list of registers is implemented as a singly linked list with a terminal node. It contains two
registers by default, but more can be added through the client user interface. When the server receives
a termination request from the client, the list is destroyed freeing all the allocated memory.
*/

// Libraries //
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "commonfunc.h"

// Preprocessor //
#define MAX_STRING 512
#define MAX_FILENAME 12
#define BOUNDS_MAX 20
#define REGID_MAX 6
#define MAX_REQ_SIZE 20

// Server Globals // 
registers_t *regs; // the registers list head - also the first register //
int numofregs = 0; // the total number of registers //
char *request; // client request //
char insertion[MAX_STRING]; // insertion properties in case of insert command //

// ********** init_reglist ********** //
// function for the server to create the list of registers. //
// the head is initialised along with a default value.      //
void init_reglist()
{
  // allocate memory for the first register //
  regs = (registers_t *)malloc(sizeof(registers_t));

  if (regs == NULL)
    {
      fprintf(stderr, "Memory allocation error in initialisation\n");
      exit(1);
    }

  // set default values for the first register //
  regs->regvalue = 0;
  strcpy(regs->regid, "REG1");
  strcpy(regs->bounds, "0-16535"); // set number bounds //
  regs->next = NULL; 

  numofregs++; // increase the number of registers //
}

// ********** add_register ********** //
// add a new register to the list at the end //
void add_register(int value, char* bounds) 
{
  registers_t *new, *current; // current node for the traversal //
  char tempid[REGID_MAX] = {"REG"};
  char stringnum[REGID_MAX];

  new = (registers_t *)malloc(sizeof(registers_t));

  if (new == NULL)
    {
      fprintf(stderr, "Memory allocation error in insertion\n");
      exit(1);
    }

  numofregs++; // increase the number of registers //
  sprintf(stringnum, "%d", numofregs);
  strcat(tempid, stringnum);
  strcpy(new->regid, tempid); // create the complete reg id, e.g. REG5 //

  new->regvalue = value; // set new register value and bounds // 
  strcpy(new->bounds, bounds);
  new->next = NULL; // new register next pointer set to NULL //

  current = regs; // assign current to the first register to traverse the list //
  while (current->next != NULL)
    {
      current = current->next; // last node always points to NULL, so traverse the list until the last node is reached //  
    }
  current->next = new; // change last node next pointer to point to the new register //
}

// ********** print_register ********** //
// display selected register value                    //
// returns the result on success and -1               //
// if the desired register does not exist on the list //
int print_register(char *targetid)
{
  registers_t *current; // for list traversal //
  int result; // to store the result for safety //

  // traverse the list to find desired register //
  current = regs; 
  while (current != NULL)
    {
      if (strcmp(current->regid, targetid) == 0)
        {
          printf("%d\n", current->regvalue); // server print the value found - for debugging purposes //
          result = current->regvalue;
          return result;
        }
      current = current->next; 
    }

  return -1; // register is not on the list //
}

// ********** print_bounds ********** //
// function to get the bounds of the desired register //
// in case of success, the bounds are returned as     //
// a string; if the register is not on the list,      //
// NULL is returned                                   //
char *print_bounds(char *targetid)
{
  registers_t *current; // for list travesral //
  char *result = NULL;

  current = regs;
  while (current != NULL)
    {
      if (strcmp(current->regid, targetid) == 0)
        {
          printf("%s\n", current->bounds); // server prints the bounds found - for debugging purposes //
          result = current->bounds;
          return result;
        }
      current = current->next; 
    }

  return NULL;
}

// ********** bound_check ********** //
// check if the number requested is allowed depending on the target //
// register bounds; returns 0 on success, -1 on failure             //
int bound_check(int target_value, char *bounds)
{
  char *distinctcheck = NULL; // to search the string if we have distinct or continuous bounds //
  int distinctnum; // in case of distinct number bounds, to check if the number is allowed //
  char *lower_bound = NULL, *upper_bound = NULL, *token = NULL; // in case of continous bounds, to separate lower from upper bound //
  char helper[MAX_STRING]; // in order not to edit the bounds //

  sprintf(helper, "%s", bounds); // copy the bounds string here, because strtok edits the haystack string //

  distinctcheck = strchr(helper,'|'); // if this character is present, then we have distinct number bounds //

  if (distinctcheck != NULL)
    { 
      token = strtok(helper, "|");
      while (token != NULL)
        {
          if (target_value == atoi(token))
            {
              return 0; // success //
            }

          token = strtok(NULL, "|");
        }

      return -1; // failure, number not allowed //
    }
  else // in any other case, we have continuous bounds //
    { 
      lower_bound = strtok(helper, "-");
      upper_bound = strtok(NULL, "-");

      if (target_value > atoi(lower_bound) && target_value < atoi(upper_bound))
        {
          return 0; // success, number within bounds //
        }
      else
        {
          return -1; // failure, number out of bounds //
        }
    }
}

// ********** replace_value ********** //
// function to replace target register value with the desired one; //
// if it is valid according to the desired regiser bounds          //
// returns 0 on sucess, -1 if the number is invalid and -2         //
// if the register does not exist in the list                      //
int replace_value(int target_value, char *targetid)
{
  int replace_result, boundcheck_result;
  registers_t *current; // for list traversal //

  current = regs;
  while (current != NULL)
    {
      if (strcmp(current->regid, targetid) == 0)
        {
          printf("Register found, commencing replace operation\n");
          boundcheck_result = bound_check(target_value, current->bounds); // call bound_check() to see if the number is valid within the bounds //

          if (boundcheck_result != -1)
            {
              current->regvalue = target_value; // success, change value //
              return 0;
            }
          else
            {
              return -1; // failure, number out of bounds //
            }
        }
      current = current->next;  
    }

  // register is not on the list; return -2 //
  return -2; 
}

// ********** clear_regs ********** //
// clear the regs list and free all the allocated memory //
void clear_regs()
{
  registers_t *current;

  while (regs != NULL)
    {
      current = regs;
      regs = regs->next;

      free(current);
    }
}

// ********** process_insertion ********** //
// function to process an insertion request from the client //
// it takes the request as an argument, parses it and       //
// adds the new register to the list                        //
void process_insertion(char *target_request)
{
  char *token = NULL; // to break the request in order to get the separate info //
  char reg_bounds[MAX_STRING]; // new reg bounds //
  int reg_value; // the new register value to take from the request //

  // get the first token from the request - 'insert' - no use for it here //
  token = strtok(target_request,"+");

  // second token: new register value //
  token = strtok(NULL, "+");
  reg_value = atoi(token);

  // third token: new register bounds //
  token = strtok(NULL, "+");
  strcpy(reg_bounds, token);

  // call the add_register() function to add the register into the lsit //
  add_register(reg_value, reg_bounds);
}

// ********** process_atcommand ********** //
// function to process any AT-command the client sends //
// it takes as argument the client request, parses it, //
// performs error checking and if there are no errors, //
// then executes the desired action; the function      //
// returns 0 if successful, 1 if reg input is invalid, //
// 2 if target number is not valid for the selected    //
// register and 3 if the desired request is not an     //
// accepted AT-command                                 //
int process_atcommand(int fd, char *target_request)
{
  int requested_value; // in case of value change, this is to convert the value from string to int //
  int reg_result = 0; // in case of print, this is to store the reg result //
  int value_swap_check; // to check if the value swap was completed successfully, or the desired value was out of bounds //
  char *reg_bounds = NULL; // to store the target reg bounds //
  char *main_command = NULL, *at_section = NULL, *target_regid = NULL, *target_value = NULL;
  char reg_result_string[2];
  // main_command is the AT+<CMD> part of the command //
  // at_section is the "AT" part of the command - used to get the reg id for searching //
  // target_value gets the value after the '=' in order to perform the desired action //

  if (strncmp(target_request, "AT+REG", 6) == 0)
    {
      main_command = strtok(target_request, "="); // "e.g. AT+REG3" //
      target_value = strtok(NULL, "="); // value after the '=' //

      at_section = strtok(main_command, "+"); // separate the "AT" to get the reg id //
      target_regid = strtok(NULL, "+"); // get the target reg id, e.g. "REG2" // 

      // select the appropriate function depending on the target_value //
      if (target_value == NULL)
        {
          // print reg value - if print returns -1, the selected register is not in the list //
          reg_result = print_register(target_regid);
          if (reg_result != -1)
            {
              printf("Value found %d, sending to client\n", reg_result);
              sprintf(reg_result_string, "%d\n", reg_result);
              my_write(fd, reg_result_string, MAX_REQ_SIZE);
              return 0;
            }
          else
            {
              fprintf(stderr, "Failure, selected reg not found\n");
              my_write(fd, "INVALID REGISTER\n", MAX_REQ_SIZE);
              return 1;
            }
        }
      else if (strcmp(target_value, "?") == 0)
        {
          // print bounds //
          reg_bounds = print_bounds(target_regid);
          if (reg_bounds != NULL)
            {
              printf("Bounds found %s, sending to client\n", reg_bounds);
              my_write(fd, reg_bounds, MAX_REQ_SIZE);
              return 0;
            }
          else
            {
              fprintf(stderr, "Failure, selected reg not found\n");
              my_write(fd, "INVALID REGISTER\n", MAX_REQ_SIZE);
              return 1;
            }
        }
      else
        {
          // check bound and insert value to target reg // 
          requested_value = atoi(target_value);
          value_swap_check = replace_value(requested_value, target_regid);

          if (value_swap_check == -2)
            {
              fprintf(stderr, "Failure, selected reg not found\n");
              my_write(fd, "INVALID REGISTER\n", MAX_REQ_SIZE);
              return 1;
            }
          else if (value_swap_check == -1)
            {
              fprintf(stderr, "Invalid input, not accepted by set bounds. Sending to client\n");
              my_write(fd, "InvalidInput\n", MAX_REQ_SIZE);
              return 2;
            }
          else
            {
              printf("Register value changed, sending OK to client\n");
              my_write(fd, "OK\n", MAX_REQ_SIZE);
              return 0;
            }
        }
    }
  else
    {
      fprintf(stderr, "ERROR: Desired request is not a valid AT-Command. Sending error message to client\n");
      my_write(fd, "INVALID AT-COMMAND\n", MAX_REQ_SIZE);
      return 3;
    }
}

// ********** main program ********** //
int main(int argc, char *argv[])
{
  // atcommand_res is used to check the result of the process_atcommand function for debugging purposes //
  int fd, atcommand_res; 
  char filename[MAX_FILENAME]; // to store the server serial port name //
  int bytes = 0; // bytes read from the read system call //

  // check argument count //
  if (argc < 2)
    {
      fprintf(stderr, "ERROR: You must specify a serial port name\n");
      return 1;
    }

  // get serial port name //
  strcpy(filename, argv[1]);

  // print server port //
  printf("Server port is: %s\n", filename);

  init_reglist(); // create the list of registers //
  add_register(3, "1|2|3"); // add the second register //

  fd = my_open(filename, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
    {
      fprintf(stderr, "ERROR: Open syscall failed from server\n");
      return 1;
    }

  // set the serial port attributes, i.e baud rate and parity //
  set_interface_attributes(fd, B115200, 0);

  // allocate memory for client request - for some reason, didn't work as a static //
  request = (char *)malloc(MAX_STRING * sizeof(char));
  memset(request, 0, sizeof(request));

  // main loop to read and process requests from the client //
  while (1)
    {
      wait_for_response(fd, 1); // block until you get a request //
      bytes = my_read(fd, request, MAX_REQ_SIZE);
      if (bytes < 0)
        {
          fprintf(stderr, "ERROR: Something went terribly wrong...\n");
        }
      else
        {
          printf("Client request: %s\n", request);  
        }

      if (strncmp(request, "insert", 6) == 0)
        {
          // add a new register to the list and inform the client //
          printf("Got insertion request from client\n");
          strcpy(insertion, request);
          process_insertion(insertion);
          my_write(fd, "INSERTION COMPLETE\n", MAX_REQ_SIZE);
        }
      else if (strncmp(request, "quit", 4) == 0)
        {
          printf("Got termination request from client. Bye\n");
          my_write(fd, "TERMINATING\n", MAX_REQ_SIZE);
          break;
        }
      else
        {
          atcommand_res = process_atcommand(fd, request); // process the AT-Command //
          if (atcommand_res == 0)
            {
              printf("OK!\n");
            }
        }
    }

  my_close(fd); // close the port //

  clear_regs(); // clear the list and free all the allocated memory //

  return 0;
}
