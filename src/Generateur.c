// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber

#include "Mc32NVMUtil.h"
#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "driver/tmr/drv_tmr_static.h"
#include <stdint.h>
#include <math.h>

// T.P. 2016 100 echantillons


uint16_t tb_Signal[MAX_ECH]={0};


// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    //Lecture du bloc sauvegradé et met à jour la structure
     
    // Relecture des paramètres
    NVM_ReadBlock((uint32_t*)pParam,14);
    
    //Test si match de la valeur Magic
    if (pParam->Magic == 0x123455AA)
    {
        //Garde automatiquement les valeurs precedentes sauvegardées
        lcd_gotoxy(1,4);
        printf_lcd("Datas Restored");
    }
    
    else
    {
        lcd_gotoxy(1,4);
        printf_lcd("Datas Default");
        //Set les valeurs aux valeurs par defaut
        pParam->Magic = 0x123455AA;
        pParam->Amplitude = AMPLITUDE_INIT;
        pParam->Forme = SignalDentDeScie;
        pParam->Frequence = FREQ_INIT;
        pParam->Offset = OFFSET_INIT;
    }
}
  

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    //initaliser la variable
    uint16_t  Periode = 0;
    //---Calculer la période en fonction de la fréquence entrée comme paramètre----/   
    //Periode = (float)f_sys/(float)(pParam -> Frequence * MAX_ECH * presceler) - 1;
    Periode = (f_sys/(pParam -> Frequence * MAX_ECH * presceler))-1;
    //modifier la periode du timer 3
    PLIB_TMR_Period16BitSet(TMR_ID_3, Periode);
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
 
    //initialisation de l'incrément
    int16_t i;
    //initialiser la variable Sted
    
    int16_t Amplitude = (pParam -> Amplitude * Max1) / 10000;
    int16_t Offset = (pParam -> Offset * Max1) / 10000;
    float pointValue = Max1 - Amplitude;
    int32_t CalculOffset = 0;
            
 for(i=0; i<=MAX_ECH; i++)
    {   
    switch(pParam->Forme)
    {
        case SignalSinus:        
                tb_Signal[i] = (Amplitude * sin(2 * M_PI * i/MAX_ECH) + 0.5) + Max1;
            
        break;
        case SignalTriangle:
                tb_Signal[i] = pointValue + 0.5;
                if(i < (MAX_ECH / 2))
                {
                    pointValue = pointValue + (Amplitude * 2 + 1) / (MAX_ECH/2.0);
                }
                else
                {
                    pointValue = pointValue - (Amplitude * 2 + 1) / (MAX_ECH/2.0);
                }
        break;
        case SignalDentDeScie:
                tb_Signal[i] = pointValue + 0.5;
                pointValue = pointValue + (Amplitude * 2 + 1) / (MAX_ECH-1) ;   
            //tb_Signal[i] = (float)((float)Amplitude/(MAX_ECH-1))*i; 
        break;
        case SignalCarre:
            if(i < (MAX_ECH / 2))
                {
                    // Assigner une valeur d'amplitude maximale pour la première moitié
                    tb_Signal[i] = Amplitude + MAX;
                }
                else
                {
                    // Assigner une valeur d'amplitude négative pour la seconde moitié
                    tb_Signal[i] = -Amplitude + MAX;
                }
            break;
    }
 }
    for(i=0; i<MAX_ECH; i++)
    {
        CalculOffset = (int32_t)tb_Signal[i] - (int32_t)Offset;
        if(CalculOffset > MAX)
        {
            CalculOffset = MAX;
        }
        else if(CalculOffset < 0)
        {
            CalculOffset = 0;
        }
        tb_Signal[i] = (uint16_t)CalculOffset;
    }

}


// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)

// Version provisoire pour test du DAC à modifier
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
   
   SPI_WriteToDac(0,tb_Signal[EchNb] );      // sur canal 0
   EchNb++;
   EchNb = EchNb % MAX_ECH;
   
}
