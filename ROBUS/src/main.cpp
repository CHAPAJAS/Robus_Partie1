/******************************************************************************/
/* Inclusions --------------------------------------------------------------- */
#include <Arduino.h>
#include "LibCHAPAJAS.h"
#include <SPI.h>


/******************************************************************************/
/* Constantes --------------------------------------------------------------- */
#define ROBUS 'A'

#if (ROBUS == 'A')
#define ENCODEUR_GAUCHE_360 (long)8169
#define ENCODEUR_DROIT_360  (long)7667
#elif (ROBUS == 'B')
#define ENCODEUR_GAUCHE_360 (long)7700
#define ENCODEUR_DROIT_360  (long)7840
#endif


#define DIAMETRE_ROUE      (3 * 2.54)
#define DIAMETRE_TOUR      18.5

#define PERIODE  3

#define KPD      0.0000169
#define KID      0.0000269
//#define KDD      12

#define KPG      0.00002
#define KIG      0.00003
//#define KDG      12



/******************************************************************************/
/* Structures --------------------------------------------------------------- */
typedef struct    // Une structure est plusieurs données mises dans un paquet,
                  // qui contient toutes ces données.
                  // Un peu comme une classe sans fonctions.
{
  int angle;
  int longueur;
} Vecteur;


/******************************************************************************/
/* Parcours ----------------------------------------------------------------- */
// Ici, les vecteur sont de la forme (angle, longueur).
// On crée des nouveaux vecteurs, mais dans un tableau.
Vecteur tab[] = { {0, 80} };



/******************************************************************************/
/* Déclarations de fonctions ------------------------------------------------ */
void avancerTest(int longueurCM);
void Sequence_Parcours();

void Virage_Gauche(int angle);
void Virage_Droit(int angle);
void Virage(int angle);

void avancer(int longueurCMG, int longueurCMD, int valeurEncodeurG, int valeurEncodeurD);

void PIDG(int valeurEncodeur);
void PIDD(int valeurEncodeur);

float CMtoCoche(int ValeurCM);
int CorrectionLongueur(int longueurBase);

void mouvementLigne(int distanceCM);



/******************************************************************************/
/* Variables globales ------------------------------------------------------- */
int retroactionG = 0;
int retroactionPrecG = 0;
int retroactionD = 0;
int retroactionPrecD = 0;

int consigneG;
int consigneD;

float cmdG;
float cmdD;

float integralG;
float integralD;
float deriveeG;
float deriveeD;

bool rdyToStopG = false;
bool rdyToStopD = false;



/******************************************************************************/
/* Définitions de fonctions ------------------------------------------------- */
void avancerTest(int longueurCM)
{
    print("avancer de %d cm\n", longueurCM);
    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

    int valeurEncodeur = ENCODER_Read(0);
    // Si la valeur lue par l'encodeur >= à distance à parcourir
    // (en valeur des encodeurs).
    while(valeurEncodeur < (3200 / (PI * DIAMETRE_ROUE)) * longueurCM)
    {
      //int longueurRestante = longueurCM - (valeurEncodeur * PI * DIAMETRE_ROUE);
      valeurEncodeur = ENCODER_Read(0);
      MOTOR_SetSpeed(0, 0.5);
      MOTOR_SetSpeed(1, 0.5);
    }

    MOTOR_SetSpeed(0, 0);
    MOTOR_SetSpeed(1, 0);
}

void Virage_Droit(int angle)
{
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);

  long valeurEncodeurGauche = ENCODER_Read(0);
  
  while(valeurEncodeurGauche <= ENCODEUR_GAUCHE_360 / (360 / angle)) 
  {
      MOTOR_SetSpeed(0, 0.3);
      MOTOR_SetSpeed(1, -0.3);
      valeurEncodeurGauche = ENCODER_Read(0);
  }

  MOTOR_SetSpeed(0, 0);
  MOTOR_SetSpeed(1, 0);
}


void Virage_Gauche(int angle) 
{
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);

  long valeurEncodeurDroit = ENCODER_Read(1);
  
  while(valeurEncodeurDroit <= ENCODEUR_DROIT_360 / (360 / angle)) 
  {
      MOTOR_SetSpeed(0, -0.3);
      MOTOR_SetSpeed(1, 0.3);
      valeurEncodeurDroit = ENCODER_Read(1);
  }

  MOTOR_SetSpeed(0, 0);
  MOTOR_SetSpeed(1, 0);
}

void Virage(int angle)
{
  print("Virage de %d°\n", angle);
  if (angle < 0)
  {
    angle = angle * -1;
    Virage_Gauche(angle);
  }
  else
  {
    Virage_Droit(angle);
  }
}


void PIDG(int valeurEncodeur)
{
  retroactionPrecG = retroactionG;
  retroactionG = valeurEncodeur;
  float prop = (consigneG - retroactionG) * KPG;

  //deriveeG = (retroactionG - retroactionPrecG) * KDG;

  float erreur = cmdG - retroactionG;
  integralG += erreur * KIG;
  //integralG = constrain(integralG + KIG * integralG, -255, 255); //Verifier les vrais bornes mon ami!!!

  cmdG = prop + KIG * integralG;

  if((erreur < 5) && (erreur > -5))
  {
    rdyToStopG = true;
  }
}

void PIDD(int valeurEncodeur)
{
  retroactionPrecD = retroactionD;
  retroactionD = valeurEncodeur;
  float prop = (consigneD - retroactionD) * KPD;
  
  //deriveeD = (retroactionD - retroactionPrecD) * KDD;

  float erreur = cmdD - retroactionD;
  integralD += erreur * KID;
  // integralD = constrain(integralD + KID * integralD, -255,255);//Verifier les vrais bornes mon ami!!!
  
  cmdD = prop + KID * integralD;

  if((erreur < 5) && (erreur > -5))
  {
    rdyToStopD = true;
  }
}

void mouvementLigne(int distanceCM)
{
  unsigned long timer = millis();

  int compteEncodeurSimilaire = 0;
  int derniereValeurEncodeurG = 0;
  int derniereValeurEncodeurD = 0;

  consigneG = CMtoCoche(CorrectionLongueur(distanceCM) ) ;
  consigneD = CMtoCoche(CorrectionLongueur(distanceCM) ) ;
  
  cmdG = consigneG;
  cmdD = consigneD;

  print("Déplacement de %d cm (%d : %d)\n", distanceCM, consigneG, consigneD);

  ENCODER_Reset(0);
  ENCODER_Reset(1);

  while(rdyToStopG == false
     || rdyToStopD == false)
  {
    // Lecture des encodeurs
    int valeurEncodeurG = ENCODER_Read(0);
    int valeurEncodeurD = ENCODER_Read(1);
    print("Valeur encodeur : %d : %d\n", valeurEncodeurG, valeurEncodeurD);

    // Déplacement
    avancer(consigneG, consigneD, valeurEncodeurG, valeurEncodeurD);

    // Vérification de si on a fini
    if (millis() - timer >= PERIODE)
    {
      PIDG(valeurEncodeurG);
      PIDD(valeurEncodeurD);

      // Mise à jour du timer
      timer = millis();
    }


    // Patch pour ne pas rester pris avec des retroactions infiniment petites.
    if (valeurEncodeurD == derniereValeurEncodeurD
     && valeurEncodeurG == derniereValeurEncodeurG)
    {
      compteEncodeurSimilaire++;
      if (compteEncodeurSimilaire >= 10)
      {
        // On sort de la boucle si jamais on a des valeurs d'encodeurs pareils
        // plus de 10 fois de suite dans les deux roues.
        break;
      }
    }
    else
    {
      // Réinitialise le compteur si jamais les valeurs ne sont plus similaires
      compteEncodeurSimilaire = 0;
    }
    // Mise a jour de la derniere valeur lue de l'encodeur
    derniereValeurEncodeurD = valeurEncodeurD;
    derniereValeurEncodeurG = valeurEncodeurG;
  }
  
  // On a fini d'avancer
  rdyToStopG = false;
  rdyToStopD = false;

  // Arrêt des moteurs
  MOTOR_SetSpeed(1,0);
  MOTOR_SetSpeed(0,0);
}


void avancer(int longueurCocheG, int longueurCocheD, int valeurEncodeurG, int valeurEncodeurD)
{
    //si la valeur lue par l'encodeur >= à distance à parcourir en valeur des encodeurs
    MOTOR_SetSpeed(0, 0.5);
    MOTOR_SetSpeed(1, 0.5);

    if(valeurEncodeurG >= longueurCocheG)
    {
      rdyToStopG = true; 
    }
    if(valeurEncodeurD >= longueurCocheD)
    {
      rdyToStopD = true;
    }
}

float CMtoCoche(int ValeurCM)
{
  float ValeurCoche = (ValeurCM / (DIAMETRE_ROUE * PI)) * 3200;
  //print("valCoche: %f", ValeurCoche);
  return(ValeurCoche);
}

int CorrectionLongueur(int longueurBase)
{
  return(longueurBase*1.03);
}


/* 
 * @brief   Suit un parcours à l'endroit et à l'envers
 * 
 * Suit chaque vecteur du tableau, en commençant par effectuer une rotation sur
 * lui-même, puis en se déplaçant d'une certaine distance.
 * Une fois tous les vecteurs suivis, il repart à partir de l'avant-dernier 
 * vecteur et refait son parcours à l'envers, en commençant par faire la 
 * la distance du vecteur, puis en faisant l'inverse de son angle.
 * 
 * @note    Lit le parcours stocké dans le tableau de vecteur `tab`.
 */
void Sequence_Parcours()
{
  for (int i = 0; i < sizeof_array(tab); i++)
  {
      print("\nVecteur #%d\n", i);

      Vecteur a = tab[i];        // Fait une copie du vecteur actuel.
      Virage(a.angle);
      delay(500);
      mouvementLigne(a.longueur);
      delay(500);
  }


  return;     // À ENLEVER


  // Parcours à l'envers
  print("Parcours fini! À l'envers maintenant!\n");
  // (démarre à l'avant-dernier élément, donc taille totale - 2)
  // (pour démarrer au dernier élément, il aurait fallu faire taille totale - 1, 
  //  car les tableaux commencent à 0 en C).
  for (int i = sizeof_array(tab) - 2; i >= 0; i--)
  {
      Vecteur a = tab[i];
      mouvementLigne(a.longueur);
      delay(500);
      Virage((-1) * a.angle);    // Tourne de l'angle * -1, pour faire l'angle
                                 // inverse.
      delay(500);
  }
}


/******************************************************************************/
/* main --------------------------------------------------------------------- */
void setup()
{
  BoardInit();
  
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);
  
  //Print de la valeur des encodeurs au temps 0
  print("Encodeur 0: %ld\n", ENCODER_Read(0));
  print("Encodeur 1: %ld\n", ENCODER_Read(1));

  delay(1500);
  
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);

  // Fait la séquence à l'endroit, puis à l'envers.
  // Fonction principale du programme, la plus importante
  Sequence_Parcours();

  // On s'assure que le robot est bien arrêté
  MOTOR_SetSpeed(0, 0.0);
  MOTOR_SetSpeed(1, 0.0);
  print("\nFin du programme!\n");
}

void loop()
{
  // On n'a rien dans le loop
}
