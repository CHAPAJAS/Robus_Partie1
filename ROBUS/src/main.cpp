#include <Arduino.h>
#include "LibCHAPAJAS.h"
#include "LibCHAPAJAS.cpp"
//#include "TimerOne.h" //demander a PASC
#include <SPI.h>

#define PERIODE  3

#define KPD      17
#define KID      0.009
#define KDD      12

#define KPG      17
#define KIG      0.009
#define KDG      12

#define ENCODEUR 3200

#define AvancerTest 100
#define DIAMETRE_ROUE     7.62

#define PI              3.1415926535

void avancer(int longueurCMG,int longueurCMD);
void PIDG();
void PIDD();

//int analogPin;
int valeurEncodeurG = 0;
int valeurEncodeurD = 0;
int retroactionG = 0;
int retroactionPrecG = 0;
int retroactionD = 0;
int retroactionPrecD = 0;
int consigneG = ENCODEUR;
int consigneD = ENCODEUR;
unsigned long timer=0;
float erreurG;
float erreurD;
float distance;
float cmdG;
float cmdD;
float propG;
float propD;
float integralG;
float integralD;
float deriveeG;
float deriveeD;
bool avancementFini=false;


void setup()
{
  
  BoardInit();
  valeurEncodeurD = ENCODER_Read(1);
  valeurEncodeurG = ENCODER_Read(0);
  //Print de la valeur des encodeurs au temps 0
  print("Encodeur 0: %ld\n",ENCODER_Read(0));
  print("Encodeur 1: %ld\n",ENCODER_Read(1));
  //Activation du bumper avant
  pinMode(26, INPUT);
  cmdG =AvancerTest;
  cmdD =AvancerTest;
  //Roues commencent à avancer
  MOTOR_SetSpeed(0, 0.1);
  MOTOR_SetSpeed(1, 0.1);
  //Timer1.initialize(500);
}

void loop()
{

  valeurEncodeurG = ENCODER_Read(0);
  valeurEncodeurD = ENCODER_Read(1);
  while(avancementFini)
  {
    avancer(cmdG,cmdD);
    PIDG();
    PIDD();
  }
  avancementFini=false;
  
  


}

void avancer(int longueurCMG,int longueurCMD)
{
  
    int valeurEncodeurG = ENCODER_Read(0);
    print("Valeur encodeur : %ld\n", valeurEncodeurG);
    //si la valeur lue par l'encodeur >= à distance à parcourir en valeur des encodeurs
    if(valeurEncodeurG >= (ENCODEUR/(PI * DIAMETRE_ROUE))* longueurCMG)
    {
      MOTOR_SetSpeed(0, 0);
      
     
    }
    if(valeurEncodeurD >= (ENCODEUR/(PI * DIAMETRE_ROUE))* longueurCMD)
    {
      
      MOTOR_SetSpeed(1, 0);
     
    }
  
}
void PIDG()
{
  if (millis()-timer >=PERIODE)
  {
    retroactionPrecG = retroactionG;
    retroactionG = valeurEncodeurG;
    propG = (consigneG-retroactionG)*KPG;
  }
  erreurG=consigneG-retroactionG;

  deriveeG=(retroactionG-retroactionPrecG)*KDG;
  integralG +=erreurG *KIG;
  integralG=constrain(integralG+KIG*integralG,-255,255);//Verifier les vrais bornes mon ami!!!
  cmdG=propG+KIG*integralG-deriveeG;
  if((erreurG<0.5)&&(erreurG>-0.5))
  {
    avancementFini =true;
  }

}
void PIDD()
{
  if (millis()-timer >=PERIODE)
  {
    retroactionPrecD = retroactionD;
    retroactionD = valeurEncodeurD;
    propD = (consigneD-retroactionD)*KPD;
  }
  erreurD=consigneD-retroactionD;

  deriveeD=(retroactionD-retroactionPrecD)*KDD;
  integralD +=erreurD *KID;
  integralD=constrain(integralD+KID*integralD,-255,255);//Verifier les vrais bornes mon ami!!!
  cmdD=propD+KID*integralD-deriveeD;
  if((erreurD<0.5)&&(erreurD>-0.5))
  {
    avancementFini =true;
  }

}

