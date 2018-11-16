#include "common_impl.h"
#include <signal.h>

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

//struct nom_machines;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
  /* on traite les fils qui se terminent */
  /* pour eviter les zombies */
  int pid,status;
  pid=waitpid(-1,&status,WNOHANG);
  if(pid==-1){
       perror ("waitpid");
  }
  else{
      num_procs_creat--;
  }
}

void countline(const char * fichier,int *num_procs){

  int sr = open(fichier, O_RDONLY);
  int i=0;
  int s;
  char buff[100];
  if (sr == -1){
    exit(EXIT_FAILURE);}
    else {
      s=read(sr,buff,sizeof(buff));
      while(s!=0){
        if(buff[i]=='\n' || (i>0 && buff[i]==EOF && buff[i-1]!='\n' )){
          (*num_procs)++;
        }
        i++;
        s--;
      }
    }

    close(sr);
  }

void get_machine_names(const char * fichier, struct nom_machines** machine_suivante,int *num_procs){
  int i=0;
  int c=0;
  int s;
  char buff[1000];
  int sr = open(fichier,O_RDONLY);
  if (sr == -1){
    exit(EXIT_FAILURE);}
    else{
      int j=0;
      s=read(sr,buff,sizeof(buff));
      *machine_suivante=malloc(sizeof(struct nom_machines));
      struct nom_machines* mach = *machine_suivante;
      while(s>0 && c<*(num_procs)){
        if(buff[i]!='\n'){

          mach->nom[j]=buff[i];
          printf("%c\n",mach->nom[j]);
          j++;
          i++;}
          else{
            printf("fin\n");
            mach->nom[j]='\0';
            c++;
            i++;
            j=0;
            if (c<*(num_procs)){
              printf("pas bon\n");
              mach->next=malloc(sizeof(struct nom_machines));
            }
            else{
              printf("bon\n");
              mach->next =NULL;
            }
            mach=mach->next;
          }
        }

      }
    }

void trouver_machine(struct nom_machines* machine, int i, char nom[taille_nom]){
  int j=0;
  while(i!=j){
    machine=machine->next;
    j++;
  }
  strcpy(nom,machine->nom);
}

int main(int argc, char *argv[])
{
  pid_t pid_pere = getpid();

  if (argc < 3){
    usage();
  } else {
    printf("start\n");
    /*declaration des variables*/
    typedef int fdpipe[2];
    pid_t pid;
    int num_procs = 0;
    int i,j;
    int port;
    fdpipe *fd1;
		fdpipe *fd2;
    fd1 = malloc(num_procs * sizeof(fdpipe));
    fd2 = malloc(num_procs * sizeof(fdpipe));
    const char * machine_file = argv[1];
    char **new_argv;
    new_argv = malloc((argc+6) * sizeof(char *));
    char cette_machine[taille_nom];
    int size=100;
    char buffer[size];
    proc_array = malloc(num_procs * sizeof(dsm_proc_t));
    int length;



    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    struct sigaction zombie;
    memset(&zombie, 0, sizeof(struct sigaction ));
    zombie.sa_handler = sigchld_handler;
    //zombie.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &zombie, NULL);
    printf("sig zombie\n");
    /* lecture du fichier de machines */
    /* 1- on recupere le nombre de processus a lancer */
    //c'est le nombre des lignes
    countline(machine_file,&num_procs);
    printf("count\n");

    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */

    struct nom_machines *tabmachine ;
    get_machine_names(machine_file,&tabmachine,&num_procs);
    printf("machines\n");

    /* creation de la socket d'ecoute */
    int sock = creer_socket(&port);
    printf("socket created\n");

    /* + ecoute effective */
    listen(sock, num_procs);

    // Nom de la machine serveur
    //char * hostname = malloc(sizeof(char) * taille_nom);
    char hostname[taille_nom];
    hostname[taille_nom-1]='\0';
    gethostname(hostname, sizeof(char) * taille_nom);


    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      /* creation du tube pour rediriger stdout */
      pipe(fd1[i]);
      printf("pip 1 \n");
      /* creation du tube pour rediriger stderr */
      pipe(fd2[i]);
      printf("pip 2 \n");

      pid = fork();
      if(pid == -1){ ERROR_EXIT("fork");}

      if (pid == 0) { /* fils */

        /* redirection stdout */
          close(fd1[i][0]);
          close(STDOUT_FILENO);
          dup(fd1[i][1]);
          close(fd1[i][1]);
          //printf("cree tube out\n");

        /* redirection stderr */
          close(fd2[i][0]);
          close(STDERR_FILENO);
          dup(fd2[i][1]);
          close(fd2[i][1]);

        //  write(STDOUT_FILENO,"hellllo",strlen("hellllo"));
          proc_array[i].connect_info.pid=getpid();

        /* Creation du tableau d'arguments pour le ssh */
          trouver_machine(tabmachine,i,cette_machine);
          *(new_argv)="ssh";
          char p[10];
          sprintf(p,"%d",port);
          *(new_argv+sizeof(char *))=cette_machine;//nom du machine
          *(new_argv+2*sizeof(char *))="/net/t/ydiki/Desktop/Memoirepartagée/Phase1/bin/dsmwrap";//chemin
          *(new_argv+3*sizeof(char *))= hostname; // Hostname du serveur (fichier courant)
          *(new_argv+4*sizeof(char *))=p;// Port du serveur
          for(j = 1; j < argc; j++){
            *(new_argv+(4+j)*sizeof(char *))=argv[j];
          }

          *(new_argv+(argc+5)*sizeof(char *))=NULL;
    /*      int k;
          for(k=0; k<argc+5; k++){
            if(*(new_argv+k*sizeof(char *)) != NULL)
              printf("%s\n",*(new_argv+k*sizeof(char *)));
            else
              printf("NULL\n");
          }*/
        /* jump to new prog : */
        if (execvp("ssh", new_argv) == -1)
          ERROR_EXIT("Erreur in ssh");


      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(fd1[i][1]);
        close(fd2[i][1]);
        num_procs_creat++;
      }
    }


    for(i = 0; i < num_procs ; i++){


      /* on accepte les connexions des processus dsm */
    //  proc_array[i].connect_info.ad_client=malloc(sizeof(struct sockaddr_in));
      printf("%d",sock);

      //proc_array[i].connect_info.socket= do_accept(sock, (struct sockaddr *)&(proc_array[i].connect_info.ad_client));
      socklen_t sizze = sizeof(struct sockaddr_in);
      proc_array[i].connect_info.socket= accept(sock, (struct sockaddr *)&(proc_array[i].connect_info.ad_client),&sizze);

      if (proc_array[i].connect_info.socket==-1)perror("");
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */

    //  length=read_line(proc_array[i].connect_info.ad_client,buffer,sizeof(buffer));
      //  printf("receve length %d \n",proc_array[i].connect_info.length);
      //int j = indice_b(buffer);
      /* 2- puis la chaine elle-meme */
        //strncpy(proc_array[i].connect_info.name,buffer,j);
      //read_line(proc_array[i].connect_info.ad_client,proc_array[i].connect_info.name,length);

      /* On recupere le pid du processus distant  */
    //  int j2=indice_b(buffer[j+1]);
        //recv(client_socket[i],buffer,sizeof(buffer),0);
      //read_line(proc_array[i].connect_info.ad_client,proc_array[i].pid,10);
      //  strncpy(proc_array[i].pid,buffer[j+1],j2-j-1);
      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
        //recv(client_socket[i],buffer,sizeof(buffer),0);
    //  read_line(proc_array[i].connect_info.ad_client,proc_array[i].connect_info.port,10);

      //  int j3=indice_b(buffer[j2+1]);
      //  strncpy(proc_array[i].connect_info.port,buffer[j2+1],j3-j2-1);
      /* On recupere le rang */
      proc_array[i].connect_info.rank = i;
      printf("---> i %d \n",proc_array[i].connect_info.rank);
    }

    for(i = 0; i < num_procs ; i++){
    //  char message[size];
    //  char nb[5];
    /* envoi du nombre de processus aux processus dsm*/
    //  sprintf(nb,"%d",num_procs-1);
    //  strcpy(message,nb);
    //  strcat(message,'\n');


    /* envoi des rangs aux processus dsm */
  //  sprintf(nb,"%d",proc_array[i].connect_info.rank);
  //  strcat(message,nb);
    //strcat(message,'\n');
  //  int j;
    /* envoi des infos de connexion aux processus */
    //for(j = 0; j < num_procs ; j++){
  //    if (j != i){
      //  sprintf(nb,"%d",proc_array[j].connect_info.port);
      //  strcat(message,nb);
        //strcat(message,'\n');
        //sprintf(nb,"%d",proc_array[j].connect_info.rank);
      //  strcat(message,nb);
      //  strcat(message,'\n');
      //  sprintf(nb,"%d",proc_array[j].pid);
      //  strcat(message,nb);
      //  strcat(message,'\n');
    //  }
    //}
}
    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */

        /*   while(1)
    {

    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)
  };
  */

  /* on attend les processus fils */

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
}
exit(EXIT_SUCCESS);
}
