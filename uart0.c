// UART0 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include <string.h>
#include "kernel.h"

// PortA masks
#define UART_TX_MASK 2
#define UART_RX_MASK 1

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void stringCopy(char *dest, const char *src)
{
    while (*src != '\0')
    {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
}

void intToHex(uint32_t x)
{
    char hexString [40];
    char hexStringReverse [40];
    uint8_t stringIndex = 0;
    uint8_t stringIndexReverse = 0;
    uint8_t i = 0;
    while (x != 0)
    {
        uint8_t modulos =  x % 16;

        if (modulos < 10)
        {
            hexString[stringIndex] = modulos+48;
        }
        else
        {
            hexString[stringIndex] = modulos + 55;
        }
        stringIndex++;
        x /= 16;
    }
    hexString[stringIndex] = '\0';

    while(stringIndex != 0)
    {
        hexStringReverse[stringIndexReverse] = hexString[stringIndex-1];
        stringIndexReverse++;
        stringIndex--;
    }
    hexStringReverse[stringIndexReverse] = '\0';
    putsUart0("0x");
    for (i = 0; i < 7-(stringIndexReverse-1); i++)
    {
        putsUart0("0");
    }
    putsUart0(hexStringReverse);

}

void iToA(uint32_t integer)
{
    if (integer == 0)
    {
        putcUart0('0');
    }
    else
    {
        char str[100];
        char stringReverse[100];
        uint8_t i = 0;
        uint16_t x = 0;
        while (integer != 0)
        {
            uint8_t mod = integer % 10;

            str[x] = mod + '0';
            x++;
            integer /= 10;
        }
        str[x] = '\0';

        while(x != 0)
        {
            //putcUart0(str[x-1]);
            stringReverse[i] = str[x-1];
            i++;
            x--;
        }
        stringReverse[i] = '\0';
        putsUart0((char*)stringReverse);
    }

}

void fToA(float floatValue)
{
    uint8_t i = 0;
    iToA((uint32_t)floatValue);
    putsUart0(".");
    float fraction = floatValue - (float)(uint32_t)floatValue;
    for (i = 0; i < 2; i++)
    {
        fraction *= 10;
    }
    iToA((uint32_t)fraction);

}

// Initialize UART0
void initUart0()
{
    // Enable clocks
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
    _delay_cycles(3);

    // Configure UART0 pins
    GPIO_PORTA_DR2R_R |= UART_TX_MASK;                  // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTA_DEN_R |= UART_TX_MASK | UART_RX_MASK;    // enable digital on UART0 pins
    GPIO_PORTA_AFSEL_R |= UART_TX_MASK | UART_RX_MASK;  // use peripheral to drive PA0, PA1
    GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA1_M | GPIO_PCTL_PA0_M); // clear bits 0-7
    GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
    // select UART0 to drive pins PA0 and PA1: default, added for clarity

    // Configure UART0 to 115200 baud, 8N1 format
    UART0_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
    UART0_IBRD_R = 21;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                                  // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
    // enable TX, RX, and module
}

// Set baud rate as function of instruction cycle frequency
void setUart0BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes128 = (fcyc * 8) / baudRate;   // calculate divisor (r) in units of 1/128,
    // where r = fcyc / 16 * baudRate
    divisorTimes128 += 1;                               // add 1/128 to allow rounding
    UART0_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART0_IBRD_R = divisorTimes128 >> 7;                // set integer value to floor(r)
    UART0_FBRD_R = ((divisorTimes128) >> 1) & 63;       // set fractional value to round(fract(r)*64)
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
    // turn-on UART0
}

// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(char c)
{
    while (UART0_FR_R & UART_FR_TXFF);               // wait if uart0 tx fifo full
    UART0_DR_R = c;                                  // write character to fifo
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{
    uint8_t i = 0;
    while (str[i] != '\0')
        putcUart0(str[i++]);
}

// Blocking function that returns with serial data once the buffer is not empty
char getcUart0()
{
    while (UART0_FR_R & UART_FR_RXFE)               // wait if uart0 rx fifo empty
        yield();
    return UART0_DR_R & 0xFF;                        // get character from fifo
}

// Returns the status of the receive buffer
bool kbhitUart0()
{
    return !(UART0_FR_R & UART_FR_RXFE);
}


//------------------------UART Subroutines--------------------------------
void getsUart0(USER_DATA *data)
{
    int count = 0;
    int run = 1;
    while (run)
    {
        char c = getcUart0();                  // get character from fifo
        if (count > 0 && (c == 127 || c == 8)) // check if char is backspace
        {
            count--;
        }
        else if (c == 13) // check if char is enter
        {
            data->buffer[count] = '\0';
            run = 0; // exit loop
        }
        else if (c >= 32) // if printable character store in array
        {
            data->buffer[count] = c;
            count++;
            if (count == MAX_CHARS) // if count is greater than Max char length then set null terminator
            {
                data->buffer[count] = '\0';
                run = 0; // exit loop
            }
        }
        yield();
    }
}

void parseFields(USER_DATA *data)
{
    data->fieldCount = 0;
    int i = 0;
    char c = data->buffer[i];
    char type;
    bool isAlphaNumeric = false;
    while (c != NULL)
    {
        if ((c > 64 && c < 91) || (c > 96 && c < 123))
        {
            type = 'a';
            isAlphaNumeric = true;
        }
        else if ((c > 47 && c < 58))
        {
            type = 'n';
            isAlphaNumeric = true;
        }
        else
        {
            if (i > 0)
            {
                data->buffer[i] = '\0';
            }
            isAlphaNumeric = false;
            if (data->fieldCount == MAX_FIELDS)
            {
                return;
            }
        }
        if (isAlphaNumeric)
        {
            if (data->buffer[i - 1] <= 47 || data->buffer[i - 1] >= 123 || (data->buffer[i - 1] >= 58 && data->buffer[i - 1] <= 40) || (data->buffer[i - 1] >= 91 && data->buffer[i - 1] <= 96))
            {
                data->fieldType[data->fieldCount] = type;
                data->fieldPosition[data->fieldCount] = i;
                data->fieldCount++;
            }
        }
        c = data->buffer[++i];
    }
}

char *getFieldString(USER_DATA *data, uint8_t fieldNumber)
{
    char *str = &data->buffer[data->fieldPosition[fieldNumber]];
    if (fieldNumber < data->fieldCount)
    {

        return str;
    }
    else
    {
        return NULL;
    }
}

int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    char *str = getFieldString(data, fieldNumber);

    if (str != NULL && data->fieldType[fieldNumber] == 'n')
    {
        int result = 0;
        int i;
        for (i = 0; str[i] != '\0'; ++i)
        {
            result = result * 10 + str[i] - '0';
        }
        return result;
    }
    else
    {
        return -1;
    }
}

bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments)
{
    char *str = getFieldString(data, 0);

    while ((*strCommand != '\0' && *str != '\0') && *strCommand == *str)
    {
        strCommand++;
        str++;
    }
    if (*strCommand == *str && (data->fieldCount - 1 == minArguments || minArguments == 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool strCmp(char *string, const char strCommand[])
{

    while ((*strCommand != '\0' && *string != '\0') && *strCommand == *string)
    {
        strCommand++;
        string++;
    }
    if (*strCommand == *string)
    {
        return true;
    }
    else
    {
        return false;
    }
}


