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
    NVM_ReadBlock((uint32_t*) pParam , sizeof(S_ParamGen)); //Taille datas = taille structutre = 14 bytes
    
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
        pParam->Forme = 2;
        pParam->Frequence = 100;
        pParam->Offset = 5000;
    }
}//End of GENSIG_Initialize
  

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
    //intialisation de la variable statique offset
    int16_t Offset;

    int i;
    uint16_t  Step;
    //intitialiser la constatante de la pente
    float const a = ((float)VAL_TIC_MAX/(float)10000)/(float)25;
    float const b = pParam->Amplitude * 65536/10000 + VAL_TIC_MAX;
    
    Ampli.Nb_Tic = (pParam->Amplitude * VAL_TIC_MAX /10000);
    Ampli.Min = ((VAL_TIC_MAX)-(Ampli.Nb_Tic));
    Ampli.Max =((VAL_TIC_MAX)+(Ampli.Nb_Tic)-1);
    
    //gestion de l'offest
    Offset = -((pParam->Offset*VAL_TIC_MAX)/10000);
            
 for(i=0; i<MAX_ECH; i++)
    {   
    switch(pParam->Forme)
    {
        case SignalSinus:        
                 tb_Signal[i] = (Ampli.Nb_Tic/1.28)*(4/M_PI * sin( M_PI *(3.6*i)/180)) + VAL_TIC_MAX + Offset;
            
        break;
        case SignalTriangle:    
                //calcul pour la pente montante du triangle (du centre Ã  la val max)
                if (i < 25 )tb_Signal[i] = (pParam->Amplitude * (a * i)) + VAL_TIC_MAX + Offset;

                //calcul pour la pente descendante du triangle (de la val max- la val min)
                if ((i < 75) && (i >= 25))tb_Signal[i] = (pParam->Amplitude *( (-a) * i) )+ b + Offset;
                
                //calcul pour la pente montante du triangle (de la val min au centre)
                if (i >= 75 )tb_Signal[i] = (pParam->Amplitude *( a * (i - 100)))+VAL_TIC_MAX + Offset ;
        break;
        case SignalDentDeScie:
                Step = ((Ampli.Nb_Tic*2) / MAX_ECH);
                tb_Signal[i] = ((Step * (MAX_ECH-i))+ Ampli.Min + Offset);
        break;
        case SignalCarre:
                if(i < 50 )
                {
                    tb_Signal[i] = Ampli.Max  +Offset ;
                }
                else
                {
                    tb_Signal[i] = Ampli.Min +Offset;
                }   
            break;
    }
 }  
}


// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)

// Version provisoire pour test du DAC à modifier
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
   
   //Si la valeur max est dÃ©passer; saturation
   if(tb_Signal[EchNb] >= (VAL_TIC_MAX*2)-1)tb_Signal[EchNb] = Ampli.Min;
   //obtien la valeur max (65535) dans son tableau
   if (tb_Signal[EchNb] < 0)tb_Signal[EchNb] = 0;
   
   //incrire la valeur de notre tableau dans le DAC sur le channel 0
   SPI_WriteToDac(0, tb_Signal[EchNb]);
   //incrÃ©menter EchNb 
   EchNb++;
   //si EchNB est supperieur Ã  100  
   EchNb = EchNb % MAX_ECH;
   
}
