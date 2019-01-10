#include "common_impl.h"
#include "dsm.h"

int main(int argc, char **argv)
{

  /*declarations des variables*/
  dsm_proc_t *proc_array = NULL;// Tableau (des strucs) contenant les infos des processus dsmwrap

  int socket_lanceur,socket_listen_procs; // les sockets
  u_short port_lanceur;
  u_short port_listen_procs; // les ports
  int my_rank;     // Rank du processus
  int num_procs;     // nombre des processus estimé du ficher machine file
  int i;
  char name[taille_nom];
  struct sockaddr_in *lanceur_addr; //struct adresse du dsmexec
  struct sockaddr_in *tmp_addr; //struct adresse du dsmexec
  lanceur_addr=malloc(sizeof(struct sockaddr_in));
  tmp_addr=malloc(sizeof(struct sockaddr_in));
  char ip_lanceur[INET_ADDRSTRLEN]; //Ip adresse


   //On recupere l'adresse du lanceur à partir du hostname donné
   port_lanceur = atoi(argv[2]);
   get_ip_from_hostname(argv[1],ip_lanceur);

   //On recupere le nom de la machine
   gethostname(name, sizeof(char) * taille_nom);


   //On recupere le pid du processus
  pid_t pid;
  pid=getpid();

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   socket_lanceur=socket(AF_INET,SOCK_STREAM,0);

   memset(lanceur_addr,0,sizeof(struct sockaddr_in));
   lanceur_addr->sin_family=AF_INET;
   lanceur_addr->sin_port=htons(port_lanceur);
   lanceur_addr->sin_addr.s_addr = inet_addr(ip_lanceur); //convert to binary numbers

   do_connect(socket_lanceur,lanceur_addr);

   int name_len = strlen(name);
   /* Envoi du nom de machine au lanceur */
   write(socket_lanceur, &name_len, sizeof(int));
   write(socket_lanceur,name,strlen(name));
   /* Envoi du pid au lanceur */

   write(socket_lanceur,&pid,sizeof(pid));

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   socket_listen_procs=creer_socket(&port_listen_procs);
   listen(socket_listen_procs,1);
   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   write(socket_lanceur,&port_listen_procs,sizeof(port_listen_procs));
  /*Récupération des infos de connexion aux autres processus*/
  /*nombre de processus */
   read(socket_lanceur,&num_procs,sizeof(int));
  /*le rang*/
   read(socket_lanceur,&my_rank,sizeof(int));


  //Autres infromations
   proc_array = malloc(num_procs * sizeof(dsm_proc_t));
   receive_all(socket_lanceur,proc_array,num_procs*sizeof(dsm_proc_t));
// Argumenets de dsm_init
   char ** newargv = malloc(4 * sizeof(char *));
   char n[5];
   char r[5];
   char socket[5];
   sprintf(n,"%d",num_procs);
   sprintf(r,"%d",my_rank);
   sprintf(socket,"%d",socket_listen_procs);
   newargv[0]=n;
   newargv[1]=r;
   newargv[2]=(char *)proc_array;
   newargv[3]=socket;
   dsm_init(4,newargv);
   printf("Fin dsm %d\n",my_rank);
   fflush(stdout);
   close(socket_lanceur);

   return 0;
}
