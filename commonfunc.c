// Functions and structures used from both the server and the client //
// Author: Vangelis Bakas //
// Last Edited: 31/1/2023 //

#include "commonfunc.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

// ********* my_open ********** //
// calls the open system call used for the server and //
// client serial ports and checks if it fails.        //
int my_open(const char *pathname, int flags)
{
  int result;

  // open the file and check if fd is below zero //
  result = open(pathname, flags);

  if (result < 0)
    {
      perror("Open");
    }

  return result;
}

// ********* my_close ********** //
// calls the close system call and checks if it fails. //
int my_close(int fd)
{
  int result;

  // close the file and check if there is an error //
  result = close(fd);

  if (result < 0)
    {
      perror("Close");
    }

  return result;
}

// ********* my_read ********** //
// same as the read system call, but it ensures that all the bytes //
// requested will be read.                                         //     
ssize_t my_read(int fd, void *buf, size_t count)
{
  ssize_t bytes_read, total_read = 0;
  ssize_t bytes_needed = count;
  ssize_t bytes_remaining;
  
  if ((bytes_read = read(fd, buf, count)) == -1) 
    { 
      perror("Read"); 
    }
  
  // count the bytes read and check if there are missing //
  // read again until all the bytes are read //
  total_read = bytes_read;
  
  while(total_read < bytes_needed && bytes_read != 0) 
    {
      bytes_remaining = bytes_needed - total_read;
      if ((bytes_read = read(fd, buf + total_read, bytes_remaining)) == -1) 
        { 
          perror("Read"); 
        }
      
      total_read = total_read + bytes_read;        
    }

  return total_read;
}

// ********* my_write ********** //
// same as the write system call, but it ensures that all the bytes //
// requested will be written.                                       //  
ssize_t my_write(int fd, const void *buf, size_t count)
{
  ssize_t bytes_written, total_written = 0;
  ssize_t bytes_needed = count;
  ssize_t bytes_remaining;
  
  if ((bytes_written = write(fd, buf, count)) == -1) 
    { 
      perror("Write");
    }
  
  // count the bytes written and check if there are missing //
  // write again until all the bytes are read //
  total_written = bytes_written;
  
  while(total_written < bytes_needed) 
    {
      bytes_remaining = bytes_needed - total_written;
      if ((bytes_written = write(fd, buf + total_written, bytes_remaining)) == -1) 
        {
          perror("Write");
        }
      
      total_written = total_written + bytes_written;        
    }

  return total_written;
}

// ********** TTY TERMINAL FUNCTIONS BELOW THIS POINT ********** // 
// ********** wait_for_response ********** //
// function to set blocking and read timeout for the serial port //
// the timeout is set to 0.5 seconds                             //
void wait_for_response(int fd, int blocksignal)
{
  struct termios tty;
  
  memset (&tty, 0, sizeof tty);
  
  if (tcgetattr (fd, &tty) != 0)
    {
      perror("tcgetattr");
      return;
    }

  tty.c_cc[VMIN]  = blocksignal ? 1 : 0;
  tty.c_cc[VTIME] = 5; 

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
      perror("tcsetattr");
      return;
    }
}

// ********** set_interface_attributes ********** //
// set the seiral port interface attributes, i.e. baud rate and parity //
// mandatory in order for the server and client to communicate         //
// the default values are present with no changes                      //
int set_interface_attributes(int fd, int speed, int parity)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);

  // check the current settings //
  if (tcgetattr (fd, &tty) != 0)
    {
      perror("tcgetattr"); // tcgetattr returned an error //
      return -1;
    }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // set to 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  tty.c_iflag &= ~IGNBRK;         // ignore break signal //
  tty.c_lflag = 0;                // no signaling chars, no echo, no canonical processing //
  tty.c_oflag = 0;                // no remapping and no delays //
  tty.c_cc[VMIN]  = 0;            // read doesn't block //
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout // 

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
      perror("tcsetattr"); // tcsetattr returned an error //
      return -1;
    }
    
  return 0;
}