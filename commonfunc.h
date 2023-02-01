// Header file for the common functions library between the server and the client //
// Author: Vangelis Bakas //
// Last Edited: 31/1/2023 //

#ifndef __COMMON_FUNCTIONS_H_
#define __COMMON_FUNCTIONS_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>

// Preprocessor //
#define BOUNDS_MAX 20
#define REGID_MAX 6

// Structs //
// Register Struct (singly linked list) //
// The structure of a register the server processes. It contains the register id, //
// the register value, the bounds of the number and the pointer to the next node  //
struct registerlist{
	char regid[REGID_MAX]; // register id, i.e. position on the list //
	int regvalue; // register value //
	char bounds[BOUNDS_MAX]; // number bounds of the selected register, as string //
	struct registerlist *next; // pointer to the next node in the list //
};

// define it as "register_t" for simplicity //
typedef struct registerlist registers_t;

// Function Prototypes //
int my_open(const char *pathname, int flags);
int my_close(int fd);
ssize_t my_read(int fd, void *buf, size_t count);
ssize_t my_write(int fd, const void *buf, size_t count);
void wait_for_response(int fd, int blocksignal);
int set_interface_attributes (int fd, int speed, int parity);

#endif