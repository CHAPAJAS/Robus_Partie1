#include <Arduino.h>
#include "LibCHAPAJAS.h"

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

void Virage_Gauche(int angle);
void Virage_Droit(int angle);
void Virage(int angle);

void setup()
{
  BoardInit();
  delay(1500);
}

void loop()
{
  
  Virage(-360);

  {
    while(true){}
  }

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