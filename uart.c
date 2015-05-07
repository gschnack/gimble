
#include <stdint.h>

#include <libpic30.h>


#include <p33FJ12MC202.h>



#include "uart.h"

#include "fix6_10.h"




unsigned char txbuffer [MAX_LENGTH];
unsigned char txhead = 0;
unsigned char txtail = 0;
unsigned char txstate = 0;



unsigned char rxbuffer [MAX_LENGTH];
volatile unsigned char rxhead = 0;
volatile unsigned char rxtail = 0;





void InitUART() {
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets

    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only

    // configure U2MODE
    U1MODEbits.UARTEN = 0; // Bit15 TX, RX DISABLED, ENABLE at end of func
    //U2MODEbits.notimplemented;	// Bit14
    U1MODEbits.USIDL = 0; // Bit13 Continue in Idle
    U1MODEbits.IREN = 0; // Bit12 No IR translation
    U1MODEbits.RTSMD = 0; // Bit11 Simplex Mode
    //U2MODEbits.notimplemented;	// Bit10
    U1MODEbits.UEN = 0; // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0; // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0; // Bit6 No Loop Back
    /// CHange 0

    U1MODEbits.ABAUD = 0; // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.URXINV = 0; // Bit4 IdleState = 1  (for dsPIC)
    U1MODEbits.BRGH = 0; // Bit3 16 clocks per bit period
    U1MODEbits.PDSEL = 0; // Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0; // Bit0 One Stop Bit

    // Load a value into Baud Rate Generator.  Example is for 9600.
    // See section 19.3.1 of datasheet.
    //  U1BRG = (Fcy / (16 * BaudRate)) - 1
    //  U1BRG = (36850000 / (16 * 9600)) - 1
    //  U1BRG = 238.908854 //Round to 239

    U1BRG = 259;

    // Load all values in for U1STA SFR
    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0; //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15
    //U2STAbits.notimplemented = 0;	//Bit12
    U1STAbits.UTXBRK = 0; //Bit11 Disabled
    U1STAbits.UTXEN = 0; //Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0; //Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0; //Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0; //Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0; //Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0; //Bit4 *Read Only Bit*
    U1STAbits.PERR = 0; //Bit3 *Read Only Bit*
    U1STAbits.FERR = 0; //Bit2 *Read Only Bit*
    U1STAbits.OERR = 0; //Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0; //Bit0 *Read Only Bit*


    U1MODEbits.UARTEN = 1; // And turn the peripheral on

    U1STAbits.UTXEN = 1;
    // I think I have the thing working now.

    U1STAbits.URXISEL1 = 0;

}




void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) {

    if (U1STAbits.OERR == 1)
        U1STAbits.OERR = 0;

    if (U1STAbits.FERR == 1)
        U1STAbits.FERR = 0;
    if (U1STAbits.PERR == 1)
        U1STAbits.PERR = 0;



    //if (U1STAbits.URXDA ==1)
    //{
    rxbuffer[rxtail++] = U1RXREG;
    if (rxtail == MAX_LENGTH)
        rxtail = 0;
    //}
    IFS0bits.U1RXIF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void) {
    //              grnLEDON;
    IFS0bits.U1TXIF = 0; // clear interrupt flag


    if (txhead != txtail) {
        U1TXREG = txbuffer [txhead++];
        if (txhead == MAX_LENGTH)
            txhead = 0;

    } else// disable interrupt last char
    {

        if (txstate == 0) {//U1STAbits.UTXEN = 0
            U1TXREG = txbuffer [txhead]; // last char out
            txstate = 1;
        } else {
            U1STAbits.UTXEN = 0; // shut up sending
            txstate = 0;
        }
    }
}

void RS232print(char *source, int len) {
    int i;


    for (i = 0; i < len; i++) {
        txbuffer[txtail++] = source[i];
        if (txtail == MAX_LENGTH)
            txtail = 0;
    }
    U1STAbits.UTXEN = 1;
}

int RS232received(void) {
    return ( rxhead != rxtail);
}

char RS232getch(void) {
    char ch;
    ch = rxbuffer[rxhead++];
    if (rxhead == MAX_LENGTH)
        rxhead = 0;
    return (ch);
}


void itoa_1(unsigned int n, char *s) {
    int tenth, min_flag;
    char swap, *p;

    min_flag = 0;
    if (0 > n) {
        *s++ = '-';
        //n = -INT_MAX > n ? min_flag = INT_MAX : -n;
    }
    p = s;
    do {
        tenth = n / 10;
        *p++ = (char) (n - 10 * tenth + '0');
        n = tenth;
    } while (n != 0);
    if (min_flag != 0) {
        ++*s;
    }
    *p-- = '\0';
    while (p > s) {
        swap = *s;
        *s++ = *p;
        *p-- = swap;
    }
}

int myAtoi(char *str) {
    int res = 0; // Initialize result
    int i;
    // Iterate through all characters of input string and update result
    for (i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';

    // return result.
    return res;
}




void print_str(char *S,int l)
{
    RS232print(S,l );
   __delay32((uint32_t) (800000L));

}

void print_CR()
{
    //char ?[]=10;;
    RS232print((char *) 13,1 );
    RS232print((char *) 10,1 );

   __delay32((uint32_t) (800000L));

}




void print_fix6_10( fix6_10_t value)
{

    
    uint16_t high,low, digit=2;

    unsigned char strbuffer[10];
    high =highfix2dec(value);
    low= lowfix2dec(value );


                    strbuffer[0] = value & 0x8000 ?'-':'+';
                    strbuffer[1] = ' ';
                    strbuffer[2] = ' ';
                    strbuffer[3] = '.';
                    strbuffer[4] = '0';
                    strbuffer[5] = '0';
                    strbuffer[6] = '0'; //13;
                    strbuffer[7] = ' '; //10;

                    strbuffer[7] = 13;
                    strbuffer[8] = 10;
			//strbuffer[9] = '\0';

//	printf("h%d l%d\n",high,low );

            digit =2;
		if ( high <=99)
		   digit =1;
		if ( high <=9)
		   digit =2;



                    itoa_1(high, &strbuffer[digit]);
		    //strbuffer[strlen( strbuffer)]='.';
			strbuffer[3] = '.';

		digit =4;

		if ( low <999)
		   digit =4;
		if ( low <99)
		   digit =5;
		if ( low <9)
		   digit =6;

                    itoa_1(low, &strbuffer[digit]);
		strbuffer[7] = ' '; //10;

//		printf("buffer %s",strbuffer );
                //print_str("low",3);
                //print_int16( low);


                    RS232print(&strbuffer[0], 9);

                   __delay32((uint32_t) (800000L));


}





#ifdef NIX
void print_fix16( fix6_10_t value)
{

    char helpbuffer[10];
    int16_t high,low;

    high =highfix2dec(value);
    low= lowfix2dec(value );


                    helpbuffer[0] = value & 0x80000000L?'-':'+';
                    helpbuffer[1] = ' ';
                    helpbuffer[2] = ' ';
                    helpbuffer[3] = '.';
                    helpbuffer[4] = ' ';
                    helpbuffer[5] = ' ';
                    helpbuffer[6] = ' '; //13;
                    helpbuffer[7] = ' '; //10;

                    helpbuffer[7] = 13;
                    helpbuffer[8] = 10;


                    itoa_1(high, &helpbuffer[1]);
                    itoa_1(low, &helpbuffer[4]);

                    RS232print(&helpbuffer[0], 9);

                   __delay32((uint32_t) (800000L));



}
#endif




void print_int16( int16_t value)
{

    char helpbuffer[10];


                    helpbuffer[0] = value & 0x8000L?'-':'+';
                    helpbuffer[1] = ' ';
                    helpbuffer[2] = ' ';
                    helpbuffer[3] = ' ';
                    helpbuffer[4] = ' ';
                    helpbuffer[5] = ' ';
                    helpbuffer[6] = ' '; //13;
                    helpbuffer[7] = ' '; //10;

                    helpbuffer[7] = 13;
                    helpbuffer[8] = 10;

                    if( value < 0)
                        value *=-1;
                    itoa_1(value, &helpbuffer[1]);

                    RS232print(&helpbuffer[0], 9);

                   __delay32((uint32_t) (80000L));



}
