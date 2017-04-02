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
char buff3[BUFSIZ];
char sqlID[1024];
char sqlName[1024];
char sqlTelefon[1024];
void sendHeader(int, int, int, char *);
int skriv_rad(void *, int, char **, char **);
void parseXMLData(char *);
void dataBaseError(int, sqlite3 *);
void databaseTilkobling(sqlite3 *,char *);
int main()
{

  struct sockaddr_in lok_adr;
  struct stat sd_buff;
  int sd, ny_sd, fdE, fdI;
  int file, four_four;
  char buffer[BUFSIZ], bufferPOST[BUFSIZ];
  char *token, *tokenPOSTREQ;
  sqlite3 *db;
  pid_t process_id = 0;
  pid_t sid = 0;
  pid_t fid = 0;
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // For at operativsystemet skal reservere porten når tjeneren dør
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  // Initierer lokal adresse
  lok_adr.sin_family = AF_INET;
  lok_adr.sin_port = htons((u_short)LOKAL_PORT);
  lok_adr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Kobler sammen socket og lokal adresse
  if (0 == bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)))
  {
    printf("Prosess %d er knyttet til port %d.\n", getpid(), LOKAL_PORT);
    //Child
    process_id = fork();
    if (process_id < 0)
      exit(1);
    //Kill parent
    if (process_id > 0)
      exit(0);
    //session
    sid = setsid();
    if (sid < 0)
      exit(1);
    close(STDERR_FILENO);
    fdE = open("errorlog.log", O_RDWR | O_TRUNC);
    dup2(2, fdE);

    fdI = open("stdout.txt", O_RDWR | O_TRUNC);
    dup2(4, fdI);

    chdir("/home/isrema/Desktop/webtjener/webroot");
    if (chroot("/home/isrema/Desktop/webtjener/webroot") != 0)
    {
      perror("chroot /home/isrema/Desktop/webtjener/webroot");
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
      .sa_flags = SA_NOCLDWAIT};
  sigaction(SIGCHLD, &sigchld_action, NULL);

  char indexPath[10] = "index.html";

  while (1)
  {
    // Aksepterer mottatt forespørsel
    ny_sd = accept(sd, NULL, NULL);
    
    if (0 == fork())
    {
      dup2(ny_sd, 1);
      int k = 0;

      recv(ny_sd, buffer, sizeof(buffer), 0);
      
      strcpy(bufferPOST, buffer);
     
      token = strtok(buffer, " ");
      memset(&filePath[0], 0, sizeof(filePath));
      while (token != NULL)
      {
        if (k == 0)
          strcpy(requestType, token);

        else if (k == 1)
          strcpy(filePath, token);

        else if (k == 2)
          strcpy(httpVer, token);

        else
          break;

        token = strtok(NULL, " ");
        k++;
      }

      /**POST REQUEST DATA**/
      int post_req = 0;
      
      char *contentDATA;
      contentDATA = strstr(bufferPOST,"<Informasjon>");
      // while (tokenPOSTREQ != NULL)
      // {
      //   perror(tokenPOSTREQ);
      //   // if (post_req == 11) //11 for firefox/chrome, 7 for curl inntil videre
      //   //   strcpy(contentData, tokenPOSTREQ);
      //   // if (post_req > 11)
      //   //   break;
      //   tokenPOSTREQ = strtok(NULL, "\n");
      //   post_req++;
      // }

      /**POST REQUEST DATA**/
      if (strlen(filePath) == 1)
        file = open(indexPath, O_RDONLY);

      else
        file = open(filePath, O_RDONLY);

      setuid(1001);
      setgid(1001);
      fstat(file, &sd_buff);
      /**BRUKES FOR Å SJEKKE FILSTIEN TIL REST API**/
      char *restToken;
      char restID[sizeof(filePath)];
      char restDB[sizeof(filePath)];
      char restTB[sizeof(filePath)];
      char rFPathCpy[sizeof(filePath)];
      strcpy(rFPathCpy, filePath);
      restToken = strtok(rFPathCpy, "/");
      int rI = 0;
      memset(&restID[0], 0, sizeof(restID));
      memset(&restDB[0], 0, sizeof(restDB));
      memset(&restTB[0], 0, sizeof(restTB));
      while (restToken != NULL)
      {
        if (rI == 0)
          strcpy(restDB, restToken);
        if (rI == 1)
          strcpy(restTB, restToken);
        if (rI == 2)
          strcpy(restID, restToken);
        if (rI > 2)
          break;
        restToken = strtok(NULL, "/");
        rI++;
      }
      char sqlQuerry[512];
      if (strcmp(requestType, "GET") == 0)
      {
        if (strcmp(restDB, "testb") == 0)
        {
          if (strcmp(restTB, "Informasjon") == 0)
          {
            memset(&buff3[0], 0, sizeof(buff3)); // clearer alt som ligger i bufferet fra før
            if (strcmp(restID, "\0") == 0)
            {
              sprintf(sqlQuerry, "SELECT * FROM Informasjon;");
            }
            else
            {
              sprintf(sqlQuerry, "SELECT * FROM Informasjon WHERE ID = %s;", restID);
            }
            /**XML SOM SENDES**/
            strcat(buff3, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            strcat(buff3, "<!DOCTYPE testb SYSTEM \"dbDTD.dtd\">\n");
            strcat(buff3, "<testb>\n");
            
            databaseTilkobling(db,sqlQuerry);

            strcat(buff3, "</testb>\n");
            sendHeader(ny_sd, 0, strlen(buff3), ".xml");
            send(ny_sd, buff3, strlen(buff3), 0);
            /**XML SOM SENDES**/
          }
        }

        if (access(filePath, F_OK) != -1)
        {
          sendHeader(ny_sd, 0, sd_buff.st_size, filePath);
          sendfile(ny_sd, file, 0, sd_buff.st_size);
          close(file);
        }
        else
        {
          four_four = open("404.html", O_RDONLY);
          fstat(four_four, &sd_buff);
          sendHeader(ny_sd, 1, sd_buff.st_size, filePath);
          sendfile(ny_sd, four_four, 0, sd_buff.st_size);
          close(four_four);
        }
      }

      if (strcmp(requestType, "POST") == 0)
      {
        if (strcmp(filePath, "testb/Informasjon"))
        {
          parseXMLData(contentDATA); //leser xml data og lagrer variablene i globale char arrays.

          sprintf(sqlQuerry, "INSERT INTO Informasjon VALUES(%s,'%s','%s');", sqlID, sqlName, sqlTelefon);
          databaseTilkobling(db,sqlQuerry);
        }
      }
      if (strcmp(requestType, "PUT") == 0)
      {
        if (strcmp(restDB, "testb") == 0)
        {
          if (strcmp(restTB, "Informasjon") == 0)
          {
            if (strcmp(restID, "\0") != 0)
            {
              parseXMLData(contentDATA);
              sprintf(sqlQuerry, "UPDATE Informasjon SET Navn='%s',Telefon='%s' WHERE ID=%s",sqlName,sqlTelefon,restID);
              databaseTilkobling(db,sqlQuerry);
            }
            
          }
        }
      }
    
    if (strcmp(requestType, "DELETE") == 0)
    {
      if (strcmp(restDB, "testb") == 0)
      {
        if (strcmp(restTB, "Informasjon") == 0)
        {
          if (strcmp(restID, "\0") == 0)
          {
            sprintf(sqlQuerry, "DELETE FROM Informasjon;");
          }
          else
          {
            sprintf(sqlQuerry, "DELETE FROM Informasjon WHERE ID=%s", restID);
          }
          databaseTilkobling(db,sqlQuerry);
        }
      }
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

void sendHeader(int fileDescriptor, int rQ, int size, char *fileInfo)
{

  int i;
  char buff[1024];
  char contentType[256];

  time_t currtime;
  struct tm *loc_time;

  strcpy(filePathCopy, fileInfo); // inholdet i filePath blir kopiert til filePathCopy

  char *p = filePathCopy;

  for (int i = 0; i < strlen(filePathCopy); i++)
  {
    if (filePathCopy[i] != '.')
    {
      p++;
    }
    else
    {
      p++;
      break;
    }
  }

  if (strcmp(p, "\0") == 0)
    sprintf(contentType, "text/html");
  if (strcmp(p, "html") == 0)
    sprintf(contentType, "text/html");
  if (strcmp(p, "txt") == 0)
    sprintf(contentType, "text/plain");
  if (strcmp(p, "png") == 0)
    sprintf(contentType, "image/png");
  if (strcmp(p, "jpg") == 0)
    sprintf(contentType, "image/jpg");
  if (strcmp(p, "dtd") == 0)
    sprintf(contentType, "application/xml-dtd");
  if (strcmp(p, "xml") == 0)
    sprintf(contentType, "application/xml");
  if (strcmp(p, "xslt") == 0)
    sprintf(contentType, "application/xml+xslt");
  if (strcmp(p, "css") == 0)
    sprintf(contentType, "text/css");
  if (strcmp(p, "js") == 0)
    sprintf(contentType, "application/javascript");

  currtime = time(NULL);
  loc_time = localtime(&currtime);
  if (rQ == 0) // hvis filen eksisterer
  {
    sprintf(buff, "HTTP/1.1 200 OK\r\n");
    send(fileDescriptor, buff, strlen(buff), 0);
  }

  if (rQ == 1) //Hvis filen ikke eksisterer
  {
    sprintf(contentType, "text/html");
    sprintf(buff, "HTTP/1.1 404 Not Found\r\n");
    send(fileDescriptor, buff, strlen(buff), 0);
  }

  strftime(buff, sizeof(buff), "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", loc_time);
  send(fileDescriptor, buff, strlen(buff), 0);

  sprintf(buff, "Server: HAL9000 ver1.0 (Ubuntu)\r\n");
  send(fileDescriptor, buff, strlen(buff), 0);

  sprintf(buff, "%s\r\n", filePath);
  send(fileDescriptor, buff, strlen(buff), 0);

  sprintf(buff, "Content-Length: %d\r\n", size);
  send(fileDescriptor, buff, strlen(buff), 0);

  sprintf(buff, "Content-Type: %s\r\n", contentType);
  send(fileDescriptor, buff, strlen(buff), 0);

  sprintf(buff, "\r\n");
  send(fileDescriptor, buff, strlen(buff), 0);
}
int skriv_rad(void *ubrukt, int ant_kol, char **kolonne, char **kol_navn)
{

  int i;
  char buff2[1024];
  char tbName[1024];
  sprintf(tbName, "<Informasjon>\n");
  strcat(buff3, tbName);

  for (i = 0; i < ant_kol; i++)
  {
    snprintf(buff2, sizeof(buff2), "<%s>%s</%s>\n", kol_navn[i], kolonne[i], kol_navn[i]);
    strcat(buff3, buff2);
    write(4, buff2, strlen(buff2));
  }
  sprintf(tbName, "</Informasjon>\n");
  strcat(buff3, tbName);
  return 0;
}
void parseXMLData(char *PostData)
{

  const char needle1[] = "<ID>";
  const char needle2[] = "<Navn>";
  const char needle3[] = "<Telefon>";

  char haystackCopy[strlen(PostData)]; // strtok ødelegger det den kjører på, så det mekkes kopi
  strcpy(haystackCopy, PostData);
  //strstr peker til begynnelse av første forekomst av "needle" i "haystack"
  //siden vi vil det skal starte etter needle dytter vi indeksen framover lik lengden til needle
  //strtok returnerer en "substring" fra der den begynner til første forekomst av delimiter
  //contentDATAatet av strtok(strstr) kopieres til id
  strcpy(sqlID, strtok((strstr(haystackCopy, needle1) + sizeof(needle1) - 1), "<"));

  strcpy(haystackCopy, PostData);
  strcpy(sqlName, strtok((strstr(haystackCopy, needle2) + sizeof(needle2) - 1), "<"));
  strcpy(haystackCopy, PostData);
  strcpy(sqlTelefon, strtok((strstr(haystackCopy, needle3) + sizeof(needle3) - 1), "<"));
}

void databaseTilkobling(sqlite3 *sqldatabase,char *sqlString)
{
	char *zErrMsg = 0;
  int sqlFileDescriptor;
  const char *data = "Callback function called";
	
	sqlFileDescriptor = sqlite3_open("testb", &sqldatabase);
  if (sqlFileDescriptor)
  {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(sqldatabase));
  }
  else
  {
    fprintf(stderr, "Opened database successfully\n");
  }
	sqlFileDescriptor = sqlite3_exec(sqldatabase, sqlString, skriv_rad, (void *)data, &zErrMsg);
  sqlite3_close(sqldatabase);
}
