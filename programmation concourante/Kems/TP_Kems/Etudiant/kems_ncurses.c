// PLANCHENAULT Simon
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <ncurses.h>
#include <commun.h>
#include <paquet.h>
#include <tapis.h>
#include <ecran.h>
#include <pthread.h>


// Variables Globales
  paquet_t * paquet = PAQUET_NULL ;
  tapis_t * tapis_central = TAPIS_NULL ;
  tapis_t ** tapis = NULL ; /* tableau des tapis */
  tapis_id_t t = 0 ; /* Compteur de tapis */
  err_t cr = OK ;
  carte_id_t c = -1 ;
  int NbJoueurs = 0;
  booleen_t fini = FAUX;
  int peutPasJouer = 0;
  char mess[256] ;
  ecran_t * ecran = NULL ;



	// crezation des mutex
	pthread_mutex_t V_Tapis = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t V_Carte1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t V_Carte2 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t V_Carte3 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t V_Carte4 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t V_Fini = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t V_Ecran = PTHREAD_MUTEX_INITIALIZER;

void arret( int sig ){
  printf( "Arret utilisateur\n");
}

void Joueur(int i){
  carte_id_t ind_carte = -1;  
  carte_id_t ind_carte_central = -1;  
	booleen_t echange = FAUX; 
	fini = FAUX; 

  pthread_mutex_lock(&V_Fini); 
  while(!fini){
    pthread_mutex_unlock(&V_Fini); 

    if(tapis_carre(tapis[i])){ 
      pthread_mutex_lock(&V_Fini); 
      fini = VRAI;
      // on verouille le mutex de l'ecran avant d afficher puis on deverouille
      pthread_mutex_unlock(&V_Fini); 
      pthread_mutex_lock(&V_Ecran); 
      sprintf(mess, "Le joueur %d a gagné", i); 
      ecran_message_pause_afficher(ecran, mess); 
      pause();
      pthread_mutex_unlock(&V_Ecran); 
      pthread_exit(0);
    }
		// verouillage du tapis avant dechanger les cartes
    pthread_mutex_lock(&V_Tapis);

		//echanges de deux carte
    if((cr = tapis_cartes_choisir(&echange, tapis[i], &ind_carte, tapis_central, &ind_carte_central))){
      sprintf(mess, "Pb dans choix des cartes, code retour = %d\n", cr);
      // on verouille le mutex de l'ecran avant d afficher puis on deverouille
      pthread_mutex_lock(&V_Ecran); 
      ecran_message_pause_afficher(ecran, mess); 
      pthread_mutex_unlock(&V_Ecran); 
      erreur_afficher(cr); 
      break; 
    }

		if(echange){
      peutPasJouer = 0;
			//on applique le mutex sur la carte que l ont va echanger
    	switch (ind_carte_central){
        case 0: pthread_mutex_lock(&V_Carte1);
        	break;
        case 1: pthread_mutex_lock(&V_Carte2);
        	break;
        case 2: pthread_mutex_lock(&V_Carte3);
        	break;
        case 3: pthread_mutex_lock(&V_Carte4);
        	break;
        	}

			// on procède à l'échange entre une carte d'un joueur avec une carte du tapis
      if((cr = tapis_cartes_echanger(tapis[i], ind_carte, tapis_central, ind_carte_central))){
        sprintf(mess, "Pb d'échange de cartes entre la carte %ld du tapis du joueur %d et la carte %ld du tapis central", ind_carte, i, ind_carte_central); 
 // on veroulle le mutex de l'ecran avant d afficher puis on deverouille
        pthread_mutex_lock(&V_Ecran); 
        ecran_message_pause_afficher(ecran, mess); 
        pthread_mutex_unlock(&V_Ecran); 
        erreur_afficher(cr); 
        break; 
      }	     

      pthread_mutex_lock(&V_Ecran); 
      sprintf(mess, "Joueur %i: Echange carte %ld avec carte %ld du tapis central ", i, ind_carte, ind_carte_central);
 // on veroulle le mutex de l'ecran avant d afficher puis on deverouille
      ecran_message_pause_afficher(ecran, mess); 
      ecran_cartes_echanger(ecran, f_tapis_f_carte_lire(ecran_tapis_central_lire(ecran), ind_carte_central), f_tapis_f_carte_lire(ecran_tapis_joueur_lire(ecran, i), ind_carte));
      ecran_afficher(ecran, tapis_central, tapis); 
      ecran_message_effacer(ecran);
      pthread_mutex_unlock(&V_Ecran); 

			//on retire le mutex de la carte que l ont avait mis precedament
			switch (ind_carte_central){
        case 0: pthread_mutex_lock(&V_Carte1);
        	break;
        case 1: pthread_mutex_lock(&V_Carte2);
        	break;
        case 2: pthread_mutex_lock(&V_Carte3);
        	break;
        case 3: pthread_mutex_lock(&V_Carte4);
        	break;
        }	 
	  
    }
    else{
      peutPasJouer++;
    }
		//on retire le mutex du tapipis
    pthread_mutex_unlock(&V_Tapis);
	}
}


void Tapis(){
//application du mutex
	pthread_mutex_lock(&V_Fini); 
  while(!fini){
    pthread_mutex_unlock(&V_Fini); 
    sleep(10);
    if(peutPasJouer>NbJoueurs*2){
      //application du mutex avant d echanger deux carte
      pthread_mutex_lock(&V_Tapis);
      pthread_mutex_lock(&V_Ecran);
      ecran_message_pause_afficher(ecran, "Pas d'échange --> Redistribution tapis central"); 
      for(c = 0; c < TAPIS_NB_CARTES; c++){
        if((cr = tapis_carte_retirer(tapis_central, c, paquet))){
          ecran_message_pause_afficher(ecran, "Pb dans retrait d'une carte du tapis central"); 
          erreur_afficher(cr); 
          break; 
        }
        if((cr = tapis_carte_distribuer(tapis_central, c, paquet))){
          ecran_message_pause_afficher(ecran, "Pb dans distribution d'une carte pour le tapis central"); 
          erreur_afficher(cr); 
          break; 
        }
      }
      ecran_afficher(ecran, tapis_central, tapis);
      ecran_message_effacer(ecran); 
      // on retire les mutex
      pthread_mutex_unlock(&V_Tapis);
      pthread_mutex_unlock(&V_Ecran);
    }  
  }
}

int main (int argc, char * argv[]){
  int i; 
  signal(SIGINT, arret); 
  printf("\n\n\n\t===========Début %s==========\n\n", argv[0]);
  if(argc != 2){
    printf(" Programme de test sur l'affichage de <nb joueurs> tapis avec ncurses\n");
    printf(" usage : %s <nb joueurs>\n", argv[0]);
    exit(0); 
  }
	NbJoueurs = atoi(argv[1]);
	if(NbJoueurs <= 0){
		printf("Le nombre de joueurs est insuffisant !\n");
		exit(0);
	}
	else if (NbJoueurs > 7){
		printf("Dans cette version, il ne peut y avoir que jusqu'à 7 joueurs !\n");
		exit(0);		
	}
 	srandom(getpid());
  /* Création du paquet */
  printf("Création du paquet..." ); fflush(stdout); 
  paquet = paquet_creer(); 
  printf("OK\n"); 

  /* Création tapis central */
  printf("Création du tapis central..."); 
  fflush(stdout); 
  if((tapis_central = tapis_creer() ) == TAPIS_NULL){
      printf("Erreur sur création du tapis central\n");
      exit(-1);
  }

  for(c = 0; c < TAPIS_NB_CARTES; c++){
    if((cr = tapis_carte_distribuer(tapis_central, c, paquet))){
	    erreur_afficher(cr); 
	    exit(-1); 
	  }
  }
  printf("OK\n"); 

  /* Création des tapis des joueurs */
  printf("Créations des %d tapis...", NbJoueurs);
  fflush(stdout); 

  if((tapis = malloc(sizeof(tapis_t *) * NbJoueurs)) == NULL){
    printf(" Erreur allocation mémoire tableau des tapis (%lu octets demandés)\n", (long unsigned int)(sizeof(tapis_t *) * NbJoueurs));
    exit(-1); 
  }

  for(t = 0; t < NbJoueurs; t++){
    if((tapis[t] = tapis_creer()) == TAPIS_NULL){
      printf("Erreur sur création du tapis %ld\n", t);
      exit(-1);
	  }

    for(c = 0; c < TAPIS_NB_CARTES; c++){
	    if((cr = tapis_carte_distribuer(tapis[t], c, paquet))){
	      if(cr == ERR_PAQUET_VIDE){
          printf("Pas assez de cartes pour tous les joueurs\n"); 
        }
	      erreur_afficher(cr) ; 
	      exit(-1) ; 
	    }
	  }
  }
  printf("OK\n"); 

  if( ( ecran = ecran_creer( tapis_central , 
			     tapis , 
			     NbJoueurs ) ) == NULL ) 
    {
      printf("Erreur sur la creation de l'ecran\n"); 
      exit(-1) ;
    }

  ecran_message_afficher( ecran , "Debut de partie: ^C pour commencer");
  pause() ; 


  //tableau avec l ensemble des thread
	int nbThreads = NbJoueurs + 1;
	pthread_t tousLesThreads[nbThreads]; 

	//creation  de l'ensemble des thread joueur
	for(i = 0; i < NbJoueurs; i++){
		pthread_create(&tousLesThreads[i], NULL, (void *)Joueur, i); 
	}
	//creation  de l'ensemble des thread tapis

	pthread_create(&tousLesThreads[nbThreads-1], NULL, (void *)Tapis, (void *)NULL); 
	
	for(i = 0; i < nbThreads; i++){
		pthread_join(tousLesThreads[i], NULL); 
	}

  /* Destruction de l'ecran */
  if( ( cr = ecran_detruire( &ecran ) ) )
    {
      fprintf( stderr , "Erreur lors de la destruction de l'ecran, cr = %d\n" , cr ) ;
      exit(-1) ; 
    }
    

  /* Destructions du tapis central */
  printf( "Destructions du tapis central...") ; fflush(stdout) ; 
  if( ( cr = tapis_detruire( &tapis_central ) ) )
    {
      fprintf( stderr , " Erreur sur destruction du tapis central\n") ;
      erreur_afficher(cr) ; 
      exit(-1) ; 
    }
  printf("OK\n"); 
  
  /* Destructions des tapis des joueurs */
  printf( "Destructions des tapis des joueurs...") ; fflush(stdout) ; 
  for( t=0 ; t<NbJoueurs ; t++ )  
    {

      if( ( cr = tapis_detruire( &tapis[t] ) ) )
       {
	 fprintf( stderr , " Erreur sur destruction du tapis %ld\n"  , t ) ;
	 erreur_afficher(cr) ; 
	 exit(-1) ; 
       }

    }
  free( tapis ) ;
  printf("OK\n"); 

  /*  Destruction du paquet */
  printf("\nDestruction du paquet..." ) ; fflush(stdout) ; 
  if( ( cr = paquet_detruire( &paquet ) ) )
    {
	 fprintf( stderr , " Erreur sur destruction du paquet\n" ) ;
	 erreur_afficher(cr) ; 
	 exit(-1) ; 
    }
  printf("OK\n") ; 

 // destruction de l'ensemble des mutex
	pthread_mutex_destroy(&V_Fini); 
	pthread_mutex_destroy(&V_Tapis); 
	pthread_mutex_destroy(&V_Carte1); 
	pthread_mutex_destroy(&V_Carte2); 
	pthread_mutex_destroy(&V_Carte3); 
	pthread_mutex_destroy(&V_Carte4); 
 	pthread_mutex_destroy(&V_Ecran); 
	printf("OK\n"); 
  printf("\n\n\t===========Fin %s==========\n\n", argv[0]);
  return(0);
}