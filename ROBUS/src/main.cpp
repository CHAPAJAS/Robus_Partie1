/******************************************************************************/
/* Inclusions --------------------------------------------------------------- */
#include <Arduino.h>
#include "LibCHAPAJAS.h"



/******************************************************************************/
/* Constantes --------------------------------------------------------------- */
#define ROBUS 'B'

#if (ROBUS == 'A')
#define ENCODEUR_GAUCHE_360 (long)8169
#define ENCODEUR_DROIT_360  (long)7667
#elif (ROBUS == 'B')
#define ENCODEUR_GAUCHE_360 (long)7700
#define ENCODEUR_DROIT_360  (long)7840
#endif


#define DIAMETRE_ROUE      (3 * 2.54)
#define DIAMETRE_TOUR      18.5



/******************************************************************************/
/* Structures --------------------------------------------------------------- */
typedef struct    // Une structure est plusieurs données mises dans un paquet,
                  // qui contient toutes ces données.
                  // Un peu comme une classe sans fonctions.
{
  int angle;
  double longueur;
} Vecteur;



/******************************************************************************/
/* Parcours ----------------------------------------------------------------- */
// Ici, les vecteur sont de la forme (angle, longueur).
// On crée des nouveaux vecteurs, mais dans un tableau.
Vecteur tab[] = { {0,50}, {45,120}, {180,0} };



/******************************************************************************/
/* Déclarations de fonctions ------------------------------------------------ */
void avancerTest(int longueurCM);
void Sequence_Parcours();

void Virage_Gauche(int angle);
void Virage_Droit(int angle);
void Virage(int angle);



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
      int longueurRestante = longueurCM - (valeurEncodeur * PI * DIAMETRE_ROUE);
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
      avancerTest(a.longueur);
  }

  // Parcours à l'envers
  print("Parcours fini! À l'envers maintenant!\n")
  // (démarre à l'avant-dernier élément, donc taille totale - 2)
  // (pour démarrer au dernier élément, il aurait fallu faire taille totale - 1, 
  //  car les tableaux commencent à 0 en C).
  for (int i = sizeof_array(tab) - 2; i >= 0; i--)
  {
      Vecteur a = tab[i];
      avancerTest(a.longueur);
      Virage((-1) * a.angle);    // Tourne de l'angle * -1, pour faire l'angle
                                 // inverse.
  }
}



/******************************************************************************/
/* main --------------------------------------------------------------------- */
void setup()
{
  BoardInit();
  delay(1500);
}

void loop()
{
  Sequence_Parcours();

  // Fin du programme
  while(true){}
}
