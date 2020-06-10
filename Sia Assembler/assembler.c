#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//left trim
char *ltrim(char *s) { //takes in a pointer to the array "line"
	while (*s == ' ' || *s == '\t') s++; //finds postion of first space character and points to next character
	return s;   
}

char getRegister(char *text) {
	if (*text == 'r' || *text=='R') text++; //finds position of the first 'r' character and points to next character
	return atoi(text); //coverts a string to an int
}

int assembleLine(char *text, unsigned char* bytes) { //takes in the pointer to the array "line" and pointer to array "bytes"
	text = ltrim(text); //left trim
	// char *keyWord = "";

	char *keyWord = "";
	keyWord = strtok(text,"\n");
	
	if (strcmp("halt",keyWord) != 0 || strcmp("return",keyWord) != 0)
		keyWord = strtok(text," ");	

	//3R Format
	if (strcmp("add",keyWord) == 0) { //compares to see if it's equal
		bytes[0] = 0x10; //bytes[] = hex 0x10 = decimal 16, when "or" with below produces a hex result which is the "assembly format" eg 11 23
		bytes[0] |= getRegister(strtok(NULL," ")); //bytes = bytes OR get... , strtok NULL tells the function to continue tokenizing the previous input string
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); //left shift and bitwise inclusive or
		return 2; //returns byte count, byte 0 and 1 == 2
	}
	if (strcmp("and",keyWord) == 0) { 
		bytes[0] = 0x20; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}
	if (strcmp("divide",keyWord) == 0) { 
		bytes[0] = 0x30; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}

	if (strcmp("halt",keyWord) == 0) { 	
		bytes[0] = 0x00; 
		bytes[1] = 0x00; 
		return 2; //returns byte count
	}


	if (strcmp("multiply",keyWord) == 0) { 
		bytes[0] = 0x40; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}


	if (strcmp("or",keyWord) == 0) { 
		bytes[0] = 0x60; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}


	if (strcmp("subtract",keyWord) == 0) { 
		bytes[0] = 0x50; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}

	//ai Format
	if (strcmp("addimmediate",keyWord) == 0) { 
		bytes[0] = 0x90; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}

	//br format
	if (strcmp("branchifequal",keyWord) == 0) { //branchifequal r1 r2 6 ---- A1 20 00 06
		bytes[0] = 0xA0; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 ; 
		int value =  atoi(strtok(NULL," "));
		bytes[1] |= value >> 16;
		bytes[2] = value >> 8;
		bytes[3] = value;
		return 4; //returns byte count
	}

	if (strcmp("branchifless",keyWord) == 0) {
		bytes[0] = 0xB0; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 ; 
		int value =  atoi(strtok(NULL," "));
		bytes[1] |= value >> 16;
		bytes[2] = value >> 8;
		bytes[3] = value;
		return 4; //returns byte count
	}

	//int format
	if (strcmp("interrupt",keyWord) == 0) { 
		int value =  atoi(strtok(NULL," "));
		bytes[0] = 0x80; 
		bytes[0] |= value >> 8; 
		bytes[1] = value;
		return 2; //returns byte count
	}


	//jump format
	if (strcmp("call",keyWord) == 0) { //eg call 444 assembled is D0 00 01 BC
		int value =  atoi(strtok(NULL," "));
		bytes[0] = 0xD0; 
		bytes[3] = value;
		bytes[2] = value >> 8;
		bytes[1] = value >> 16;
		return 4; //returns byte count
	}

	if (strcmp("jump",keyWord) == 0) { //eg jump 3 assembled is C0 00 00 03 //assembled wrong because cant take in big value
		int value =  atoi(strtok(NULL," "));
		bytes[0] = 0xC0; 
		bytes[3] = value;
		bytes[2] = value >> 8;
		bytes[1] = value >> 16;
		return 4; //returns byte count
	}
	
	//ls format
	if (strcmp("load",keyWord) == 0) { 
		bytes[0] = 0xE0; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}
	
	if (strcmp("store",keyWord) == 0) { 
		bytes[0] = 0xF0; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," ")); 
		return 2; //returns byte count
	}

	//stack format
	if (strcmp("pop",keyWord) == 0) { //push r1 --- push = 0100 binary == 8 hex ---7180
		char *value = "8";
		bytes[0] = 0x70; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = atoi(value) << 4; 
		return 2; //returns byte count
	}

	if (strcmp("push",keyWord) == 0) { //pop r1 --- push = 1000 binary == 4 hex ---7140
		char *value = "4";
		bytes[0] = 0x70; 
		bytes[0] |= getRegister(strtok(NULL," ")); 
		bytes[1] = atoi(value) << 4; 
		return 2; //returns byte count
	}

	if (strcmp("return",keyWord) == 0) { //return = 000 binary == 0 hex ---7000
		char *value = "0";
		bytes[0] = 0x70; 
		bytes[1] = atoi(value) << 4; 
		return 2; //returns byte count
	}

	return 0;
}

int main(int argc, char **argv) {
	
	FILE *src = fopen(argv[1],"r"); //This is the pointer to a FILE object that identifies the stream, arg1 file is read
	FILE *dst = fopen(argv[2],"w"); //Opening file in argument 2 to write 
	while (!feof(src)) { //tests the end-of-file indicator for the given stream (file)
		unsigned char bytes[4]; //array of size 4
		char line[1000]; //char array
		if (NULL != fgets(line, 1000, src)) { //fgets read a line from "src" and stores it in the array line, stops at 1000-1 or newline is read
			printf ("read: %s\n",line); //display the line that was read
			int byteCount = assembleLine(line,bytes); //function call return the bytes based on the opcode, this is the size in bytes of each element to be written.
			fwrite(bytes,byteCount,1,dst); //writes data from array "bytes" to the stream "dst", 1 is the number of elements with the size of byteCount
		}
	}
	fclose(src); //close streams
	fclose(dst);
	return 0;
}
