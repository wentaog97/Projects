#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>   
#include <stdlib.h>
#include "stdio.h"
using namespace std;

char decimal[100];

// recursive_itoa
// Recursive Integer to Char (ASCII) conversion.  Helper for *itoa
// Populates the decimal char array that represents a given int
int recursive_itoa(int arg) 
{
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if (div > 0)
	{
		index = recursive_itoa(div);
	}
	decimal[index] = mod + '0';
	return ++index;
}

// *itoa
// Integer to character array (c string)
char *itoa(const int arg) 
{
	bzero(decimal, 100);
	int order = recursive_itoa(arg);
	char *new_decimal = new char[order + 1];
	bcopy(decimal, new_decimal, order + 1);
	return new_decimal;
}

// printf 
// Implementation of the STL printf function
// Takes a format string and a variable number of arguments
//  and prints the formatted output to the standard output stream
int printf(const void *format, ...) 
{
	va_list list; 					// variable argument list type
	va_start(list, format);

	char *msg = (char *)format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0, k = 0;
	while (msg[i] != '\0') 
	{
		if (msg[i] == '%' && msg[i + 1] == 'd')
		{
			buf[j] = '\0';
			nWritten += write(1, buf, j);
			j = 0;
			i += 2;

			int int_val = va_arg(list, int);
			char *dec = itoa(abs(int_val));
			if (int_val < 0)
			{
				nWritten += write(1, "-", 1);
			}	
			nWritten += write(1, dec, strlen(dec));
			delete dec;
		}
		else
		{
			buf[j++] = msg[i++];
		}	
	}
	if (j > 0)
	{
		nWritten += write(1, buf, j);
	}	
	va_end( list );
	return nWritten;
}

// setvbuf
// Sets the buffering mode and the size of the buffer for a stream
// Returns 0 if successful, EOF/-1 if using an unsupported mode
int setvbuf(FILE *stream, char *buf, int mode, size_t size) 
{
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
	{
		return -1;
	}	
	stream->mode = mode;
	stream->pos = 0;
	if (stream->buffer != (char *)0 && stream->bufown == true)
	{
		delete stream->buffer;
	}
	
	switch ( mode ) 
	{
		case _IONBF:
			stream->buffer = (char *)0;
			stream->size = 0;
			stream->bufown = false;
			break;
		case _IOLBF:
		case _IOFBF:
			if (buf != (char *)0) 
			{
				stream->buffer = buf;
				stream->size   = size;
				stream->bufown = false;
			}
			else 
			{
				stream->buffer = new char[BUFSIZ];
				stream->size = BUFSIZ;
				stream->bufown = true;
			}
			break;
	}
	return 0;
}

// setbuf
// Sets the buffering mode and the size of the buffer for a stream
// Will use the defualt buffer size of 8192 if a buffer is null
void setbuf(FILE *stream, char *buf) 
{
	setvbuf(stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ);
}

// *fopen
// Opens a file with the specified mode and returns a file pointer. 
// It uses the open system call to open the file and sets the buffering mode to fully 
//  buffered with a buffer size of 8192 using setvbuf.
FILE *fopen(const char *path, const char *mode) 
{
	FILE *stream = new FILE();
	setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);
	
	// fopen( ) mode
	// r or rb = O_RDONLY
	// w or wb = O_WRONLY | O_CREAT | O_TRUNC
	// a or ab = O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b = O_RDWR
	// w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

  switch(mode[0]) 
  {
  case 'r':
	  if (mode[1] == '\0')            // r
	  {
		  stream->flag = O_RDONLY;
	  }  
	  else if ( mode[1] == 'b' ) 
	  {    
		  if (mode[2] == '\0')          // rb
		  {
			  stream->flag = O_RDONLY;
		  } 
		  else if (mode[2] == '+')      // rb+
		  {
			  stream->flag = O_RDWR;
		  }			  
	  }
	  else if (mode[1] == '+')        // r+  r+b
	  {
		  stream->flag = O_RDWR;
	  }  
	  break;
  case 'w':
	  if (mode[1] == '\0')            // w
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
	  }	  
	  else if (mode[1] == 'b') 
	  {
		  if (mode[2] == '\0')          // wb
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		  }	  
		  else if (mode[2] == '+')      // wb+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		  }	  
	  }
	  else if (mode[1] == '+')        // w+  w+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
	  }
	  break;
  case 'a':
	  if (mode[1] == '\0')            // a
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
	  } 
	  else if (mode[1] == 'b')
	  {
		  if (mode[2] == '\0')          // ab
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		  }  
		  else if (mode[2] == '+')      // ab+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_APPEND;
		  }	  
	  }
	  else if (mode[1] == '+')        // a+  a+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_APPEND;
	  } 
	  break;
  }
  
  mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  if ((stream->fd = open(path, stream->flag, open_mode)) == -1) 
  {
	  delete stream;
	  printf("fopen failed\n");
	  stream = NULL;
  }
  
  return stream;
}

int fpurge(FILE *stream)
{
    if (stream == NULL) {
        return -1; // Invalid file pointer
    }

    // Clear the input buffer if the last operation was a read
    if (stream->lastop == 'r') {
        stream->pos = 0;           // Reset the position in the buffer
        stream->actual_size = 0;   // Clear the actual size of the buffer
    }

    // Clear the output buffer if the last operation was a write
    if (stream->lastop == 'w') {
        stream->pos = 0;           // Reset the position in the buffer
        stream->actual_size = 0;   // Clear the actual size of the buffer
        if (stream->buffer != NULL) {
            memset(stream->buffer, 0, stream->size); // Clear the buffer content
        }
    }

	return 0;
}

int fflush(FILE *stream) 
{
	// comlete it
    if (stream == NULL) {
        return -1; // Invalid file pointer
    }

    // Handle output buffer
    if (stream->lastop == 'w') {
        // If there is data in the buffer, write it to the file
        if (stream->buffer != NULL && stream->pos > 0) {
            ssize_t written = write(stream->fd, stream->buffer, stream->pos);
            if (written == -1) {
                return -1; // Write error
            }
            stream->pos = 0; // Reset buffer position after flushing
        }
    }

    // Handle input buffer
    if (stream->lastop == 'r') {
        // Discard any buffered input
        stream->pos = 0;
        stream->actual_size = 0;
    }

	return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
	// complete it
	if (stream == NULL || ptr == NULL) {
        return -1;
    }

    size_t total_bytes = size * nmemb; // Total bytes to read
    size_t bytes_read = 0;             // Total bytes actually read
    char *buffer_ptr = (char *)ptr;    // Pointer to the buffer where data will be stored

    // If the last operation was a write, flush the output buffer
    if (stream->lastop == 'w') {
        if (fflush(stream) == -1) {
            return 0; // Error flushing the output buffer
        }
    }

    stream->lastop = 'r';

    while (bytes_read < total_bytes) {
        // If the buffer is empty, read data from the file into the buffer
        if (stream->pos >= stream->actual_size) {
            stream->actual_size = read(stream->fd, stream->buffer, stream->size);
            if (stream->actual_size == 0) {
                stream->eof = true; // End of file reached
                break;
            }
            if (stream->actual_size == -1) {
                return bytes_read / size; // Read error
            }
            stream->pos = 0;
        }

        // Calculate the number of bytes to copy from the buffer
        size_t bytes_to_copy = total_bytes - bytes_read;
        size_t buffer_remaining = stream->actual_size - stream->pos;
        if (bytes_to_copy > buffer_remaining) {
            bytes_to_copy = buffer_remaining;
        }

        // Copy the data from the buffer to the destination pointer
        memcpy(buffer_ptr + bytes_read, stream->buffer + stream->pos, bytes_to_copy);
        stream->pos += bytes_to_copy;
        bytes_read += bytes_to_copy;
    }

    return bytes_read / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
	if (stream == NULL || ptr == NULL) {
        return -1;
    }

    size_t total_bytes = size * nmemb; // Total bytes to write
    size_t bytes_written = 0;          // Total bytes actually written
    const char *buffer_ptr = (const char *)ptr; // Pointer to the data to be written

    // If the last operation was a read, flush the input buffer
    if (stream->lastop == 'r') {
        stream->pos = 0;
        stream->actual_size = 0;
    }

    stream->lastop = 'w';

    while (bytes_written < total_bytes) {
        // If the buffer is full or unbuffered, write it to the file
        if (stream->mode == _IONBF || (stream->pos >= stream->size && stream->mode == _IOFBF)) {
            ssize_t written = write(stream->fd, stream->buffer, stream->pos);
            if (written == -1) {
                return bytes_written / size; // Write error
            }
            stream->pos = 0;
        }

        // Calculate the number of bytes to copy to the buffer
        size_t bytes_to_copy = total_bytes - bytes_written;
        size_t buffer_remaining = stream->size - stream->pos;
        if (bytes_to_copy > buffer_remaining) {
            bytes_to_copy = buffer_remaining;
        }

        // Copy the data from the source pointer to the buffer
        memcpy(stream->buffer + stream->pos, buffer_ptr + bytes_written, bytes_to_copy);
        stream->pos += bytes_to_copy;
        bytes_written += bytes_to_copy;
    }

    // If unbuffered, write the data immediately
    if (stream->mode == _IONBF) {
        ssize_t written = write(stream->fd, stream->buffer, stream->pos);
        if (written == -1) {
            return bytes_written / size; // Write error
        }
        stream->pos = 0;
    }

    return bytes_written / size;
}

int fgetc(FILE *stream) 
{
	// complete it
	if (stream == NULL) {
        return EOF;
    }

    // If the last operation was a write, flush the output buffer
    if (stream->lastop == 'w') {
        if (fflush(stream) == -1) {
            return EOF; // Error flushing the output buffer
        }
    }

    stream->lastop = 'r';

    // If the buffer is empty, read data from the file into the buffer
    if (stream->pos >= stream->actual_size) {
        stream->actual_size = read(stream->fd, stream->buffer, stream->size);
        if (stream->actual_size == 0) {
            stream->eof = true; // End of file reached
            return EOF;
        }
        if (stream->actual_size == -1) {
            return EOF; // Read error
        }
        stream->pos = 0;
    }

    // Return the next character from the buffer
    return (unsigned char)stream->buffer[stream->pos++];
}

int fputc(int c, FILE *stream) 
{
	// complete it
	if (stream == NULL) {
        return EOF;
    }

    // If the last operation was a read, flush the input buffer
    if (stream->lastop == 'r') {
        stream->pos = 0;
        stream->actual_size = 0;
    }

    stream->lastop = 'w';

    // If the buffer is full or unbuffered, write it to the file
    if (stream->mode == _IONBF || (stream->pos >= stream->size && stream->mode == _IOFBF)) {
        ssize_t written = write(stream->fd, stream->buffer, stream->pos);
        if (written == -1) {
            return EOF; // Write error
        }
        stream->pos = 0;
    }

    // Add the character to the buffer
    if (stream->mode != _IONBF) {
        stream->buffer[stream->pos++] = (char)c;
    } else {
        // If unbuffered, write the character immediately
        ssize_t written = write(stream->fd, &c, 1);
        if (written != 1) {
            return EOF; // Write error
        }
    }

    return (unsigned char)c;
}

char *fgets(char *str, int size, FILE *stream) 
{
	// complete it
	if (stream == NULL || str == NULL || size <= 0) {
        return NULL;
    }

    int i = 0;
    while (i < size - 1) {
        // If the buffer is empty, read data from the file into the buffer
        if (stream->pos >= stream->actual_size) {
            stream->actual_size = read(stream->fd, stream->buffer, stream->size);
            if (stream->actual_size == 0) {
                stream->eof = true; // End of file reached
                if (i == 0) {
                    return NULL; // Nothing read, return NULL
                }
                break; // Stop reading, but return the string read so far
            }
            if (stream->actual_size == -1) {
                return NULL; // Read error
            }
            stream->pos = 0;
        }

        // Read characters from the buffer
        str[i] = stream->buffer[stream->pos++];
        if (str[i] == '\n') {
            i++;
            break;
        }
        i++;
    }

    str[i] = '\0'; // Null-terminate the string
    return str;
}

int fputs(const char *str, FILE *stream) 
{
	if (stream == NULL || str == NULL) {
        return EOF;
    }

    size_t len = strlen(str);
    size_t bytes_written = 0;

    // If the last operation was a read, flush the input buffer
    if (stream->lastop == 'r') {
        stream->pos = 0;
        stream->actual_size = 0;
    }

    stream->lastop = 'w';

    while (bytes_written < len) {
        // If the buffer is full or unbuffered, write it to the file
        if (stream->mode == _IONBF || (stream->pos >= stream->size && stream->mode == _IOFBF)) {
            ssize_t written = write(stream->fd, stream->buffer, stream->pos);
            if (written == -1) {
                return EOF; // Write error
            }
            stream->pos = 0;
        }

        // Calculate the number of bytes to copy to the buffer
        size_t bytes_to_copy = len - bytes_written;
        size_t buffer_remaining = stream->size - stream->pos;
        if (bytes_to_copy > buffer_remaining) {
            bytes_to_copy = buffer_remaining;
        }

        // Copy the data from the source pointer to the buffer
        memcpy(stream->buffer + stream->pos, str + bytes_written, bytes_to_copy);
        stream->pos += bytes_to_copy;
        bytes_written += bytes_to_copy;
    }

    // If unbuffered, write the data immediately
    if (stream->mode == _IONBF) {
        ssize_t written = write(stream->fd, stream->buffer, stream->pos);
        if (written == -1) {
            return EOF; // Write error
        }
        stream->pos = 0;
    }

    return 1; // fputs returns a non-negative number on success
}

int feof(FILE *stream) 
{
	return stream->eof == true;
}

int fseek(FILE *stream, long offset, int whence) 
{
	if (stream == NULL) {
        return -1;
    }

    // Clear the buffer
    stream->pos = 0;
    stream->actual_size = 0;

    // Reset the EOF flag
    stream->eof = false;

    // Use lseek to reposition the file offset
    off_t result = lseek(stream->fd, offset, whence);
    if (result == (off_t)-1) {
        return -1; // Error
    }

    return 0;
}

int fclose(FILE *stream) 
{
	// complete it
    if (stream == NULL) {
        return EOF;
    }

    // Flush the output buffer if the last operation was a write
    if (stream->lastop == 'w') {
        if (fflush(stream) == EOF) {
            return EOF; // Error flushing the output buffer
        }
    }

    // Close the file descriptor
    if (close(stream->fd) == -1) {
        return EOF; // Error closing the file
    }

    // Free the buffer if it was allocated by stdio.h
    if (stream->bufown && stream->buffer != NULL) {
        delete[] stream->buffer;
    }

    // Free the FILE structure
    delete stream;

    return 0; // Success
}
