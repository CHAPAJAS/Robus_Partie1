#include <Arduino.h>
#include "LibCHAPAJAS.h"


#define ENCODEUR_GAUCHE 3145
#define ENCODEUR_DROIT  2692

#define DIAMETRE_ROUE     7.62



typedef struct    // Une structure est plusieurs donnees mis dans un paquet qui contient toutes ces donnees
                  // Un peu comme une classe sans fonction
{

  double angle;
  double longueur;

} Vecteur;



// Ici les vecteur sont de la forme (angle, longueur)
Vecteur tab[] = { {0,50}, {45,120}, {180,0} };    //creer 5 nouveaux vecteurs mais dans un tableau





void avancer(int longueurCM);



void tournerTest(double angle);
void avancerTest(double longueurCm);

void sequenceParcours()
{
  for (int i = 0; i < sizeof_array(tab); i++)
  {
      Vecteur a = tab[i];
      tournerTest(a.angle);
      avancerTest(a.longueur);
  }

  for (int i = sizeof_array(tab) - 2; i >= 0; i--)
  {
      Vecteur a = tab[i];
      avancerTest(a.longueur);
      tournerTest((-1) * a.angle);
  }
}


void tournerTest(double angle)
{
  print("Virage à %d\n", (int)angle);
}

void avancerTest(double longueurCm)
{
  print("Déplacement de %d cm\n", (int)longueurCm);
}


void setup() {
  BoardInit();
  sequenceParcours();
}


void loop() {

}

void avancer(int longueurCM)
{
  while(true)
  {

    int valeurEncodeur = ENCODER_Read(0);
    print("Valeur encodeur : %ld\n", valeurEncodeur);
    //si la valeur lue par l'encodeur >= à distance à parcourir en valeur des encodeurs
    if(valeurEncodeur >= (ENCODEUR_GAUCHE/(PI * DIAMETRE_ROUE))* longueurCM)
    {
      MOTOR_SetSpeed(0, 0);
      MOTOR_SetSpeed(1, 0);
      while(true)
      {}
    }
  }
}