#include <Arduino.h>
#include "LibCHAPAJAS.h"

#define ENCODEUR_GAUCHE_360 (long)8169
#define ENCODEUR_DROIT_360  (long)7667
#define DIAMETRE_ROUE      (3 * 2.54)
#define DIAMETRE_TOUR      18.5


//void avancer(int longueurCM);
void Virage_Gauche();
void Virage_Droit();
//void Virage_Gauche(float valeurDesireeCM);
//void VirageGrandRayonGauche(int distanceCMG, float rapportD);
//float CMtoCoche(float ValeurCM);




void setup()
{
  BoardInit();
  delay(1500);

}

//int conteur = 0;


void loop()
{
  Virage_Droit();
  delay(2000);
  Virage_Gauche();
  //conteur = conteur + 1;

  //if (conteur >= 1)
  {
    while(true){}
  }

}



void Virage_Droit() 
{

  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);

  long valeurEncodeurGauche = ENCODER_Read(0);
  
  while(valeurEncodeurGauche <= ENCODEUR_GAUCHE_360) 
  {
      MOTOR_SetSpeed(0, 0.3);
      MOTOR_SetSpeed(1, -0.3);
      valeurEncodeurGauche = ENCODER_Read(0);
  }

  MOTOR_SetSpeed(0, 0);
  MOTOR_SetSpeed(1, 0);
}

void Virage_Gauche() 
{

  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);

  long valeurEncodeurDroit = ENCODER_Read(1);
  
  while(valeurEncodeurDroit <= ENCODEUR_DROIT_360) 
  {
      MOTOR_SetSpeed(0, -0.3);
      MOTOR_SetSpeed(1, 0.3);
      valeurEncodeurDroit = ENCODER_Read(1);
  }

  MOTOR_SetSpeed(0, 0);
  MOTOR_SetSpeed(1, 0);
}

