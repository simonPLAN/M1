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

struct sembuf op_p1 = {0, -1, 0};
struct sembuf op_v1 = {0, 1, 0};

static void P1(int Sem_id){
  semop(Sem_id,&op_p1,1);
}
static void V1(int Sem_id){
  semop(Sem_id,&op_v1,1);
}
// on deverouille
void V(int semid, int numSemaphore){
  struct sembuf op_p = {numSemaphore, -1, 0};
  semop(semid, &op_p, 1);
}

// on verouille
void P(int semid, int numSemaphore){
  struct sembuf op_v = {numSemaphore, 1, 0};
  semop(semid, &op_v, 1);
}

int main (int nb_arg, char * tab_arg[]){
  int ind_cheval_lst;
  
  elem_t cheval_a_decaniller;
  cell_t c_arrivee;
  int ind_cheval_a_decaniller;

  int cle_piste;
  piste_t * piste = NULL;

  int cle_liste;
  liste_t * liste = NULL;

  char marque;

  booleen_t fini = FAUX;
  piste_id_t deplacement = 0;
  piste_id_t depart = 0;
  piste_id_t arrivee = 0;

  cell_t cell_cheval;

  elem_t elem_cheval;

  if(nb_arg != 4){
    fprintf(stderr,"usage : %s <clé de piste> <clé de liste> <marque>\n", tab_arg[0]);
    exit(-1);
  }

  if(sscanf(tab_arg[1],"%d",&cle_piste) != 1){
    fprintf(stderr,"%s : erreur , mauvaise clé de piste (%s)\n", tab_arg[0], tab_arg[1]);
    exit(-2);
  }

  if(sscanf(tab_arg[2],"%d",&cle_liste) != 1){
    fprintf(stderr,"%s : erreur , mauvaise clé de liste (%s)\n", tab_arg[0], tab_arg[2]);
    exit(-2);
  }

  if(sscanf(tab_arg[3],"%c",&marque) != 1){
    fprintf(stderr,"%s : erreur , mauvaise marque de cheval (%s)\n", tab_arg[0], tab_arg[3]);
    exit(-2);
  }

   /*recupere la piste*/
  int shmid_piste = shmget(cle_piste, PISTE_LONGUEUR*sizeof(cell_t), IPC_CREAT | 0666);
  piste = shmat(shmid_piste,0,0);
  int semidPiste = semget(cle_piste,PISTE_LONGUEUR,0666);

  /*recuperation de la liste */

  int shmid_lst = shmget(cle_liste, LISTE_MAX*sizeof(elem_t), IPC_CREAT | 0666);
  liste = shmat(shmid_lst,0,0);
  int semidListe = semget(cle_liste,1,0666);
  
  
  commun_initialiser_attentes();
  cell_pid_affecter(&cell_cheval, getpid());
  cell_marque_affecter(&cell_cheval, marque);
  elem_cell_affecter(&elem_cheval, cell_cheval);
  elem_etat_affecter(&elem_cheval, EN_COURSE);

  /*
   * Enregistrement du cheval dans la liste
   */
//on creer le semaphore
  elem_sem_creer(&elem_cheval);
  P1(semidListe);
  //on l'ajoute à la liste
  liste_elem_ajouter(liste, elem_cheval);
  V1(semidListe);

  // Récupération de l'indice du cheval dans la liste 
  liste_elem_rechercher(&ind_cheval_lst, liste, elem_cheval);
    

  while(!fini){
    /* Attente entre 2 coup de dé */
    commun_attendre_tour() ;

    /*
      * Vérif si pas décanillé
      */
    

    //actualise la place du cheval dans la Liste
    P1(semidListe);
    liste_elem_rechercher(&ind_cheval_lst, liste, elem_cheval);
    elem_cheval = liste_elem_lire(liste,ind_cheval_lst);
    V1(semidListe);

    // On vérifie si le cheval est décanillé 
    if(elem_decanille(elem_cheval)){
      printf("\ndécanillé\n");
      break;
    }

    /*
      * Avancée sur la piste
      */

    /* Coup de dé */
    deplacement = commun_coup_de_de();

    arrivee = depart + deplacement;

    if(arrivee > PISTE_LONGUEUR-1){
	    arrivee = PISTE_LONGUEUR-1;
	    fini = VRAI;
	  }

    if(depart != arrivee){
      elem_sem_verrouiller(&elem_cheval);
      P1(semidPiste);
     /*
      * Si case d'arrivée est occupée alors on décanille le cheval existant
      */
      if(piste_cell_occupee(piste,arrivee)){
        piste_cell_lire(piste, arrivee, &c_arrivee);
        cell_marque_affecter(&cheval_a_decaniller.cell, cell_marque_lire(c_arrivee));
        cell_pid_affecter(&cheval_a_decaniller.cell, cell_pid_lire(c_arrivee));
        P1(semidListe);
        liste_elem_rechercher(&ind_cheval_a_decaniller, liste, cheval_a_decaniller);
        // on met a jour le cheval a decaniller
        cheval_a_decaniller = liste_elem_lire(liste, ind_cheval_a_decaniller);
        elem_sem_verrouiller(&cheval_a_decaniller);
        //on le decanille

        liste_elem_decaniller(liste, ind_cheval_a_decaniller);
        elem_sem_deverrouiller(&cheval_a_decaniller);
        V1(semidListe);
      }
//deveroulle la piste
    V1(semidPiste);

	  /*
	   * Déplacement: effacement case de depart, affectation case d'arrivée
	   */
// on pose les semaphore
    P(semidPiste, arrivee);
    P(semidPiste, depart);
    //on efface la cellule de depars
    piste_cell_effacer(piste,depart);
    commun_attendre_fin_saut();
    elem_sem_deverrouiller(&elem_cheval);
    //on atribut la celule d arriver
    piste_cell_affecter(piste, arrivee, cell_cheval);
    //on deverouille
    V(semidPiste, arrivee);
    V(semidPiste, depart);

	}
    // on affiche la piste
    piste_afficher_lig(piste);

    depart = arrivee;
  }

  if(elem_decanille(elem_cheval)){
    printf("Le cheval \"%c\" A FINI DECANILLE\n", marque);
  }
  else {
    printf("Le cheval \"%c\" A FRANCHIT LA LIGNE D ARRIVEE\n", marque);
  }

  /*
   * Suppression du cheval de la liste
   */

//on veroulle la piste avant de supprimer la  cellule
  P(semidPiste,depart);
  //on l'eface de la piste
  piste_cell_effacer(piste, depart);
  V(semidPiste, depart);
  P1(semidListe);
  liste_elem_rechercher(&ind_cheval_lst, liste, elem_cheval);
  elem_cheval = liste_elem_lire(liste,ind_cheval_lst);
  //on l eface de la liste
  liste_elem_supprimer(liste, ind_cheval_lst);
  V1(semidListe);
  // on detruit le semaphore du cheval
  elem_sem_detruire(&elem_cheval);

  exit(0);
}//fin