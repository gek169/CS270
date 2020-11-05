#include <cstdio>
#include <errno.h>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
//Library functions
static inline std::vector<std::string> SplitString(const std::string& s, char delim) {
	std::vector<std::string> elems;

	const char* cstr = s.c_str();
	unsigned int strLength = s.length();
	unsigned int start = 0;
	unsigned int end = 0;

	while (end <= strLength) {
		while (end <= strLength) {
			if (
				cstr[end] == delim && //we found the delimiter
				!(end > start && cstr[end-1] == '\\' //This character is not escaped!
				)
			) 
				break;
			end++;
		}

		elems.push_back(s.substr(start, end - start));
		start = end + 1;
		end = start;
	}
	if(elems.size() > 0 && delim != '&') //forget ampersands!
		for(size_t i = elems.size(); i > 0; i--)
			if(elems[i-1] == "")
				elems.erase(elems.begin()+i-1);
	return elems;
}
void refreshScreen() {
	//write(STDOUT_FILENO, "\x1b[2J", 4);
	//write(STDOUT_FILENO, "\x1b[H", 3);
	printf("%c[2K\r", 27);
}
typedef struct{
	std::string command; //command running in background
	pid_t pid; //process ID
} backgroundp;
int isNotTTY = 0;
pid_t foregroundp;
std::vector<backgroundp> processes;
std::string prompt = "uksh >";
std::string environment = "/bin/";
std::string binpath = "/bin/"; //path variable.
struct termios orig_termios;
#define CTRL_KEY(k) ((k) & 0x1f)
std::string parseEscapes(std::string in){
    for(int i = in.size() - 1; i > 0; i--)
    {
        int j, k=0;
        if(in[i-1] == '\\'){
            for(j = i-1; j >= 0 && in[j] == '\\'; j--, k++);
            if(k%2) in.erase(in.begin()+(--i));
        }
    }
    return in;
}

void die(const char* s){
    //write(STDOUT_FILENO, "\x1b[2J", 4);
    //write(STDOUT_FILENO, "\x1b[H", 3);
    puts("\nuksh exiting...");
    for(auto& a: processes)
    	kill(a.pid, SIGKILL);
    perror(s);exit(1);
}
void disableRawMode() {
	if(isNotTTY)return;
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)==-1)
  	die("tcsetattr");
}
void enableRawMode(int dummy) {
  if(tcgetattr(STDIN_FILENO, &orig_termios)==-1)isNotTTY=1;//die("tcgetattr");
  if(isNotTTY)return;
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_lflag &= ~(OPOST);
  raw.c_lflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1)die("tcsetattr");
}
void sigint_handler(int signo){
	if(signo == SIGINT)
		kill(foregroundp, SIGINT);
}
void setSigInt(){
	disableRawMode();
	signal(SIGINT, sigint_handler);
}
std::string inputline;
char eReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}
void listProcesses(){
	std::cout << "\nProcess list:" << std::endl;
	for(auto& p : processes)
		std::cout << "\n PID: " << p.pid << "| COMMAND: " << p.command << std::flush;
	std::cout << "\n~~~~~~~~~~~~~~~~" << std::endl;
}
void updateChildProcesses(){
	int status;
	pid_t retval;
	do{
		retval = waitpid(-1, &status, WNOHANG | WUNTRACED);
		if(retval == 0) return;
		int foundit=0;
	    for(int i = processes.size() - 1; i >= 0; i--){
	        auto& p = processes[i];
			if(retval == p.pid){
				std::cout << "\n<DEAD> PID: " << p.pid << "| COMMAND: " << p.command << std::flush;
				processes.erase(processes.begin() + i);
				break; //breaks out of the for.
				foundit=1;
			}
	    }
	    if(!foundit)return; //bad return value from waitpid
    }while(retval != 0);
}

void RunProgram(std::string command, int isforeground){
	// while(command.length() > 0 && isspace(command[0]) && command[0] != '\0'){ //remove leading whitespace
		// command.erase(command.front());
	// }
	if(command.length() <= 0) {
	return;
	}
	std::vector<std::string> duck = SplitString(command,' ');
	if(duck.size() > 0 && (duck[0].length() == 0)){
		puts("\nERROR:Don't put leading spaces before commands.\n");
		return;
	}
	bool isRelative = !(command[0] == '/');
	//TEST IF THIS IS A UKSH COMMAND
	if(duck[0] == "cd" && isforeground){
		if(duck.size() < 2) return;
		if(duck[1][0] == '/') //absolute environment
			chdir(parseEscapes(duck[1]).c_str());
		else //relative environment
			{
				std::string b = get_current_dir_name();b += "/";
				b = b + parseEscapes(duck[1]);
				chdir(b.c_str());
			}
		return;
	} else if(duck[0] == "getpath" && isforeground){
		std::cout << binpath << std::endl;
		return;
	} else if(duck[0] == "setpath" && isforeground){
		if(duck.size() < 2) return;
		binpath = parseEscapes(duck[1]);
		puts("\nPath variable set!\n");
		return;
	} else if(duck[0] == "plist" && isforeground) {
		listProcesses();
		return;
	} else if(duck[0] == "setprompt" && isforeground){
		if(duck.size() < 2) return;
		prompt = parseEscapes(duck[1]);
		return;
	} else if(duck[0].find("quit") == 0 && isforeground){
		die("Accepted UKSH quit command");
	} 
	//Finding the binary location.
	if(isRelative){
		if(duck[0][0] == '.') //in current working directory
		{
			duck[0].erase(duck[0].begin()); //it will now be just /program
			duck[0] = std::string(get_current_dir_name()) + duck[0]; 
		} else { //in the binary path
			//OLD:
			//duck[0] = binpath + duck[0];
			//NEW:
			std::vector<std::string> s = SplitString(binpath, ':');
			std::string validpath;
			int foundone = 0;
			for(unsigned int i = 0;!foundone && i < s.size();i++){
				validpath = s[i] + duck[0];
				if(access(validpath.c_str(),X_OK) != -1)
					foundone = 1;
			}
			if(!foundone){
				if(isforeground)
					puts("\n<No valid path was found for this program>");
				else
					puts("\n<No valid path was found for this program>\nTip: Shell commands cannot be invoked in the background.\n");
				return;
			} else {
				duck[0] = validpath;
			}
		}
	}
	{
		//std::vector<std::string> duckenv = SplitString(environment,':');
		// const char** args = (const char**)malloc(duck.size() + 1);
		std::vector<const char*> args; 
		args.resize(duck.size() + 1);
		
		// const char** env = (const char**)malloc(duckenv.size() + 1);
		//if(!args) return;
		for(size_t i = 0; i < duck.size(); i++){
			if(duck[i] != ""){
                duck[i] = parseEscapes(duck[i]);
				args[i] = duck[i].c_str(); 
			}else
				args[i] = NULL;
		}
		// for(size_t i = 0; i < duckenv.size(); i++){
			// env[i] = duckenv[i].c_str();
		// }
		// env[duckenv.size()] = NULL;
		args[duck.size()] = NULL;
		//execvpe(args[0],(char* const*)args,(char* const*)env);
		auto pid = fork();
		if(pid == 0){
			//setpgid(0, 0);
			if(isforeground)disableRawMode();
			execv(args[0],(char* const*)&(args[0]));
			exit(0);
		} else {
			int status;
			if(isforeground)
				{foregroundp = pid;setSigInt();}
			do{
				waitpid(pid, &status, 0);
			}while(
			!WEXITSTATUS(status) && 
			!WIFSIGNALED(status) && 
			!WIFEXITED(status)&& 
			!WSTOPSIG(status)
			);
			kill(pid,SIGKILL);

			if(isforeground)enableRawMode(0);
		}
	}
}
void forkBackgroundProcess(std::string command){
	auto forkretval = fork();
	if(forkretval == 0){
		setpgid(0, 0);
		RunProgram(std::string(command),0);
		exit(0);
	} else {
		backgroundp a;
		a.pid = forkretval;
		a.command = command;
		processes.push_back(a);
        listProcesses();
	}
}

void ProcessCommand(){
    updateChildProcesses();
	puts("\n"); //THIS
	//First we must find the first non-escaped percent sign.
	for(size_t i = 0; i < inputline.size(); i++)
	{
		if(i==0 && inputline[i] == '%')
		{
			inputline = "";
			return;
		}
		if(inputline[i-1] != '\\' && inputline[i] == '%'){ //Non-escaped percent sign.
			inputline = inputline.substr(0,i-1);
			break;
		}
	}
	auto ducks_in_a_row = SplitString(inputline, '&');
	for(size_t i = 0; i < ducks_in_a_row.size(); i++)
	{
		if(i==ducks_in_a_row.size() - 1)
			RunProgram(ducks_in_a_row[i],1);
		else
			forkBackgroundProcess(ducks_in_a_row[i]);
	}
	inputline = "";
}
void eProcessKeypress() {
    char c = eReadKey();
    if(iscntrl(c))
    {
        if(c==27) die("Handled Keyboard Invalid Input.");
        if(c == CTRL_KEY('q')) die("Accepted UKSH quit command");
        if(c == CTRL_KEY('D')) die("Accepted UKSH quit command");
        if(c == CTRL_KEY('H'))
            if(inputline.size() > 0)
                inputline.resize(inputline.size()-1,' ');
        // if(c== '\t')
            // if(top_suggestion != "") inputline = top_suggestion;
        if(c=='\n')
            ProcessCommand();
    }
    else switch (c) {
        case 27:
          die("Handled Keyboard Invalid Input.");
        break;
        case CTRL_KEY('H'):
            if(inputline.size() > 2)
            inputline.resize(inputline.size()-1,' ');
        break;
        default:
            if(isprint(c) && c != '\n' && c != '\t')
            inputline.push_back(c);
        break;
    }
}
int main(int argc, char** argv){
	std::string larg = "uksh";
	for(int i = 1; i < argc; i++){
		std::string carg = std::string(argv[i]);
		if(larg == "--path" || larg == "-p")
			binpath = carg;
		if(carg == "--help" || larg == "-h"){
			std::cout << "\nHelp text:" << std::endl <<
			"This is a rather mediocre, unstable shell which could replace "
			"The shell on your system.It meets the project 3 specification, with "
			"bonus features to make it more useful. It is not based on Shellex.c. "
			"For convenience sake, it was written in C++11, purely for std::string. "
			"\nCommands and special chars:"
			"\nquit - quit the shell"
			"\ncd - change working directory"
			"\ngetpath - get the path variable for finding executables"
			"\nsetpath - set the path variable for finding executables"
			"\n% - Comment character.Everything after it before the next line is ignored."
			"\n\\ - Escape character. It isn't perfect, but it works."
			"\nplist - list background processes"
			"\nsetprompt - set the commandline prompt"
			"\n\nBe weary of leading whitespace before command names."
			"\nDo not put whitespace after an ampersand before the next command." << std::endl;
			exit(0);
		}
		larg = carg;
	}
	enableRawMode(0);
	while(1){
		refreshScreen();
		std::cout << prompt << inputline << std::flush;
		eProcessKeypress();
	}
	return 0;
}
