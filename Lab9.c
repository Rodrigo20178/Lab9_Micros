/*
 * File:   Lab9.c
 * Author: Rodrigo García 
 *
 * Created on 21 de abril de 2023, 11:26 AM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>



//Constantes
#define _XTAL_FREQ 4000000      // Oscilador de 4 MHz


//Variables
uint8_t pot = 0;

//Prototipos de funciones
void setup (void);
uint8_t read_EEPROM(uint8_t address);
void write_EEPROM(uint8_t address, uint8_t data);

//Interrupciones
void __interrupt() isr (void)
{
    if (PIR1bits.ADIF)  // Interrupción del ADC
    {
        pot = ADRESH;           // Enviar contenido de ADRESH  a pot
        PORTC = pot;            // Enviar el contenido de pot a PORTC
        PIR1bits.ADIF = 0;      
    }
    
    
    if (INTCONbits.RBIF)
    {
        if (!PORTBbits.RB2)
        {
            write_EEPROM(0x00, pot);    //Interrupción para guardar el valor del pot, y mostrarlo en el puerto D
            PORTD = read_EEPROM(0x00);
        }
        
        
        
        if (!PORTBbits.RB1){    //Interrupción para despertar el PIC
            __delay_ms(100);
        }
                
        INTCONbits.RBIF = 0;    // Limpiamos bandera de interrupción RBIF
    }
    return;
}
//MAIN
void main(void) {
    setup();
    ADCON0bits.GO = 1;      // primera conversión del ADC
    
    while(1){
    
    if (ADCON0bits.GO == 0) {       // Cuando se termine de convertir, iniciar una nueva conversión
        __delay_us(50);
        ADCON0bits.GO = 1;
    } 
    if (!PORTBbits.RB0)
        {
            SLEEP();        //Interrupción del modo sleep
        }   
    PORTD = read_EEPROM(0x00);
    }        
    return;
}

//Configuraciones
void setup (void){
    
    // Configuración de los puertos
    ANSEL = 0b00000001;          //AN0 como entrada analógica
    ANSELH = 0;
    
    TRISA = 0b00000001;         // PORTA como salida, RA0 como entrada 
    PORTA = 0;                  // Limpiamos PORTA 
    
    TRISC = 0;                  // PORTC como salida
    PORTC = 0;                  // Limpiamos PORTC 
    
    TRISD = 0;                  // PORTD como salida
    PORTD = 0;                  // Limpiamos PORTD 
   
    // Configuración de las IOCB
    TRISB = 0b00000111;         // RB0 & RB1 de PORTB como entradas
    PORTB = 0;
    
    OPTION_REGbits.nRBPU = 0;   // Habilitamos resistencias de pull-up del PORTB
    WPUBbits.WPUB0 = 1;         // Habilitamos resistencia de pull-up de RB0 & RB1
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;
    
    INTCONbits.RBIE = 1;        // Habilitamos interrupciones del PORTB
    IOCBbits.IOCB1 = 1;         // Habilitamos IOCB en RB1 & RB2
    IOCBbits.IOCB2 = 1;
    
    INTCONbits.RBIF = 0;        // Limpiamos bandera de interrupción de PORTB
    
    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0110;    // IRCF <2:0> -> 110 4 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    
    //COnfiguración del ADC
    ADCON1bits.ADFM = 0;        // Ajustado a la izquierda
    ADCON1bits.VCFG0 = 0;       // Referencia en VDD
    ADCON1bits.VCFG1 = 0;       // Referencia en VSS
    
    ADCON0bits.ADCS = 0b01;     // ADCS <1:0> -> 01 FOSC/8
    ADCON0bits.CHS = 0b0000;    // CHS  <3:0> -> 0000 AN0
    
    ADCON0bits.ADON = 1;        // Encender ADC
    __delay_us(50);
    
    //Configuración de las interrupciones
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de los puertos
    
    PIE1bits.ADIE = 1;          // Habilitamos interrupciones del ADC
    PIR1bits.ADIF = 0;          // Flag del ADC en 0
    
 
}
//Funciones

uint8_t read_EEPROM(uint8_t address){
    EEADR = address;
    EECON1bits.EEPGD = 0;       // Lectura a la EEPROM
    EECON1bits.RD = 1;          // Obtenemos dato de la EEPROM
    return EEDAT;               // Regresamos dato 
}

void write_EEPROM(uint8_t address, uint8_t data){
    EEADR = address;
    EEDAT = data;
    EECON1bits.EEPGD = 0;       // Escritura a la EEPROM
    EECON1bits.WREN = 1;        // Habilitamos escritura en la EEPROM
    
    INTCONbits.GIE = 0;         // Deshabilitamos interrupciones
    INTCONbits.PEIE = 0;        // Deshabilitamos interrupciones de los puertos
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;          // Iniciamos escritura
    
    EECON1bits.WREN = 0;        // Deshabilitamos escritura en la EEPROM
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de los puertos
}