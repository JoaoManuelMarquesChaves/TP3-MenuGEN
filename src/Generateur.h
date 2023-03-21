#ifndef Generateur_h
#define Generateur_h

// TP3 MenuGen 2016
// C. HUBER  03.02.2016
// Fichier Generateur.h
// Prototypes des fonctions du générateur  de signal

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "DefMenuGen.h"



#define MAX_ECH 100
#define f_sys  80000000 
#define presceler 8

#define MAX 65535 //2^16

#define VAL_TIC_MAX 32767

typedef struct {
            uint16_t Nb_Tic;
            uint16_t Min;             // Amplitude min
            uint16_t Max;             // Amplitude max  
            
} S_Amplitude;







// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam);


// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam);


// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam);

// A appeler dans int Timer3
void  GENSIG_Execute(void);


#endif