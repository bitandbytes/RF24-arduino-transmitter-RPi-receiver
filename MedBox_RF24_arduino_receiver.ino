
/*
* Getting Started example sketch for nRF24L01+ radios
 * This is a very basic example of how to send data from one node to another
 * Updated: Dec 2014 by TMRh20
 */

#include <SPI.h>
#include "RF24.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(A1,A0);
/**********************************************************/

/****************** Varibale Declarations *****************/
int wakePin = 2;                 // pin used for waking up
int sleepStatus = 0;             // variable to store a request for sleep
int count = 0;                   // counter
int debug_count = 0;             //Counter for debugging
int re_transmitCounter = 0;      //Counter overflows at 20
unsigned long packet = 0;        //Packet Number 
short int sleep_time= 15000;    //Counter for the clock (450 = 1h, 225 = 30 mins)
short int clock_count; 


byte addresses[][6] = {
  "1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 1; //If 1 a transmitter, if 0 receiver(pong back). 

/****************** Custom method deifinitions ***********/
// First method that will run after the wakeup
void wakeUpNow()        // here the interrupt is handled after wakeup
{

  WDTCSR &= ~(_BV(WDIE));     //Disable the watchdog
  
  Serial.println("\n **** Input Interrupt **** \n");
  
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we
  // just want the thing to wake up
}

//Will run when the watchdog timer overflows

ISR(WDT_vect)
{
  WDTCSR &= ~(_BV(WDIE));     //Disable the watchdog
  
  Serial.println("\n!!!! WDT Interrupt !!!! \n");
 
}

// Method that put Arduino in to sleep
void sleepNow()         // here we put the arduino to sleep
{ 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

  sleep_enable();          // enables the sleep bit in the mcucr register
  // so sleep is possible. just a safety pin

  attachInterrupt(0,wakeUpNow, RISING); // use interrupt 0 (pin 2) and run function
  // wakeUpNow when pin 2 is at RISING
  WDTCSR |= _BV(WDIE);     //Start the watch dog

  sleep_mode();            // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  
  sleep_disable();         // first thing after waking from sleep:
  // disable sleep...
  detachInterrupt(0);      // disables interrupt 0 on pin 2 so the
  // wakeUpNow code will not be executed
  // during normal running time.

}

/*void sleep_long(){
 while (milliseconds >= 8000) { sleepNow(WDTO_8S); milliseconds -= 8000; }
  if (milliseconds >= 4000)    { sleepNow(WDTO_4S); milliseconds -= 4000; }
  if (milliseconds >= 2000)    { sleepNow(WDTO_2S); milliseconds -= 2000; }
  if (milliseconds >= 1000)    { sleepNow(WDTO_1S); milliseconds -= 1000; }
  if (milliseconds >= 500)     { sleepNow(WDTO_500MS); milliseconds -= 500; }
  if (milliseconds >= 250)     { sleepNow(WDTO_250MS); milliseconds -= 250; }
  if (milliseconds >= 125)     { sleepNow(WDTO_120MS); milliseconds -= 120; }
  if (milliseconds >= 64)      { sleepNow(WDTO_60MS); milliseconds -= 60; }
  if (milliseconds >= 32)      { sleepNow(WDTO_30MS); milliseconds -= 30; }
  if (milliseconds >= 16)      { sleepNow(WDTO_15MS); milliseconds -= 15; } 
}*/

void setup() {
  
  clock_count = sleep_time;
  
  Serial.begin(115200);
  delay(100); //Remove the delay in the real operation
  Serial.println(F("MedBox Initializing..."));
  //Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }
  else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();

  /******** Confurations for the Sleep Modes *******************/

  attachInterrupt(0, wakeUpNow, RISING);   // use interrupt 0 (pin 2) and run function
  // wakeUpNow when pin 2 is at RISING   
  /*
  ////////////////////////////////////////////////////////////
  /*** Setup the WDT ***/
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE); 
  
  ///////////////////////////////////////////////////////
  

}

void loop() {


  /****************** Ping Out Role ***************************/
  if (role == 1)  {

    if (re_transmitCounter < 20){
      radio.stopListening();                                    // First, stop listening so we can talk.

      Serial.print(F("Now sending packet: "));
      Serial.println(debug_count);

                                   // Packet 1 of Radio 1
      if (!radio.write( &packet, sizeof(unsigned long) )){
        Serial.println(F("failed"));
      }
 
                                                      //Increment the packet number  
      radio.startListening();                                    // Now, continue listening

      unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
      boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not

      while ( ! radio.available() ){                             // While nothing is received
        if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
        }      
      }

      if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
        re_transmitCounter++;
        delay(1000);
      }
      else{
        unsigned long got_packet;                                 // Grab the response, compare, and send to debugging spew
        radio.read( &got_packet, sizeof(unsigned long) );
        unsigned long time = micros();                         //Not necessary

        // Spew it
        Serial.print(F("Sent "));
        Serial.print(packet);
        Serial.print(F(", Got response "));
        Serial.print(got_packet);
        Serial.print(F(", Round-trip delay "));
        Serial.print(time-started_waiting_at);
        Serial.println(F(" microseconds"));
        
        packet++;                                                 //Payload (Number is used for checking onl)
        re_transmitCounter = 0;                                   //Reset the re-transmitter counter
        
        radio.powerDown();                                        //Turn the low power mode of RF24
        
        //Entering in to the sleep mode
        Serial.println("Timer: Entering Sleep mode");
        delay(100);     // this delay is needed, the sleep
        Serial.flush();
        //function will provoke a Serial error otherwise!!
        sleepNow();     // sleep function called here
        
        radio.powerUp();                                         //Turn back to the Normal mode of RF24              
      }
      debug_count++;
      // Try again 1s later
      //delay(1000);
    }

  }



  /****************** Pong Back Role (Not required) ***************************
   * 
   * if ( role == 0 )
   * {
   * unsigned long got_time;
   * 
   * if( radio.available()){
   * // Variable for the received timestamp
   * while (radio.available()) {                                   // While there is data ready
   * radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
   * }
   * 
   * radio.stopListening();                                        // First, stop listening so we can talk   
   * radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
   * radio.startListening();                                       // Now, resume listening so we catch the next packets.     
   * Serial.print(F("Sent response "));
   * Serial.println(got_time);  
   * }
   * }
   */



  /****************** Change Roles via Serial Commands ***************************/

  /* if ( Serial.available() )
   {
   char c = toupper(Serial.read());
   if ( c == 'T' && role == 0 ){      
   Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
   role = 1;                  // Become the primary transmitter (ping out)
   
   }else
   if ( c == 'R' && role == 1 ){
   Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
   role = 0;                // Become the primary receiver (pong back)
   radio.startListening();
   
   }
   }*/


} // Loop


