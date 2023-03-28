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

    //control d'activité pour eteindre le back light
    if(Pec12.NoActivity == 1)
    {
        lcd_bl_off();
    }
    else
    {
        lcd_bl_on();
    }
    //control si on a selectioner quelque chose dans le menu
    if(Pec12.OK == 1 && PtrPosition == 0)
    {
        PtrPosition = 1;
        Menu = Ptr;
        Pec12ClearOK();
    }
    //quiter le menu pour le menu principal
    if(Pec12.ESC == 1 && PtrPosition == 1 )
    {
        Menu = MenuPrincipal;
        Pec12ClearESC();
    } 
    //on veut effectuer la sauvegarde
    if(S9.OK == 1 && Menu != MenuSave )
    {
        Menu = MenuSave;
        PtrSave = 1;
        lcd_ClearLine(1);
        lcd_ClearLine(4);
    }
    //gestion des diferent menu
    switch(Menu)
    {
        //Gestion de selection des menu
        case MenuPrincipal:
                GestMenu(4,1,1,&Ptr,&PtrOk);
                //mise a zero certaine variable de control et de compteur
                Count3 =0;
                Count2 =0;
                Count1 =0;
                PtrPosition = 0;
                PtrSave = 0;
                Save = 0;
                PtrOk = 0;
                
            break;
        //Gestion du menu de selection de forme
        case MenuForme:
                GestMenu(3,0,1,&Choix,&PtrOk);
                //si presion ok en garde la valeur dans la structure
                if(PtrOk == 1)pParam->Forme = Choix;
                
            break;
        //Gestion du menu la frequence souhaité
        case MenuFreq:
                GestMenu(2000,20,20,&Frequence,&PtrOk);
                //si presion ok en garde la valeur dans la structure
                if((PtrOk == 1))pParam->Frequence = Frequence;
            break;
        //Gestion du menu de l'amplitude souhaité
        case MenuAmpl:
                GestMenu(10000,0,100,&Amplitude,&PtrOk);
                //si presion ok en garde la valeur dans la structure
                if((PtrOk == 1))pParam->Amplitude = Amplitude;
            break;
        //Gestion du menu de l'offset souhaité
        case MenuOffset:
                GestMenu(5000,-5000,100,&Offset,&PtrOk);
                //si presion ok en garde la valeur dans la structure
                if((PtrOk == 1))pParam->Offset = Offset;
            break;
        //Gestion du menu de sauvegarde
        case MenuSave:
                Count3 ++;
                //Controle si il y'a presion sur le bouton
                if(S9.OK == 1 && Count3 > 100)Count1 ++;
                //Controle si on a relacher le bouton
                if(S9.OK == 0 && Count1 >0)
                { 
                    Count3 = 0; 
                    //control de la durée de pression
                    if(Count1 > 100)Save=2;
                    if(Count1 < 100)Save=1;
                    //si durée pression ok en sauvegarde dans la memoire
                    if(Save == 2 && Count2 == 0)NVM_WriteBlock((uint32_t*)pParam , sizeof(S_ParamGen));
                    Count2 ++;
                    //Si 2 seconde passée retour au menu principal
                    if(Count2 > 200)
                    {
                        Menu = MenuPrincipal;
                        //Lecture de la memoire pour control
                        NVM_ReadBlock((uint32_t*)pParam , sizeof(S_ParamGen));
                    }
                }
                
            break;
    }
    //si mode save actif affichage de sauvegarde
    if(PtrSave == 1)
    {
        lcd_gotoxy(1,2);    
        printf_lcd("    Sauvegarde ?  "); 
        lcd_gotoxy(1,3);
        // indique si sauvegarde ok ou annulé
        printf_lcd("    %s  ",Selection[Save]);
    }
    else//sinon afichage menu normal
    {   //effacement de la colonne 1 pour en suite pouvoir afficher le symbole de selection de menu    
        for(i=1;i<5;i++)
        {
            lcd_gotoxy(1,i);
            printf_lcd(" ");
        }
        //selection menu 
        lcd_gotoxy(1,Ptr);
        //affiche le symole * ou ?
        printf_lcd("%s",Curseur[PtrPosition]);
        //si menu principal affichage des valeur garder
        if(Menu == MenuPrincipal)diplay(pParam->Frequence,pParam->Amplitude,pParam->Offset, pParam->Forme);
        //si menu specifique affichage des valeur provisoir
        if(Menu != MenuPrincipal)diplay(Frequence,Amplitude,Offset,Choix);
    }
}

//s'occupe de la gestion des menu en fonction des parametre d'entré
void GestMenu(int16_t ValueMax,int16_t ValueMin,uint8_t Step,int16_t* PtrValue,bool* Save)
{
    int16_t Value = 0;  
    
    Value = *PtrValue;
    //si presion ok en indique que on veut garder la valeur et en reviens au menu principal        
    if(Pec12.OK == 1)
    {
        *Save = 1;
        Menu = 0;
        Pec12ClearOK();
    }
    //si incrementation en augmente la valeur par rapport au step d'entré
    if(Pec12.Inc == 1)Value = Value + Step;
    //rebouclement
    if(Value > ValueMax )
    {
        Value = ValueMin;
        //si dans menu offset dispositf bloquant au lieu de rebouclement
        if(Menu == MenuOffset)Value = ValueMax;
    }
    //si decrementation en diminue la valeur par rapport au step d'entré
    if(Pec12.Dec == 1)Value = Value - Step;
    //rebouclement
    if(Value < ValueMin )
    {
        Value = ValueMax;
        //si dans menu offset dispositf bloquant au lieu de rebouclement
        if(Menu == MenuOffset)Value = ValueMin;
    }
    //remise a zero des flag
    Pec12ClearPlus();
    Pec12ClearMinus();
    //Récuperation la valeur
    *PtrValue = Value;
}
//affichage en fonction des parametre d'entre, valeur quand sauvgarde ou non
void diplay(int16_t ValFreq,int16_t ValAmp,int16_t ValOffs,int16_t Choix)
{
    char *Forme[] = {"Sinus     ","Triangle  ","DentDeScie","Carre     "};
    //ligne 1 selection de la forme
    lcd_gotoxy(2,1); 
    printf_lcd("Forme = %s",Forme[Choix]);         
    //ligne 2 valeur de la frequence
    lcd_gotoxy(2,2);    
    printf_lcd("Freq [Hz] = %-6d",ValFreq); 
    //ligne 3 valeur de l'amplitude
    lcd_gotoxy(2,3);    
    printf_lcd("Ampl [mV] = %-5d",ValAmp); 
    //ligne 4 valeur de l'offset
    lcd_gotoxy(2,4);    
    printf_lcd("Offset [mV] = %-5d",ValOffs);
};