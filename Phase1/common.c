#include "common_impl.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*
void affichages_dans_les_tubes(int fd1[],int fd2[],int *num_procs){
	int rfds=1; //taille courante du pollfds
	int p,i;
	struct pollfd pollfds[2][100];
	for(i=0,i<num_procs,i++){
	pollfds[0][i].fd=fd1[i][1];
	pollfds[0][i].events=POLLIN;
	pollfds[1][i].fd=fd2[i][1]
	pollfds[1][i].events=POLLIN;
}

	     while(1)
{
poll(pollfds,rfds,-1);
/*je recupere les infos sur les tubes de redirection
jusqu'à ce qu'ils soient inactifs (ie fermes par les
processus dsm ecrivains de l'autre cote ...)*/
//if(users->pollfds.revents==POLLIN)

//};






//}



int receive_length(int socket, char *buffer)
{
	int k = 0;
  int length=500;
	memset(buffer, '\0', length);

	while (recv(socket,buffer,1,0) > 0)
	{
		if (buffer[k] == '\n')
		{
			buffer[k] = '\0';
			break;
		}

		k++;

		if (k == length-1)
			break;
	}
  return k;
}

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
				perror("Voici l'erreur concernant l'acceptation : ");
		}

	/*	if (fda == -1 && errno == EINTR) {
				// EINTR The accept() function was interrupted by a signal
				//         that was caught before a valid connection arrived.

		}*/

	} while (fda < 0);

	return fda;
}

int do_connect(int socket, struct sockaddr_in* server_add) {
	int con = connect(socket, (struct sockaddr *) server_add, sizeof(struct sockaddr));
	if (con == -1) {
		printf("Connexion ERROR ");
	}
	else{
		printf("connexion done :) \n");
	}
	return con;
}

void do_listen(int socket, int maxconn) {

	int listen_return = listen(socket, maxconn);

	if (listen_return == -1) {
		error("Listen ERROR");
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
   { herror("ERROR in gethostbyname");
     return 1;}
   addr_list = (struct in_addr **) he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {   strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;}
    return 1;
}


/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
