#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
char parse_int(char* input){
	int isHex = 0;
	char result = 0;
	/*char* originput = input;*/
	while(!isspace(*input) && !(*input == '\0') && !(*input == '\t')){
		if(isHex) result *= 16; else result *= 10;
		switch(*input){
			case '0':	break;
			case '1':result+=1;				break;
			case '2':result+=2;				break;
			case '3':result+=3;				break;
			case '4':result+=4;				break;
			case '5':result+=5;				break;
			case '6':result+=6;				break;
			case '7':result+=7;				break;
			case '8':result+=8;				break;
			case '9':result+=9;				break;
			case 'A':case 'a':result+=10;	break;
			case 'B':case 'b':result+=11;	break;
			case 'C':case 'c':result+=12;	break;
			case 'D':case 'd':result+=13;	break;
			case 'E':case 'e':result+=14;	break;
			case 'F':case 'f':result+=15;	break;
			case 'X':case 'x': isHex = 1;	break;
			default: break;
		}
		input++;
	}
	/*printf("\nDEBUG: found character %c, text is %s\n",result,originput);*/
	return result;
}

int main(){
	char buffer[1024];
	int shouldQuit = 0; int i = 0;
	while(!shouldQuit){
		if(NULL == fgets(buffer, 1024, stdin))	exit(0);
		int len = strlen(buffer); /* get length of input*/
		for(i = 0; i < len && i < 1024; i++){
			char c = buffer[i];
			if(!isspace(c) && c!= '\0' && c!= '\t' && c!='"'){ /*we have found the beginning of an int*/
				printf("%c",parse_int(buffer + i)); /*print the character.*/
				while(
					i < len && !isspace(	buffer[i]	)
				) i++;
			} else if(c == '"'){ /*we have found the beginning of a string */
				i++;c = buffer[i];
				while(i < len && buffer[i] != '"'){
					printf("%c",buffer[i]);
					i++;c = buffer[i];
				}
			}
		}
	}
	return 0;
}