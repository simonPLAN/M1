// PLANCHENAULT Simon
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <paquet.h>
#include <tapis.h>
#include <pthread.h>


// Variables Globales
    paquet_t * paquet = PAQUET_NULL ;
    tapis_t * tapis_central = TAPIS_NULL ;
    tapis_t ** tapis = NULL ; /* tableau des tapis */
    err_t cr = OK ;
    carte_id_t c = -1 ;
    int nbJoueur = 0;
    booleen_t fini = FAUX;
	int peutPasJouer = 0;
	int nb_joueur = 0;

// On initialises tout les mutex 
    pthread_mutex_t mutex_Tapis = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_Carte1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_Carte2 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_Carte3 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_Carte4 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_Fini = PTHREAD_MUTEX_INITIALIZER;




void Joueur(int i){
    carte_id_t ind_carte = -1 ;
    carte_id_t ind_carte_central = -1 ;
    booleen_t echange = FAUX; 
    fini = FAUX;
	pthread_mutex_lock(&mutex_Fini);

	while(!fini){
	    pthread_mutex_unlock(&mutex_Fini);

        //Affichage du joueur
	    printf( "Tapis joueur %d\n" , i ) ;
	    tapis_stdout_afficher( tapis[i] ) ;
	    printf( "\n" );

	    // On test si le joueur i a un carre(=4 cartes identiques)
	    if(tapis_carre(tapis[i])){
			//on veroulle la valeur avant de la modifier puis on la deverouille
	        pthread_mutex_lock(&mutex_Fini);
	        fini = VRAI;
	        pthread_mutex_unlock(&mutex_Fini);
            printf( "* Le joueur%2d a gagné *\n" , i ) ;
            break ; 
	    }

		//on active le mutex du tapis vaant tout changement et modification des cartes
		pthread_mutex_lock(&mutex_Tapis);

		//echange de deux carte entre deux personnage
        if( ( cr = tapis_cartes_choisir( &echange , tapis[i] , &ind_carte , tapis_central , &ind_carte_central) ) ){
            printf( "Pb dans choix des cartes, code retour = %d\n", cr ) ;
            erreur_afficher(cr) ;
            exit(-1) ;
        }

		if( echange ){
			peutPasJouer = 0;
			//on applique le mutex sur la carte que l ont va echanger
        	switch (ind_carte_central){
        		case 0: pthread_mutex_lock(&mutex_Carte1);
        			break;
        		case 1: pthread_mutex_lock(&mutex_Carte2);
        			break;
        		case 2: pthread_mutex_lock(&mutex_Carte3);
        			break;
        		case 3: pthread_mutex_lock(&mutex_Carte4);
        			break;
        	}

			// on procède à l'échange entre une carte d'un joueur avec une carte du tapis
            if( ( cr = tapis_cartes_echanger( tapis[i] , ind_carte , tapis_central , ind_carte_central ) ) ){
                printf( "Pb d'echange de cartes entre la carte %ld du tapis du joueur %d\n" , ind_carte , i );
                carte_stdout_afficher( tapis_carte_lire( tapis[i] , ind_carte ) ) ;
                printf( "\n et la carte %ld du tapis central\n" , ind_carte_central );
                carte_stdout_afficher( tapis_carte_lire( tapis_central , ind_carte_central ) ) ;
                erreur_afficher(cr) ;
                exit(-1) ;
            }

			//on retire le mutex de la carte que l ont avait mis precedament
        	switch (ind_carte_central){
        		case 0: pthread_mutex_unlock(&mutex_Carte1);
        			break;
        		case 1: pthread_mutex_unlock(&mutex_Carte2);
        			break;
        		case 2: pthread_mutex_unlock(&mutex_Carte3);
        			break;
        		case 3: pthread_mutex_unlock(&mutex_Carte4);
        			break;
        	}
        }
		else{
			peutPasJouer ++;
		}

		//on retire le mutex du tapis
    	pthread_mutex_unlock(&mutex_Tapis);
	}
}


void Tapis(){
	pthread_mutex_lock(&mutex_Fini);
    while(!fini){
		pthread_mutex_unlock(&mutex_Fini);
		if(peutPasJouer > nb_joueur * 2){
//affichage du  tapis central
			printf( "Tapis central \n" ) ;
			tapis_stdout_afficher( tapis_central ) ;
			printf( "\n" );

//verouillage du tapis avant les permutations
			pthread_mutex_lock(&mutex_Tapis);
			
			printf( "Redistribution tapis central\n") ;
			for( c=0 ; c<TAPIS_NB_CARTES ; c++ ){
				if( ( cr = tapis_carte_retirer( tapis_central , c , paquet ) ) ){
					printf( "Pb dans retrait d'une carte du tapis central\n" );
					erreur_afficher(cr) ;
					exit(-1) ;
				}

				if( ( cr = tapis_carte_distribuer( tapis_central , c , paquet ) ) ){
					printf( "Pb dans distribution d'une carte pour le tapis central\n" );
					erreur_afficher(cr) ;
					exit(-1) ;
				}
			}
			
			pthread_mutex_unlock(&mutex_Tapis);
		}
		
	}
}

int main( int argc , char * argv[] ){
    int i;

    printf("\n\n\n\t===========Debut %s==========\n\n" , argv[0] );

    if( argc != 2 ){
    	printf( " Programme de test des joueurs de Kems\n" );
    	printf( " usage : %s <Nb joueurs>\n" , argv[0] );
    	exit(0);
    }

    if(atoi( argv[1] ) <= 0){
    	printf( " Nombre de joueurs insuffisant ! \n" );
    	exit(0);
    }

    nb_joueur  = atoi( argv[1] ) ;

    srandom(getpid());

    printf("Creation du paquet de cartes\n") ;
    if( ( paquet = paquet_creer() ) == PAQUET_NULL ){
    	printf("Erreur sur creation du paquet\n" ) ;
    	exit(-1) ;
    }

    printf("Creation du tapis central\n")  ;
    if( ( tapis_central = tapis_creer() ) == TAPIS_NULL ){
    	printf("Erreur sur creation du tapis central\n" ) ;
    	exit(-1) ;
    }

    for( c=0 ; c<TAPIS_NB_CARTES ; c++ ){
    	if( ( cr = tapis_carte_distribuer( tapis_central  , c , paquet ) ) ){
    		erreur_afficher(cr) ;
    		exit(-1) ;
    	}
    }

    printf( "Tapis Central\n" ) ;
    tapis_stdout_afficher( tapis_central ) ;
    printf("\n");

    printf("Creation des %d tapis des joueurs\n" , nb_joueur ) ;
    if( ( tapis = malloc( sizeof(tapis_t *) * nb_joueur ) ) == NULL ){
    	printf(" Erreur allocation memoire tableau des tapis (%lu octets demandes)\n" ,(long unsigned int)(sizeof(tapis_t *) * nb_joueur) ) ;
    	exit(-1) ;
    }

    for( i=0 ; i<nb_joueur ; i++ ){
    	if( ( tapis[i] = tapis_creer() ) == TAPIS_NULL ){
    		printf("Erreur sur creation du tapis %d\n" , i ) ;
    		exit(-1) ;
    	}

    	for( c=0 ; c<TAPIS_NB_CARTES ; c++ ){
    		if( ( cr = tapis_carte_distribuer( tapis[i]  , c , paquet ) ) ){
    			if( cr == ERR_PAQUET_VIDE ) printf("Pas assez de cartes pour tous les joueurs\n");
    			erreur_afficher(cr) ;
    			exit(-1) ;
    		}
    	}

    	printf( "Tapis joueur %d\n" , i ) ;
    	tapis_stdout_afficher( tapis[i] ) ;
    	printf("\n");
    }

    int nThread = nb_joueur + 1;

    pthread_t tThread[nThread];


    for(i = 0; i<nb_joueur; i++){
    	pthread_create(&tThread[i], NULL, (void *)Joueur, i);
    }

    pthread_create(&tThread[nThread - 1], NULL, (void *)Tapis, (void *)NULL);

    for(i = 0; i<nThread; i++){
    	pthread_join(tThread[i], NULL);
    }

    printf("\nDestruction des tapis..." ) ; fflush(stdout) ;
     for (i=0 ; i<nb_joueur ; i++ ){
    	if( ( cr = tapis_detruire( &tapis[i] ) ) ){
    	    printf(" Erreur sur destruction du tapis du joueur %d\n"  , i ) ;
    	    erreur_afficher(cr) ;
    	    exit(-1) ;
        }
     }
     printf("OK\n") ;


     printf("\nDestruction du paquet..." ) ; fflush(stdout) ;
     paquet_detruire( &paquet ) ;
     printf("OK\n") ;

     printf("\n\n\t===========Fin %s==========\n\n" , argv[0] );



// destruction de tout les thread
    pthread_mutex_destroy(&mutex_Fini);
    pthread_mutex_destroy(&mutex_Tapis);
    pthread_mutex_destroy(&mutex_Carte1);
    pthread_mutex_destroy(&mutex_Carte2);
    pthread_mutex_destroy(&mutex_Carte3);
    pthread_mutex_destroy(&mutex_Carte4);

    return 0;
}