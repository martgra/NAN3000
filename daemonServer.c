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
#include <sqlite3.h>
#define LOKAL_PORT 80
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 
char fileExt[100];
char filePath[100];
char filePathCopy[100];
char requestType[10];
char httpVer[10];
char input[50];
char *sqlOutput;
void sendHeader(int,int,int);
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
      freopen("error.log","w+",stderr);

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
  while(1){ 
    // Aksepterer mottatt forespørsel
    ny_sd = accept(sd, NULL, NULL);
   if(0==fork()) {
	int j=5;
	int k = 0;
  int p =0;
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
		file=open(indexPath,O_RDONLY);

	else
		file=open(filePath,O_RDONLY);	

  
      
      
  fstat(file,&sd_buff);
  dup2(ny_sd, 1); // redirigerer socket til standard utgang
  setuid(500);
  setgid(500);
     
  if(access(filePath,F_OK)!=-1)
  {
    sendHeader(ny_sd,0,sd_buff.st_size);
    if(strcmp(requestType,"GET")==0)
    {
      sendfile(ny_sd,file,0,sd_buff.st_size);
      close(file);
    }
    if(strcmp(requestType,"POST"))
    {
      //DO POST REQUESTS
    }
    if(strcmp(requestType,"PUT"))
    {
      //DO PUT REQUESTS
    }
    if(strcmp(requestType,"DELETE"))
    {
      //DO DELETE REQUEST
    }
    
  }
  else
  {
    sendHeader(ny_sd,0,sd_buff.st_size);
    char fileDoesentExist[]="<h1>404 FILE DOES NOT EXIST</h1>";
    send(ny_sd,fileDoesentExist,strlen(fileDoesentExist),0);
  }
  
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

void sendHeader(int fileDescriptor,int rQ,int size)
{


  int i;
	char buff[1024];
  char contentType[256];
  
	time_t currtime;
	struct tm *loc_time;
	
  strcpy(filePathCopy,filePath); // inholdet i filePath blir kopiert til filePathCopy

  char *p = filePathCopy;

  for(int i=0;i<strlen(filePathCopy);i++)
  {
    if(filePathCopy[i]!='.')
    {
      p++;
    }
    else
    {
      p++;
      break;
    }
  }
 
  if(strcmp(p,"\0")==0) sprintf(contentType,"text/html");
  if(strcmp(p,"html")==0) sprintf(contentType,"text/html");
  if(strcmp(p,"txt")==0) sprintf(contentType,"text/plain");
  if(strcmp(p,"png")==0) sprintf(contentType,"image/png");
  if(strcmp(p,"jpg")==0) sprintf(contentType,"image/jpg");
  if(strcmp(p,"xml")==0) sprintf(contentType,"application/xml");
  if(strcmp(p,"xslt")==0) sprintf(contentType,"application/xml+xslt");
  if(strcmp(p,"css")==0) sprintf(contentType,"text/css");
  if(strcmp(p,"js")==0) sprintf(contentType,"application/javascript");

	currtime = time(NULL);
	loc_time = localtime(&currtime);
  if(rQ==0) // hvis filen eksisterer
  {
      sprintf(buff,"HTTP/1.1 200 OK\r\n");
      send(fileDescriptor,buff,strlen(buff),0);
  }
      
  if(rQ==1) //Hvis filen ikke eksisterer
  {
      sprintf(buff,"HTTP/1.1 404 Not Found\r\n");
      send(fileDescriptor,buff,strlen(buff),0);
  }
  
  
	strftime(buff,sizeof(buff),"Date: %a, %d %b %Y %H:%M:%S %Z\r\n", loc_time);
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Server: HAL9000 ver1.0 (Ubuntu)\r\n");
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Content-Length: %d\r\n",size);
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Content-Type: %s\r\n",contentType);
	send(fileDescriptor,buff,strlen(buff),0);

	sprintf(buff,"\r\n");
	send(fileDescriptor,buff,strlen(buff),0);

}