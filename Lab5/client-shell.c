#include  <stdio.h>
#include  <fcntl.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int cd(char **args)
{
	if ( args[1] == NULL )
	{
		fprintf(stderr, "Argument should be of form \"cd <dir>\"\n");
	}
	else if( args[2] !=NULL)
	{
		fprintf(stderr, "Argument should be of form \"cd <dir>\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0) {
      perror("Directory does not exist");
    }
  }
  return 1;
}


int server(char **args, char ** serverinfo)
{
	if(args[1]==NULL || args[2]==NULL)
	{
		fprintf(stderr, "Too less arguments. Argument should be of form \"server <server-IP> <server-port>\"\n");
		return -1;
	}
	else if(args[3]!=NULL)
	{
		fprintf(stderr, "Too many arguments. Argument should be of form \"server <server-IP> <server-port>\"\n");
		return -2;
	}
	else
	  {
	    strcpy(serverinfo[0],args[1]);
	    strcpy(serverinfo[1],args[2]);
	  }
	return 1;
}


int getf1(char **args,char ** serverinfo)
{
  int dopipe = 0;
  int redirect = 0;
  char* filename;
  char* command;
  
  if(args[1]==NULL)
    {
      fprintf(stderr, "Too less arguments. Argument should be of form \"getf1 <filename>\"\n");
      return -1;
    }
  
  if(args[2]!=NULL)
    {
      if(strcmp(args[2],">")==0)
	{
	  if(args[3]==NULL)
	    {
	      fprintf(stderr, "Too less arguments. Argument should be of form \"getf1 <filename>\"\n");
	      return -1;
	    }
	  else
	    {
	      redirect = 1;
	      filename = args[3];
	    }
	}
      else if(strcmp(args[2],"|")==0)
	{
	  if(args[3]==NULL)
	    {
	      fprintf(stderr, "Too less arguments. Argument should be of form \"getf1 <filename>\"\n");
	      return -1;
	    }
	  else
	    {
	      dopipe = 1;
	      command = args[3];
	    }
	}
      else
	{
	  fprintf(stderr, "Too many arguments. Argument should be of form \"getf1 <filename>\"\n");
	  return -2;
	}
    }
  
  pid_t pid, wpid;
  int status,fd;

  pid = fork();
  if (pid == 0) 
    {
      // Child process
      if(redirect) {
	int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	dup2(fd,1);
      }
	
      char * myargs[6];
      myargs[0]="./get-one-file-sig";
      myargs[1]=args[1];
      myargs[2]=serverinfo[0];
      myargs[3]=serverinfo[1];
      myargs[4]="display";
      myargs[5]=NULL;
      if (execvp(myargs[0], myargs) == -1) {
	fprintf(stderr, "Executable ./get-one-file-sig doesn't exist.\n");
      }
      exit(EXIT_FAILURE);
    } 
  else if (pid < 0) 
    {
      // Error forking
      fprintf(stderr, "Error forking.\n");
    } 
  else 
    {
      // Parent process
      do 
	{
	  wpid = waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  return 1;
}


int getsq(char **args,char ** serverinfo)
{
	if(args[1]==NULL)
	{
		fprintf(stderr, "Too less arguments. Argument should be of form \"getf1 <filename>\"\n");
		return -1;
	}
	else
	{
		int i=1;
		while(args[i]!=NULL)
		{
			
			pid_t pid, wpid;
			int status;

			pid = fork();
			if (pid == 0) 
			{
			// Child process
				char * myargs[6];
				myargs[0]="./get-one-file-sig";
				myargs[1]=args[i];
				myargs[2]=serverinfo[0];
				myargs[3]=serverinfo[1];
				myargs[4]="nodisplay";
				myargs[5]=NULL;
				printf("Getting file %s\n",args[i]);
				if (execvp(myargs[0], myargs) == -1) {
					fprintf(stderr, "Executable ./get-one-file-sig doesn't exist.\n");
				}
				exit(EXIT_FAILURE);
			} 
			else if (pid < 0) 
			{
				// Error forking
				fprintf(stderr, "Error forking.\n");
			} 
			else 
			{
				// Parent process
				do 
				{
					wpid = waitpid(pid, &status, WUNTRACED);
				} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			}
			i++;
		}
	}
	return 1;
}

int linux_default(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) 
	{
	// Child process
		if (execvp(args[0], args) == -1) {
			fprintf(stderr, "No such command found.\n");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		fprintf(stderr, "Error forking.\n");
	} 
	else 
	{
		// Parent process
		do 
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
}

void  main(void)
{
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	
	char * serverinfo[2];
	bool serverInitialized=0;
	
	serverInitialized=0;
        serverinfo[0] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	serverinfo[1] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	///////////
	
	
	while (1)
 {           
		printf("Hello>");     
		bzero(line, MAX_INPUT_SIZE);
		gets(line);           
		//~ printf("Got command %s\n", line);
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		//do whatever you want with the commands, here we just print them
		if(strcmp(tokens[0],"cd") == 0)
		{
			cd(tokens);
		}
		else if(strcmp(tokens[0],"server")==0)
		{
			int a=server(tokens,serverinfo);
			if(a>=0)
			{
				serverInitialized=1;
				printf("Server details changed succesfully.\nIP: %s \nPort: %s \n",serverinfo[0],serverinfo[1]);
			}
		}
		else if(strcmp(tokens[0],"getf1")==0)
		{
			if(serverInitialized==0)
			{
				fprintf(stderr, "Server details have not been initialized.\n");
			}
			else
			{
				getf1(tokens,serverinfo);
			}
				
		}
		else if(strcmp(tokens[0],"getsq")==0)
		{
			if(serverInitialized==0)
			{
				fprintf(stderr, "Server details have not been initialized.\n");
			}
			else
			{
				getsq(tokens,serverinfo);
			}
				
		}
		else
		{
			linux_default(tokens);
		}


		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++)
		{
			free(tokens[i]);
		}
		free(tokens);
	}
	free(serverinfo[0]);
	free(serverinfo[1]);
}

                
