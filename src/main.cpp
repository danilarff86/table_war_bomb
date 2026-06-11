#include <Arduino.h>
#define _SS_MAX_RX_BUFF 64
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
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
// DFPlayer Mini serial pins
const int DF_RX_PIN = 7;  // Arduino RX ← DFPlayer TX
const int DF_TX_PIN = 6;  // Arduino TX → DFPlayer RX (via 1kΩ resistor)

// SD card sound file indices in folder /01/
const int GAME_SOUND_FOLDER = 1;
const int SOUND_SPY = 1;   // 001.mp3 – three-beep spy peek
const int SOUND_BOOM = 2;  // 002.mp3 – explosion

// Meme sounds: folder /02/ on SD card
const int SOUND_MEME_FOLDER = 2;
int memeFileCount = 0;  // populated in setup() via readFileCountsInFolder()

void ( *resetFunc )( void ) = 0;

// Module connection pins (Digital Pins)
const auto CLK = A2;
const auto DIO = A3;
TM1637Display display( CLK, DIO );
SoftwareSerial dfSerial( DF_RX_PIN, DF_TX_PIN );
DFRobotDFPlayerMini dfPlayer;

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
printDFPlayerDetail( uint8_t type, int value )
{
    switch ( type )
    {
    case TimeOut:
        Serial.println( "DF: timed out (no response)" );
        break;
    case DFPlayerCardInserted:
        Serial.println( "DF: SD inserted" );
        break;
    case DFPlayerCardRemoved:
        Serial.println( "DF: SD removed" );
        break;
    case DFPlayerCardOnline:
        Serial.println( "DF: SD online" );
        break;
    case DFPlayerPlayFinished:
        Serial.print( "DF: finished file #" );
        Serial.println( value );
        break;
    case DFPlayerError:
        Serial.print( "DF error: " );
        switch ( value )
        {
        case Busy:
            Serial.println( "no SD / busy" );
            break;
        case Sleeping:
            Serial.println( "sleeping" );
            break;
        case FileIndexOut:
            Serial.println( "file index out of range" );
            break;
        case FileMismatch:
            Serial.println( "file not found" );
            break;
        default:
            Serial.println( value );
            break;
        }
        break;
    default:
        break;
    }
}

void
playMeme( int track )
{
    Serial.print( "Meme track: " );
    Serial.println( track );
    dfPlayer.playFolder( SOUND_MEME_FOLDER, track );
    delay( 200 );  // let the player start before polling
    unsigned long start = millis( );
    while ( millis( ) - start < 30000UL )
    {
        if ( dfPlayer.available( ) )
        {
            uint8_t type = dfPlayer.readType( );
            if ( type == DFPlayerPlayFinished )
                break;
            printDFPlayerDetail( type, dfPlayer.read( ) );
        }
    }
}

void
spyPeek( )
{
    Serial.println( "Spy Peek" );
    dfPlayer.playFolder( GAME_SOUND_FOLDER, SOUND_SPY );
    delay( 1000 );
}

void
boom( )
{
    Serial.println( "Boom!" );
    loseDisplay( );
    dfPlayer.playFolder( GAME_SOUND_FOLDER, SOUND_BOOM );
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
    digitalWrite( VIBRA_PIN, LOW );
    delay( 500 );
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
    digitalWrite( VIBRA_PIN, LOW );
    delay( 500 );
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
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

bool
buttonReleased( )
{
    return digitalRead( BUTTON_PIN ) == HIGH;
}

//***********************************
// Execute active game play
// Return true on timeout
//***********************************
void
playGame( int count = MAX_COUNT, int spy = 5, bool spyflag = true )
{
    while ( buttonReleased( ) )
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
                while ( buttonReleased( ) )
                {
                    blinkLose( );
                }
                if ( memeFileCount > 0 )
                {
                    playMeme( random( 1, memeFileCount + 1 ) );
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
    pinMode( BUTTON_PIN, INPUT_PULLUP );
    pinMode( VIBRA_PIN, OUTPUT );
    randomSeed( analogRead( RANDOM_SEED_ANALOG_PIN ) + millis( ) );

    //  TESTING ONLY
    Serial.begin( 9600 );  // To debug search and replace (Ctl-F) "////Serial." with "//Serial." or
                           // reverse...
    Serial.println( "Program start." );

    dfSerial.begin( 9600 );
    delay( 1000 );  // YX5200 needs ~1 s to boot before accepting commands
    // second arg false = no ACK; YX5200 clones often don't send one
    if ( !dfPlayer.begin( dfSerial, false ) )
    {
        Serial.println( "DFPlayer init failed!" );
        while ( true )
            ;
    }
    dfPlayer.volume( 25 );  // 0–30

    Serial.print( "Volume readback : " );
    Serial.println( dfPlayer.readVolume( ) );
    Serial.print( "Files on SD     : " );
    Serial.println( dfPlayer.readFileCounts( ) );
    Serial.print( "Folders on SD   : " );
    Serial.println( dfPlayer.readFolderCounts( ) );
    memeFileCount = dfPlayer.readFileCountsInFolder( SOUND_MEME_FOLDER );
    Serial.print( "Meme files (/02/): " );
    Serial.println( memeFileCount );
    delay( 200 );

    Serial.println( "--- Test 1: playFolder(1,1) ---" );
    dfPlayer.playFolder( GAME_SOUND_FOLDER, SOUND_SPY );
    delay( 3000 );
    dfPlayer.stop( );
    delay( 300 );

    Serial.println( "DFPlayer ready." );

    // flash vibration motor to indicate ready
    digitalWrite( VIBRA_PIN, HIGH );
    delay( 1000 );
    digitalWrite( VIBRA_PIN, LOW );

    // blink display to indicate ready
    for ( int i = 0; i < 3; ++i )
    {
        const uint8_t allSegments[] = { 0X7F, 0XFF, 0X7F, 0X7F };
        display.setSegments( allSegments );  // all segments on
        delay( 300 );
        blankDisplay( );
        delay( 300 );
    }
}

void
loop( )
{
    if ( dfPlayer.available( ) )
        printDFPlayerDetail( dfPlayer.readType( ), dfPlayer.read( ) );

    blankDisplay( );

    if ( buttonReleased( ) )
    {
        delay( 30 );
        Serial.println( "button release" );
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
