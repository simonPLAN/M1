#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <commun.h>
#include <liste.h>
#include <piste.h>

// creation des structures  pour liberer ou prendre les semaphores
struct sembuf op_p={0,-1,0}; 
struct sembuf op_v={0,+1,0};


static void P(int Sem_id){
  semop(Sem_id, &op_p,1);
}

static void V(int Sem_id){
  semop(Sem_id, &op_v,1);

}

static void P1(int Sem_id,int pos){
  struct sembuf Op_P1 = {pos,-1,0}; 
  semop(Sem_id, &Op_P1,1);

}
static void V1(int Sem_id,int pos){
  struct sembuf Op_V1 = {pos,1,0}; 
  semop(Sem_id, &Op_V1,1);

}



int 
main( int nb_arg , char * tab_arg[] )
{    
  int cle_piste ;
  piste_t * piste=NULL;
  int cle_liste ;
  liste_t * liste=NULL ; 
  char marque ;

  booleen_t fini = FAUX ;
  piste_id_t deplacement = 0 ;
  piste_id_t depart = 0 ;
  piste_id_t arrivee = 0 ;
  cell_t cell_cheval ;
  elem_t elem_cheval ;
  cell_t cellChevalDec;
  elem_t chevalDeca;
  int index=0;
  int chevaladec;

  /*-----*/

  if( nb_arg != 4 )
    {
      fprintf( stderr, "usage : %s <cle de piste> <cle de liste> <marque>\n" , tab_arg[0] );
      exit(-1);
    }
  
  if( sscanf( tab_arg[1] , "%d" , &cle_piste) != 1 )
    {
      fprintf( stderr, "%s : erreur , mauvaise cle de piste (%s)\n" , 
	       tab_arg[0]  , tab_arg[1] );
      exit(-2);
    }


  if( sscanf( tab_arg[2] , "%d" , &cle_liste) != 1 )
    {
      fprintf( stderr, "%s : erreur , mauvaise cle de liste (%s)\n" , 
	       tab_arg[0]  , tab_arg[2] );
      exit(-2);
    }

  if( sscanf( tab_arg[3] , "%c" , &marque) != 1 )
    {
      fprintf( stderr, "%s : erreur , mauvaise marque de cheval (%s)\n" , 
	       tab_arg[0]  , tab_arg[3] );
      exit(-2);
    }
     
  /*recupere la piste*/
  int shmid_piste = shmget(cle_piste, PISTE_LONGUEUR*sizeof(cell_t), IPC_CREAT | 0666);
  piste = shmat(shmid_piste,0,0);
  int semidPiste = semget(cle_piste,PISTE_LONGUEUR,0666);

  /*recuperation de la liste */

  int shmid_liste = shmget(cle_liste, LISTE_MAX*sizeof(elem_t), IPC_CREAT | 0666);
  liste = shmat(shmid_liste,0,0);
  int semidListe = semget(cle_liste,1,0666);
  
  
  /* Init de l'attente */
  commun_initialiser_attentes() ;


  /* Init de la cellule du cheval pour faire la course */
  cell_pid_affecter( &cell_cheval  , getpid());
  cell_marque_affecter( &cell_cheval , marque );

  /* Init de l'element du cheval pour l'enregistrement */
  elem_cell_affecter(&elem_cheval , cell_cheval ) ;
  elem_etat_affecter(&elem_cheval , EN_COURSE ) ;

  /* 
   * Enregistrement du cheval dans la liste
   */
  elem_sem_creer(&elem_cheval);
  P(semidListe);
  liste_elem_ajouter(liste,elem_cheval);
  V(semidListe);

// on recupere l'index du cheval
 liste_elem_rechercher(&index,liste,elem_cheval);



  

  while( ! fini )
    {
      
    /* Attente entre 2 coup de de */
    commun_attendre_tour() ;


    // on veroulle la liste
    P(semidListe);
    if(elem_decanille(elem_cheval )){

      fini=VRAI;
      // on veroulle la piste  pour modifier
      P(semidPiste);
      piste_cell_effacer(piste,index);
      V(semidPiste);
      
      // on recherche l'element a supprimer
      liste_elem_rechercher(&index,liste,elem_cheval);
      liste_elem_supprimer(liste,index);
      V(semidListe);
      elem_sem_deverrouiller(&elem_cheval);
      elem_sem_detruire(&elem_cheval);


      // on retire le semaphore du cheval
      elem_sem_detruire(&elem_cheval);
      exit(0);
    }
    else{
      V(semidListe);
    }
      /*
       * Avancee sur la piste
       */

      /* Coup de de */
      deplacement = commun_coup_de_de() ;
#ifdef _DEBUG_
      printf(" Lancement du De --> %d\n", deplacement );
#endif

    arrivee = depart+deplacement ;

    if( arrivee > PISTE_LONGUEUR-1 )
	  {
	  arrivee = PISTE_LONGUEUR-1 ;
	  fini = VRAI ;
	  }

    if( depart != arrivee )
	  {

	  /* 
	   * Si case d'arrivee occupee alors on decanille le cheval existant 
	   */


    elem_sem_verrouiller(&elem_cheval);

    if(piste_cell_occupee( piste , arrivee)){
      /* on decanille le cheval sur la case*/
      // on decanille 
      piste_cell_lire(piste,arrivee,&cellChevalDec);
      elem_cell_affecter(&chevalDeca,cellChevalDec);
      // on verouille la liste pour rechercher le cheval
      P(semidListe);
      liste_elem_rechercher(&chevaladec,liste,chevalDeca);
      elem_sem_verrouiller(&chevalDeca);
      liste_elem_decaniller(liste,chevaladec);
      //on deveroulle les elements
      elem_sem_deverrouiller(&chevalDeca);
      V(semidListe);
    }
    
    /* 
	   * Deplacement: effacement case de depart, affectation case d'arrivee 
	   */
    /*verouller la piste et la liste*/
    P1(semidPiste,arrivee);
    P1(semidPiste,depart);

    // on met le cheval sur le depart
    piste_cell_effacer(piste,depart);
    commun_attendre_fin_saut();
    // on deveroulle le cheval sur le depart
    elem_sem_deverrouiller(&elem_cheval); 


    piste_cell_affecter(piste,arrivee,cell_cheval);
   
   // on debloque le tout
   V1(semidPiste,depart);
   V1(semidPiste,arrivee);
	       
	  

#ifdef _DEBUG_
	  printf("Deplacement du cheval \"%c\" de %d a %d\n",
		 marque, depart, arrivee );
#endif

	  
	} 
      /* Affichage de la piste  */
      piste_afficher_lig( piste );
     
      depart = arrivee ;
    }

  printf( "Le cheval \"%c\" A FRANCHIT LA LIGNE D ARRIVEE\n" , marque );

 
     
  /* 
   * Suppression du cheval de la liste 
   */
  P(semidPiste);
  piste_cell_effacer(piste,depart);
  V(semidPiste);
  P(semidListe);
  liste_elem_supprimer(liste,index);
  V(semidListe);
  exit(0);
}
