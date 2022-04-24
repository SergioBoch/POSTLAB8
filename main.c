/* 
 * File:   main.c
 * Author: Sergio Boch 20887
 *
 * Created on April 21, 2022, 1:44 PM
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

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdint.h>  // Para poder usar los int de 8 bits

#define _XTAL_FREQ 500000
//#define _tmr0_value 255

char UNI;      // Declaración de variables Unidades
char DECC;     // Variable decenas
char CEN;      // Variable centenas
char ttmr0;    // Variable de bandera de timer
int POTENCIOMETRO;  
int SENAL;     // Primera señal de voltaje
int SENAL2;    // Segunda señal de voltaje 

char valores[10] = {0B00111111, 0B00000110, 0B01011011, 0B01001111, 
0B01100110, 0B01101101, 0B01111101, 0B00000111, 0B01111111, 0B01100111};

void setup(void);
void calculos(void);

void __interrupt() isr(void){
    if(INTCONbits.T0IF){        //Interrupción de timer0
        INTCONbits.T0IF = 0;
        TMR0 = 255;             // Valor de timer0
        
        switch(ttmr0){              //Multiplexeo de displays
            case 0:
                PORTE = 0;
                PORTC = valores[CEN];   // Display de centenas
                PORTEbits.RE0 = 1;
                PORTCbits.RC7 = 1;
                ttmr0++;
                break;
                
            case 1:
                PORTE = 0;
                PORTC = valores[DECC];  // Display de decenas
                PORTEbits.RE1 = 1;
                ttmr0++;
                break;
                
            case 2:
                PORTE = 0;
                PORTC = valores[UNI];       // Display de unidades
                PORTEbits.RE2 = 1;
                ttmr0 = 0;
                break;
        }
        INTCONbits.T0IF = 0;        // Reinicio del timer0
    }
    if(PIR1bits.ADIF){              // Interrupción del ADC
        if(ADCON0bits.CHS == 11){   // Verificar si se esta en canal 11
            PORTA = ADRESH;         // Mostrar valores mas significativos
        }
        else
            PORTD = ADRESH;         // Si esta en canal 10
        PIR1bits.ADIF = 0;
    }
}

void main(void){
    setup();
    __delay_us(50);
    ADCON0bits.GO = 1;          // Inicia la conversión
    while(1){
        if(ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 11)
                ADCON0bits.CHS = 10;
            else 
                ADCON0bits.CHS = 11;
            __delay_us(50);
            ADCON0bits.GO = 1;      // Se repite la conversión
        }
        calculos();
    }
}

void calculos(void){
    SENAL = PORTD;          // Guardar valor del puerto D en señal
    POTENCIOMETRO = (SENAL << 2);     // Mover bits, mutiplicar por 4
    SENAL2 = POTENCIOMETRO >> 1;        // Mover a la derecha para divir entre 2 
    
    CEN = SENAL2/100;                   // Operación para obtener centenas
    DECC = (SENAL2-(CEN*100))/10;       // Operación para decenas
    UNI = (SENAL2-(CEN*100+DECC*10));   // Operación para unidades
    return;
}

void setup(void){
    ANSEL = 0;
    ANSELH = 0b1100;    
    
    TRISA = 0;          // Puerto A como salida
    TRISB = 1;          // Puerto B como entrada
    
    TRISC = 0;          // Puerto C como salida
    TRISD = 0;          // Puerto D como salida
    TRISE = 0;          // Puerto E como salida
    
    PORTA = 0;          // Se limpia el puerto A
    PORTB = 0;          // Se limpia el puerto B
    PORTC = 0;          // Se limpia el puerto C
    PORTD = 0;          // Se limpia el puerto D
    PORTE = 0;          // Se limpia el puerto E
    
    OSCCONbits.IRCF = 0b0011; // Oscilador configurdo a 500KHz
    OSCCONbits.SCS = 1; 
    
     //Timer0 Registers Prescaler= 256 - TMR0 Preset = 60 - Freq = 9.96 Hz - Period = 0.100352 seconds
    OPTION_REGbits.T0CS = 0;  // bit 5  Selección de oscilador usado, en este caso oscilador interno
    OPTION_REGbits.T0SE = 0;  // bit 4 Bits de pulsos
    OPTION_REGbits.PSA = 0;   // bit 3  Preescaler asignado para el timer0
    OPTION_REGbits.PS2 = 1;   // bits 2-0  Selección de bits de preescaler
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = 255;
    
    INTCONbits.T0IF = 0;        // Limpiamos bandera del timer0
    INTCONbits.T0IE = 1;        // Se habilita la interrupción del timer0
    
    ADCON0bits.ADCS = 0b00;     // Fosc/2
    ADCON0bits.CHS = 10;        // Seleccionamos el canal 10
    
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;       // Voltaje de referencia VSS y VDD
    ADCON1bits.VCFG1 = 0;       // Voltaje de referencia VSS y VDD
        
    PIR1bits.ADIF = 0;          
    PIE1bits.ADIE = 1;          // Interrupciones analogicas habilitadas
    INTCONbits.PEIE = 1;        // Interrupciones externas habilitadas
    INTCONbits.GIE = 1;         // Interrupciones globales habilitadas
    
    __delay_us(50);             
    ADCON0bits.ADON = 1;        // Se habilita el módulo 
    
    return;
}