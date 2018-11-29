#include "common_impl.h"

int main(int argc, char **argv)
{

  /*declarations des variables*/
  dsm_proc_t *proc_array = NULL;// Tableau (des strucs) contenant les infos des processus dsmwrap

  int socket_lanceur,socket_listen_procs; // les sockets
  u_short port_lanceur;
  u_short port_listen_procs; // les ports
  u_short my_rank = atoi(argv[3]);     // Rank du processus
  int num_procs,i;     // nombre des processus estimé du ficher machine file
  char name[taille_nom];

  struct sockaddr_in *lanceur_addr; //struct adresse du dsmexec
  lanceur_addr=malloc(sizeof(struct sockaddr_in));
  char ip_lanceur[INET_ADDRSTRLEN]; //Ip adresse

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   //On recupere l'adresse du lanceur à partir du hostname donné
   port_lanceur = atoi(argv[2]);
   get_ip_from_hostname(argv[1],ip_lanceur);

   //On recupere le rang
   my_rank=atoi(argv[3]);

   //On recupere le nom de la machine
   gethostname(name, sizeof(char) * taille_nom);

   //On recupere le pid du processus
   pid_t pid=getpid();

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   socket_lanceur=socket(AF_INET,SOCK_STREAM,0);

   memset(lanceur_addr,0,sizeof(struct sockaddr_in));
   lanceur_addr->sin_family=AF_INET;
   lanceur_addr->sin_port=htons(port_lanceur);
   lanceur_addr->sin_addr.s_addr = inet_addr(ip_lanceur); //convert to binary numbers

   do_connect(socket_lanceur,lanceur_addr);
   /* Envoi du nom de machine au lanceur */
   handle_message(socket_lanceur,&name,taille_nom);
   /* Envoi du pid au lanceur */

    handle_message(socket_lanceur,pid,sizeof(pid_t));

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   socket_listen_procs=creer_socket(&port_listen_procs);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   handle_message(socket_lanceur,port_listen_procs,sizeof(port_listen_procs));

  /*Récupération des infos de connexion aux autres processus*/
  /*nombre de processus */
    read_line(socket_lanceur,&num_procs,sizeof(int));
/*   proc_array = malloc(num_procs * sizeof(dsm_proc_t));
  //autres infromations
  for(i = 0; i < num_procs ; i++){
  //le rang
  read_line(socket_lanceur,&proc_array[i].connect_info.rank,sizeof(int));
//le port
  read_line(socket_lanceur,&proc_array[i].connect_info.port,sizeof(int));
//le pid
  read_line(socket_lanceur,&proc_array[i].pid,sizeof(pid_t));
//l'adresse client i'
  proc_array[i].connect_info.ad_client=malloc(sizeof(struct sockaddr));
  read_line(socket_lanceur,&proc_array[i].connect_info.ad_client,sizeof(struct sockaddr));
}*/

   /* on execute la bonne commande */
   //char new_arv[argc+1];
  // for(i=0,i<argc,i++){
  //   new_arv[i]=argv[i+1];
//   }
//   new_arv[argc+1]=NULL;

   //execvp("truc", new_arv);

   return 0;
}
