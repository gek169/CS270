/* There are two bugs in this program.  In both cases the bug is
 * an incorrect constant in the program.  Find the incorrect constant,
 * fix it, and you will get the memcmp() call to be equal -- i.e., match!
 * 
 * Your goal is to use gdb to debug the program.  You should record your
 * debugging session using the script program, and then upload the
 * resulting file to the csportal.
 * 
 * YOU MUST PERFORM THIS EXERCISE ON YOUR VM.
 * 
 * To record your session:
 *   1. Use the script command (script MyDebugSession.txt).
 * 
 * To record debug the program:
 *   2. Compile the program so that it can be debugged
 *      (gcc -Wall -g -o match match.c)
 *   3. Run gdb on the program (gdb ./match).
 *   4. Use gdb commands to run the program and find the bugs.
 *   5. Edit the program to fix the bugs, recompile, and rerun the
 *      program without gdb, which should print out a message indicating
 *      that you have debugged both phases correctly.
 *   6. Display your new version of the program (cat match.c).
 * 
 * To stop recording your session:
 *   7. Type exit.
 * 
 * Now submit MyDebugSession.txt to https://www.cs.uky.edu/csportal.
 * You can either start a browser on your VM, or you can copy the file to some
 * other machine and use a browser there.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define buflen 100
char buf1[buflen];
char buf2[buflen];

/* --- sorry --- */
void sorry(char *test) {
	printf("Sorry!  You did not debug the %s correctly.\n", test);
	exit(-1);
} // sorry()

/* --- firstTest --- */
void firstTest(char *name1, char *name2) {
	int fd1, fd2;
	int retval;

	if ((fd1 = open(name1, O_RDONLY)) < 0) {
		perror(name1);
		exit(-1);
	}
	if ((fd2 = open(name2, O_RDONLY)) < 0) {
		perror(name2);
		exit(-1);
	}

	retval = lseek(fd1, 10, SEEK_CUR);
	if (retval < 0) perror("lseek error");
	retval = read(fd1, buf1, 5);
	if (retval < 0) perror("read error");

	/* Hint -- the error is an incorrect constant in the next line */
	retval = lseek(fd2, 72, SEEK_CUR);
	if (retval < 0) perror("lseek error");
	retval = read(fd2, buf2, 5);
	if (retval < 0) perror("read error");
	
	retval = memcmp(buf1, buf2, 5);
	if (retval != 0) {
		sorry("firstTest");
	} else {
		printf("You debugged the first one correctly!\n");
	}
}

/* --- secondTest --- */
void secondTest(char *filename1, char *filename2) {
	int filedes1, filedes2;
	int *value;
	int retval;

	if ((filedes1 = open(filename1, O_RDONLY)) < 0) {
		perror(filename1);
		exit(-1);
	}
	if ((filedes2 = open(filename2, O_RDONLY)) < 0) {
		perror(filename2);
		exit(-1);
	}
	
	retval = lseek(filedes1, 64, SEEK_CUR);
	if (retval < 0) perror("lseek error");
	retval = read(filedes1, buf1, 4);
	if (retval < 0) perror("read error");

	retval = lseek(filedes2, 124, SEEK_CUR);
	if (retval < 0) perror("lseek error");
	retval = read(filedes2, buf2, 4);
	if (retval < 0) perror("read error");

	value = (int *) buf2;
	/* Hint -- the error is in the next line */
	*value = *value | 0x00000000;
	
	retval = memcmp(buf1, buf2, 4);
	if (retval != 0) {
		sorry("secondTest");
	} else {
		printf("You debugged the second one correctly!\n");
	}
} // secondTest()

/* --- main --- */
int main() {
	char file1[buflen];
	char file2[buflen];

	strcpy(file1, "/etc/hosts");
	strcpy(file2, "/etc/networks");
	firstTest(file1, file2);

	strcpy(file1, "/bin/ls");
	strcpy(file2, "/bin/cat");
	secondTest(file1, file2);
	return(0); // success
} // main()
