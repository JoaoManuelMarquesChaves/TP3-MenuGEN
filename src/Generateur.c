// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber

#include "Mc32NVMUtil.h"
#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32DriverLcd.h"
#include "Mc32gestSpiDac.h"
#include "driver/tmr/drv_tmr_static.h"
#include <stdint.h>
#include <math.h>

// T.P. 2016 100 echantillons

uint32_t tb_Signal[MAX_ECH]={0};

S_Amplitude Ampli;
// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{           
    //Recuperation des datas sauvegardées au demarrage precedent
    NVM_ReadBlock((uint32_t*) pParam , sizeof(S_ParamGen));
    
    //Test si match de la valeur Magic
    if (pParam->Magic == MAGIC)
    {
        //Garde automatiquement les valeurs precedentes sauvegardées
        lcd_gotoxy(1,4);
        printf_lcd("   Datas  Restored");
    }
    else
    {
        lcd_gotoxy(1,4);
        printf_lcd("   Datas  Default");
        //Set les valeurs aux valeurs par defaut
        pParam->Magic = MAGIC;
        pParam->Amplitude = 10000;
        pParam->Forme = 3;
        pParam->Frequence = 100;
        pParam->Offset = 0;
    }
}//End of GENSIG_Initialize 

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    //initaliser la variable
    uint16_t  Periode = 0;
    //---Calculer la période en fonction de la fréquence entrée comme paramètre----/   
    //Periode = (float)f_sys/(float)(pParam -> Frequence * MAX_ECH * presceler) - 1;
    Periode = ((float)f_sys/((float)pParam -> Frequence * (float)MAX_ECH * (float)presceler))-1;
    //modifier la periode du timer 3
    PLIB_TMR_Period16BitSet(TMR_ID_3, Periode);
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    //intialisation de la variable statique offset
    int16_t Offset;
    uint16_t  Step;
    int i; 
    //intitialiser la constatante de la pente  
    Ampli.Nb_Tic = (float)pParam -> Amplitude *(float)3.2767;
    Ampli.Min = (VAL_TIC_MAX)-(Ampli.Nb_Tic);
    Ampli.Max =((VAL_TIC_MAX)+(Ampli.Nb_Tic))-1;
    
    //gestion de l'offest
    Offset = -((float)pParam -> Offset * (float)3.2767);          
    for(i=0; i<MAX_ECH; i++)
    {   
        switch(pParam->Forme)
        {
            case SignalSinus:        
                    tb_Signal[i] = ( (float)Ampli.Nb_Tic * sin(2 * M_PI * i/MAX_ECH) + 0.5) + VAL_TIC_MAX;            
                    break;
            case SignalTriangle:
                    Step = ((float)Ampli.Nb_Tic*2)/50;
                    if(i <= 50)
                    {
                        tb_Signal[i] = ((float)Step * (50-i))+ (float)Ampli.Min + (float)Offset;
                    }
                    else
                    {
                        tb_Signal[i] = ((float)Step * (i-50))+ (float)Ampli.Min  + (float)Offset;
                    }
            break;
            case SignalDentDeScie:
                    Step = ((float)Ampli.Nb_Tic*2) / MAX_ECH;
                    tb_Signal[i] = ((float)Step * (MAX_ECH-i))+ (float)Ampli.Min + (float)Offset;
            break;
            case SignalCarre:
                    if(i <= 50 )
                    {
                        tb_Signal[i] = (float)Ampli.Min + (float)Offset ;
                    }
                    else
                    {
                        tb_Signal[i] = (float)Ampli.Max + (float)Offset;
                    }   
                break;
        }
        //Control saturation
        if(pParam->Offset < 0 && tb_Signal[i] >= MAX)tb_Signal[i] =MAX;
        if(pParam->Offset > 0 && tb_Signal[i] >= MAX)tb_Signal[i] =0;
    } 
}

// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)
// Version provisoire pour test du DAC à modifier
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
   //incrire la valeur de notre tableau dans le DAC sur le channel 0
   SPI_WriteToDac(0, tb_Signal[EchNb]);
   //incrémenter EchNb 
   EchNb++;
   //si EchNB est supperieur Ã  100  
   EchNb = EchNb % MAX_ECH;
}
