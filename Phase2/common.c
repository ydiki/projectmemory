#include "common_impl.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>


//void affichages_dans_les_tubes(int **fd1,int **fd2,int *num_procs){



int creer_socket(u_short *port)
{
  int sock = 0;
  int yes=1;
  struct sockaddr_in serveur_addr;
  int size_len = sizeof(struct sockaddr_in);

  // creation de la socket
  sock =socket(PF_INET,SOCK_STREAM,0);

  // verification de la validite de la socket
  if (sock == -1)
  ERROR_EXIT("Socket error");

  // socket option pour le probleme "already in use"
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  ERROR_EXIT("Error setting socket options");

  memset(&serveur_addr, 0,size_len);
  serveur_addr.sin_port = htons(0);
  serveur_addr.sin_family = AF_INET;
  serveur_addr.sin_addr.s_addr = INADDR_ANY;

  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  if (bind(sock, (struct sockaddr *)&serveur_addr, size_len) == -1)
  ERROR_EXIT("Bind error")

  getsockname(sock, (struct sockaddr *)&serveur_addr, (socklen_t *)&size_len);


  /*  modifie le parametre port_num */
  *port = ntohs(serveur_addr.sin_port);

  /* renvoie le numero de descripteur */
  return sock;
}

int do_accept(int s, struct sockaddr* adresse) {

	socklen_t length__client = sizeof(struct sockaddr);

	int fda=-1;

	do{
		fda = accept(s, adresse, &length__client);
		if (fda < 0 ) {
				perror("accept :");
		}
	} while (fda < 0);

	return fda;
}
int do_connect(int socket, struct sockaddr_in* server_add) {
	int con = connect(socket, (struct sockaddr *) server_add, sizeof(struct sockaddr));
	if (con == -1) {
		perror("connect");
	}
	else{
		//printf("connexion done :) \n");
	}
	return con;
}

void do_listen(int socket, int maxconn) {

	int listen_return = listen(socket, maxconn);

	if (listen_return == -1) {
		perror("Listen");
	}
}

int readline(int sock, void *buffer, size_t len){

    int i;
    char* str_tmp = (char*) buffer;

		char c;

		int a = read(sock,&c,1);


    if(a <= 0)
      return a;


		i = 0;
    while( a > 0 && i < len && c != '\n'){
      str_tmp[i] = c;
      a = read(sock,&c,1);
      i++;
    }
    str_tmp[i] = '\0';
    return i;
}


int sendall(int fd, void *buffer, int size){
  ssize_t s = 0;
  do{
    s += write(fd,(char *)buffer+s,size-s);
  } while(s != size);
  return s;
}

int receive_all(int fd, void *buffer, int size){
  ssize_t s = 0;
  do{
    s += read(fd,(char *)buffer+s,size-s);
  } while(s != size);
  return s;
}
void do_read(int s, void *input, int length) {

	int r = 0;

	do {
		r = read(s, input + r, length - r);

		if (r == -1) {
			perror("read ERROR ");
		}
	}
	while (r!=length);
}


//this_function_is_not_ours
int get_ip_from_hostname(char * hostname , char* ip)
{  struct hostent *he;
   struct in_addr **addr_list;
   int i;
   if ( (he = gethostbyname( hostname ) ) == NULL)
   { herror("ERROR gethostbyname");
     return 1;}
   addr_list = (struct in_addr **) he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {   strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;}
    return 1;
}

in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }

    return (((struct sockaddr_in6*)sa)->sin6_port);
}
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
