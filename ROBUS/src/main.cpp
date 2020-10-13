/******************************************************************************/
/* Inclusions --------------------------------------------------------------- */
#include "LibCHAPAJAS.h"
#include <Arduino.h>
#include <SPI.h>



/******************************************************************************/
/* Constantes --------------------------------------------------------------- */
#define ROBUS 'B'

#if(ROBUS == 'A')
#define ENCODEUR_GAUCHE_360 (long)8169
#define ENCODEUR_DROIT_360  (long)7667

#define SPD                 1.076
#define ANGULO              0.96

#elif(ROBUS == 'B')
#define ENCODEUR_GAUCHE_360 (long)7700
#define ENCODEUR_DROIT_360  (long)7840

#define SPD                 0.967
#define ANGULO              0.975
#endif


#define DIAMETRE_ROUE (3 * 2.54)
#define DIAMETRE_TOUR 18.5

#define PERIODE 3

#define KPD 0.0000169
#define KID 0.0000269
//#define KDD      12

#define KPG 0.00001
#define KIG 0.00003
//#define KDG      12



/******************************************************************************/
/* Structures --------------------------------------------------------------- */
typedef struct        // Une structure est plusieurs données mises dans un paquet,
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
Vecteur tab[] = {{0, 123}, {-90, 88}, {90, 86}, {45, 182}, {-90, 61}, {45, 110}, {180, 0}};


/******************************************************************************/
/* Déclarations de fonctions ------------------------------------------------ */
// void avancerTest(int longueurCM);
void Sequence_Parcours();

void Virage_Gauche(int angle);
void Virage_Droit(int angle);
void Virage(int angle);

void avancer(int32_t encodeur, int32_t consigne);

float   CMtoCoche(int32_t valeurCM);
int32_t CorrectionLongueur(int32_t longueurBase);

void mouvementLigne(int distanceCM);



/******************************************************************************/
/* Définitions de fonctions ------------------------------------------------- */

void Virage_Droit(int angle)
{
    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

    long valeurEncodeurGauche = ENCODER_Read(0);

    while(valeurEncodeurGauche <= ENCODEUR_GAUCHE_360 / (360 / (angle * ANGULO)))
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

    while(valeurEncodeurDroit <= ENCODEUR_DROIT_360 / (360 / (angle * ANGULO)))
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
    if(angle < 0)
    {
        angle = angle * -1;
        Virage_Gauche(angle);
    }
    else
    {
        Virage_Droit(angle);
    }
}


void mouvementLigne(int distanceCM)
{
    if(distanceCM == 0)
    {
        return;
    }

    // Le timer est utilisé pour faire les calculs du PID uniquement à une certaine période fixe.
    unsigned long timer = millis();
    bool rdyToStopG = false;
    bool rdyToStopD = false;

    // La consigne est la distance à faire en nombre de coches d'encodeur
    // La commande est la distance qu'il reste à faire, avec rétroaction du PID
    int32_t consigne = CMtoCoche(CorrectionLongueur(distanceCM));
    float cmdG = consigne;
    float cmdD = consigne;


    print("Déplacement de %d cm (%ld)\n", distanceCM, consigne);

    ENCODER_Reset(0);
    ENCODER_Reset(1);

    while(rdyToStopG == false || rdyToStopD == false) /*??????*/
    {
        // Lecture des encodeurs
        int32_t valeurEncodeurG = ENCODER_Read(0);
        int32_t valeurEncodeurD = ENCODER_Read(1);
        print("Valeur encodeur : %ld : %ld\n", valeurEncodeurG, valeurEncodeurD);
        print("Commandes : %ld : %ld\n", cmdG, cmdD);

        if(millis() - timer >= PERIODE)
        {
            // Déplacement
            avancer(valeurEncodeurG, consigne);

            // Mise à jour du timer
            timer = millis();
        }

        // Vérification de si on a fini
        if(valeurEncodeurG >= consigne)
        {
            rdyToStopG = true;
        }
        if(valeurEncodeurD >= consigne)
        {
            rdyToStopD = true;
        }
    }

    // On a fini d'avancer
    rdyToStopG = false;
    rdyToStopD = false;

    // Arrêt des moteurs
    MOTOR_SetSpeed(1, 0);
    MOTOR_SetSpeed(0, 0);
}


void avancer(int32_t encodeur, int32_t consigne)
{
    if(encodeur < (consigne * 0.05) || encodeur > (consigne * 0.95))
    {
        MOTOR_SetSpeed(0, 0.3);
        MOTOR_SetSpeed(1, 0.3 * SPD);
    }
    else if(encodeur < (consigne * 0.2) || encodeur > (consigne * 0.8))
    {
        MOTOR_SetSpeed(0, 0.5);
        MOTOR_SetSpeed(1, 0.5 * SPD);
    }
    else if(encodeur < (consigne * 0.3) || encodeur > (consigne * 0.7))
    {
        MOTOR_SetSpeed(0, 0.6);
        MOTOR_SetSpeed(1, 0.6 * SPD);
    }
    else
    {
        MOTOR_SetSpeed(0, 0.7);
        MOTOR_SetSpeed(1, 0.7 * SPD);
    }
}

float CMtoCoche(int32_t valeurCM)
{
    float valeurCoche = (valeurCM / (DIAMETRE_ROUE * PI)) * 3200;
    return valeurCoche;
}

// Cette fonction ne fait plus rien
// Avant, elle multipliait la distance par 1.03
int32_t CorrectionLongueur(int32_t longueurBase)
{
    return (longueurBase * 1);
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
    for(int i = 0; i < sizeof_array(tab); i++)
    {
        print("\nVecteur #%d\n", i);

        Vecteur a = tab[i];        // Fait une copie du vecteur actuel.
        Virage(a.angle);
        mouvementLigne(a.longueur);
    }

    // return;//À commenter pour retour
    // Parcours à l'envers
    print("Parcours fini! À l'envers maintenant!\n");
    // (démarre à l'avant-dernier élément, donc taille totale - 2)
    // (pour démarrer au dernier élément, il aurait fallu faire taille totale - 1,
    //  car les tableaux commencent à 0 en C).
    for(int i = sizeof_array(tab) - 2; i >= 0; i--)
    {
        Vecteur a = tab[i];
        mouvementLigne(a.longueur);
        Virage((-1) * a.angle);        // Tourne de l'angle * -1, pour faire l'angle
                                       // inverse.
    }
}


/******************************************************************************/
/* main --------------------------------------------------------------------- */
void setup()
{
    BoardInit();

    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

    // Print de la valeur des encodeurs au temps 0
    print("Encodeur 0: %ld\n", ENCODER_Read(0));
    print("Encodeur 1: %ld\n", ENCODER_Read(1));

    delay(500);

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
