#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<netdb.h>


/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}
#define taille_nom 20
/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
  char name[taille_nom];
  int socket;
  int rank;
  u_short port;
  pid_t pid;
  struct sockaddr_in ad_client;
};

struct nom_machines{
  char nom[taille_nom];
  struct nom_machines *next;
};
// Liste chainée des noms de machines

typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int *port);
int receive_length(int socket, char * buffer);
int indice_b (char buffer[]);
int do_connect(int socket, struct sockaddr_in *serv_add);
int do_accept(int s, struct sockaddr* adresse);
void do_listen(int socket, int maxconn);
void handle_message(int s, const void *input, int length);
int get_ip_from_hostname(char * hostname , char* ip);
