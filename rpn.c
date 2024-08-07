/*
	Color Computer RPN calculator

	Features Motorola's 6839 floating point ROM

	tim lindner
	December 2020
*/

#include <coco.h>
#include "fp09.h"

/* Define SINGLE to use single precision */
#ifdef DOUBLE
typedef fp09_double Float;
#define FLOAT_TYPE_ID fp09_double_type
#else
typedef fp09_single Float;
#define FLOAT_TYPE_ID fp09_single_type
#endif

byte is_numeric( byte x );
void print_float( Float *d );
void stack_input_buffer();
void draw_stack();
void move_stack_up();
void check_error( fp09_FPCB *cb );
void go_help();

Float stack[8];
byte input_buffer[31];
byte blank;
byte decimal_point;
fp09_FPCB fpcb = {FLOAT_TYPE_ID, 0, 0, 0, trap_6839};

int main()
{
	go_help();

	byte c, x;

	// blank input buffer
	blank = TRUE;
	for( c=0; c<31; c++ )
	{
		input_buffer[c] = ' ';
	}

	decimal_point = FALSE;

	locate(0, 15);
	printf("SIZE OF FLOAT TYPE: %u BYTES", sizeof(Float));

	draw_stack();

	while( x != 'E' )
	{
		locate( 0, 0 );
		for( c=0; c<31; c++ )
		{
			putchar(input_buffer[c]);
		}

		// draw stack
		if( blank == TRUE )
		{
			draw_stack();
		}

		// wait for input
		check_error( &fpcb );
		locate(31,0);
		x = waitkey(TRUE);

		if( is_numeric(x) ) // typing a number
		{
			if( is_numeric(input_buffer[1]) ) continue;

			if( decimal_point == TRUE && x == '.' ) continue;

			blank = FALSE;
			for( c=1; c<31; c++ )
			{
				input_buffer[c-1] = input_buffer[c];
			}

			input_buffer[30] = x;

			if( x == '.' ) decimal_point = TRUE;

		}
		else if( x == '-' ) // negate value in buffer
		{
			for( c=1; c<31; c++ )
			{
				if( is_numeric(input_buffer[c]) )
				{
					if( input_buffer[c-1] == '-')
					{
						input_buffer[c-1] = ' ';
					}
					else
					{
						input_buffer[c-1] = '-';
					}

					break;
				}
			}

			if( c == 32 )
			{
				if( input_buffer[31] == '-')
				{
					input_buffer[31] = ' ';
				}
				else
				{
					input_buffer[31] = '-';
				}
			}
		}
		else if( x == 13 ) // enter value in stack
		{
			// signify operation
			putchar( '=' );

			stack_input_buffer();
		}
		else if (x == 'A' ) // addition
		{
			// signify operation
			putchar( '+' );

			if( blank == FALSE ) stack_input_buffer();
			fp09_FADD( &fpcb, stack[1], stack[0], stack[0] );
			move_stack_up();
		}
		else if (x == 'S' ) // Subtraction
		{
			// signify operation
			putchar( '-' );

			if( blank == FALSE ) stack_input_buffer();
			fp09_FSUB( &fpcb, stack[1], stack[0], stack[0] );

			move_stack_up();
		}
		else if (x == 'M' ) // Multiply
		{
			// signify operation
			putchar( '*' );

			if( blank == FALSE ) stack_input_buffer();
			fp09_FMUL( &fpcb, stack[1], stack[0], stack[0] );

			move_stack_up();
		}
		else if (x == 'D' ) // Divide
		{
			// signify operation
			putchar( '/' );

			if( blank == FALSE ) stack_input_buffer();
			fp09_FDIV( &fpcb, stack[1], stack[0], stack[0] );
			move_stack_up();
		}
		else if (x == 'O' )
		{
			// shift stack up
			for( c=0; c<7; c++ )
			{
				memcpy(stack[c], stack[c+1], sizeof(Float));
			}

			memset( stack[7], 0, sizeof(Float) );
		}
		else if( x=='W' )
		{
			Float temp;

			memcpy( temp, stack[0], sizeof(Float));
			memcpy( stack[0], stack[1], sizeof(Float));
			memcpy( stack[1], temp, sizeof(Float));
		}
		else if( x=='Q' )
		{
			// signify operation
			putchar( 'Q' );

			if( blank == FALSE ) stack_input_buffer();
			fp09_FSQRT( &fpcb, stack[0], stack[0] );
		}
		else if (x == 8 ) // delete key;
		{
			if( input_buffer[30] == '.' )
			{
				decimal_point = FALSE;
			}

			for( c=30; c > 0; c-- )
			{
				input_buffer[c] = input_buffer[c-1];
			}

			if( input_buffer[30] == ' ' )
			{
				blank = TRUE;
			}

			input_buffer[0] = ' ';
		}
		else if ( x == '?' )
		{
			go_help();
		}
	}

	return 0;
}

byte is_numeric( byte x )
{
	if( x >= '0' && x <= '9' )
	{
		return TRUE;
	}

	if( x == '.' )
	{
		return TRUE;
	}

	return FALSE;
}

void trap_6839()
{
	char trap_type;

	asm
	{
		sta trap_type
	}

	printf( "trap: %d\n", trap_type );
}

void print_float( Float *d )
{
	byte c;

	fp09_bcd result;
	memset( &result, 0, sizeof(fp09_bcd) );
	fp09_FPCB cb = {FLOAT_TYPE_ID, 0, 0, 0, trap_6839};
	fp09_BINDEC( &cb, 19, d, &result );


	if( result.exp_sign == 0x0c )
	{
		printf( "NAN                            " );
		return;
	}

	else if( result.exp_sign == 0x0a )
	{
		printf( "+INFINITY                      " );
		return;
	}
	else if( result.exp_sign == 0x0b )
	{
		printf( "-INFINITY                      " );
		return;
	}

	if( result.fraction_sign == 0 )
	{
		putchar( '+' );
	}
	else
	{
		putchar( '-' );
	}

	int exp = (result.exp[0] * 1000) + (result.exp[1] * 100) + (result.exp[2] * 10) + result.exp[3];
	exp *= (result.exp_sign == 0 ? 1 : -1 );
	int position = 18 + exp;

	if( position < 2 )
	{
		position = 2;
		exp += 16;
	}
	else if( position > 18 )
	{
		position = 2;
		exp += 18;
	}
	else
	{
		exp = 0;
	}

	for( c=0; c<19; c++ )
	{
		putchar( result.fraction[c] + '0' );
		if( position == c ) putchar( '.' );
	}

	if( exp != 0 )
	{
		putchar( ' ' );
		putchar( 'E' );

		printf( "%d", exp );
	}
	else
	{
		putchar( ' ' );
		putchar( ' ' );
		putchar( ' ' );
		putchar( ' ' );
		putchar( ' ' );
		putchar( ' ' );
	}

	putchar( ' ' );
	putchar( ' ' );
	putchar( ' ' );
}

void stack_input_buffer()
{
	Float result;
	sbyte i;
	byte c;
// 	fp09_bcd bcd = {0,{0,0,0,0},0x00,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0};
	fp09_bcd bcd;
	memset(&bcd, 0, sizeof(fp09_bcd));

	// stuff ascii buffer into bcd array
	c = 30;
	for( i=18; i>-1; i-- )
	{
		if( input_buffer[c] == '-' )
		{
			bcd.fraction_sign = 0x0f;
		}
		else if( input_buffer[c] == '.' )
		{
			bcd.fraction_digits = 30-c;
			i++;
		}
		else
		{
			bcd.fraction[i] =  input_buffer[c] - '0';
		}

		c--;
	}

	fp09_DECBIN( &fpcb, &bcd, result);

	if( (fpcb.status & ~fp09_status_inexact_result) == 0 )
	{
		// shift stack down
		for( i=7; i>-1; i-- )
		{
			memcpy(stack[i+1], stack[i], sizeof(Float));
		}

		memcpy(stack[0], result, sizeof(Float));

		// blank input buffer
		blank = TRUE;
		for( c=0; c<31; c++ )
		{
			input_buffer[c] = ' ';
		}

		decimal_point = FALSE;

		locate(1,0);
		printf( "                              ");

	}
}

void draw_stack()
{
	byte c;

	for(c=0; c<8; c++ )
	{
		locate( 0, c+3 );
		print_float( stack[c] );
	}
}


void move_stack_up()
{
	byte c;

	// shift stack up
	for( c=1; c<7; c++ )
	{
		memcpy(stack[c], stack[c+1], sizeof(Float));
	}

	memset( stack[7], 0, sizeof(Float) );
}

void check_error( fp09_FPCB *cb )
{
	locate(0,1);
 	printf( "ERROR: ");
    if( cb->status == 0 )
    {
        printf( "NONE\n" );
    }
    else
    {
        if( cb->status & fp09_status_inexact_result) printf( "INEXACT RESULT ");
        if( cb->status & fp09_status_undefined) printf( "UNDEFINED ");
        if( cb->status & fp09_status_integer_overflow) printf( "INT OVERFLOW ");
        if( cb->status & fp09_status_unordered) printf( "UNORDERED ");
        if( cb->status & fp09_status_division_zero) printf( "DIV BY 0 ");
        if( cb->status & fp09_status_underflow) printf( "UNDERFLOW ");
        if( cb->status & fp09_status_overflow) printf( "OVERFLOW ");
        if( cb->status & fp09_status_invalid_operation) printf( "INVALID ");
        printf("\n");
    }

	cb->status = 0;
	cb->secondary_status = 0;
}

void go_help()
{
	cls(255);
	printf( "COLOR COMPUTER RPN CALCULATOR\n");
	printf( "MATH ROUTINES PROVIDED BY THE   MOTOROLA 6839\n");
	printf( "BY TIM LINDNER, DECEMBER 2020\n\n");
	printf( "USAGE: TYPE IN VALUE AND <ENTER>THEM ON TO THE STACK\n");
	printf( "THEN PRESS A KEY TO OPERATE ON  THE TOP OF THE STACK:\n");
	printf( "A - ADDITION S - SUBTRACTION\n");
	printf( "M - MULTIPLY D - DIVISION\n");
	printf( "Q - SQUARE ROOT\n");
	printf( "W - SWAP O - DROP\n");
	printf( "? - HELP E - EXIT\n");
	waitkey(TRUE);
	cls(255);
}
