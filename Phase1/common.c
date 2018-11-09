#include "common_impl.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


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

  memset(&serveur_addr, 0, sizeof(struct sockaddr_in));
  serveur_addr.sin_port = htons(0);
  serveur_addr.sin_family = AF_INET;
  serveur_addr.sin_addr.s_addr = INADDR_ANY;

  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  if (bind(sock, (struct sockaddr *)&serveur_addr, sizeof(struct sockaddr_in)) == -1)
  ERROR_EXIT("Bind error")

  getsockname(sock, (struct sockaddr *)&serveur_addr, (socklen_t *)&size_len);


  /*  modifie le parametre port_num */
  *port = ntohs(serveur_addr.sin_port);

  /* renvoie le numero de descripteur */
  return sock;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
