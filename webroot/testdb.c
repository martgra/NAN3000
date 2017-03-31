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
int main()
{
      sqlite3 *db;
      char *zErrMsg =0;
      int sqFd;
      const char* data = "Callback function called";

      sqFd = sqlite3_open("testb",&db);
      if( sqFd ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      }
      else{
        fprintf(stderr, "Opened database successfully\n");
      }
      char settInnData[1024];
      sprintf(settInnData,"INSERT INTO Informasjon VALUES(14,'testUser15','90012300');");
      perror(settInnData);
      sqFd = sqlite3_exec(db,settInnData,skriv_rad,(void*)data,&zErrMsg);
      sqlite3_close(db);

}
