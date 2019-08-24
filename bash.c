#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>   
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#define buff_size 1000000

char s[10000];
char s_save[10000];
char inputstring[10000];
char homedir[10000];
char currdir[10000];
char prevdir[10000];
char* name;
int stx=0;

void init(){
	name = (char *)malloc(100*sizeof(char));
	name = getlogin();
	/*
		strcpy(homedir,"/home/");
		strcat(homedir,name);
	*/
	getcwd(name,100*sizeof(char));
	/**/
	strcpy(homedir,name);
	strcpy(currdir,name);
	/**/
}

void truncatedir(char * name)
{
	if(strstr(name,homedir)!=NULL)
	{
		char * match;
		int len = strlen(homedir);
		while(match = strstr(name,homedir))
		{
			*match = '\0';
			strcat(name,match+len);
		}
		char temp[1000] ="~";
		strcat(temp,name);
		strcpy(name,temp); 
	}
}

void printdetails(){
	name = getlogin();
	printf("<\x1b[31m%s\x1b[0m@", name);
	gethostname(name,100*sizeof(char));
	printf("\x1b[33m%s\x1b[0m:", name);
	// getcwd(name,100*sizeof(char));
	strcpy(name,currdir);
	truncatedir(name);
	printf("\x1b[32m%s\x1b[0m> ", name);
}



void cmd_pwd(){
	char* tok = strtok(s," ");
	if(strcmp(tok,"pwd")==0)
	{
		char* name = (char *)malloc(100*sizeof(char));
		// strcpy(currdir,name);
		getcwd(name,100*sizeof(char));
		printf("%s\n",name);
		stx=1;
	}
	strcpy(s,s_save);

}

void cmd_cd(){
	char tempdir[10000];
	strcpy(tempdir,currdir);
	char* tok;
	tok = strtok(s," ");
	if(strcmp(tok,"cd")==0)
	{
		tok = strtok(0," ");
		if(tok==NULL){
			chdir(homedir);
		}
		else if(strcmp(tok,".")==0){
			chdir(currdir);
		}
		else if(strcmp(tok,"..")==0){
			chdir("..");
		}
		else if(strcmp(tok,"-")==0){
			chdir(prevdir);
		}
		else if(tok[0]=='~')
		{
			char temp[10000];
			strcpy(temp,homedir);
			if(sizeof(tok)>1)
				strcat(temp,&tok[1]);
			if(chdir(temp)!=0) perror(temp);
		}
		else{
			if(chdir(tok)!=0)
			{
				perror(tok);
			}
		}
		stx=1;
	}
	getcwd(name,100*sizeof(char));
	strcpy(currdir,name);
	if(strcmp(currdir,tempdir)!=0)
	{
		strcpy(prevdir,tempdir);
	}
	char s[10000];
	strcpy(s,s_save);
}

void strrem(char *s, char c){ 
	int j, n = strlen(s); 
	for (int i=j=0; i<n; i++) if (s[i] != c) s[j++] = s[i]; 
		s[j] = '\0'; 
} 

void cmd_echo()
{
	char* tok;
	tok = strtok(s," ");
	if(strcmp(tok,"echo")==0)
	{
		tok = strtok(0,"");
		strrem(tok,'"');
		strrem(tok,'\'');
		printf("%s\n", tok);
		stx=1;
	}
	strcpy(s,s_save);
}

int fl_a=0,fl_l=0,fl_dir=0,ctdir=0;
struct stat fileStat;
void call(int id,char filename[])
{
	if(stat(filename,&fileStat) < 0)    
		return;
	if(fl_a==0 && id==0)
		if(filename[0]=='.') return;
	if(fl_l==1){
		printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
		printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
		printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
		printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
		printf(" %d",fileStat.st_nlink);
		printf(" %s",getpwuid(fileStat.st_uid)->pw_name);
		printf(" %s",getgrgid(fileStat.st_gid)->gr_name);
		printf(" %d",fileStat.st_size);
		char time[1025]="";
		strftime (time, 1024,"%b %d %R",localtime (&fileStat.st_mtime));
		printf(" %s",time);
	}
	printf(" %s\n",filename);
}

void cmd_ls()
{
	if(strstr(s,"&")==NULL)
	{
		char * tok = strtok(s," ");
		if(strcmp(tok,"ls")==0)
		{
			char dirarr[100][1000];
			tok = strtok(0," ");
			ctdir = 0;fl_l = 0;fl_a = 0;
			while(tok!=NULL)
			{
				if(tok[0]=='-')
				{
					if(strstr(tok,"a")) fl_a=1;
					if(strstr(tok,"l")) fl_l=1;
				}
				else
				{
					strcpy(dirarr[ctdir++],tok);
				}
				tok = strtok(0," ");
			}
			// while(ctdir--)
			// 	printf("%s\n", dirarr[ctdir]);
			char filename[1024]="";
			DIR *d;
			struct dirent *dir;
			if(ctdir==0)
			{
				d = opendir(currdir);
				struct dirent *dir;
				if (d)
				{
					while ((dir = readdir(d)) != NULL)
					{
						strcpy(filename,dir->d_name);
						call(0,filename);
					}
				}
			}
			else
			{
				for (int i = 0; i < ctdir; ++i)
				{
					name = dirarr[i];
					if(stat(name,&fileStat)<0){
						perror(name);
						continue;
					}
					if(S_ISDIR(fileStat.st_mode))
					{
						char temp[10000]={},tempcurr[10000]={},tempprev[10000]={};
						strcpy(tempcurr,currdir);
						strcpy(tempprev,prevdir);
						strcpy(temp,"cd ");
						strcat(temp,name);
						strcpy(s,temp);
						strcpy(temp,name);
						cmd_cd();
						if(i!=0) printf("\n");
						if(strstr(currdir,tempcurr))
							printf("./%s:\n",temp);
						else
							printf("%s:\n",currdir );
						stx=0;
						d = opendir(currdir);
						struct dirent *dir;
						if (d)
						{
							while ((dir = readdir(d)) != NULL)
							{
								strcpy(filename,dir->d_name);
								call(0,filename);
							}
						}
						// call(d);
						strcpy(temp,"cd ");
						strcat(temp,tempcurr);
						strcpy(s,temp);
						cmd_cd();
						strcpy(prevdir,tempprev);
					}
					else
					{
						if(i!=0) printf("\n");
						strcpy(filename,name);
						call(1,filename);
					}
				}
			}

			stx=1;
		}
		strcpy(s,s_save);
	}
}


void cmd_fg()
{
	char * tok = strtok(s," ");
	char *args[100]={}; 
	int i=0;
	while(tok!=NULL)
	{
		args[i++]=tok;
		tok = strtok(0," ");
	}
	int status,st=0; 
	pid_t pid = fork(); 
	if (pid == -1)
	{
		printf("can't fork, error occured\n"); 
		exit(EXIT_FAILURE); 
	} 
	else if (pid == 0){ 
		execvp(args[0],args); 
		st=1;
	} 
	else{  
		if (wait(&status) <= 0 || st==1) perror(args[0]);
	} 
	stx=1;
	strcpy(s,s_save);
}

int counter=0;
void cmd_bg()
{
	if(strstr(s,"&"))
	{
		counter++;
		strtok(s,"&");
		char * tok = strtok(s," ");
		char *args[100]={}; 
		int i=0;
		while(tok!=NULL)
		{
			args[i++]=tok;
			tok = strtok(0," ");
		}
		int status,st=0; 
		pid_t pid = fork(),sid; 
		if (pid == -1)
		{
			printf("can't fork, error occured\n"); 
			exit(EXIT_FAILURE); 
		}
		else if(pid > 0) { 
			stx=1;
			return;
		} 
		else
		{
			stx=1;
			printf("[%d] %d\n",counter,getpid());
			// fflush(stdout);
			// execvp(args[0],args);
			if(execvp(args[0],args)!=-1) {
				// addbg(getpid());
			}else perror(args[0]);
			exit(0);
		}
	}
	strcpy(s,s_save);
}

void cmd_pinfo()
{
	char * tok = strtok(s," ");
	if(strcmp(tok,"pinfo")==0)
	{
		char location[1000] = "/proc/";
		tok = strtok(0," ");
		if(tok==NULL || tok=="")
		{
			strcat(location,"self/stat");
		}
		else
		{
			char * temp = strtok(0," ");
			if(temp != NULL)
			{
				printf("Single argument expected\n");
				strcpy(s,s_save);
				stx=1;return;
			}
			else{
				strcat(location,tok);
				strcat(location,"/stat");

			}
		}

		int fileIn = open(location, O_RDONLY);
		if (fileIn<0){ 
			printf("Invalid pid\n");
			strcpy(s,s_save);
			stx=1;return;
		}
		char *data = (char *) calloc(buff_size,sizeof(char));
		int temp = read(fileIn, data, buff_size);
		tok = strtok(data," ");
		char *args[100]={};int i=0;
		while(tok!=NULL)
		{
			args[i++]=tok;
			tok = strtok(0," ");
		}
		printf("pid -- %s\n",args[0]);
		printf("Process Status -- %s\n",args[2]);
		printf("memory -- %s\n",args[22]);
		strcpy(location,"/proc/");
		strcat(location,args[0]);
		strcat(location,"/exe");
		strcpy(data,"");
		ssize_t len = readlink(location, data, buff_size);
		if (len != -1) data[len] = '\0';
		truncatedir(data);
		printf("Executable Path -- %s\n",data);
		stx=1;
	}
	strcpy(s,s_save);
}

int pidarr[100000];

void functions()
{
	stx=0;
	cmd_bg();if(stx==1) return;
	cmd_pinfo();if(stx==1) return;
	cmd_cd();if(stx==1) return;
	cmd_pwd();if(stx==1) return;
	cmd_echo();if(stx==1) return;
	cmd_ls();if(stx==1) return;
	cmd_fg();if(stx==1) return;
}

void trim()
{
	char str[10000];
	strcpy(str,s);
	int n = strlen(str); 
	int i = 0, j = -1; 
	int spaceFound = 0; 
	while (++j < n && str[j] == ' '); 
	while (j < n) 
	{ 
		if (str[j] != ' ') 
		{ 
			if ((str[j] == '.' || str[j] == ',' || 
				str[j] == '?') && i - 1 >= 0 && 
				str[i - 1] == ' ') 
				str[i - 1] = str[j++]; 
			else
				str[i++] = str[j++]; 
			spaceFound = 0; 
		} 
		else if (str[j++] == ' ') 
		{ 
			if (spaceFound==0) 
			{ 
				str[i++] = ' '; 
				spaceFound = 1; 
			} 
		} 
	} 
	char temp_str[10000];
	if (i <= 1)
		strncpy(temp_str,str,i+1);
	else
		strncpy(temp_str,str,i);
	strcpy(s,temp_str);
	strcpy(s_save,temp_str);
}

void wrapper()
{
	char str[10000];
	strcpy(str,s);
	char *end_str;
	char *token = strtok_r(str, ";", &end_str);
	int i=0;
	while (token != NULL)
	{
		if(!i++==0){printf("\n");}
		strcpy(s,token);
		trim();
		functions();
		token = strtok_r(NULL, ";", &end_str);
	}
}



int main()
{
	memset(s,0,10000*sizeof(char));
	memset(pidarr,-1,100000);
	memset(inputstring,0,10000*sizeof(char));
	init();
	while(strcmp(s,"exit")!=0)
	{
		printdetails();
		fgets(s,10000,stdin);
		strtok(s,"\n");
		strcpy(inputstring,s);
		wrapper();
	}
}
