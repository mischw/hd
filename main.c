#include <stdio.h>
#include <stdlib.h>
#include <errno.h>		// errno
#include <string.h>		// memcpy()
#include <sys/stat.h>	// stat()
#include <sys/mman.h>	// mmap()
#include <fcntl.h>		// open()
#include <unistd.h>		// close()


/* Block format:
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
│XXXXXXXX│ YY YY YY YY YY YY YY YY │ YY YY YY YY YY YY YY YY │ZZZZZZZZ│ZZZZZZZZ│
├────────┼─────────────────────────┼─────────────────────────┼────────┼────────┤
*/


// Block template:
char block_buffer[] = 
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"│\x1b[90mXXXXXXXX\x1b[00m│ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │ \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m \x1b[90mYY\x1b[00m │\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m\x1b[90mZ\x1b[00m│\n"
"├────────┼─────────────────────────┼─────────────────────────┼────────┼────────┤\n";


// offsets in blockbuffer
size_t line_bytes = 423; // bytes needed to display one line
size_t offset_offset = 8; // offset to most significant digit (color never changes)
size_t byte_offset[16] = {25, 38, 51, 64, 77, 90, 103, 116, 133, 146, 159, 172, 185, 198, 211, 224}; // 13 chars offset between, +4 in the middle, offset to color code
size_t char_offset[16] = {240, 251, 262, 273, 284, 295, 306, 317, 331, 342, 353, 364, 375, 386, 397, 408}; // 11 chars offset between, +3 in the middle, offset to color code


// 5 bytes per code
#define ANSI_COLOR_BLACK "\x1b[30m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"

#define ANSI_BG_COLOR_BLACK "\x1b[40m"
#define ANSI_BG_COLOR_RED "\x1b[41m"
#define ANSI_BG_COLOR_GREEN "\x1b[42m"
#define ANSI_BG_COLOR_YELLOW "\x1b[43m"
#define ANSI_BG_COLOR_BLUE "\x1b[44m"
#define ANSI_BG_COLOR_MAGENTA "\x1b[45m"
#define ANSI_BG_COLOR_CYAN "\x1b[46m"
#define ANSI_BG_COLOR_WHITE "\x1b[47m"

#define ANSI_COLOR_BRIGHT_BLACK "\x1b[90m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_BRIGHT_GREEN "\x1b[92m"
#define ANSI_COLOR_BRIGHT_YELLOW "\x1b[93m"
#define ANSI_COLOR_BRIGHT_BLUE "\x1b[94m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define ANSI_COLOR_BRIGHT_CYAN "\x1b[96m"
#define ANSI_COLOR_BRIGHT_WHITE "\x1b[97m"

#define ANSI_COLOR_RESET "\x1b[00m"


// 3 bytes per char
#define UPPER_TBL "┌────────┬─────────────────────────┬─────────────────────────┬────────┬────────┐"
#define LOWER_TBL "└────────┴─────────────────────────┴─────────────────────────┴────────┴────────┘"


size_t get_count_bytes(const char *filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}


void write_offset_to_buffer(size_t index, unsigned int offset) {
	char temp = block_buffer[index + 8]; // save what snprintf overwrites with \0
	snprintf(block_buffer + index, 9, "%08X", offset);
	block_buffer[index + 8] = temp; // restore what snprintf did overwrite with \0
}


const char *get_colored_format_string_for_byte(char byte) {
	switch (byte) {
		case 0: // NULL bytes
			return ANSI_COLOR_BRIGHT_BLUE"%02X"ANSI_COLOR_RESET;
			break;
		case 1 ... 31: // control characters
			return ANSI_COLOR_BRIGHT_BLUE"%02X"ANSI_COLOR_RESET;
			break;
		case 32: // space
			return ANSI_COLOR_RESET"%02X"ANSI_COLOR_RESET;
			break;
		case 33 ... 126: // printable characters
			return ANSI_COLOR_GREEN"%02X"ANSI_COLOR_RESET;
			break;
		case 127: // delete character
			return ANSI_COLOR_BRIGHT_RED"%02X"ANSI_COLOR_RESET;
			break;
		default: //  128 ... 255: // extended ascii
			return ANSI_COLOR_BRIGHT_CYAN"%02X"ANSI_COLOR_RESET;
			break;
		}
}

const char *get_colored_string_for_char(char byte, char interp[]) {
	char temp_interp[12];
	switch (byte) {
		case 0: // NULL bytes
			return ANSI_BG_COLOR_BLUE"0"ANSI_COLOR_RESET;
			break;
		case 1 ... 31: // control characters
			return ANSI_BG_COLOR_BLUE"C"ANSI_COLOR_RESET;
			break;
		case 32: // space
			return ANSI_COLOR_RESET" "ANSI_COLOR_RESET;
			break;
		case 33 ... 126: // printable characters
			snprintf(temp_interp, 12, ANSI_COLOR_GREEN"%c"ANSI_COLOR_RESET, byte);
			break;
		case 127: // delete character
			return ANSI_COLOR_BRIGHT_RED"<"ANSI_COLOR_RESET;
			break;
		default: //  128 ... 255: // extended ascii
			return ANSI_BG_COLOR_MAGENTA"E"ANSI_COLOR_RESET;
			break;
		}
		memcpy(interp, temp_interp, 11); // to get rid of the damn null terminator
		return interp;
}

void write_byte_and_interp_to_buffer(size_t index, size_t index_interp, size_t index_mmap, size_t count_bytes, char data[]) {
	char temp = block_buffer[index + 12]; // save what snprintf overwrites with \0
	if(index_mmap < count_bytes) {
		char byte = data[index_mmap];
		snprintf(block_buffer + index, 13, get_colored_format_string_for_byte(byte), (unsigned char)data[index_mmap]);
		char interp[11];
		memcpy(block_buffer + index_interp, get_colored_string_for_char(byte, interp), 11);
	} else { // no more bytes, fill with blanks
		snprintf(block_buffer + index, 13, ANSI_COLOR_RESET"  "ANSI_COLOR_RESET);
		memcpy(block_buffer + index_interp, ANSI_COLOR_RESET" "ANSI_COLOR_RESET, 11);
	}
	block_buffer[index + 12] = temp; // restore what snprintf did overwrite with \0
}



void dump(const char *input) {
	// Check for stdin
	if (input == NULL) {
		fprintf(stderr, "ERROR: reading from stdin not implemented\n");
		exit(EXIT_FAILURE);
	}

	// get file size
	size_t count_bytes = get_count_bytes(input);

	// open file read only
	int fd = open(input, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error opening file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// map file to memory
	char *data = mmap(NULL, count_bytes, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
	if(data == MAP_FAILED) {
		fprintf(stderr, "Error opening file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// calculate count of (partially filled) blocks
	size_t count_blocks = count_bytes / 512 + ((count_bytes % 512)? 1 : 0);

	// print table header
	printf(UPPER_TBL"\n");
	unsigned int hex_offset = 0;
	size_t index;
	size_t index_interp;
	size_t index_mmap;
	for(size_t i = 0; i < count_blocks; i++) {
		for(int j = 0; j < 32; j++) {
			// write offset
			index = j * line_bytes + offset_offset;
			write_offset_to_buffer(index, hex_offset);
			for(int k = 0; k < 16; k++) {
				// write bytes and chars
				index = j * line_bytes + byte_offset[k];
				index_interp = j * line_bytes + char_offset[k];
				index_mmap = i * 512 + j * 16 + k;
				write_byte_and_interp_to_buffer(index, index_interp, index_mmap, count_bytes, data);
			}
			hex_offset += 16;
		}
		fwrite(block_buffer, 1, 13777, stdout);
	}
	// print table footer
	printf(LOWER_TBL"\n");
	// print some stats
	printf("\x1b[4mStatistics\x1b[0m:\nFilesize (Byte): \t%ld\nBlocks (512 Byte): \t%ld\n", count_bytes, count_blocks);	

	// Cleanup
	int val = munmap(data, count_bytes);
	if(val != 0) {
		fprintf(stderr, "Error while cleaning: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(fd);
}


int main(int argc, char *argv[]) {	
	switch (argc) {
		case 1:
			// 0 arguments, dump stdin
			dump(NULL);
			break;
		case 2:
			// 1 argument, dump file
			dump(argv[1]);
			break;
		default:
			fprintf(stderr, "ERROR: Too many arguments\n");
			exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
