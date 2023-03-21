// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms



#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32DriverLcd.h"
#include "GesPec12.h"
#include "Mc32NVMUtil.h"

// Structure pour les traitement du Pec12
S_Pec12_Descriptor Pec12;
S_S9_Descriptor S9;

E_Menu Menu;

int16_t Choix,Frequence,Amplitude,Offset;


// Initialisation du menu et des paramètres
void MENU_Initialize(S_ParamGen *pParam)
{
    Choix = pParam->Forme;
    Frequence = pParam->Frequence;
    Amplitude = pParam->Amplitude;
    Offset = pParam->Offset;
}


// Execution du menu, appel cyclique depuis l'application
void MENU_Execute(S_ParamGen *pParam)
{
    char *Curseur[] = {"*","?"};
    char *Selection[] = {"(appui long)","  ANNULEE!  ","    OK!    "};
    static uint8_t i=0;
    static uint8_t Count1,Count2,Count3;
    static bool PtrPosition = 0;
    static bool PtrOk,PtrSave;
    static int16_t Ptr = 1;
    static int16_t Save;
    
    if(Pec12.NoActivity == 1)
    {
        lcd_bl_off();
    }
    else
    {
        lcd_bl_on();
    }
    
    if(Pec12.OK == 1 && PtrPosition == 0)
    {
        PtrPosition = 1;
        Menu = Ptr;
        Pec12ClearOK();
    }
    
    if(Pec12.ESC == 1 && PtrPosition == 1 )
    {
        Menu = MenuPrincipal;
        Pec12ClearESC();
    } 
    
    if(S9.OK == 1 && Menu != MenuSave )
    {
        Menu = MenuSave;
        PtrSave = 1;
        lcd_ClearLine(1);
        lcd_ClearLine(4);
    }
    
    switch(Menu)
    {
        case MenuPrincipal:
                GestMenu(4,1,1,&Ptr,&PtrOk);
                Count3 =0;
                Count2 =0;
                Count1 =0;
                PtrPosition = 0;
                PtrSave = 0;
                Save = 0;
                PtrOk = 0;
                
            break;
        case MenuForme:
                GestMenu(3,0,1,&Choix,&PtrOk);
                if(PtrOk == 1)pParam->Forme = Choix;
                
            break;
        case MenuFreq:
                GestMenu(2000,20,20,&Frequence,&PtrOk);
                if((PtrOk == 1))pParam->Frequence = Frequence;
            break;
        case MenuAmpl:
                GestMenu(10000,0,100,&Amplitude,&PtrOk);
                if((PtrOk == 1))pParam->Amplitude = Amplitude;
            break;
        case MenuOffset:
                GestMenu(5000,-5000,100,&Offset,&PtrOk);
                if((PtrOk == 1))pParam->Offset = Offset;
            break;
        case MenuSave:
                Count3 ++;
                if(S9.OK == 1 && Count3 > 100)
                    Count1 ++;
                if(S9.OK == 0 && Count1 >0)
                { 
                    Count3 = 0; 
                    if(Count1 > 100)
                        Save=2;
                    if(Count1 < 100)
                        Save=1;
                    if(Save == 2 && Count2 == 0)
                        NVM_WriteBlock((uint32_t*)pParam , sizeof(S_ParamGen));
                    Count2 ++;                
                    if(Count2 > 200)
                    {
                        Menu = MenuPrincipal;
                        NVM_ReadBlock((uint32_t*)pParam , sizeof(S_ParamGen));
                    }
                }
                
            break;
    }
    if(PtrSave == 1)
    {
        lcd_gotoxy(1,2);    
        printf_lcd("    Sauvegarde ?  "); 
        lcd_gotoxy(1,3);
        printf_lcd("    %s  ",Selection[Save]);
    }
    else
    {        
        for(i=1;i<5;i++)
        {
            lcd_gotoxy(1,i);
            printf_lcd(" ");
        }
        lcd_gotoxy(1,Ptr);
        printf_lcd("%s",Curseur[PtrPosition]);
        
        if(Menu == MenuPrincipal)diplay(pParam->Frequence,pParam->Amplitude,pParam->Offset, pParam->Forme);
        if(Menu != MenuPrincipal)diplay(Frequence,Amplitude,Offset,Choix);
    }
}


void GestMenu(int16_t ValueMax,int16_t ValueMin,uint8_t Step,int16_t* PtrValue,bool* Save)
{
    int16_t Value = 0;  
    
    Value = *PtrValue;
            
    if(Pec12.OK == 1)
    {
        *Save = 1;
        Menu = 0;
        Pec12ClearOK();
    }
    
    if(Pec12.Inc == 1)Value = Value + Step;
    if(Value > ValueMax )
    {
        Value = ValueMin;
        if(Menu == MenuOffset)Value = ValueMax;
    }
    if(Pec12.Dec == 1)Value = Value - Step;
    if(Value < ValueMin )
    {
        Value = ValueMax;
        if(Menu == MenuOffset)Value = ValueMin;
    }
    
    Pec12ClearPlus();
    Pec12ClearMinus();
    
    *PtrValue = Value;
}

void diplay(int16_t ValFreq,int16_t ValAmp,int16_t ValOffs,int16_t Choix)
{
    char *Forme[] = {"Sinus     ","Triangle  ","DentDeScie","Carre     "};
    //ligne 1
    lcd_gotoxy(2,1); 
    printf_lcd("Forme = %s",Forme[Choix]);         
    //ligne 2
    lcd_gotoxy(2,2);    
    printf_lcd("Freq [Hz] = %-6d",ValFreq); 
    //ligne 3
    lcd_gotoxy(2,3);    
    printf_lcd("Ampl [mV] = %-5d",ValAmp); 
    //ligne 4
    lcd_gotoxy(2,4);    
    printf_lcd("Offset [mV] = %-5d",ValOffs);
};