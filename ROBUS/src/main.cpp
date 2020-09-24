#include <Arduino.h>
#include "LibCHAPAJAS.h"
#include "LibCHAPAJAS.cpp"

#define ENCODEUR_GAUCHE 3145
#define ENCODEUR_DROIT  2692

#define DIAMETRE_ROUE     7.62

#define PI              3.1415926535
void avancer(int longueurCM);


void setup() {
  BoardInit();
  print("Encodeur 0: %ld\n",ENCODER_Read(0));
  print("Encodeur 1: %ld\n",ENCODER_Read(1));

  pinMode(26, INPUT);

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

    if(valeurEncodeur >= (ENCODEUR_GAUCHE/(PI * DIAMETRE_ROUE))* longueurCM)
    {
      MOTOR_SetSpeed(0, 0);
      MOTOR_SetSpeed(1, 0);
      while(true)
      {}
    }
  }
}