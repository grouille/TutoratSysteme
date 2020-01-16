#include <avr/io.h>		// for the input/output register
#include <util/delay.h>
#include <stdio.h>
#include <math.h>

// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz
#define SPEED           9600

void init_serial(int speed)
{
    /* Set baud rate */
    UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;

    /* Enable transmitter & receiver */
    UCSR0B = (1<<TXEN0 | 1<<RXEN0);

    /* Set 8 bits character and 1 stop bit */
    UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);

    /* Set off UART baud doubler */
    UCSR0A &= ~(1 << U2X0);
}

void send_serial(unsigned char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

unsigned char get_serial(void) 
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

int serial_available(void) 
{
    return (UCSR0A & (1<<RXC0))!=0?1:0;
}

// For the AD converter

void ad_init(unsigned char channel)   // initialise le port d'adc
{   
    ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);   
    ADMUX|=(1<<REFS0)|(1<<ADLAR);
    ADMUX=(ADMUX&0xf0)|channel;   
    ADCSRA|=(1<<ADEN);
}   

unsigned int ad_sample(void) // récupère la valeur sur le port adc
{
    ADCSRA|=(1<<ADSC);
    while(bit_is_set(ADCSRA, ADSC));
    return ADCH;
}

// For the I/O 

void output_init(void)
{
    DDRB |= 0x3F; // PIN 8-13 as output
}

void output_set(unsigned char value, unsigned char num_port)
{
    if(value==0) PORTB &= (0xFF^(num_port)); else PORTB |= (uint8_t)num_port;
}

void blink(unsigned char c)
{
    unsigned char value;
    uint8_t tab[6] = {1,2,4,8,16,32};
    if(c <= 70 && c >= 65)
    {
        c -= 65;
        value = 1;
        output_set(value, tab[c]);
    }
    else if(c <=102 && c>=97)
    {
        c -= 97;
        value = 0;
        output_set(value, tab[c]);
    }
}

void input_init(void)
{
    DDRD &= 0x83;  // PIN 2-6 as input
    PORTD |= 0x7C; // Pull-up activated on PIN 2-6
}

unsigned char input_get(void)
{
    //return ((PIND&0x04)!=0)?1:0;
    return PIND&0x7C;
}

// Dummy main

int main(void)
{
    init_serial(SPEED);
    output_init();
    input_init();
    
    unsigned int joystick[2] = {0,0};
    uint8_t message;
    int i;
    
    while(1)
    {
        //contrôle des leds
        if(serial_available())
            blink(get_serial());
        
        //joysticks
        for(i = 0 ; i<2 ; i++)
        {
            ad_init(i); //initialisation de l'ADC
            unsigned int new_joystick = ad_sample();
            new_joystick = new_joystick>>2;
            if(new_joystick!=joystick[i])
            {
                joystick[i]=new_joystick;
                send_serial(joystick[i] | 0x20);
            }
        }
        
        //boutons
        unsigned char message_out = input_get();
        if (message_out!=0x7C)
        {
            message = ((message_out^0xFF)>>2 & 0x3F);
            send_serial(message);
            while(input_get()==message_out) _delay_ms(20);
        }
    }
    
    return 0;
}
