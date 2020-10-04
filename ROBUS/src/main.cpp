#include <Arduino.h>
#include "LibCHAPAJAS.h"



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



typedef struct    // Une structure est plusieurs donnees mis dans un paquet qui contient toutes ces donnees
                  // Un peu comme une classe sans fonction
{

  double angle;
  double longueur;

} Vecteur;



// Ici les vecteur sont de la forme (angle, longueur)
Vecteur tab[] = { {0,50}, {45,120}, {180,0} };    //creer 5 nouveaux vecteurs mais dans un tableau





void avancer(int longueurCM);


void Virage_Gauche(int angle);
void Virage_Droit(int angle);
void Virage(int angle);


void avancer(int longueurCM)
{
    print("avancer de %d cm\n", longueurCM);
    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

    int valeurEncodeur = ENCODER_Read(0);
    //si la valeur lue par l'encodeur >= à distance à parcourir en valeur des encodeurs
    while(valeurEncodeur < (3200/(PI * DIAMETRE_ROUE)) * longueurCM)
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

void sequenceParcours()
{
  for (int i = 0; i < sizeof_array(tab); i++)
  {
      print("Vecteur #%d\n", i);
      Vecteur a = tab[i];
      Virage(a.angle);
      avancer(a.longueur);
  }

  for (int i = sizeof_array(tab) - 2; i >= 0; i--)
  {
      Vecteur a = tab[i];
      avancer(a.longueur);
      Virage((-1) * a.angle);
  }
}

void setup()
{
  BoardInit();
  delay(1500);
}

void loop()
{
  
  sequenceParcours();

  {
    while(true){}
  }

}
