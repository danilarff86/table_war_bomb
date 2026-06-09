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
// The maximum count for the game
const int MAX_COUNT = 30;
// The minimum count for the game
const int MIN_COUNT = 15;

// The amount of time (in milliseconds) between tests
const int COUNTDOWN_DELAY = 1000;
// The analog pin used to generate a random seed
const int RANDOM_SEED_ANALOG_PIN = A6;
// The button pin
const int BUTTON_PIN = A0;
// The vibration motor pin
const int VIBRA_PIN = 4;
// The buzzer pin
const int BUZZER_PIN = 6;

void ( *resetFunc )( void ) = 0;

// Module connection pins (Digital Pins)
const auto CLK = A2;
const auto DIO = A3;
TM1637Display display( CLK, DIO );

void
blankDisplay( )
{
    const uint8_t blank[] = { 0, 0, 0, 0 };
    display.setSegments( blank );
}

void
loseDisplay( )
{
    const uint8_t lose[] = { 0X38, 0X3F, 0X6D, 0X79 };
    display.setSegments( lose );
}

void
rstDisplay( )
{
    const uint8_t reset[] = { 0X00, 0X50, 0X6D, 0X78 };
    display.setSegments( reset );
}

void
spyPeek( )
{
    Serial.println( "Spy Peek" );
    delay( 100 );
    digitalWrite( BUZZER_PIN, HIGH );
    delay( 100 );
    digitalWrite( BUZZER_PIN, LOW );
    delay( 200 );
    digitalWrite( BUZZER_PIN, HIGH );
    delay( 100 );
    digitalWrite( BUZZER_PIN, LOW );
    delay( 200 );
    digitalWrite( BUZZER_PIN, HIGH );
    delay( 100 );
    digitalWrite( BUZZER_PIN, LOW );
    delay( 200 );
}

void
boom( )
{
    Serial.println( "Boom!" );
    loseDisplay( );
    digitalWrite( VIBRA_PIN, HIGH );
    digitalWrite( BUZZER_PIN, HIGH );
    delay( 1000 );
    digitalWrite( BUZZER_PIN, LOW );
    digitalWrite( VIBRA_PIN, LOW );
    delay( 500 );
    digitalWrite( BUZZER_PIN, HIGH );
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
    digitalWrite( BUZZER_PIN, LOW );
    digitalWrite( VIBRA_PIN, LOW );
    delay( 500 );
    digitalWrite( BUZZER_PIN, HIGH );
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
    digitalWrite( BUZZER_PIN, LOW );
    digitalWrite( VIBRA_PIN, LOW );
    delay( 4000 );
}

void
blinkLose( )
{
    delay( 100 );
    blankDisplay( );
    delay( 300 );
    loseDisplay( );
}

//***********************************
// Execute active game play
// Return true on timeout
//***********************************
void
playGame( int count = MAX_COUNT, int spy = 5, bool spyflag = true )
{
    while ( digitalRead( BUTTON_PIN ) == LOW )
    {
        // update display
        display.showNumberDec( count, false );
        // check for spy time
        if ( ( count == spy ) && ( spyflag ) )
        {
            spyPeek( );
            --count;
            // update display
            display.showNumberDec( count, false );
        }

        // Here is time has run out!
        else
        {
            if ( count <= 0 )
            {
                boom( );
                while ( digitalRead( BUTTON_PIN ) == LOW )
                {
                    blinkLose( );
                }
                return;
            }
        }
        if ( !( ( count == spy ) && ( spyflag ) ) )
        {
            delay( COUNTDOWN_DELAY );
            --count;
        }
    }
}

void
setup( )
{
    display.setBrightness( 7 );
    pinMode( BUTTON_PIN, INPUT );
    pinMode( BUZZER_PIN, OUTPUT );
    pinMode( VIBRA_PIN, OUTPUT );

    //  TESTING ONLY
    Serial.begin( 9600 );  // To debug search and replace (Ctl-F) "////Serial." with "//Serial." or
                           // reverse...
    Serial.println( "Program start." );
}

void
loop( )
{
    blankDisplay( );

    if ( digitalRead( BUTTON_PIN ) == LOW )
    {
        delay( 30 );
        Serial.println( "button release" );
        randomSeed( analogRead( RANDOM_SEED_ANALOG_PIN ) + millis( ) );
        const int count = random( MIN_COUNT, MAX_COUNT );
        const int spy = random( count );
        const bool spyflag = random( 0, X ) != 0;
        Serial.print( "spy flag " );
        Serial.println( spyflag );
        Serial.print( "spy time " );
        Serial.println( spy );

        // start game
        playGame( count, spy, spyflag );
        delay( 30 );
    }
}
