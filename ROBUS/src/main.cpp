/******************************************************************************/
/* Inclusions --------------------------------------------------------------- */
#include "LibCHAPAJAS.h"
#include <Arduino.h>
#include <SPI.h>



/******************************************************************************/
/* Constantes --------------------------------------------------------------- */
#define ROBUS 'A'

#if(ROBUS == 'A')
#define ENCODEUR_GAUCHE_360 (long)8169
#define ENCODEUR_DROIT_360  (long)7667

#define ANGULOD 0.98
#define ANGULOG 0.955

#define SPD  1.0625 //1.089
#define SPECIAL_SPD   1.075
#define LONGULO 1.05

#elif(ROBUS == 'B')
#define ENCODEUR_GAUCHE_360 (long)7700
#define ENCODEUR_DROIT_360  (long)7840

// Diminuer ces valeurs pour réduire l'angle parcouru
#define ANGULOD             0.961        // Cette valeur multiplie l'angle droite à parcourir.
#define ANGULOG             0.955        // Cette valeur multiplie l'angle gauche à parcourir.

// Diminuer cette valeur pour aller plus à droite.
#define SPD                 0.972       // Cette valeur multiplie le moteur de droite.
#define SPECIAL_SPD         0.993

// Augmenter cette valeur pour aller plus loin.
#define LONGULO 1.03                     // Cette valeur multiplie la distance à parcourir.
#else
    #error(Constante `ROBUS` doit être définie à `'A'` ou `'B'`)
#endif


#define DIAMETRE_ROUE (3 * 2.54)
#define DIAMETRE_TOUR 18.5

#define PERIODE      3
#define DELAY_VIRAGE 1000

#define FAIRE_RETOUR true

#define DEPLACEMENTS true
#define VIRAGES      true
#define HALF_TURN    false


int i_parcours = 0;



/******************************************************************************/
/* Structures --------------------------------------------------------------- */
struct Vecteur        // Une structure est plusieurs données mises dans un paquet,
                      // qui contient toutes ces données.
                      // Un peu comme une classe sans fonctions.
{
    int angle;
    int longueur;
    int delay;
};


/******************************************************************************/
/* Parcours ----------------------------------------------------------------- */
// Ici, les vecteur sont de la forme (angle, longueur).
// On crée des nouveaux vecteurs, mais dans un tableau.
#if ROBUS == 'A'
static Vecteur tab[] = 
                        {{0, 227, DELAY_VIRAGE},        // A
                         {-88, 100, DELAY_VIRAGE},      // B
                         {85, 45, DELAY_VIRAGE},        // C
                         {88, 50, DELAY_VIRAGE},        // D
                         {-88, 104, DELAY_VIRAGE},      // E
                         {88, 44, DELAY_VIRAGE},        // F
                         {-88, 125, DELAY_VIRAGE},      // G
                         {176, 0, DELAY_VIRAGE}};       // H

#elif ROBUS == 'B'
static Vecteur tab[] = 
                        {{0, 227, DELAY_VIRAGE},        // A
                         {-90, 100, DELAY_VIRAGE},      // B
                         {88, 47, DELAY_VIRAGE},        // C
                         {88, 55, DELAY_VIRAGE},        // D
                         {-92, 104, DELAY_VIRAGE},      // E
                         {90, 44, DELAY_VIRAGE},        // F
                         {-90, 125, DELAY_VIRAGE},      // G
                         {180, 0, DELAY_VIRAGE}};       // H
#endif
//{{-91, 0, DELAY_VIRAGE / 3}};
//   {{0, 250}};


/******************************************************************************/
/* Déclarations de fonctions ------------------------------------------------ */
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

    while(valeurEncodeurGauche <= ENCODEUR_GAUCHE_360 / (360 / (angle * ANGULOD)))
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

    while(valeurEncodeurDroit <= ENCODEUR_DROIT_360 / (360 / (angle * ANGULOG)))
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
#if VIRAGES == false
    return;
#endif

     for(; abs(angle) > 100; angle = (angle >= 0) ? angle - 90 : angle + 90)
     {
      Virage((angle % 90 == 0) ? ((angle >= 0) ? 90 : -90) : angle % 90);
        delay(DELAY_VIRAGE);
    }
  
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
#if DEPLACEMENTS == false
    return;
#endif

    if(distanceCM == 0)
    {
        return;
    }

    // Le timer est utilisé pour faire les calculs du PID uniquement à une certaine période fixe.
    uint32_t timer      = millis();
    bool     rdyToStopG = false;
    bool     rdyToStopD = false;

    // La consigne est la distance à faire en nombre de coches d'encodeur
    // La commande est la distance qu'il reste à faire, avec rétroaction du PID
    int32_t consigne = CMtoCoche(CorrectionLongueur(distanceCM));
    float   cmdG     = consigne;
    float   cmdD     = consigne;


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
    float spd = (i_parcours == 0) ? SPECIAL_SPD : SPD;

    // Palier 5%
    if(encodeur < (consigne * 0.05) || encodeur > (consigne * 0.95))
    {
        MOTOR_SetSpeed(0, 0.3);
        MOTOR_SetSpeed(1, 0.3 * spd);
    }
    // Palier 20%
    else if(encodeur < (consigne * 0.2) || encodeur > (consigne * 0.8))
    {
        MOTOR_SetSpeed(0, 0.5);
        MOTOR_SetSpeed(1, 0.5 * spd);
    }
    // Palier 30%
    else if(encodeur < (consigne * 0.3) || encodeur > (consigne * 0.7))
    {
        MOTOR_SetSpeed(0, 0.6);
        MOTOR_SetSpeed(1, 0.6 * spd);
    }
    // Palier continu
    else
    {
        MOTOR_SetSpeed(0, 0.7);
        MOTOR_SetSpeed(1, 0.7 * spd);
    }
}

float CMtoCoche(int32_t valeurCM)
{
    float valeurCoche = (valeurCM / (DIAMETRE_ROUE * PI)) * 3200;
    return valeurCoche;
}

// Avant, elle multipliait la distance par 1.03
int32_t CorrectionLongueur(int32_t longueurBase)
{
    return (longueurBase / LONGULO);
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
    for(i_parcours = 0; i_parcours < sizeof_array(tab); i_parcours++)
    {
        print("\nVecteur #%d\n", i_parcours);

        Vecteur a = tab[i_parcours];        // Fait une copie du vecteur actuel.

        delay(a.delay / 2);
        Virage(a.angle);
        delay(a.delay / 2);
        mouvementLigne(a.longueur);
    }


    // Parcours à l'envers
#if FAIRE_RETOUR == true
    // (démarre à l'avant-dernier élément, donc taille totale - 2)
    // (pour démarrer au dernier élément, il aurait fallu faire taille totale - 1,
    //  car les tableaux commencent à 0 en C).
    for(int i = sizeof_array(tab) - 2; i >= 0; i--)
    {
        Vecteur a = tab[i];

        delay(a.delay / 2);
        mouvementLigne(a.longueur);
        delay(a.delay / 2);
        Virage((-1) * a.angle);        // Tourne de l'angle * -1, pour faire l'angle
                                       // inverse.
    }


    // Retour sur lui-même
#if HALF_TURN == true
    delay(DELAY_VIRAGE);
    Virage(180);
#endif

#endif
}


/******************************************************************************/
/* main --------------------------------------------------------------------- */
void setup()
{
    BoardInit();

    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

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
