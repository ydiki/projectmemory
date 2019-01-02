#include "common_impl.h"

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

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   char ** newargv = malloc((argc-2) * sizeof(char *));

 	 for (i = 0; i < argc-3; i++)
 		newargv[i] = argv[4+i];

 	  newargv[argc-3] = NULL;

  /*  for (i = 0; i < argc-3; i++)
     printf("arg %d : %s \n",i,newargv[i]);*/

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
   read(socket_lanceur,&my_rank,sizeof(int));

   proc_array = malloc(num_procs * sizeof(dsm_proc_t));

  //autres infromations
   recv_all(socket_lanceur,proc_array,num_procs*sizeof(dsm_proc_t));
   printf("Server is done sending.\n");
   fflush(stdout);

/* ============================================================== \*\
                    Connexion aux autres processus
\* ============================================================== */
int fd_pross[100];
int *tab_sock=malloc(2*num_procs*sizeof(int));
socklen_t length__client = sizeof(struct sockaddr);
struct sockaddr_in* ad_accept = malloc(length__client);




printf("my rank %d my port %d\n", my_rank, port_listen_procs);
fflush(stdout);


for (i=0; i<my_rank;i++){

    fd_pross[i]=accept(socket_listen_procs,(struct sockaddr *)ad_accept,&length__client);
    //perror("A");
    printf("n° %d accepted n°socket: %d\n",my_rank,fd_pross[i]);
    fflush(stdout);

}
for (i=my_rank+1;i<num_procs;i++){

  tab_sock[i]=socket(AF_INET,SOCK_STREAM,0);

  memset(tmp_addr,0,length__client);
  tmp_addr->sin_family=AF_INET;
  tmp_addr->sin_port=htons(proc_array[i].connect_info.port);
  tmp_addr->sin_addr.s_addr = proc_array[i].connect_info.ad_client.sin_addr.s_addr;
  printf("n° %d trying to connect to n° %d add : %s port : %d\n",my_rank,i,inet_ntoa(tmp_addr->sin_addr), proc_array[i].connect_info.port);
  fflush(stdout);

  do_connect(tab_sock[i],tmp_addr);
  printf("%d connected \n", my_rank);
  fflush(stdout);
}


  execvp(newargv[0], newargv);
printf("%d is done \n", my_rank);
  sleep(1);
   return 0;
}
