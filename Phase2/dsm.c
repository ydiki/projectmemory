#include "dsm.h"
#include "common_impl.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */

/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

int address2num(char * ptr)
{
  int numpage=0;
  for(numpage=DSM_NODE_NUM;numpage>-2;numpage--){
    if(ptr == (char *)(BASE_ADDR+(numpage*(PAGE_SIZE))))
      return numpage;
  }
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return ;
   }
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}
void dsm_send(int dest,void *buf,size_t size)
{
   sendall(dest,buf,size);
}

void dsm_recv(int from,void *buf,size_t size)
{
   receive_all(from,buf,size);
}

static void *dsm_comm_daemon( void *arg){

  int i,p;
  int *fds=(int *)arg;
  struct pollfd *req=NULL;

  for(i=0;i<DSM_NODE_NUM;i++){
      req[i].fd=fds[i];
      req[i].events=POLLIN;
  }
  void *value=malloc(PAGE_SIZE);
  void  **addr=malloc(sizeof(*addr));

  while(1)
  {
    p=poll(req,DSM_NODE_NUM,-1);
    for(i=0;i<DSM_NODE_NUM;i++){
        if(req[i].revents==POLLIN){
          dsm_recv(req[i].fd,addr,sizeof(*addr));
          value=*addr;
          dsm_send(req[i].fd,value,PAGE_SIZE);
        }
    }
    printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
    sleep(2);
  }
  return ;
}



static void dsm_handler(void *page_addr,void *addr,int fd[])
{
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   int owner_rank=get_owner(address2num(page_addr));
   void *value=NULL;
   dsm_send(fd[owner_rank],&page_addr,sizeof(page_addr));
   printf("rank owner=%d\n",owner_rank);
   abort();
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context,int fd_pross[])
{
   /* A completer */
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	      dsm_handler(page_addr,addr,fd_pross);
     }
   else
     {
       printf("[%i] ! \n",DSM_NODE_ID);
	/* SIGSEGV normal : ne rien faire*/
     }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index,socket_listen_procs,i;
   dsm_proc_t *proc_array = NULL;

   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   DSM_NODE_NUM=atoi(argv[0]);
   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/
   DSM_NODE_ID=atoi(argv[1]);
   /* reception des informations de connexion des autres */
   /* Par Argument argv[2]*/
   proc_array=malloc(sizeof(dsm_proc_t)*DSM_NODE_NUM);
   proc_array=(dsm_proc_t *)argv[2];
   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   socket_listen_procs=atoi(argv[3]);
   int fd_pross[100];
   int *tab_sock=malloc(2*DSM_NODE_NUM*sizeof(int));
   struct sockaddr_in *tmp_addr; //struct adresse du dsmexec
   tmp_addr=malloc(sizeof(struct sockaddr_in));
   socklen_t length__client = sizeof(struct sockaddr);
   struct sockaddr_in* ad_accept = malloc(length__client);
   printf("my dsm rank %d\n", DSM_NODE_ID);
   fflush(stdout);
   int c;

   for (i=0; i<DSM_NODE_ID;i++){
       fd_pross[i]=accept(socket_listen_procs,(struct sockaddr *)ad_accept,&length__client);
       printf("%d: accept: %d\n",DSM_NODE_ID,fd_pross[i]);
       fflush(stdout);
   }

   for (i=DSM_NODE_ID+1;i<DSM_NODE_NUM;i++){
     tab_sock[i]=socket(AF_INET,SOCK_STREAM,0);
     memset(tmp_addr,0,length__client);
     tmp_addr->sin_family=AF_INET;
     tmp_addr->sin_port=htons(proc_array[i].connect_info.port);
     tmp_addr->sin_addr.s_addr = proc_array[i].connect_info.ad_client.sin_addr.s_addr;
     printf("%d trying to connect to %d %s %d\n",DSM_NODE_ID,i,inet_ntoa(tmp_addr->sin_addr), proc_array[i].connect_info.port);
     fflush(stdout);
     c = do_connect(tab_sock[i],tmp_addr);
     printf("%d connect %d\n", DSM_NODE_ID,c);
     fflush(stdout);
   }
   sleep(2);


   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info(index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, (void *)fd_pross);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
