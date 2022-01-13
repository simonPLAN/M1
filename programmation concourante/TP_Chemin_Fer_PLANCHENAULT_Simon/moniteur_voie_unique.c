#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <sens.h>
#include <train.h>
#include <moniteur_voie_unique.h>

/*---------- MONITEUR ----------*/

extern moniteur_voie_unique_t * moniteur_voie_unique_creer( const train_id_t nb )
{
  moniteur_voie_unique_t * moniteur = NULL ; 

  /* Creation structure moniteur */
  if( ( moniteur = malloc( sizeof(moniteur_voie_unique_t) ) ) == NULL  )
    {
      fprintf( stderr , "moniteur_voie_unique_creer: debordement memoire (%lu octets demandes)\n" , 
	       sizeof(moniteur_voie_unique_t) ) ;
      return(NULL) ; 
    }

  /* Creation de la voie */
  if( ( moniteur->voie_unique = voie_unique_creer() ) == NULL )
    return(NULL) ; 
  
  /* Initialisations du moniteur */

  /***********/
  pthread_mutex_init(&moniteur->mutex_Voie, NULL); 
  moniteur->cond_Acces_Ouest=(pthread_cond_t)PTHREAD_COND_INITIALIZER;
  moniteur->cond_Acces_Est=(pthread_cond_t)PTHREAD_COND_INITIALIZER;

  moniteur->nbMax_Trains_Sur_La_Voie=nb; 
  moniteur-> nb_Train_Est=0; 
  moniteur-> nb_Train_Ouest=0;
 
  /***********/

  return(moniteur) ; 
}

extern int moniteur_voie_unique_detruire( moniteur_voie_unique_t ** moniteur )
{
  int noerr ; 

  /* Destructions des attribiuts du moniteur */

  /***********/
  pthread_mutex_destroy(&(*moniteur)-> mutex_Voie);
  pthread_cond_destroy(&(*moniteur) -> cond_Acces_Ouest);
  pthread_cond_destroy(&(*moniteur) -> cond_Acces_Est);
  /***********/

  /* Destruction de la voie */
  if( ( noerr = voie_unique_detruire( &((*moniteur)->voie_unique) ) ) )
    return(noerr) ;

  /* Destruction de la strcuture du moniteur */
  free( (*moniteur) );

  (*moniteur) = NULL ; 

  return(0) ; 
}

extern void moniteur_voie_unique_entree_ouest( moniteur_voie_unique_t * moniteur ) 
{
  // verouillage de la voie
  pthread_mutex_lock(&moniteur->mutex_Voie);
   // si train > est alors on fait attendre
   while(moniteur->nb_Train_Est>0){
     pthread_cond_wait(&moniteur->cond_Acces_Ouest,&moniteur->mutex_Voie);
   }
   moniteur->nb_Train_Ouest++;
   pthread_cond_signal(&(moniteur->cond_Acces_Ouest)); 

   // on deverouille le mutex 
     pthread_mutex_unlock(&(moniteur->mutex_Voie));

}

extern void moniteur_voie_unique_sortie_est( moniteur_voie_unique_t * moniteur ) 
{

  // verouillage de la voie
  pthread_mutex_lock(&(moniteur->mutex_Voie)); 

  // On diminue le nombre de train 
  moniteur->nb_Train_Ouest--;
//si train ouest =0 on emet un signal en est
  if((moniteur->nb_Train_Ouest == 0)){
    pthread_cond_signal(&(moniteur->cond_Acces_Est)); 
  }
  // train ouest = nb train sur la voie on sigal l ouest
  if(moniteur->nb_Train_Ouest== moniteur->nbMax_Trains_Sur_La_Voie-1){
    pthread_cond_signal(&(moniteur->cond_Acces_Ouest)); 

  }
   // on deverouille le mutex 
  pthread_mutex_unlock(&(moniteur->mutex_Voie)); 
}

extern void moniteur_voie_unique_entree_est( moniteur_voie_unique_t * moniteur ) 
{
 // verouillage de la voie
  pthread_mutex_lock(&moniteur->mutex_Voie);
   // si train  ouest alor on met en attente le train de  l autre coter
   while(moniteur->nb_Train_Ouest>0){
     pthread_cond_wait(&moniteur->cond_Acces_Est,&moniteur->mutex_Voie);
   }
   moniteur->nb_Train_Est++;
   pthread_cond_signal(&(moniteur->cond_Acces_Est)); 

   // on deverouille le mutex 
     pthread_mutex_unlock(&(moniteur->mutex_Voie));

}

extern void moniteur_voie_unique_sortie_ouest( moniteur_voie_unique_t * moniteur ) 
{
   // verouillage de la voie
  pthread_mutex_lock(&(moniteur->mutex_Voie)); 

  // On diminue le nombre de train 
  moniteur->nb_Train_Est--;
//si train est =0 on emet un signal en est
  if((moniteur->nb_Train_Est == 0)){
    pthread_cond_signal(&(moniteur->cond_Acces_Ouest)); 
  }
    // train est = nb train sur la voie on sigal l est

  if(moniteur->nb_Train_Est== moniteur->nbMax_Trains_Sur_La_Voie-1){
    pthread_cond_signal(&(moniteur->cond_Acces_Est)); 

  }
   // on deverouille le mutex 
  pthread_mutex_unlock(&(moniteur->mutex_Voie)); 
}

/*
 * Fonctions set/get 
 */

extern 
voie_unique_t * moniteur_voie_unique_get( moniteur_voie_unique_t * const moniteur )
{
  return( moniteur->voie_unique ) ; 
}


extern 
train_id_t moniteur_max_trains_get( moniteur_voie_unique_t * const moniteur )
{
    return (moniteur->voie_unique); 

}



extern
int moniteur_voie_unique_extraire( moniteur_voie_unique_t * moniteur , train_t * train , zone_t zone  ) 
{
  int ai;
   pthread_mutex_lock(&moniteur->mutex_Voie);
  ai=voie_unique_extraire( moniteur->voie_unique,(*train), zone ,train_sens_get(train) )  ; 
    pthread_mutex_unlock(&moniteur->mutex_Voie);

return ai;

  
}

extern
int moniteur_voie_unique_inserer( moniteur_voie_unique_t * moniteur , train_t * train , zone_t zone ) 
{ 
  int ia;
     pthread_mutex_lock(&moniteur->mutex_Voie);

  ia=voie_unique_inserer( moniteur->voie_unique, 
			       (*train), 
			       zone, 
			       train_sens_get(train) ) ;
       pthread_mutex_unlock(&moniteur->mutex_Voie);
          
 return ia;
}
