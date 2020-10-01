#include <Arduino.h>
#include "LibCHAPAJAS.h"
#include "LibCHAPAJAS.cpp"

#define ENCODEUR_GAUCHE 3145
#define ENCODEUR_DROIT  2692

#define DIAMETRE_ROUE     7.62

#define PI              3.1415926535





typedef struct    // Une structure est plusieurs donn/es mis dans un paquet qui contient toutes ces donnees
                  // Un peu comme une classe sans fonction
{

  float angle;
  float longueur;

} Vecteur;



/*
Vecteur v1, v2, v3, v4, v5 ;    //creer 5 nouveaux vecteurs 
*/



// Ici les vecteur sont de la forme (angle, longueur)
Vecteur tab[5] = { {0,5}, {45,12}, {180,0}, {0,12}, {-45,5} };    //creer 5 nouveaux vecteurs mais dans un tableau

















void avancer(int longueurCM);


void setup() {
  BoardInit();
  //Print de la valeur des encodeurs au temps 0
  print("Encodeur 0: %ld\n",ENCODER_Read(0));
  print("Encodeur 1: %ld\n",ENCODER_Read(1));
  //Activation du bumper avant
  pinMode(26, INPUT);
  //Roues commencent à avancer
  MOTOR_SetSpeed(0, 0.1);
  MOTOR_SetSpeed(1, 0.1);
}

void loop() {

  avancer(17);

  // if (digitalRead(26) == HIGH)
  // {
  //   MOTOR_SetSpeed(0, 0);
  //   MOTOR_SetSpeed(1, 0);

  //   print("Encodeur 0: %ld\n",ENCODER_Read(0));
  //   print("Encodeur 1: %ld\n",ENCODER_Read(1));
  //   while(true)
  //   {}
  // }  
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