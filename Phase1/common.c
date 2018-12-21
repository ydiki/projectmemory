#include "common_impl.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>


//void affichages_dans_les_tubes(int **fd1,int **fd2,int *num_procs){





int indice_b (char buffer[]){
	int j=0;
	while (buffer[j] != '/0' & buffer[j] != '\n'){
		j++;
	}
	return j;
}
int creer_socket(int *port)
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
		error("Listen");
	}
}

ssize_t read_line(int fd, char * buf, size_t len){

	/* Variables */
	int i;
	char c;
	int ret;
	char * ptr;
	ptr = buf;
	int cnt = 0;

	/* How to read */
	for (i = 0 ; i < len; i++){

		ret = read(fd, &c, 1);

		if( ret == 1 ){
			ptr[cnt++] = c;

			if( c == '\n'){
				ptr[cnt] = '\0';
				return i+1;
			}
		}
		else if( 0 == ret ) {
			ptr[cnt] = '\0';
			break;
		}
	}
	ptr[len] = '\0';

	/* Empty stdin buffer in the case of too large user_input */
	if( fd == STDIN_FILENO && i == len ){
		char ss[10*100];
		ret = read(fd, ss, 10*100);
	}

	return i;
}
int send_all(int fd, void *buffer, int size){
  ssize_t ret = 0;
  do{
    ret += write(fd,(char *)buffer+ret,size-ret);
  } while(ret != size);
  return ret;
}

int recv_all(int fd, void *buffer, int size){
  ssize_t ret = 0;
  do{
    ret += read(fd,(char *)buffer+ret,size-ret);
  } while(ret != size);
  return ret;
}
void do_read(int s, const void *input, int length) {

	int r = 0;

	do {
		r = read(s, input + r, length - r);

		if (r == -1) {
			error("read ERROR ");
		}
	}
	while (r!=length);
}

void handle_message(int s, const void *input, int length) {

	int p = 0;

	do {
		p = send(s, input + p, length - p, 0);

		if (p == -1) {
			error("Send ERROR ");
		}
	}
	while (p!=length);
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
