/* This program sets up UART0 on TI ARM LaunchPad (TM4C123GH6PM) to do terminal echo.
 * When a key is pressed at the terminal emulator of the PC, the character is received by
 * UART0 and it is sent out of UART0 back to the terminal.
 */

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "inc/tm4c123gh6pm.h"

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

/* U0Rx receive connected to PA0 */
/* U0Tx transmit connected to PA1 */

#define UART_FR_TXFF            0x00000020  /* UART Transmit FIFO Full */
#define UART_FR_RXFE            0x00000010  /* UART Receive FIFO Empty */
#define UART_LCRH_WLEN_8        0x00000060  /* 8 bit word length */
#define UART_LCRH_FEN           0x00000010  /* UART Enable FIFOs */
#define UART_CTL_UARTEN         0x00000001  /* UART Enable */
#define SYSCTL_RCGC1_UART0      0x00000001  /* UART0 Clock Gating Control */
#define SYSCTL_RCGC2_GPIOA      0x00000001  /* port A Clock Gating Control */

void Program_Init(void);
void UART_Init(void);
char UART_InChar(void);
void UART_OutChar(char data);
char process_input(char str_arr[100]);
void led_on();
void led_off();
void led_blink();
void mem_read(int bit);
void mem_write(int bit);
void delayMs(int n);
void help();

char command_msg[] = "\n\rWrite \"help\" to see all available commands\n\rEnter command :\n\r";
char command_help[] = "\n\r1. led on <colour>\n\rcolour = red, blue, green, purple, white\n\r\n\r2. led off\n\r\n\r3. led blink <colour>\n\rcolour = red, blue, green, purple, white\n\r\n\r4. read <mem_address> <no_of_bytes>\n\r\n\r5. write <mem_address> <no_of_bytes> <data>\n\r";
char command_invalid[] = "\n\rINVALID COMMAND\n\r";
char input_buffer[100];
char command, colour_reg;
char colour_arr[5][8] = {"red","purple","blue","green","white"};
char colour_val[5] = {0x02,0x06,0x04,0x08,0x0E};
int no_of_bytes_read;
int address;

int main(void)
{
    char c;
    int i = 0;
    char* msg;

    Program_Init();
    UART_Init();

    msg = command_msg;
    while( *msg)
        UART_OutChar(*msg++);

    while( 1 )
    {
        c = UART_InChar();   /* receive char */
        if (isalnum(c) || c == SP)
        {
            UART_OutChar(c);     /* transmit received char */
            input_buffer[i]= c;
            i++;
        }
        else if (c == CR)
        {
            UART_OutChar('\n');     /* transmit received char */
            UART_OutChar('\r');     /* transmit received char */
            input_buffer[i] = '\0';
            command = process_input(input_buffer);
            switch (command)
            {
            case '8' : help();
                break;
            case '1' : led_on();
                break;
            case '2' : led_off();
                break;
            case '3' : led_blink();
                break;
            case '4' : mem_read(0);
                break;
            case '5' : mem_read(1);
                break;
            case '6' : mem_write(0);
                break;
            case '7' : mem_write(1);
                break;
            default :
                msg = command_invalid;
                while( *msg)
                    UART_OutChar(*msg++);
            }
            i = 0;
            msg = command_msg;
            while( *msg)
                UART_OutChar(*msg++);
        }
    }
}

void Program_Init(void)
{
    /* enable clock to GPIOF at clock gating control register */
    SYSCTL_RCGC2_R |= 0x00000020;
    /* enable the GPIO pins for the LED (PF3, 2 1) as output */
    GPIO_PORTF_DIR_R = 0x0E;

    /* enable the GPIO pins for digital function */
    GPIO_PORTF_DEN_R = 0x0E;
}

/* UART_Init
* Initialize the UART for 115,200 baud rate (assuming 16 MHz bus clock),
* 8 bit word length, no parity bits, one stop bit, FIFOs enabled
* Input: none
* Output: none
*/
void UART_Init(void)
{
      SYSCTL_RCGCUART_R |= 0x01;            /* activate UART0 */
      SYSCTL_RCGCGPIO_R |= 0x01;            /* activate port A */

      while((SYSCTL_PRGPIO_R&0x0001) == 0){};/* ready? */
      UART0_CTL_R &= ~UART_CTL_UARTEN;      /* disable UART */
      UART0_IBRD_R = 8;        /* IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680) */
      UART0_FBRD_R = 44;       /* FBRD = round(0.5104 * 64 ) = 44 */
                               /* 8 bit word length (no parity bits, one stop bit, FIFOs) */
      UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
      UART0_CTL_R |= UART_CTL_UARTEN;       /* enable UART */
      GPIO_PORTA_AFSEL_R |= 0x03;           /* enable alt funct on PA1-0 */
      GPIO_PORTA_DEN_R |= 0x03;             /* enable digital I/O on PA1-0 */
                                            /* configure PA1-0 as UART */
      GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
      GPIO_PORTA_AMSEL_R &= ~0x03;          /* disable analog functionality on PA */
}

/* UART_InChar
* Wait for new serial port input
* Input: none
* Output: ASCII code for key typed
*/
char UART_InChar(void)
{
      while( (UART0_FR_R & UART_FR_RXFE) != 0)
          ;
      return((char)(UART0_DR_R & 0xFF));
}

/* UART_OutChar
* Output 8-bit to serial port
* Input: letter is an 8-bit ASCII character to be transferred
* Output: none
*/
void UART_OutChar(char data)
{
      while((UART0_FR_R & UART_FR_TXFF) != 0)
          ;
      UART0_DR_R = data;
}

char process_input(char str_arr[100])
{
    int i = 0;
    int j = 0;
    int flag;
    char temp[6];
    char* msg;
    unsigned int val;
    if (!strcmp(str_arr,"help"))
    {
        return '8';
    }
    else
    {
        for (i=0,j=0;str_arr[i]!=SP;i++,j++)
            temp[j] = str_arr[i];
        temp[j] = '\0';
        if (!strcmp(temp,"led"))
        {
            for (i=i+1,j=0;str_arr[i]!=SP;i++,j++)
            {
                if (str_arr[i]=='\0')
                    break;
                temp[j] = str_arr[i];
            }
            temp[j] = '\0';
            if (!strcmp(temp,"on"))
            {
                for (i=i+1,j=0;str_arr[i]!='\0';i++,j++)
                    temp[j] = str_arr[i];
                temp[j] = '\0';
                flag = 0;
                for (i=0;i<5;i++)
                {
                    if (!strcmp(temp,colour_arr[i]))
                    {
                        colour_reg = colour_val[i];
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1)
                    return '1';
                else
                    return '0';
            }
            else if (!strcmp(temp,"off"))
            {
                return '2';
            }
            else if (!strcmp(temp,"blink"))
            {
                for (i=i+1,j=0;str_arr[i]!='\0';i++,j++)
                    temp[j] = str_arr[i];
                temp[j] = '\0';
                flag = 0;
                for (i=0;i<5;i++)
                {
                    if (!strcmp(temp,colour_arr[i]))
                    {
                        colour_reg = colour_val[i];
                        flag = 1;
                        break;
                    }
                }
                if (flag == 1)
                    return '3';
                else
                    return '0';
            }
            else
            {
                return '0';
            }
        }
        else if (!strcmp(temp,"read"))
        {
            for (i=i+1,j=0;str_arr[i]!=SP;i++,j++)
            {
                if (str_arr[i]=='\0')
                    break;
                temp[j] = str_arr[i];
            }
            temp[j] = '\0';
            if (!strcmp(temp,"__str1") || !strcmp(temp,"__str2") || !strcmp(temp,"__str3"))
            {
                msg = command_help;
                while( *msg)
                    UART_OutChar(*msg++);
                if (!strcmp(temp,"__str1"))
                    no_of_bytes_read = 1;
                else if (!strcmp(temp,"__str2"))
                    no_of_bytes_read = 2;
                else if (!strcmp(temp,"__str3"))
                    no_of_bytes_read = 3;
                return '4';
            }
            else if (strlen(temp)==8)
            {
                address = 0;
                val = 0;
                for (j=0;temp[j]!='\0';j++)
                {
                    if (temp[j]=='A')
                    {
                        val = 0x0000000A;
                    }
                    else if (temp[j]=='B')
                    {
                        val = 0x0000000B;
                    }
                    else if (temp[j]=='C')
                    {
                        val = 0x0000000C;
                    }
                    else if (temp[j]=='D')
                    {
                        val = 0x0000000D;
                    }
                    else if (temp[j]=='E')
                    {
                        val = 0x0000000E;
                    }
                    else if (temp[j]=='F')
                    {
                        val = 0x0000000F;
                    }
                    else
                    {
                        val = temp[j]-'0';
                    }
                    address = (address<<4)|((unsigned int)val);
                }
                for (i=i+1,j=0;str_arr[i]!='\0';i++,j++)
                    temp[j] = str_arr[i];
                temp[j] = '\0';
                no_of_bytes_read = 0;
                val = 0;
                for (j=0;temp[j]!='\0';j++)
                {
                    val = temp[j]-'0';
                    no_of_bytes_read = no_of_bytes_read*10 + val;
                }
                return '5';
            }
            else
            {
                return '0';
            }
        }
        else if (!strcmp(temp,"write"))
        {
            return '5';
        }
        else
        {
            return '0';
        }
    }
}

void led_on()
{
    GPIO_PORTF_DATA_R = colour_reg; /* turn on LEDs */
}

void led_off()
{
    GPIO_PORTF_DATA_R = 0;     /* turn off all LEDs */
}

void led_blink()
{
    GPIO_PORTF_DATA_R = colour_reg; /* turn on all LEDs */
    delayMs(500);
    GPIO_PORTF_DATA_R = 0;     /* turn off all LEDs */
    delayMs(500);
}

void mem_read(int bit)
{
    int i;
    int j;
    char mem_output;
    int* ptr;
    if (bit==0)
    {
        //linker variable
    }
    else if (bit==1)
    {
        ptr = address;
        //Read memory address
        for (i=1;i<=no_of_bytes_read*2;i++)
        {
            mem_output = (((unsigned int)(*ptr))>>(((i-1)%8)*4))&(0x0000000F);
            if (mem_output<0x0000000A)
                mem_output = mem_output + '0';
            else
                mem_output = mem_output + (char)55;
            UART_OutChar((unsigned char)mem_output);
            if (i%2==0)
                UART_OutChar(' ');
            if (i%8==0)
            {
                ptr = ptr+1;
            }
        }
    }
}

void mem_write(int bit)
{
    int i;
    if (bit==0)
    {
        //linker variable
    }
    else if (bit==1)
    {
        //Read memory address
        for (i=0;i<no_of_bytes_read;i++)
        {

        }
    }
}

/* delay n milliseconds (16 MHz CPU clock) */
void delayMs(int n)
{
    int i, j;
    for(i = 0 ; i < n; i++)
        for(j = 0; j < 3180; j++){} /* do nothing for 1 ms */
}

/* delay n milliseconds (16 MHz CPU clock) */
void help()
{
    char* msg;
    msg = command_help;
    while( *msg)
        UART_OutChar(*msg++);
}
