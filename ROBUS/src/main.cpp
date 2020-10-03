#include <Arduino.h>
#include "LibCHAPAJAS.h"
#include "LibCHAPAJAS.cpp"

#define ENCODEUR_GAUCHE 3145
#define ENCODEUR_DROIT  2692

#define DIAMETRE_ROUE     7.62
#define DIAMETRE_TOUR      7.8

#define PI              3.1415926535
//void avancer(int longueurCM);

void Virage_Droit();

void setup() {
  BoardInit();
  
}

int conteur = 0;

void loop() {

  Virage_Droit();
  conteur = conteur + 1;

  if (conteur >= 3)
  {
    while(true){}
  }

  //avancer(17);

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

/*void avancer(int longueurCM)
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
}*/

void Virage_Droit()
{



  /*MOTOR_SetSpeed(0, 0.3);
  MOTOR_SetSpeed(1, -0.3);
  delay(3000);
  MOTOR_SetSpeed(0, 0);
  MOTOR_SetSpeed(1, 0);
  delay(3000);
*/
  int valeurEncodeurGauche = ENCODER_Read(0);
  int valeurEncodeurDroit = ENCODER_Read(1);
  float DistanceParcouru = (ENCODEUR_GAUCHE/(PI * DIAMETRE_ROUE))*((DIAMETRE_TOUR * PI)/2);

  if (DistanceParcouru >= valeurEncodeurGauche)
  {
    MOTOR_SetSpeed(0, 0.3);
    MOTOR_SetSpeed(1, -0.3);
    Serial.println("allo");
    Serial.println(valeurEncodeurGauche);
    Serial.println(valeurEncodeurDroit);
    Serial.println(DistanceParcouru);
  }
  else
  { 
    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);
    MOTOR_SetSpeed(0, 0);
    MOTOR_SetSpeed(1, 0);
    while(true)
    {}
  }
  
  
  

  
  
 

}