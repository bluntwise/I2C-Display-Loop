//===========================================================//
// Projet Micro - INFO1 - ENSSAT - S2 2018							 //
//===========================================================//
// File                : Programme de départ
// Hardware Environment: Open1768	
// Build Environment   : Keil µVision
//===========================================================//

#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_libcfg_default.h"
#include "lpc17xx_timer.h"
#include "touch\ili_lcd_general.h"
#include "touch\lcd_api.h"
#include "affichagelcd.h"
#include "touch\touch_panel.h"
#include "lpc17xx_i2c.h"
#include <string.h>
#include "globaldec.h" // fichier contenant toutes les déclarations de variables globales
#include <stdio.h>

void pin_Configuration(){
	PINSEL_CFG_Type config_broche_uart;
	LPC_PINCON->PINSEL4 &= ~((3 << 20) | (3 << 22));
	// --- I2C SDA0 ---
	config_broche_uart.Funcnum = 1; // SCL0
	config_broche_uart.Portnum = PINSEL_PORT_0;
	config_broche_uart.Pinnum = PINSEL_PIN_27;
	config_broche_uart.Pinmode = PINSEL_PINMODE_PULLUP;
	config_broche_uart.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
	PINSEL_ConfigPin(&config_broche_uart);

	// --- I2C SCL0 ---
	config_broche_uart.Pinnum = PINSEL_PIN_28;
	PINSEL_ConfigPin(&config_broche_uart);

	// boutton poussoire 2.10
	config_broche_uart.Funcnum = 0; // GPIO
	config_broche_uart.Portnum = PINSEL_PORT_2;
	config_broche_uart.Pinnum = PINSEL_PIN_10;
	config_broche_uart.Pinmode = PINSEL_PINMODE_PULLUP;
	config_broche_uart.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&config_broche_uart);

	// boutton poussoire 2.11
	config_broche_uart.Funcnum = 0; // GPIO
	config_broche_uart.Portnum = PINSEL_PORT_2;
	config_broche_uart.Pinmode = PINSEL_PINMODE_PULLUP;
	config_broche_uart.OpenDrain = PINSEL_PINMODE_NORMAL;
	config_broche_uart.Pinnum = PINSEL_PIN_11;
	PINSEL_ConfigPin(&config_broche_uart);

	// broches entrée
	GPIO_SetDir(2, (1 << 10) | (1 << 11), 0); 
	
}


#define precision 1000 // précion des temps en microsecondes
#define demie_periode 1000 // duree de la demie-periode du signal en ms
#define quart_periode 1000


void init_timer(){
	// declaration des structures de configuration
	// structure pour config des broches
	PINSEL_CFG_Type maconfig;
	
	// structure qui permet de config le mode ici
	// timer et la précision
	TIM_TIMERCFG_Type maconfigtimer ;
		
	// stucture pour le comportement lorsqu'il atteint
	// une certaine valeur
	TIM_MATCHCFG_Type maconfigmatch ; // pour timer match

	// remplissage pour P1.28 sur MAT0.0
	maconfig.Portnum=PINSEL_PORT_1;
	maconfig.Pinnum=PINSEL_PIN_28;
	
	// permet de connecter une broche au timer MAT 0.0
	maconfig.Funcnum=PINSEL_FUNC_3;  
	maconfig.Pinmode=0; // pullup par défault
	
	// opendrain non utilisé
	maconfig.OpenDrain=PINSEL_PINMODE_NORMAL;
	
	PINSEL_ConfigPin(&maconfig); // appel de la fonction qui va initialiser les registres

	// remplissage pour choisir le mode timer et la precision
	maconfigtimer.PrescaleOption =TIM_PRESCALE_USVAL; // valeur donnee en micro-seconde
	maconfigtimer.PrescaleValue = precision ;  // precision du timer
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&maconfigtimer); // appel de la fonction qui va initialiser les registres

	// remplissage pour choisir les actions quand ça match
	maconfigmatch.ExtMatchOutputType= TIM_EXTMATCH_TOGGLE; // inverse la sortie a chaque match
	maconfigmatch.IntOnMatch = ENABLE; // pas d'interruption generee quand ca match
	maconfigmatch.StopOnMatch = DISABLE ;  //le timer n'arrete pas
	maconfigmatch.MatchChannel = 0; // on utilise MR0 (canal 0)
	maconfigmatch.MatchValue = demie_periode; // correspond a la duree d'une demie-periode
	maconfigmatch.ResetOnMatch = ENABLE; // remet TC a 0 quand ca match
	TIM_ConfigMatch(LPC_TIM0,&maconfigmatch);// appel de la fonction qui va initialiser les registres
	
	// autorisation interruption généré par timer0 dans controlleur d'interruptions
	// afin que TIMER_IRQn soit appelé à chaque match
	NVIC_EnableIRQ(TIMER0_IRQn);
	
	// lancement du timer-compteur
	TIM_Cmd(LPC_TIM0,ENABLE);
	
}

void selectCouleur(uint8_t index, int* x, int* y, uint16_t* couleur, char* couleur_toDisplay) {
	
	// choisit la couleur en fonction de l'index
	// set la position et la couleur
	switch (index % 4) {
    	case 0:
        	*x = 10;
        	*y = 60;
        	*couleur = Blue;
        	strcpy(couleur_toDisplay, "Bleu ");
        	break;
    	case 1:
        	*x = 10;
        	*y = 170;
        	*couleur = Yellow;
        	strcpy(couleur_toDisplay, "Jaune");
        	break;
    	case 2:
        	*x = 120;
        	*y = 170;
        	*couleur = Green;
        	strcpy(couleur_toDisplay, "Vert ");
        	break;
    	case 3:
        	*x = 120;
        	*y = 60;
        	*couleur = Red;
        	strcpy(couleur_toDisplay, "Rouge");
        	break;
	}
}

void init_i2c_eeprom(){
	// structure pour config les broches
	PINSEL_CFG_Type PinCfg;
    
	// Activation du bit 7 pour activer l'alimentation
	// de la mémoire i2c dans le micro controleur
	LPC_SC->PCONP |= ( 1 << 7 );
	// On va activer les broches au port GPIO numéro 0
	PinCfg.Portnum = 0;
	// On va affecter à la fonction numéro 1 ces broches
	PinCfg.Funcnum = 1; // Fonction I2C
    
	// pas de courant, donc opendrain inactif
	PinCfg.OpenDrain = 1;
    
	// Activation d'un pullup interne pour SDA et SCL
	// qui doivent être tirés vers le haut par défault
	PinCfg.Pinmode = 0;
    
	// Config de la broche 27 -> SDA0 afin de pouvoir
	// recevoir et envoyer des données
	PinCfg.Pinnum = 27;
    
	// Config de la pin 27
	PINSEL_ConfigPin(&PinCfg);
    
	// On se positionne sur la 28
	PinCfg.Pinnum = 28;
    
	// Config de la 28
	PINSEL_ConfigPin(&PinCfg);
    
    
	// activation du module I2C0 à 100Khz pour la vitesse du bus i2C
	I2C_Init(LPC_I2C0, 100000);
    
    
	// Activation du module I2C
	I2C_Cmd(LPC_I2C0, ENABLE);
}


#define EEPROM_I2C_ADDR  0x50
#define EEPROM_WRITE_MAX 64  // Taille max des données à écrire

void i2c_eeprom_read(uint16_t addr, uint8_t * data, int length){
	// structure qui permet de décrire un transfert I2C (maitre, esclave)
	I2C_M_SETUP_Type transfer;
    
    
	uint8_t addr_buff[1];
	// on stocke les 8 premiers bits de poids faible
  addr_buff[0] = addr & 0xFF;
    
	// slave puis récupération des 3 bit A2, A1, A0 dans addr
	// A2, A1, A0 sont les 3 bits de poids fort
	transfer.sl_addr7bit = EEPROM_I2C_ADDR | (addr >> 8);
	// on mets le buffer de l'adresse des 8 bits de poids faible
	transfer.tx_data = addr_buff;
	// 1 seul octet envoyé
	transfer.tx_length = 1;
	// on mets les data dans rx_data
	transfer.rx_data = data;
	// on lit la longueur des data
	transfer.rx_length = length;
	// si erreur on réessaye trois fois
	transfer.retransmissions_max = 3;
    
	// transfert i2c le programme attend la fin du transfert
	I2C_MasterTransferData(LPC_I2C0, &transfer, I2C_TRANSFER_POLLING);
}
void i2c_eeprom_write(uint16_t addr, uint8_t* data, int length) {
    
  // buffer pour stocker l'adresse mémoire 1 octet puis les data
	uint8_t buffer[1 + EEPROM_WRITE_MAX];
  int i;
    
  // structure qui permet de décrire un transfert I2C (maitre, esclave)
	I2C_M_SETUP_Type transfer;
    
  // vérification si longueur problème
	if (length > EEPROM_WRITE_MAX){
      return;
  }
   	 
	// on mets les 8 bits de poids faible dans le premier octet du buffer
	buffer[0] = addr & 0xFF;
 	 
  // on mets les données dans le buffer
	for (i = 0; i < length; i++){
        	buffer[1 + i] = data[i];
    	}
   	 
  // slave puis récupération des 3 bit A2, A1, A0 dans addr
  // A2, A1, A0 sont les 3 bits de poids fort
	transfer.sl_addr7bit = EEPROM_I2C_ADDR |(addr >> 8);
  // data à envoyer : buffer
	transfer.tx_data = buffer;
  // on mets la taille du buffer
	transfer.tx_length = 1 + length;
   	 
  // pas de données à recevoir
	transfer.rx_data = NULL;
	transfer.rx_length = 0;
   	 
  // 3 tentatives en cas d'échec
	transfer.retransmissions_max = 3;
   	 
  // transfert i2c le programme attend la fin du transfert
	I2C_MasterTransferData(LPC_I2C0, &transfer, I2C_TRANSFER_POLLING);
}



void TIMER0_IRQHandler(void){

	// verification si l'interruption provient bien du timer 0
	if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
		
		flag_timer = 1; // activation du drapeau
		
		// effacement du signal d'interruption dans le registre 
		// du timer
		TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
	}
	
}



//===========================================================//
// Function: Main
//===========================================================//
int main(void)
{	  
	  // Init(); // init variables globales et pinsel pour IR => à faire
	
		// déclaration des variables avant de faire des opérations
		int n;
	  uint8_t index;
    char chaine[32];
		int x;
		int y;
		uint16_t couleur;
	  int i;
		char couleur_toDisplay[7];
		
	
		// configuration de la pin
    pin_Configuration();
	
		// initialisation de la mémoire i2c
    init_i2c_eeprom();
		
		// lecture de la case 0x010 dans la mémoire i2c
		i2c_eeprom_read(0x010, &index, 1);
	  
		
		lcd_Initializtion(); // init pinsel ecran et init LCD
		
		// on dessine un rectangle bleu sur l'écran bleu
		dessiner_rect(10,60,220,220,0,1, Black, Blue); 
		
		// on séléctionne la couleur, les positions en fonction de l'index 
		selectCouleur((index-1)%4, &x, &y, &couleur, couleur_toDisplay);
		
		sprintf(chaine, "Ancienne couleur : %s", couleur_toDisplay);
		
		// affichage de l'ancienne couleur
		LCD_write_english_string(10,10,chaine,White,Blue);
		
		// dessin d'un carré noir pour délimiter ou va être le cycle
		dessiner_rect(x,y,110,110, 2, 1, Black, couleur);
		
		// 3 secondes d'attente avant le début du cycle
		for (i = 0; i < 30000000; i++){
			
		}
		init_timer();
		
		// démarrage de la boucle
		
    while(1){
			// on vérifie si le timer a été mis à jour
			if (flag_timer) {
				flag_timer = 0;

				// Effacer proprement la zone du haut
				dessiner_rect(10, 10, 220, 50, 0, 1, Blue, Blue);

				// Effacer la zone du carré (rectangle noir sur fond bleu)
				dessiner_rect(10, 60, 220, 220, 0, 1, Black, Blue); 

				// Lire l’index actuel
				i2c_eeprom_read(0x010, &index, 1);

				// Déterminer la couleur
				selectCouleur(index, &x, &y, &couleur, couleur_toDisplay);

				// Afficher le nom de la couleur
				LCD_write_english_string(10, 10, couleur_toDisplay, White, Blue);

				// Dessiner le carré de couleur
				dessiner_rect(x, y, 110, 110, 2, 1, Black, couleur);

				// Préparer la prochaine couleur
				index = (index + 1) % 4;
				i2c_eeprom_write(0x010, &index, 1);
			}




		}
		
	}

//---------------------------------------------------------------------------------------------	
#ifdef  DEBUG
void check_failed(uint8_t *file, uint32_t line) {while(1);}
#endif
