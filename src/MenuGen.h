#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du générateur
// Traitement cyclique à 1 ms du Pec12


#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

typedef enum {MenuPrincipal, MenuForme, MenuFreq, MenuAmpl, MenuOffset} E_Menu;

void MENU_Initialize(S_ParamGen *pParam);


void MENU_Execute(S_ParamGen *pParam);

void GestMenu(int16_t ValueMax,int16_t ValueMin,uint8_t Step,int16_t* PtrValue,int16_t* PtSave);
void diplay(int16_t ValFreq,int16_t ValAmp,int16_t ValOffs,int16_t Choix);
void GestSave(int16_t* ValSave,int16_t* Activity);

#endif




  
   







