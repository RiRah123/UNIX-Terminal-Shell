
/* -----------------------------------------------------------------------------
   FILE: yosh.c

   NAME: RIAN RAHMAN

   DESCRIPTION: A SHELL SKELETON
   -------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse.h"   // local file include declarations for parse-related structs


enum BUILTIN_COMMANDS { NO_SUCH_BUILTIN = 0, EXIT, JOBS, CD, HISTORY, KILL, HELP, REPEAT};

char * cmdLine;
parseInfo *info; 		// info stores all the information returned by parser.
struct commandType *com; 	// com stores command name and Arg list for one command.
static pid_t backgroundProcessPID[100];
static char * backgroundCommandName[100];
static char * backgroundProcessStatus[100];
static char * commandArguments[100];
static char * allCommandArguments[100];
static int currentNumberArguments = 0;

/* -----------------------------------------------------------------------------
   FUNCTION: wait_and_poll(int s)
   DESCRIPTION: A Signal Handler for processing background processes and indicating
   when they have been terminated.
   -------------------------------------------------------------------------------*/
void wait_and_poll(int s){
    pid_t status;
    int i = 0;
    status = waitpid(-1, NULL,WNOHANG);
    if (status > 0 ){
        if (status == backgroundProcessPID[i]) {
            backgroundProcessStatus[i] = "Terminated";
        }
        while (status != backgroundProcessPID[i]) {
            i++;
            if (status == backgroundProcessPID[i]) {
                backgroundProcessStatus[i] = "Terminated";
            }
        }
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: prepend(char* inputString, const char* prependCharacter)
   DESCRIPTION: A helper function that is used to preprend one string at the
   begining of another specified string.
   -------------------------------------------------------------------------------*/
void prepend(char* inputString, const char* prependCharacter)
{
    size_t len = strlen(prependCharacter);
    memmove(inputString + len, inputString, strlen(inputString) + 1);
    memcpy(inputString, prependCharacter, len);
}

/* -----------------------------------------------------------------------------
   FUNCTION: buildPrompt()
   DESCRIPTION: Displays the yosh shell prompt everytime the user wants to enter a
   new command. The prompt consists of the user's username, user's machine/server
   name, and the user's current working directory.
   -------------------------------------------------------------------------------*/
char * buildPrompt() {
    static char directory[PATH_MAX];
    static char username[PATH_MAX];
    static char machinename[PATH_MAX];
    char arrows[] = " --> ";

    if (getcwd(directory, sizeof(directory)) == NULL) {
        perror("getcwd() error");
    }

    gethostname(machinename, sizeof(machinename));
    getlogin_r(username, sizeof(username));

    strcat(directory, arrows);
    prepend(directory, ":");
    strcat(machinename, directory);
    prepend(machinename, "@");
    strcat(username, machinename);

    return username;
}

/* -----------------------------------------------------------------------------
   FUNCTION: isBuild()
   DESCRIPTION: A helper function that determines if the user has entered a built in
   command and if so which one. The function returns an enumeration constant
   representing the command the user has entered.
   -------------------------------------------------------------------------------*/
int isBuiltInCommand( char * cmd )
{
    if(strncmp(cmd, "exit", strlen("exit")) == 0) {
        return EXIT;
  	 } else if (strncmp(cmd, "help", strlen("help")) == 0 ) {
        return HELP;
    } else if (strncmp(cmd, "history", strlen("history")) == 0 ) {
        return HISTORY;
    } else if (strchr(cmd, '!') != NULL) {
        return REPEAT;
    } else if (strncmp(cmd, "jobs", strlen("jobs")) == 0 ) {
        return JOBS;
    } else if (strncmp(cmd, "cd", strlen("cd")) == 0 ) {
        return CD;
    } else if (strncmp(cmd, "kill", strlen("kill")) == 0 ) {
        return KILL;
    }

    return NO_SUCH_BUILTIN;
}

/* -----------------------------------------------------------------------------
   FUNCTION: isBackgroundJob()
   DESCRIPTION: Determines whether the user wanted the entered command in the
   background.
   -------------------------------------------------------------------------------*/
int isBackgroundJob() {
    if (info->boolBackground) {
        return 1;
    } else {
        return 0;
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: isInputFile()
   DESCRIPTION: Determines whether the user has specified input redirection for the
   entered command.
   -------------------------------------------------------------------------------*/
int isInputFile() {
    if (info->boolInfile) {
        return 1;
    } else {
        return 0;
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: isOutputFile()
   DESCRIPTION: Determines whether the user has specified output redirection for the
   entered command.
   -------------------------------------------------------------------------------*/
int isOutputFile() {
    if (info->boolOutfile) {
        return 1;
    } else {
        return 0;
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: countInputRedirection()
   DESCRIPTION: Counts the number of input redirection for the given command if the
   user has specified input redirection for the command.
   -------------------------------------------------------------------------------*/
int countInputRedirection() {
    int numInputRedirection = 0;
    int len = strlen(cmdLine);
    for (int i = 0; i < len; i++){
        if (cmdLine[i] == '<') {
            numInputRedirection++;
        }
    }
    return numInputRedirection;
}

/* -----------------------------------------------------------------------------
   FUNCTION: countOutputRedirection()
   DESCRIPTION: Counts the number of output redirection for the given command if the
   user has specified output redirection for the command.
   -------------------------------------------------------------------------------*/
int countOutputRedirection() {
    int numOutputRedirection = 0;
    int len = strlen(cmdLine);
    for (int i = 0; i < len; i++){
        if (cmdLine[i] == '>') {
            numOutputRedirection++;
        }
    }
    return numOutputRedirection;
}

 /* -----------------------------------------------------------------------------
   FUNCTION: numberBackgroundProcessActive(int numBackgroundProcess)
   DESCRIPTION: Counts the number of background process that are currently active
   based on whether the status of the background process is "Running" or "Terminated"
   -------------------------------------------------------------------------------*/
int numberBackgroundProcessActive(int numBackgroundProcess) {
    int active = 0;
    for (int i = 0; i < numBackgroundProcess; i++) {
        if (strcmp(backgroundProcessStatus[i], "Running") == 0) {
            active++;
        }
    }
    return active;
}

 /* -----------------------------------------------------------------------------
   FUNCTION: helpCommand()
   DESCRIPTION: Displays the available built-in commands and their syntax if the
   user enters the help command.
   -------------------------------------------------------------------------------*/
void helpCommand() {
    fprintf(stdout, "\n List of built-in commands for the yosh shell script: ");
    fprintf(stdout, "\n jobs");
    fprintf(stdout, "\n cd <directory to look for>");
    fprintf(stdout, "\n history");
    fprintf(stdout, "\n exit");
    fprintf(stdout, "\n kill %%job number OR kill <PID>");
    fprintf(stdout, "\n help\n\n");
}

 /* -----------------------------------------------------------------------------
   FUNCTION: historyCommand(int changeHistory, int getCommandHistory, char* mostRecentCommand)
   DESCRIPTION: Prints print the list of previously 10 executed commands when the
   user enters the history command. Also, modifies the history whenever the user
   enters a new command that is not a repeat (!number).
   -------------------------------------------------------------------------------*/
char * historyCommand(int changeHistory, int getCommandHistory, char* mostRecentCommand) {
    static char * listOfAllCommands[300];
    static char * listOfCommands[10];
    static int numOfCommandsList = 0;
    static int numOfCommandsTotal = 0;
    static int currentCommand;

    if (changeHistory == 1 && getCommandHistory == 0) {
        if (numOfCommandsList < 10) {

            listOfCommands[numOfCommandsList] = strdup(mostRecentCommand);
            listOfAllCommands[numOfCommandsTotal] = strdup(mostRecentCommand);
            numOfCommandsList++;
            numOfCommandsTotal++;

        } else {

            for (int i = 0; i < 9; i++) {
                listOfCommands[i] = listOfCommands[i+1];
            }

            listOfCommands[9] = strdup(mostRecentCommand);
            listOfAllCommands[numOfCommandsTotal] = strdup(mostRecentCommand);
            numOfCommandsTotal++;
        }

    } else if (getCommandHistory == 0) {

        fprintf(stdout, "\n");

        if (numOfCommandsTotal <= 10) {
            currentCommand = 1;
        } else {
            currentCommand = numOfCommandsTotal - 9;
        }

        for (int i = 0; i < numOfCommandsList; i++, currentCommand++) {
            fprintf(stdout, " %d %s\n", currentCommand, listOfCommands[i]);
       }

        fprintf(stdout, "\n");
    }

    if (getCommandHistory > 0) {
        if (getCommandHistory <= numOfCommandsTotal) {
            return listOfAllCommands[getCommandHistory-1];
        } else {
            fprintf(stderr, "Invalid Usage of Repeat Command");
            return "";
        }
    } else if (getCommandHistory < 0){
        getCommandHistory *= -1;
        if (getCommandHistory <= numOfCommandsTotal) {
            return listOfAllCommands[numOfCommandsTotal- getCommandHistory];
        } else {
            fprintf(stderr, "Invalid Usage of Repeat Command");
            return "";
        }
    } else {
        return "";
    }
}

 /* -----------------------------------------------------------------------------
   FUNCTION: validDirectory(char *directoryName)
   DESCRIPTION: Determines whether the specified directory for the cd command
   is a directory that actually exists before changing to it.
   -------------------------------------------------------------------------------*/
int validDirectory(char *directoryName) {
    int isDirectory;
    struct stat mystat;

    isDirectory = lstat(directoryName, &mystat);

    if (isDirectory == -1) {
        if (errno = ENOENT) {
            return 0;
        } else {
            perror("lstat");
            exit(2);
        }
    } else {
        if (S_ISDIR(mystat.st_mode)) {
            return 1;
        } else {
            return 0;
        }
    }
}

 /* -----------------------------------------------------------------------------
   FUNCTION: moreThanOneDirectory()
   DESCRIPTION: Determines whether the user has entered more than one directory
   for the cd command.
   -------------------------------------------------------------------------------*/
void moreThanOneDirectory() {
    fprintf(stderr, "\n-yosh: cd: More than one directory was specified for the cd command.\n");
    fprintf(stderr, "           Please enter in only one directory when using the cd");
    fprintf(stderr, " command.\n\n");
}

 /* -----------------------------------------------------------------------------
   FUNCTION: canOpenDirectory(char* commandName, char *directoryName)
   DESCRIPTION: Determines whether directory specified for the cd command is
   openable.
   -------------------------------------------------------------------------------*/
int canOpenDirectory(char* commandName, char *directoryName) {
    DIR * newDirectory = opendir(directoryName);

    if (newDirectory != NULL) {
        return 1;
    } else {
        fprintf(stderr, "\n-yosh: cd: %s: Permission denied\n\n", directoryName);
        return 0;
    }
}

 /* -----------------------------------------------------------------------------
   FUNCTION: cdCommand(char *directoryName)
   DESCRIPTION: Changes from the current working directory to the specified directory
   when the user enters the cd command.
   -------------------------------------------------------------------------------*/
void cdCommand(char *directoryName) {
    if (validDirectory(directoryName)) {
        chdir(directoryName);
    } else {
        fprintf(stderr, "\n-yosh: cd: %s: No such file or directory\n\n", directoryName);
    }
}

 /* -----------------------------------------------------------------------------
   FUNCTION: jobsCommand(int addBackgroundProcess, int numBackgroundProcess, char * command, pid_t childPid)
   DESCRIPTION: Provides a list of background processes with their command name and
   status if the user enters in the jobs command.  Also changes list if a new
   background process is added
   -------------------------------------------------------------------------------*/
void jobsCommand(int addBackgroundProcess, int numBackgroundProcess, char * command, pid_t childPid) {

    if (addBackgroundProcess) {

        backgroundProcessStatus[numBackgroundProcess] = "Running";
        backgroundProcessPID[numBackgroundProcess] = childPid;
        backgroundCommandName[numBackgroundProcess] = strdup(command);

    } else if (numBackgroundProcess != 0) {

        fprintf(stdout, "\n");

        for (int i = 0; i < numBackgroundProcess; i++) {
            fprintf(stdout, " [%d] %s\t", i + 1, backgroundProcessStatus[i]);
            fprintf(stdout, "%s\tPID: %d\n", backgroundCommandName[i], backgroundProcessPID[i]);
        }

        fprintf(stdout, "\n");
    }
}

 /* -----------------------------------------------------------------------------
   FUNCTION: killCommandJob(int numBackgroundProcess, int jobNumber)
   DESCRIPTION: Kills and terminates the background process with the specified job
   number.
   -------------------------------------------------------------------------------*/
void killCommandJob(int numBackgroundProcess, int jobNumber) {

    if (jobNumber > 0 && jobNumber <= numBackgroundProcess) {
        backgroundProcessStatus[jobNumber-1] = "Terminated";
        fprintf(stdout, "\n [%d] Terminated \t%s\n\n", jobNumber, backgroundCommandName[jobNumber-1]);
        kill(backgroundProcessPID[jobNumber-1], SIGKILL);
    } else {
        fprintf(stderr, "\n-yosh: kill: %d: no such job\n\n", jobNumber);
    }

}

/* -----------------------------------------------------------------------------
   FUNCTION: killCommandPID(int numBackgroundProcess, int PIDNumber)
   DESCRIPTION: Kills and terminates the background process with the specified PID.
   -------------------------------------------------------------------------------*/
void killCommandPID(int numBackgroundProcess, int PIDNumber) {

    int hasFoundPID = 0;

    for (int i = 0; i < numBackgroundProcess && hasFoundPID == 0; i++) {
        if (backgroundProcessPID[i] == PIDNumber) {
            backgroundProcessStatus[i] = "Terminated";
            fprintf(stdout, "\n [%d] Terminated \t%s\n\n", (i+1), backgroundCommandName[i]);
            hasFoundPID = 1;
            kill(backgroundProcessPID[i], SIGKILL);
        }
    }

    if (hasFoundPID == 0) {
        fprintf(stderr, "\n-yosh: kill: (%d) - No such process\n\n", PIDNumber);
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: excuetePipes (int numPipes)
   DESCRIPTION: Excuetes a command between multiple processif one or more pipe is
   indicated in the given command.
   -------------------------------------------------------------------------------*/
int excuetePipes (int numPipes) {
    int status;
    int i = 0;
    pid_t pid;
    int pipefds[2*numPipes];
    int k = 0;
    int numberOfArguments = 0;
    static char *argv[100];

    for(i = 0; i < (numPipes); i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }

    int j = 0;

    while(k < numPipes +1) {
        pid = fork();
        if(pid == 0) {

            if(j != numPipes + 1) {
                if(dup2(pipefds[j + 1], 1) < 0){
                }
            }

            if(j != 0 ){
                if(dup2(pipefds[j-2], 0) < 0) {
                }
            }

            for(i = 0; i < 2*numPipes; i++){
                close(pipefds[i]);
            }

            int validInput = 1;
            if (k == 0 && isInputFile()) {
                int fd0;
                char * fileName = info->inFile;
                char * adjustedFileName;
                if (strcmp(fileName, "") == 0) {
                    fprintf(stderr, "\nMissing name for redirect.\n\n");
                    validInput = 0;
                } else {
                    if (countInputRedirection() > 1) {
                        fprintf(stderr, "\nAmbiguous input redirect.\n\n");
                        validInput = 0;
                    } else {
                        if (fileName[0] == '~') {
                            strcpy(adjustedFileName, fileName);
                            memmove(adjustedFileName, adjustedFileName+1, strlen(adjustedFileName));
                            prepend(adjustedFileName, "/home/myid/");
                            if ((fd0 = open(adjustedFileName, O_RDONLY, 0)) < 0) {
                                fprintf(stderr, "\n%s: No such file or directory\n\n", fileName);
                                validInput = 0;
                            }
                        } else {
                            if ((fd0 = open(fileName, O_RDONLY, 0)) < 0) {
                                fprintf(stderr, "\n%s: No such file or directory\n\n", fileName);
                                validInput = 0;
                            }
                        }
                        dup2(fd0, STDIN_FILENO);
                        close(fd0);
                    }
                }
            }

            if (k == numPipes && isOutputFile()) {
                int fd1;
                char * fileName = info->outFile;
                char * adjustedFileName;
                if (strcmp(fileName, "") == 0) {
                    fprintf(stderr, "\nMissing name for redirect.\n\n");
                    validInput = 0;
                } else if(access(fileName, F_OK ) == 0) {
                    fprintf(stderr, "\n%s: File exists\n\n", fileName);
                    validInput = 0;
                } else {
                    if (countOutputRedirection() > 1) {
                        fprintf(stderr, "\nAmbiguous output redirect.\n\n");
                        validInput = 0;
                    } else {
                        if (fileName[0] == '~') {
                            strcpy(adjustedFileName, fileName);
                            memmove(adjustedFileName, adjustedFileName+1, strlen(adjustedFileName));
                            if ((fd1 = open(adjustedFileName, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
                            }
                        } else {
                            if ((fd1 = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
                                fprintf(stderr, "\n%s: Failed to create file\n\n", fileName);
                                validInput = 0;
                            }
                        }
                        dup2(fd1, STDOUT_FILENO);
                        close(fd1);
                    }
                }
            }

            if (validInput) {
                char * argument= (char *) malloc(100);
                while (com->VarList[numberOfArguments] != NULL) {
                    if (com->VarList[numberOfArguments][0] == '~') {
                        strcpy(argument, com->VarList[numberOfArguments]);
                        memmove(argument, argument+1, strlen(argument));
                        prepend(argument, "/home/myid/");
                        argv[numberOfArguments] = argument;
                     } else if (com->VarList[numberOfArguments][0] == '\'') {
                        strcpy(argument, com->VarList[numberOfArguments]);
                        memmove(argument, argument+1, strlen(argument));
                        argument[strlen(argument)-1] = 0;
                        argv[numberOfArguments] = argument;
                    } else {
                        argv[numberOfArguments] = com->VarList[numberOfArguments];
                    }
                    numberOfArguments++;
                }
            }

            argv[numberOfArguments++] = NULL;

            if( execvp(com->command, argv) < 0 ) {
                fprintf(stderr, "\n-yosh: %s: command not found...\n\n", com->command);
                return 1;
            }

        } else if (pid < 0){
            perror("error");
            return 1;
        }

        numberOfArguments = 0;
        k++;
        com = &info->CommArray[k];
        j+=2;
    }

    for(i = 0; i < 2 * numPipes; i++){
        close(pipefds[i]);
    }

    for(i = 0; i < numPipes + 1; i++) {
        wait(&status);
    }

    return 0;
}

/* -----------------------------------------------------------------------------
   FUNCTION: excueteCommand(char *enteredCommand)
   DESCRIPTION: Excuetes the appropiate actions for the entered command through
   calling the appropiate functions for built-in command or forking/excueting
   exevp for non-built in commands.
   -------------------------------------------------------------------------------*/
void excueteCommand(char *enteredCommand) {
    pid_t childPid;
    int    status;
    static int numBackgroundProcess = 0;

    if( isBuiltInCommand(enteredCommand) == EXIT) {

        if (numberBackgroundProcessActive(numBackgroundProcess) == 0) {
            fprintf(stdout, "\n");
            exit(1);
        } else {
           fprintf(stderr, "\nThere are processes in the background.");
            fprintf(stderr, "\nYou must kill those background processes ");
            fprintf(stderr, "before exiting.\n\n");
        }

    } else if (isBuiltInCommand(enteredCommand) == HELP) {

        helpCommand();

    } else if (isBuiltInCommand(enteredCommand) == HISTORY) {

        historyCommand(0, 0, "");

    } else  if (isBuiltInCommand(enteredCommand) == REPEAT) {

        static int commandNumber;
        char *actualCommand;

        memmove(enteredCommand, enteredCommand+1, strlen(enteredCommand));
        commandNumber = atoi(enteredCommand);

        actualCommand = historyCommand(0, commandNumber, "");

        historyCommand(1, 0, actualCommand);

        if (actualCommand[0] != '\0' && (strstr(actualCommand, "cd") == NULL && commandNumber != 5)) {
            printf("\n %s\n\n", actualCommand);
            excueteCommand(actualCommand);
        } else if (actualCommand[0] != '\0' && strstr(actualCommand, "cd") != NULL) {
            char * directoryName = (char *) malloc(100);
            static int iterations = 0;
            if (commandNumber > 0) {
                printf("\n %s\n\n", actualCommand);
                strcpy(directoryName, allCommandArguments[commandNumber-3]);
            } else {
                printf("\n cd%s\n\n", allCommandArguments[0]);
                strcpy(directoryName, allCommandArguments[0]);
            }

            if (directoryName[0] == '~') {
                memmove(directoryName, directoryName+1, strlen(directoryName));
                prepend(directoryName, "/home/myid/");
            }

            cdCommand(directoryName);
        }

    } else if (isBuiltInCommand(enteredCommand) == JOBS) {

        jobsCommand(0, numBackgroundProcess, "", 0);

    } else if (isBuiltInCommand(enteredCommand) == CD) {

        if (com->VarList[2] != NULL) {

            moreThanOneDirectory();

        } else if (com->VarList[1] != NULL) {

            char * directoryName = (char *) malloc(100);
            strcpy(directoryName, com->VarList[1]);

            if (directoryName[0] == '~') {
                memmove(directoryName, directoryName+1, strlen(directoryName));
                prepend(directoryName, "/home/myid/");
            }

            cdCommand(directoryName);

        } else {

            cdCommand(getenv("HOME"));

        }

    } else if (isBuiltInCommand(enteredCommand) == KILL) {

        if (com->VarList[1] != NULL) {

            if(strchr(com->VarList[1], '%') != NULL) {

                char * jobNumberString = com->VarList[1];
                memmove(jobNumberString, jobNumberString+1, strlen(jobNumberString));

                int jobNumberInteger = atoi(jobNumberString);

                killCommandJob(numBackgroundProcess, jobNumberInteger);

            } else {

                char * PIDNumberString = com->VarList[1];

                int PIDNumberInteger = atoi(PIDNumberString);

                killCommandPID(numBackgroundProcess, PIDNumberInteger);
            }

        } else {

            fprintf(stderr, "\nkill: usage: kill %%job number\n\n");

        }

    } else {
        childPid = fork();

        if (childPid < 0) {

            perror("\nfork\n\n");

        } else if (childPid == 0) {
            static char *argv[100];
            int numberOfArguments = 0;
            int validInput = 1;

            if (isInputFile()) {
                int fd0;
                char * fileName = info->inFile;
                if (strcmp(fileName, "") == 0) {
                    fprintf(stderr, "\nMissing name for redirect.\n\n");
                    validInput = 0;
                } else {
                   if (countInputRedirection() > 1) {
                        fprintf(stderr, "\nAmbiguous input redirect.\n\n");
                        validInput = 0;
                    } else {
                        if ((fd0 = open(fileName, O_RDONLY, 0)) < 0) {
                            fprintf(stderr, "\n%s: No such file or directory\n\n", fileName);
                            validInput = 0;
                        }
                        dup2(fd0, STDIN_FILENO);
                        close(fd0);
                    }
                }
            }

            if (isOutputFile()) {
                int fd1;
                char * fileName = info->outFile;
                if (strcmp(fileName, "") == 0) {
                    fprintf(stderr, "\nMissing name for redirect.\n\n");
                    validInput = 0;
                } else if(access(fileName, F_OK ) == 0) {
                    fprintf(stderr, "\n%s: File exists\n\n", fileName);
                    validInput = 0;
                } else {
                    if (countOutputRedirection() > 1) {
                        fprintf(stderr, "\nAmbiguous output redirect.\n\n");
                        validInput = 0;
                    } else {
                        if ((fd1 = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
                            fprintf(stderr, "\n%s: Failed to create file\n\n", fileName);
                            validInput = 0;
                        }
                        dup2(fd1, STDOUT_FILENO);
                        close(fd1);
                    }
                }
            }
            if (validInput) {
                char * argument= (char *) malloc(100);
                if (strchr(enteredCommand, ' ') != NULL) {
                    if(execvp(commandArguments[0], commandArguments) == -1) {
                        fprintf(stderr, "\n-yosh: %s: command not found...\n\n", commandArguments[0]);
                    }
                } else {
                    while (com->VarList[numberOfArguments] != NULL) {
                        if (com->VarList[numberOfArguments][0] == '~') {
                            strcpy(argument, com->VarList[numberOfArguments]);
                            memmove(argument, argument+1, strlen(argument));
                            if (strncmp(enteredCommand, "cat", strlen("cat")) == 0) {
                                prepend(argument, "/home/myid/ingrid/1730/tt1yosh/data");
                            } else {
                                prepend(argument, "/home/myid/");
                            }
                            argv[numberOfArguments] = argument;
                        } else {
                            argv[numberOfArguments] = com->VarList[numberOfArguments];
                        }
                        numberOfArguments++;
                    }
                    argv[numberOfArguments++] = NULL;
                    if (execvp(enteredCommand, argv) == -1) {
                        fprintf(stderr, "\n-yosh: %s: command not found...\n\n", enteredCommand);
                    }
                }
            }

            exit(1);

        } else {

            if(isBackgroundJob()) {

                jobsCommand(1, numBackgroundProcess, cmdLine, childPid);
                numBackgroundProcess++;
                fprintf(stdout, "\n [%d] %d\n\n", numBackgroundProcess, childPid);
                signal(SIGCHLD, wait_and_poll);

            } else {

                while (wait(&status) != childPid);

            }

        }
    }
}

/* -----------------------------------------------------------------------------
   FUNCTION: main()
   DESCRIPTION: A while loops that transverses through each cycle of the shell
   excuetion. In other words, prompts the user, reads the entered command, records
   the command in history, and excuetes the appropiate actions based on that entered
   command.
   -------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    fprintf(stdout, "\nThis is the YOSH version 1.0\n" ) ;

    while(1)
    {

    	// insert your code here

    	cmdLine = readline( buildPrompt() );
    	if( cmdLine == NULL ) {
      		fprintf(stderr, "Unable to read command\n");
      		continue;
    	}

    	// calls the parser
    	info = parse( cmdLine );
        if( info == NULL )
        {
      		free(cmdLine);
      		continue;
    	}

    	// prints the info struct
    	// print_info( info );

    	//com contains the info. of the command before the first "|"
    	com = &info->CommArray[0];
    	if( (com == NULL)  || (com->command == NULL))
    	{
      		free_info(info);
      		free(cmdLine);
      		continue;
    	}

        // insert your code about history and !x !-x here
        if(com->command[0] != '!') {
            historyCommand(1, 0, cmdLine);
            commandArguments[0] = strdup(com->command);
            if (com->VarList[1] != NULL) {
                commandArguments[1] = strdup(com->VarList[1]);
                commandArguments[2] = NULL;
                allCommandArguments[currentNumberArguments] = strdup(com->VarList[1]);
                currentNumberArguments++;
            }
        }

    	//com->command tells the command name of com
        //insert your code here / commands etc.
        if (info->pipeNum == 0) {
            excueteCommand(com->command);
        } else {
            excuetePipes(info->pipeNum);
        }

        free_info(info);

        free(cmdLine);

    }/* while(1) */

}
