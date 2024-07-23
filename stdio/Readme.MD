# Custom stdio
This is a custom stdio library implemented in C++ that allows C/C++ programs to read and write files using a file stream interface (FILE *). The functions provide buffering and a user-friendly interface for reading and writing data. The stdio.cpp implementation utilizes the underlying system calls for file I/O operations.

## Functions
### `recursive_itoa`
This helper function converts an integer to a character array (C string) by recursively populating the decimal array with the digits of the integer. It is used internally by `itoa`.

### `itoa`
This function converts an integer to a character array (C string) by using the `recursive_itoa` function. It returns a dynamically allocated char array representing the integer.

### `printf`
This function mimics the behavior of the standard `printf` function and takes a format string and a variable number of arguments. It handles the format specifiers, such as `%d`, by converting the corresponding argument to a string and writing it to the standard output stream.

### `setvbuf`
This function sets the buffering mode and size for a file stream. It supports three modes: `_IONBF` (no buffering), `_IOLBF` (line buffering), and `_IOFBF` (full buffering). It allocates a buffer if needed and sets the mode, position, buffer size, and ownership attributes of the stream.

### `setbuf`
This function is a convenience function that calls `setvbuf` with either `_IONBF` or `_IOFBF` mode based on the provided buffer. If the buffer is null, it uses the default buffer size of 8192.

### `fopen`
This function opens a file with the specified mode and returns a file pointer (`FILE *`) representing the opened file. It translates the mode string to the appropriate flags for the open system call and sets the buffering mode to fully buffered with a default buffer size of 8192.

### `fpurge`
This function clears the input and output buffers for the given stream, depending on the last operation performed (read or write). It resets the buffer position and actual size, and clears the buffer content if necessary.

### `fflush`
This function flushes the output buffer for the given stream by writing its content to the file. It also discards any buffered input data. If the stream is null or a write error occurs, it returns an error.

### `fread`
This function reads data from a file stream into a buffer pointed by `ptr`. It takes the size of each element, the count of elements, and the file stream as parameters. It uses the underlying read system call to read the data and returns the total number of elements read.

### `fwrite`
This function writes data from a buffer pointed by `ptr` to a file stream. It takes the size of each element, the count of elements, and the file stream as parameters. It uses the underlying write system call to write the data and returns the total number of elements written.

### `fgetc`
This function reads a character from the file stream and returns it as an int. It utilizes the underlying read system call to read the character. If the buffer is empty, it reads data from the file into the buffer.

### `fputc`
This function writes a character specified by `c` to the file stream. It utilizes the underlying write system call to write the character. If the buffer is full or unbuffered, it writes the buffer content to the file.

### `fgets`
This function reads a line from the file stream into the character array `str`. It takes the maximum size of `str` as a parameter and uses the underlying read system call to read the line. It stops reading after reaching a newline character or the end of the buffer.

### `fputs`
This function writes the null-terminated string `str` to the file stream. It utilizes the underlying write system call to write the string. If the buffer is full or unbuffered, it writes the buffer content to the file.

### `feof`
This function checks if the end-of-file indicator for the given file stream is set. It returns 1 if the indicator is set, and 0 otherwise.

### `fseek`
This function repositions the file offset of the given stream to the specified offset according to the directive `whence`. It clears the buffer and resets the EOF flag.

### `fclose`
This function closes the given file stream, flushing the output buffer if necessary. It frees any allocated buffer and the FILE structure.

---

Testing codes:
eval.cpp
driver.cpp
compile.sh
eval_tests.sh

Input files:
hamlet.txt
othello.txt
