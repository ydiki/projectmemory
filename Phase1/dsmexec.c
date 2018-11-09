#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "common_impl.h"

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
      while(s>0 && c<*(num_procs)){
        if(buff[i]!='\n'){

          (*machine_suivante)->nom[j]=buff[i];
          j++;
          i++;}
          else{
            (*machine_suivante)->next=malloc(sizeof(struct nom_machines));
            *machine_suivante=(*machine_suivante)->next;
            c++;
            i++;
            j=0;
          }
        }
      }
    }

void trouver_machine(struct nom_machines* machine, int i, char nom[taille_nom]){
  int j;
  for (j=0;j<i;j++){
    machine=machine->next;
    i++;
  }
  strcpy(nom,machine->nom);
}

int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  } else {
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




    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    struct sigaction zombie;
    //zombie=malloc(sizeof(struct sigaction *));
    memset(&zombie, 0, sizeof(struct sigaction ));
    zombie.sa_handler = sigchld_handler;
    //zombie.sa_flags = SA_RESTART;
    //sigaction(SIGCHLD, &zombie, NULL);

    /* lecture du fichier de machines */
    /* 1- on recupere le nombre de processus a lancer */
    //c'est le nombre des lignes
    countline(machine_file,&num_procs);

    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */

    struct nom_machines *tabmachine ;
    get_machine_names(machine_file,&tabmachine,&num_procs);

    /* creation de la socket d'ecoute */
    int sock = creer_socket(&port);

    /* + ecoute effective */
    listen(sock, num_procs);


    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      /* creation du tube pour rediriger stdout */
      pipe(fd1[i]);

      /* creation du tube pour rediriger stderr */
      pipe(fd2[i]);

      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

        /* redirection stdout */
          close(fd1[i][0]);
          close(STDOUT_FILENO);
          dup(fd1[i][1]);
          printf("cree tube  out");

        /* redirection stderr */
          close(fd2[i][0]);
          close(STDERR_FILENO);
          dup(fd2[i][1]);
          printf("cree tube  err");


        /* Creation du tableau d'arguments pour le ssh */
          char **new_argv;
          new_argv = malloc((argc+1) * sizeof(char *));
          char cette_machine[taille_nom];
          trouver_machine(tabmachine,i,cette_machine);
          strcpy(new_argv[0],cette_machine);

          for(j = 1; j < argc; j++){
            new_argv[j]=argv[j];
          }
        /* jump to new prog : */
          execvp("ssh",new_argv);
          printf("ssh");

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(fd1[i][1]);
        close(fd2[i][1]);
        printf("sedina unitule");
        num_procs_creat++;
      }
    }


    for(i = 0; i < num_procs ; i++){

      /* on accepte les connexions des processus dsm */
    if (int s= accept(sock, (struct sockaddr *)&, sizeof(struct sockaddr_in)) == -1)
          ERROR_EXIT("Connect error");
    else{
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */

      /* 2- puis la chaine elle-meme */

      /* On recupere le pid du processus distant  */

      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
    }

    /* envoi du nombre de processus aux processus dsm*/

    /* envoi des rangs aux processus dsm */

    /* envoi des infos de connexion aux processus */

    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /* while(1)
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
