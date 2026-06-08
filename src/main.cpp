#include <Arduino.h>
#include <TM1637Display.h>

// ############################
//
//  updates: remove 'reset'
//           just flash lose
//
//  move 'odds' of skipping
//    spy mode to top of code
//
// ###########################

// odds of skipping spy mode
//    1/X
const int X = 4;

// Module connection pins (Digital Pins)
#define CLK A2
#define DIO A3

// The amount of time (in milliseconds) between tests
#define countdown_delay 1000

const int button = A0;
const int vibra = 4;
const int buzzer = 6;

void ( *resetFunc )( void ) = 0;
TM1637Display display( CLK, DIO );
uint8_t data[] = { 0, 0, 0, 0 };
static int count = 30;
static int spy = 5;
bool spyflag = true;

void
blankDISPLAY( )
{
    const uint8_t blank[] = { 0, 0, 0, 0 };
    display.setSegments( blank );
}

void
loseDISPLAY( )
{
    const uint8_t lose[] = { 0X38, 0X3F, 0X6D, 0X79 };
    display.setSegments( lose );
}

void
rstDISPLAY( )
{
    const uint8_t reset[] = { 0X00, 0X50, 0X6D, 0X78 };
    display.setSegments( reset );
}

//***********************************
// Execute active game play
// Return true on timeout
//***********************************
void
playGame( )
{
    while ( digitalRead( A0 ) == LOW )
    {
        // update display
        data[ 3 ] = display.encodeDigit( count / 1 % 10 );
        data[ 2 ] = display.encodeDigit( count / 10 % 10 );
        //    data[1] = display.encodeDigit(count / 100 % 10);
        //    data[0] = display.encodeDigit(count / 1000 % 10);
        display.setSegments( data );
        // check for spy time
        if ( ( count == spy ) && ( spyflag ) )
        {
            Serial.println( "Spy Peek" );
            delay( 100 );
            digitalWrite( buzzer, HIGH );
            delay( 100 );
            digitalWrite( buzzer, LOW );
            delay( 200 );
            digitalWrite( buzzer, HIGH );
            delay( 100 );
            digitalWrite( buzzer, LOW );
            delay( 200 );
            digitalWrite( buzzer, HIGH );
            delay( 100 );
            digitalWrite( buzzer, LOW );
            delay( 200 );
            --count;
            // update display
            data[ 3 ] = display.encodeDigit( count / 1 % 10 );
            data[ 2 ] = display.encodeDigit( count / 10 % 10 );
            //      data[1] = display.encodeDigit(count / 100 % 10);
            //      data[0] = display.encodeDigit(count / 1000 % 10);
            display.setSegments( data );
        }

        // Here is time has run out!
        else
        {
            if ( count <= 0 )
            {
                Serial.println( "Boom!" );
                digitalWrite( vibra, HIGH );
                digitalWrite( buzzer, HIGH );
                delay( 1000 );
                loseDISPLAY( );
                digitalWrite( buzzer, LOW );
                digitalWrite( vibra, LOW );
                delay( 500 );
                digitalWrite( buzzer, HIGH );
                digitalWrite( vibra, HIGH );
                delay( 1000 );
                digitalWrite( buzzer, LOW );
                digitalWrite( vibra, LOW );
                delay( 500 );
                digitalWrite( buzzer, HIGH );
                digitalWrite( vibra, HIGH );
                delay( 1000 );
                digitalWrite( buzzer, LOW );
                digitalWrite( vibra, LOW );
                delay( 4000 );
                while ( digitalRead( A0 ) == LOW )
                {
                    delay( 100 );
                    blankDISPLAY( );
                    delay( 300 );
                    loseDISPLAY( );
                }
                return;
            }
        }
        if ( !( ( count == spy ) && ( spyflag ) ) )
        {
            delay( countdown_delay );
            --count;
        }
    }
}

void
setup( )
{
    display.setBrightness( 7 );
    pinMode( button, INPUT );
    pinMode( buzzer, OUTPUT );
    pinMode( vibra, OUTPUT );

    //  TESTING ONLY
    Serial.begin( 9600 );  // To debug search and replace (Ctl-F) "////Serial." with "//Serial." or
                           // reverse...
    Serial.println( "Program start." );
}

void
loop( )
{
    blankDISPLAY( );

    if ( digitalRead( A0 ) == LOW )
    {
        delay( 30 );
        Serial.println( "button release" );
        randomSeed( analogRead( A6 ) + millis( ) );
        count = random( 15, 30 );
        spy = random( count );
        if ( random( 0, X ) == 0 )
            spyflag = false;
        else
            spyflag = true;
        Serial.print( "spy flag " );
        Serial.println( spyflag );
        Serial.print( "spy time " );
        Serial.println( spy );

        // start game
        playGame( );
        delay( 30 );
    }
}
