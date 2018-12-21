#include "common_impl.h"

int main(int argc, char **argv)
{
  //write(STDOUT_FILENO,"hhhhh",strlen("hhhhh"));
  //printf("hhhhh\n");

  /*declarations des variables*/
  dsm_proc_t *proc_array = NULL;// Tableau (des strucs) contenant les infos des processus dsmwrap

  int socket_lanceur,socket_listen_procs; // les sockets
  u_short port_lanceur;
  u_short port_listen_procs; // les ports
  int my_rank;     // Rank du processus
  int num_procs;     // nombre des processus estimé du ficher machine file
  int i,p,k;
  char name[taille_nom];

  struct sockaddr_in *lanceur_addr; //struct adresse du dsmexec
  struct sockaddr_in *tmp_addr; //struct adresse du dsmexec
  struct pollfd *sockpoll=NULL;
  lanceur_addr=malloc(sizeof(struct sockaddr_in));
  tmp_addr=malloc(sizeof(struct sockaddr_in));
  char ip_lanceur[INET_ADDRSTRLEN]; //Ip adresse
  char ip_tmp[INET_ADDRSTRLEN];

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   char ** newargv = malloc((argc-3) * sizeof(char *));

 	 for (i = 0; i < argc-3; i++)
 		newargv[i] = argv[i+1];

 	  newargv[argc-3] = NULL;
   //On recupere l'adresse du lanceur à partir du hostname donné
   port_lanceur = atoi(argv[2]);
   get_ip_from_hostname(argv[1],ip_lanceur);



   //On recupere le rang
   my_rank=atoi(argv[3]);
  /* char asb[10];
   sprintf(asb,"%s",argv[3]);
  write(STDOUT_FILENO,argv[3],strlen(argv[3]));*/

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


   /* Envoi du nom de machine au lanceur */
   write(socket_lanceur,name,sizeof(name));
   /* Envoi du pid au lanceur */

  write(socket_lanceur,&pid,sizeof(pid));
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   socket_listen_procs=creer_socket(&port_listen_procs);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   write(socket_lanceur,&port_listen_procs,sizeof(port_listen_procs));
  /*Récupération des infos de connexion aux autres processus*/
  /*nombre de processus */
   read(socket_lanceur,&num_procs,sizeof(int));

   proc_array = malloc(num_procs * sizeof(dsm_proc_t));

  //autres infromations
   recv_all(socket_lanceur,proc_array,num_procs*sizeof(dsm_proc_t));

// for(i = 0; i < num_procs ; i++){//test affichage
//    write(STDOUT_FILENO,&proc_array[i].connect_info.name,sizeof(proc_array[i].connect_info.name));
 //}
/*  for(i = 0; i < num_procs ; i++){
  //le rang
  read(socket_lanceur,&proc_array[i].connect_info.rank,sizeof(proc_array[i].connect_info.rank));
//le port
  read(socket_lanceur,&proc_array[i].connect_info.port,sizeof(proc_array[i].connect_info.port));
  write(STDOUT_FILENO,&proc_array[i].connect_info.port,sizeof(proc_array[i].connect_info.port));
  //printf("%s port wrap \n",proc_array[i].connect_info.port);
//le pid
  read(socket_lanceur,&proc_array[i].pid,sizeof(proc_array[i].pid));
  write(STDOUT_FILENO,&proc_array[i].pid,sizeof(proc_array[i].pid));
//l'adresse client i'
  proc_array[i].connect_info.ad_client=malloc(sizeof(struct sockaddr));
  read(socket_lanceur,&proc_array[i].connect_info.ad_client,sizeof(proc_array[i].connect_info.ad_client));
}*/

//jusqu'à ce qu'ils soient inactifs (ie fermes par les
//processus dsm ecrivains de l'autre cote ...)
/*for (i = 0; i < num_procs; i++) {

  // La cellule à supprimer à été localisée
  if(proc_array[i].connect_info.rank == my_rank) {

    // Fermeture des descripteurs de fichier
    close(proc_array[i].fd_stderr);
    close(proc_array[i].fd_stdout);

    // Fermeture de la socket
    if (proc_array[i].connect_info.socket != 0)
      close(proc_array[i].connect_info.socket);

    num_procs--;
}}*/
/* ============================================================== *\
                    Connexion aux autres processus
\* ============================================================== */
int fd_pross[100];
int *tab_sock=malloc(2*num_procs*sizeof(int));
socklen_t length__client = sizeof(struct sockaddr);
//struct sockaddr* ad_accept;
struct sockaddr_in* ad_accept;
/*
// Nbr de process de rang inf.
int nb_process_inf = 0;
char tabchar[100];
// Nombre de processus de rang inférieur
for (i = 0; i < num_procs && proc_array[i].connect_info.rank < my_rank; i++)
    nb_process_inf++;

listen(port_listen_procs, 2);
fflush(stdout);
for (i = 0; i < num_procs; i++) {

    // Pour les rangs inférieurs, ce sont eux qui font la connexion
    if (proc_array[i].connect_info.rank < my_rank) {
        //fd_pross[i]=do_accept(socket_listen_procs,ad_accept);
        fd_pross[i]=accept(socket_listen_procs,(struct sockaddr *)&ad_accept,&length__client);
        perror("A");
        recv_all(fd_pross[i],tabchar,100);
        write(STDOUT_FILENO,"Accpetpross\n",strlen("Accpetpross\n"));
    }

    // Pour les rangs supérieurs, c'est ce fichier qui établi la
    // connexion
    else {
      tab_sock[i]=socket(AF_INET,SOCK_STREAM,0);
      memset(tmp_addr,0,sizeof(tmp_addr));
      tmp_addr->sin_family=AF_INET;
      tmp_addr->sin_port=htons(proc_array[i].connect_info.port);
      tmp_addr->sin_addr.s_addr = proc_array[i].connect_info.ad_client->sin_addr.s_addr;
    //  get_ip_from_hostname("ritahayworth",ip_tmp);
      //tmp_addr->sin_addr.s_addr = inet_addr(ip_tmp);
      write(STDOUT_FILENO,"before cont \n",strlen("before cont \n"));
      do_connect(tab_sock[i],tmp_addr);
      char tabchar[100]="haha\n";
      send_all(tab_sock[i],tabchar,100);
      write(STDOUT_FILENO,"ConneDone\n",strlen("ConneDone\n"));

    }
}*/

/*
char tabchar[100];
i=my_rank+1;
sockpoll=malloc((num_procs-my_rank)*sizeof(struct pollfd));
sockpoll[0].fd=socket_listen_procs;
sockpoll[0].events=POLLIN;

//listen(port_listen_procs,3);
while(i<num_procs){
  p=poll(sockpoll,num_procs-my_rank,-1);
  if(sockpoll[0].revents==POLLIN){
    write(STDOUT_FILENO,"sm3\n",strlen("sm3\n"));
  //  addrlen=sizeof(struct sockaddr_in);
    //socklen_t length__client = sizeof(struct sockaddr);
  //  tab_sock[i]=do_accept(socket_listen_procs,&ad_accept);
    write(STDOUT_FILENO,"before accept \n",strlen("before accept \n"));
    fd_pross[i]=accept(socket_listen_procs,(struct sockaddr *)&ad_accept,&length__client);
    perror("A");
    write(STDOUT_FILENO,"after accept \n",strlen("after accept \n"));
    for(k=0;k<num_procs;k++){
      if(proc_array[i].connect_info.ad_client->sin_addr.s_addr==ad_accept->sin_addr.s_addr && proc_array[i].connect_info.port==ad_accept->sin_port){
        break;
      }
    }
    sockpoll[k-my_rank].fd=fd_pross[i];
    //ii=find_dsm_client(proc_conn,num_proc_conn,ad_client,a);
    i++;
    //recv_all(fd_pross[i],tabchar,100);
  }
}

wait(NULL);
/*se connecter aux autres proc*/
//int C[100];
/*
for(i=0;i<my_rank;i++){
  tab_sock[i]=socket(AF_INET,SOCK_STREAM,0);
  memset(tmp_addr,0,sizeof(tmp_addr));
  tmp_addr->sin_family=AF_INET;
  tmp_addr->sin_port=htons(proc_array[i].connect_info.port);
  tmp_addr->sin_addr.s_addr = proc_array[i].connect_info.ad_client->sin_addr.s_addr;
  //get_ip_from_hostname("ritahayworth",ip_tmp);
  //tmp_addr->sin_addr.s_addr = inet_addr(ip_tmp);
  write(STDOUT_FILENO,"before cont \n",strlen("before cont \n"));
  do_connect(tab_sock[i],tmp_addr);
  //char tabchar[100]="haha\n";
  //send_all(tab_sock[i],tabchar,100);

}
*/


// Nombre de processus de rang inférieur
//for (i = 0; i < num_procs && proc_array[i].connect_info.rank < my_rank; i++)
  //  nb_inf++;


listen(socket_listen_procs,num_procs);
write(STDOUT_FILENO,"listen\n",strlen("listen\n"));

for (i=0; i<my_rank;i++){
    write(STDOUT_FILENO,"Avant Acc\n",strlen("Avant Acc\n"));
    fd_pross[i]=accept(socket_listen_procs,(struct sockaddr *)ad_accept,&length__client);
    perror("A");
}
for (i=my_rank+1;i<num_procs;i++){
    write(STDOUT_FILENO,"c1\n",strlen("c1\n"));
  tab_sock[i]=socket(AF_INET,SOCK_STREAM,0);
  write(STDOUT_FILENO,"c2\n",strlen("c2\n"));
  memset(tmp_addr,0,sizeof(tmp_addr));
  tmp_addr->sin_family=AF_INET;
  tmp_addr->sin_port=htons(proc_array[i].connect_info.port);
  write(STDOUT_FILENO,"c3\n",strlen("c3\n"));
    //write(STDOUT_FILENO,proc_array[i].connect_info.ad_client->sin_addr.s_addr,4);
  tmp_addr->sin_addr.s_addr = (proc_array[i].connect_info.ad_client)->sin_addr.s_addr;
  //tmp_addr->sin_addr.s_addr = proc_array[my_rank+1+i].connect_info.ad_client.sin_addr.s_addr;
  write(STDOUT_FILENO,"c4\n",strlen("c4\n"));
/*  char s[10];
  sprintf(s,"%d",proc_array[my_rank+1+i].connect_info.port);
  write(STDOUT_FILENO,s,strlen(s));*/
  write(STDOUT_FILENO,"av con\n",strlen("av con\n"));
  do_connect(tab_sock[my_rank+i+1],tmp_addr);


}
/*
for (i = 0; i < num_procs; i++) {

  if(proc_array[i].connect_info.rank>my_rank){
    fd_petites[i]=socket(AF_INET,SOCK_STREAM,0);
    do_connect(fd_petites[i],proc_array[i].connect_info.ad_client);
    fprintf(stdout, "Connexion");
  }
  if(proc_array[i].connect_info.rank<my_rank){
    fd_pross[i]=do_accept(socket_listen_procs,proc_array[i].connect_info.ad_client);
      fprintf(stdout, "Acceptation du process");
  }

}*/


//   execvp(newargv[0], newargv);

   return 0;
}
