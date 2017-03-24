#include <arpa/inet.h>
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#define LOKAL_PORT 80
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 
char filePath[100];
char requestType[10];
char httpVer[10];
FILE* fileInfo;
void sendHeader(int fileDescriptor);
int main ()
{

  struct sockaddr_in  lok_adr;
  struct stat sd_buff;
  int sd, ny_sd;
  int file; 
  char buffer[BUFSIZ];
  char *token;
  pid_t process_id =0;
  pid_t sid = 0;
  pid_t fid = 0;
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  // For at operativsystemet skal reservere porten når tjeneren dør
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

  // Initierer lokal adresse
  lok_adr.sin_family      = AF_INET;
  lok_adr.sin_port        = htons((u_short)LOKAL_PORT); 
  lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);

  // Kobler sammen socket og lokal adresse
  if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
  {
      printf("Prosess %d er knyttet til port %d.\n", getpid(), LOKAL_PORT);
      //Child
      process_id = fork();
      if(process_id<0)
	    exit(1);
      //Kill parent
      if(process_id>0)
	    exit(0);
      //session
      sid = setsid();
      if(sid<0)
	    exit(1);
      chdir("/home/nan3000/Desktop/webtjener/webroot");
      if(chroot("/home/nan3000/Desktop/webtjener/webroot")!=0){
          perror("chroot /home/nan3000/Desktop/webtjener/webroot");
          return 1;
      }
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);  

  }
  else
    exit(1);

  // Venter på forespørsel om forbindelse
  listen(sd, BAK_LOGG);
  struct sigaction sigchld_action = {
        .sa_handler = SIG_DFL,
          .sa_flags = SA_NOCLDWAIT
  };
  sigaction(SIGCHLD, &sigchld_action, NULL);
  int result;
  int removeFile;
	char indexPath[10] = "index.html";
  //char webroot[8] ="webroot";
  while(1){ 
    // Aksepterer mottatt forespørsel
    ny_sd = accept(sd, NULL, NULL);
    //memset(filePath,0,100);
   if(0==fork()) {
	int j=5;
	int k = 0;
	


      recv(ny_sd,buffer,sizeof(buffer),0);

	token = strtok(buffer, " ");
	while(token != NULL)
	{
		if(k==0)
			strcpy(requestType, token);

		else if(k==1)
			strcpy(filePath, token);

		else if(k==2)
			strcpy(httpVer, token);
	
		else
			break;

		token = strtok(NULL, " ");
		k++;
	}
	
	if(strlen(filePath) == 1)
	{
		file=open(indexPath,O_RDONLY);
	}

	else
	{
		file=open(filePath,O_RDONLY);
		
	}

      
      fstat(file,&sd_buff);
      dup2(ny_sd, 1); // redirigerer socket til standard utgang
      setuid(500);
      setgid(500);
      sendHeader(ny_sd);
      sendfile(ny_sd,file,0,sd_buff.st_size);
      close(file);
      // Sørger for å stenge socket for skriving og lesing
      // NB! Frigjør ingen plass i fildeskriptortabellen
      shutdown(ny_sd, SHUT_RDWR);
    }

    else {
      exit(0);
      close(ny_sd);
    }
  }
  return 0;
}
void sendHeader(int fileDescriptor)
{
	
	char data[1024];
	char buff[1024];
	const char* bla;
	//fileInfo = popen("file -i /xml.xml | awk '{print $2,$3}'","r");
	time_t currtime;
	struct tm *loc_time;
	
	currtime = time(NULL);
	loc_time = localtime(&currtime);
	
	
	
	sprintf(buff,"HTTP/1.1 200 OK\r\n");
	send(fileDescriptor,buff,strlen(buff),0);
	
	strftime(buff,sizeof(buff),"Date: %a, %d %b %Y %H:%M:%S %Z\r\n", loc_time);
	send(fileDescriptor,buff,strlen(buff),0);
	
	sprintf(buff,"Server: hal9000 ver 1.0 (Ubuntu)\r\n");
	send(fileDescriptor,buff,strlen(buff),0);
	
	//fgets(data,sizeof(data),fileInfo);
	//fclose(fileInfo);
	//send(fileDescriptor,data,strlen(data),0);
	
	sprintf(buff,"\n");
	send(fileDescriptor,buff,strlen(buff),0);

}
void fileType(char type)
{
	
}
