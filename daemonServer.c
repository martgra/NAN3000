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
int stop =0;
char buff3[BUFSIZ];
char sqlTableName[1024];
char sqlID[1024];
char sqlName[1024];
char sqlTelefon[1024];
void sendHeader(int,int,int,char *);
int skriv_rad(void *,int , char **,char **);
int main ()
{

  struct sockaddr_in  lok_adr;
  struct stat sd_buff;
  int sd, ny_sd,fdE,fdI;
  int file, four_four; 
  char buffer[BUFSIZ],bufferPOST[BUFSIZ];
  char *token,*tokenPOSTREQ;
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
      close(STDERR_FILENO);
      fdE = open("errorlog.log",O_RDWR | O_TRUNC);
      dup2(2,fdE);

      fdI = open("stdout.txt", O_RDWR | O_TRUNC);
      dup2(4,fdI);


      chdir("/home/nan3000/Desktop/webtjener/webroot");
      if(chroot("/home/nan3000/Desktop/webtjener/webroot")!=0){
          perror("chroot /home/nan3000/Desktop/webtjener/webroot");
          return 1;
      }
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
     


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
  strcpy(bufferPOST,buffer);
  //perror(bufferPOST);
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

  /**POST REQUEST DATA**/
  int post_req =0;
  tokenPOSTREQ = strtok(bufferPOST, "\n");
  char contentLength[1024];
  char contentData[BUFSIZ];
  
  while(token != NULL)
	{
		
		if(post_req==7)
			strcpy(contentData, tokenPOSTREQ);
		if(post_req>7)
			break;
		tokenPOSTREQ = strtok(NULL, "\n");
		post_req++;
	}

  /**POST REQUEST DATA**/
  
  perror(contentData);
  perror(contentLength);
  int iiii = strlen(contentData); 

 
  
  
  


  dup2(ny_sd, 1); // redirigerer socket til standard utgang

  	if(strlen(filePath) == 1)
		file=open(indexPath,O_RDONLY);
	
	else
		file=open(filePath,O_RDONLY);	

/**DATABASE TILKOBLING**/
      sqlite3 *db;
      char *zErrMsg =0;
      int sqFd;
      const char* data = "Callback function called";
      
      
      
      
	
  setuid(500);
  setgid(500);
  fstat(file,&sd_buff);
  char *restToken;
  char restID[sizeof(filePath)];
  char restDB[sizeof(filePath)];
  char restTB[sizeof(filePath)];
  char rFPathCpy[sizeof(filePath)];
  strcpy(rFPathCpy,filePath);
  restToken = strtok(rFPathCpy,"/");
  int rI=0;
  memset(&restID[0], 0, sizeof(restID));
  while(restToken!=NULL)
  {
    if(rI==0)
      strcpy(restDB,restToken);
    if(rI==1)
      strcpy(restTB,restToken);
    if(rI==2)
      strcpy(restID,restToken);
    if(rI>2)
      break;
		restToken = strtok(NULL, "/");
	  rI++;
  }
  
    if(strcmp(requestType,"GET")==0)
    { 
      if(strcmp(restDB,"testb")==0)
      {
        if(strcmp(restTB,"Informasjon")==0)
        {
          sqFd = sqlite3_open("testb",&db);
          //sqFd = sqlite3_open("testb",&db);
      if( sqFd ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      }
      else{
        fprintf(stderr, "Opened database successfully\n");
      }
          char sqlQuerry[512];
          memset(&buff3[0], 0, sizeof(buff3)); // clearer alt som ligger i bufferet fra før
          if(strcmp(restID,"\0")==0)
          {
            sprintf(sqlQuerry,"SELECT * FROM Informasjon;");
          }
          else
          {
            sprintf(sqlQuerry,"SELECT * FROM Informasjon WHERE ID = %s;",restID);
          }
            
          char xM[5];
          sprintf(xM,".xml");
          char xmlVersion[512];
          char dbName[512];
          char DTDINFO[512];
          
          /**XML SOM SENDES**/
          sprintf(xmlVersion,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
          strcat(buff3,xmlVersion);
          sprintf(DTDINFO,"<!DOCTYPE testb SYSTEM \"dbDTD.dtd\">\n");
          strcat(buff3,DTDINFO);
          sprintf(dbName,"<testb>\n");
          strcat(buff3,dbName);
          sqFd = sqlite3_exec(db,sqlQuerry,skriv_rad,(void*)data,&zErrMsg);
          sqlite3_close(db);
          sprintf(dbName,"</testb>\n");
          strcat(buff3,dbName);
          /**XML SOM SENDES**/

          sendHeader(ny_sd,0,strlen(buff3),xM);
          send(ny_sd,buff3,strlen(buff3),0);
        }
      }
          

      if(access(filePath,F_OK)!=-1)
      {  
        sendHeader(ny_sd,0,sd_buff.st_size,filePath);
        sendfile(ny_sd,file,0,sd_buff.st_size);
        close(file); 
      }
      else
      {
        four_four = open("404.html",O_RDONLY);
        fstat(four_four,&sd_buff);
        sendHeader(ny_sd,1,sd_buff.st_size,filePath);
        sendfile(ny_sd,four_four,0,sd_buff.st_size);
        close(four_four);
      }
    }

    if(strcmp(requestType,"POST")==0)
    {
      sqFd = sqlite3_open("testb",&db);
      if( sqFd ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      }
      else{
        fprintf(stderr, "Opened database successfully\n");
      }
      // char replyPOST[512];
      // sprintf(replyPOST,"SERVEREN SVARER TILBAKE :):)");
      // send(ny_sd,replyPOST,strlen(replyPOST),0);
      //DO POST REQUESTS
      char settInnData[1024];
      sprintf(settInnData,"INSERT INTO Informasjon VALUES(14,'testUser15','90012300');");
      perror(settInnData);
      sqFd = sqlite3_exec(db,settInnData,skriv_rad,(void*)data,&zErrMsg);
      sqlite3_close(db);
    
    }
    if(strcmp(requestType,"PUT"))
    {
      //DO PUT REQUESTS
    }
    if(strcmp(requestType,"DELETE"))
    {
      //DO DELETE REQUEST
    }
    
  
  else
  {
    
    
  }
  
  // Sørger for å stenge socket for skriving og lesing
  // NB! Frigjør ingen plass i fildeskriptortabellen
    shutdown(ny_sd, SHUT_RDWR);
  } 

  else 
  {
    exit(0);
    close(ny_sd);
  }
  }
  return 0;
}

void sendHeader(int fileDescriptor,int rQ,int size,char *fileInfo)
{


  int i;
	char buff[1024];
  char contentType[256];
  
	time_t currtime;
	struct tm *loc_time;
	
  strcpy(filePathCopy,fileInfo); // inholdet i filePath blir kopiert til filePathCopy

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
  if(strcmp(p,"dtd")==0) sprintf(contentType,"application/xml-dtd");
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
      sprintf(contentType,"text/html");
      sprintf(buff,"HTTP/1.1 404 Not Found\r\n");
      send(fileDescriptor,buff,strlen(buff),0);
  }
  
  
	strftime(buff,sizeof(buff),"Date: %a, %d %b %Y %H:%M:%S %Z\r\n", loc_time);
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Server: HAL9000 ver1.0 (Ubuntu)\r\n");
	send(fileDescriptor,buff,strlen(buff),0);
  
sprintf(buff,"%s\r\n",filePath);
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Content-Length: %d\r\n",size);
	send(fileDescriptor,buff,strlen(buff),0);

  sprintf(buff,"Content-Type: %s\r\n",contentType);
	send(fileDescriptor,buff,strlen(buff),0);

	sprintf(buff,"\r\n");
	send(fileDescriptor,buff,strlen(buff),0);

}
int skriv_rad(void *ubrukt,
              int ant_kol, 
              char **kolonne,
              char **kol_navn) {

  int i;
  char buff2[1024];
  char tbName[1024];
  sprintf(tbName,"<Informasjon>\n");
  strcat(buff3,tbName);
  
  for(i=0; i<ant_kol; i++)
  {
    
    snprintf(buff2,sizeof(buff2),"<%s>%s</%s>\n", kol_navn[i], kolonne[i],kol_navn[i] );
    strcat(buff3,buff2);
    write(4,buff2,strlen(buff2));
  }
  sprintf(tbName,"</Informasjon>\n");
  strcat(buff3,tbName);
  //strcat(buff3,buff4);
  
  
  
  return 0;
}
// void parseXML(char data)
// {
//   char dataCopy[BUFSIZ];
//   strcpy(dataCopy,data);

// }