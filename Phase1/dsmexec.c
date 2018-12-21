#include "common_impl.h"
#include <signal.h>
//#include <poll.h>

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
          j++;
          i++;}
          else{
            mach->nom[j]='\0';
            c++;
            i++;
            j=0;
            if (c<*(num_procs)){
              mach->next=malloc(sizeof(struct nom_machines));
            }
            else{
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
    //typedef int fdpipe[2];
    pid_t pid;
    int num_procs = 0;
    int i,j;
    int port;
    int fd1[num_procs][2];
		int fd2[num_procs][2];
    //fd1 = malloc(num_procs * sizeof(fdpipe));
    //fd2 = malloc(num_procs * sizeof(fdpipe));
    const char * machine_file = argv[1];
    char **new_argv;
    new_argv = malloc((argc+7) * sizeof(char *));
    char cette_machine[taille_nom];
    int size=100;
    char buffer[20];
    proc_array = malloc(num_procs * sizeof(dsm_proc_t));
    int length;

    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    struct sigaction zombie;
    memset(&zombie, 0, sizeof(struct sigaction ));
    zombie.sa_handler = sigchld_handler;
    //zombie.sa_flags = SA_RESTART;
    //sigaction(SIGCHLD, &zombie, NULL);
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
    listen(sock,num_procs);

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
        close(STDOUT_FILENO);
        dup(fd1[i][1]);
        close(fd1[i][0]);

          //close(fd1[i][0]);
        //  close(STDOUT_FILENO);
          //dup2(STDOUT_FILENO,fd1[i][1]);
         //close(fd1[i][1]);
          //printf("cree tube out\n");

        /* redirection stderr */
          close(STDERR_FILENO);
          dup(fd2[i][1]);
            close(fd2[i][0]);


          //close(fd2[i][0]);
          //close(STDERR_FILENO);
          //dup2(STDERR_FILENO,fd2[i][1]);
          //close(fd2[i][1]);

        /* Creation du tableau dSuccess'arguments pour le ssh */
          trouver_machine(tabmachine,i,cette_machine);
          new_argv[0]="ssh";
          char p[10];
          char *rr=malloc(5*sizeof(char));
          sprintf(p,"%d",port);
          sprintf(rr,"%d",i);
        // printf("AAAAAAh!!!! %s\n",rr);
          new_argv[1]=cette_machine;//nom du machine
          new_argv[2]="/net/t/ydiki/Desktop/Memoirepartagée/Phase1/bin/dsmwrap"; //chemin
          new_argv[3]=hostname; // Hostname du serveur (fichier courant)
          new_argv[4]=p;// Port du serveur   printf("num proc %d \n",num_procs);
          new_argv[5]=rr;
        //  printf("eieieieieie %s\n",new_argv[5]);
          for(j = 1; j < argc; j++){
            new_argv[5+j]=argv[j];
          }
          new_argv[argc+6]=NULL;
        /* int k;
          for(k=0; k<argc+5; k++){
            if(new_argv[k] != NULL)        //  close(STDOUT_FILENO);

              printf("%s\n",new_argv[k]);
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

        // Enregistrement des extrémités utiles pour le reste du
  			// programme :
  			proc_array[i].fd_stderr = fd1[i][0];
  			proc_array[i].fd_stdout = fd2[i][0];

      }
    }
    socklen_t length__client = sizeof(struct sockaddr);



    for(i = 0; i < num_procs ; i++){
      /* on accepte les connexions des processus dsm */
      //proc_array[i].connect_info.ad_client=malloc(sizeof(struct sockaddr*));
      proc_array[i].connect_info.ad_client=malloc(sizeof(struct sockaddr_in));
      proc_array[i].connect_info.socket=accept(sock,(struct sockaddr *)proc_array[i].connect_info.ad_client,&length__client);
      if (proc_array[i].connect_info.socket==-1)
        perror("");
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      /* 2- puis la chaine elle-meme */
      read(proc_array[i].connect_info.socket,buffer,sizeof(buffer));
      int length=strlen(buffer);
      strcpy(proc_array[i].connect_info.name,buffer);
      printf("-----------name  %s\n",proc_array[i].connect_info.name);
      /* On recupere le pid du processus distant  */
      read(proc_array[i].connect_info.socket,&proc_array[i].pid,sizeof(proc_array[i].pid));
      perror("");
      printf("-------- pid %d\n ",proc_array[i].pid);
      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      read(proc_array[i].connect_info.socket,&proc_array[i].connect_info.port,sizeof(proc_array[i].connect_info.port));
      perror("");
        printf("--------port %d\n ",proc_array[i].connect_info.port);
      /* On recupere le rang */
      proc_array[i].connect_info.rank = i;
    }

    for(i = 0; i < num_procs ; i++){
      /* envoi du nombre de processus aux processus dsm*/
    write(proc_array[i].connect_info.socket,&num_procs,sizeof(int));
    send_all(proc_array[i].connect_info.socket,proc_array,num_procs*sizeof(dsm_proc_t));

    /* envoi des rangs aux processus dsm */
      //write(proc_array[i].connect_info.socket,&proc_array[i].connect_info.rank,sizeof(int));

    /* envoi des infos de connexion aux processus */
    //    write(proc_array[i].connect_info.socket,&proc_array[i].connect_info.port,sizeof(proc_array[i].connect_info.port));
      //  write(proc_array[i].connect_info.socket,&proc_array[i].pid,sizeof(proc_array[i].pid));
      //  write(proc_array[i].connect_info.socket,&proc_array[i].connect_info.ad_client,sizeof(struct sockaddr));
}
/*int c;
int r=read(fd1[0][0],&c,sizeof(c));
perror("");
printf("---------------rang r = %d et %d\n",r,c);*/


    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
   int p;
    int t_n=2*num_procs;
    struct pollfd pollfds[t_n];

    for(i=0;i<num_procs;i++){
      //stdout
      pollfds[2*i].fd=fd1[i][0];
      pollfds[2*i].events=POLLIN;
      //stderr
      pollfds[2*i+1].fd=fd2[i][0];
      pollfds[2*i+1].events=POLLIN;
    }
   int bb=0;
   int bu;
   int closed_pipes = 0;
        //sleep(1);
         while(1){
           //printf("%d\n", closed_pipes);
           memset(buffer,0,sizeof(buffer));
           if(closed_pipes == t_n){
                printf("fermé \n");
                fflush(stdout);
                break;}

            poll(pollfds,t_n,-1);
            if ( errno != EINTR )
              //perror("");
              ;
            for (i = 0; i < num_procs ; i++) {
            /*je recupere les infos sur les tubes de redirection
            jusqu'à ce qu'ils soient inactifs (ie fermes par les
            processus dsm ecrivains de l'autre cote ...)*/
         if (pollfds[2*i].revents == POLLIN) {
            memset(buffer,0,sizeof(buffer));
              read(fd1[i][0], (char *)buffer, sizeof(buffer));
              //read(fd1[i][0], bu, sizeof(bu));
              printf("[ Proc %d : %s stdout ]: %s\n",i,proc_array[i].connect_info.name,buffer);
              bb++;

            }else if(pollfds[2*i].events & POLLHUP){
              pollfds[2*i].fd = -1;
              closed_pipes++;
            }
            if (pollfds[2*i+1].revents == POLLIN) {
              memset(buffer,0,sizeof(buffer));
              read(fd2[i][0], buffer, sizeof(buffer));

            printf("[ Proc %d : %s stderr ]: %s\n",i,proc_array[i].connect_info.name,buffer);
            }
            else if(pollfds[2*i+1].events & POLLHUP){
              pollfds[2*i+1].fd = -1;
              closed_pipes++;}
          }
        //  printf("bb %d\n",bb);
}

  /* on attend les processus fils */
  //while(wait(NULL) > 0);


  /* on ferme les descripteurs proprement */


  /* on ferme la socket d'ecoute */

}
exit(EXIT_SUCCESS);
}
