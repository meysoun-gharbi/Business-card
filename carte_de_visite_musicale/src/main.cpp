#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>


#define SILENCE 0
#define NOTE_A0 3
#define NOTE_AS0 3
#define NOTE_B0 3
#define NOTE_C1 3
#define NOTE_CS1 4
#define NOTE_D1 4
#define NOTE_DS1 4
#define NOTE_E1 4
#define NOTE_F1 5
#define NOTE_FS1 5
#define NOTE_G1 5
#define NOTE_GS1 6
#define NOTE_A1 6
#define NOTE_AS1 6
#define NOTE_B1 7
#define NOTE_C2 7
#define NOTE_CS2 7
#define NOTE_D2 8
#define NOTE_DS2 8
#define NOTE_E2 9
#define NOTE_F2 9
#define NOTE_FS2 10
#define NOTE_G2 10
#define NOTE_GS2 11
#define NOTE_A2 12
#define NOTE_AS2 12
#define NOTE_B2 13
#define NOTE_C3 14
#define NOTE_CS3 15
#define NOTE_D3 16
#define NOTE_DS3 17
#define NOTE_E3 18
#define NOTE_F3 19
#define NOTE_FS3 20
#define NOTE_G3 21
#define NOTE_GS3 22
#define NOTE_A3 23
#define NOTE_AS3 25
#define NOTE_B3 26
#define NOTE_C4 28
#define NOTE_CS4 30
#define NOTE_D4 31
#define NOTE_DS4 33
#define NOTE_E4 35
#define NOTE_F4 37
#define NOTE_FS4 39
#define NOTE_G4 42
#define NOTE_GS4 44
#define NOTE_A4 47
#define NOTE_AS4 50
#define NOTE_B4 53
#define NOTE_C5 56
#define NOTE_CS5 59
#define NOTE_D5 63
#define NOTE_DS5 66
#define NOTE_E5 70
#define NOTE_F5 75
#define NOTE_FS5 79
#define NOTE_G5 84
#define NOTE_GS5 89
#define NOTE_A5 94
#define NOTE_AS5 99
#define NOTE_B5 105
#define NOTE_C6 112
#define NOTE_CS6 118
#define NOTE_D6 125
#define NOTE_DS6 133
#define NOTE_E6 141
#define NOTE_F6 149
#define NOTE_FS6 158
#define NOTE_G6 167
#define NOTE_GS6 177
#define NOTE_A6 188
#define NOTE_AS6 199
#define NOTE_B6 211
#define NOTE_C7 223
#define NOTE_CS7 237
#define NOTE_D7 251
#define NOTE_DS7 265
#define NOTE_E7 281
#define NOTE_F7 298
#define NOTE_FS7 316
#define NOTE_G7 335
#define NOTE_GS7 354
#define NOTE_A7 375
#define NOTE_AS7 398
#define NOTE_B7 421
#define NOTE_C8 447


#define RYTHM_PIN A2
#define NO_MUSIC_DELAY_IN_MS 1000
#define SIZE_OF_LUT 4 //lookup table size 
#define DELAY_UPDATE_TICK_IN_MS 125



const uint8_t square50LUT[SIZE_OF_LUT]= {

  0, 0, 85, 85
};


typedef struct 
{
  uint8_t index;
  uint16_t counter;
  uint16_t counter_step;
}Voice;

Voice simpleVoice;
Voice otherVoice;
Voice thirdVoice;

// notes in the melody:
const int melody[] PROGMEM = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4, NOTE_C4
};

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  
const uint8_t noteDurations[] PROGMEM = {

  4, 2, 2, 4, 4, 4, 4, 4, 4  //Duration: 28
};

// notes in the bassline:
const uint16_t bassline[] PROGMEM = {
  NOTE_C3, NOTE_G2, SILENCE, NOTE_G2, NOTE_B2};

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  
const uint8_t basslineDurations[] PROGMEM = {
  2, 3, 4, 5, 6}; //Duration: 28



const uint16_t chord[] PROGMEM = {

  NOTE_E4, NOTE_C3, SILENCE, NOTE_E3, SILENCE
};

const uint8_t chordDurations[] PROGMEM = {
  8, 2, 6, 4, 8}; //Duration: 28


void initVoice(Voice * voice, uint8_t counter_step){ 
  voice->index = 0;
  voice->counter = 0; 
  voice->counter_step = counter_step;
}

int main(void) {

  DDRB |= _BV(PB0);
  DDRB |= _BV(PB1);
  DDRB |= _BV(PB2);
 

 

  //set the comparison, we will flip the bit every time this is hit
  // (actually this will set TOP of the timer and we flip on the overflow )


  //Enable the overflow interrupt
  TIMSK0 |= _BV(TOIE0);

  //start the timer
  TCCR0A = 0b10000011; 
 
  
  sei(); //We have to enable interrupts here even if they were disabled coming in, otherwise its not going to do anything 

  initVoice(&otherVoice, NOTE_C4);



  while(1) {

    // iterate over the notes of the melody:

    //for (int thisNote = 0; thisNote < 8; thisNote++) {
      // Formula is: f(a) = (a/682) + 0.5 with the analog value, to have following specification
      // at a = 0, rythm_coeff = 0.5
      // at a = 1023, rythm_coeff = 2
      // To avoid the use of floating point, we multiply everything by 100, so we have:

        /*  uint16_t rythm_coeff = ((uint32_t)100*analogRead(RYTHM_PIN)/ 682) + 50;
          
          uint16_t notePitch = pgm_read_word(&melody[thisNote]); 
          uint16_t noteDuration = pgm_read_word(&noteDurations[thisNote]*rythm_coeff); 
          tone(PB0, notePitch, noteDuration);



          // to distinguish the notes, set a minimum time between them.

          // the note's duration + 30% seems to work well:

          int pauseBetweenNotes = noteDuration * 13/10;

          delay(pauseBetweenNotes);

          // stop the tone playing:

          noTone(PB0); */

      // Regarding to Default configuration & compilation: Internal clock (p.31), System clock prescaler (p.32), Register CLKPR default value (p.34),
      // Fclk TO = F_cpu = 9.6MHz. (Cf board build. f cpu in platformio. int file).
      // By setting PM prescaler (p.80) without prescaling (N=1). we have (Regarding the ATTiny13A datasheet p.72 Fast PUM formula) :
      // Fpum = Fclk IO / (N*256) 9.6MH / (1+256) # 37.5KH2
      TCCR0B |= 0b00000001;

      uint16_t melodyNoteDuration = 1; 
      uint16_t basslineNoteDuration = 1;
      uint16_t chordNoteDuration = 1;

      uint8_t melodyNote = 0;
      uint8_t basslineNote = 0; 
      uint8_t chordNote = 0;

      uint8_t melodyTempoCounter = 0;
      uint8_t basslineTempoCounter = 0;
      uint8_t chordTempoCounter = 0; 

      while(melodyNote < 8){
        if (melodyTempoCounter  % melodyNoteDuration ==0 ){
          uint8_t notePitch =pgm_read_byte(&melody[melodyNote]);
          melodyNoteDuration =pgm_read_byte(&noteDurations[melodyNote]);
          initVoice(&simpleVoice, notePitch);
          melodyTempoCounter = 0;
          melodyNote++;
        }
        if (basslineTempoCounter % basslineNoteDuration ==0 ){
          uint8_t notePitch =pgm_read_byte(&melody[basslineNote]);
          basslineNoteDuration =pgm_read_byte(&noteDurations[basslineNote]);
          initVoice(&otherVoice, notePitch);
          basslineTempoCounter = 0;
          basslineNote++;
        }
        if (chordTempoCounter % chordNoteDuration == 0){
          uint8_t notePitch =pgm_read_byte(&chord[chordNote]);
          basslineNoteDuration =pgm_read_byte(&chordDurations[chordNote]);
          initVoice(&thirdVoice, notePitch);
          chordTempoCounter = 0;
          chordNote++;
        }
        PINB = (1 << PB2);
        _delay_ms(DELAY_UPDATE_TICK_IN_MS - 36);
        melodyTempoCounter++;
        basslineTempoCounter++;
        chordTempoCounter++;
      }
    }
    

      TCCR0B &= ~0b00000111; 
      _delay_ms (NO_MUSIC_DELAY_IN_MS);
}


ISR(TIM0_OVF_vect){
  uint8_t cumulatedVoices = 0;
  PINB = (1 << PB1); //toggle the pin on overflow
  cumulatedVoices = square50LUT[simpleVoice.index];
  cumulatedVoices += square50LUT[otherVoice.index];
  cumulatedVoices += square50LUT[thirdVoice.index];


  OCR0A = cumulatedVoices;
  simpleVoice.counter+=simpleVoice.counter_step;


  if (simpleVoice.counter>= 1000){
    simpleVoice.counter -= 1000;
    simpleVoice.index++;
    simpleVoice.index %= SIZE_OF_LUT;
  }
  otherVoice.counter+=otherVoice.counter_step;


  if(otherVoice.counter>= 1000){
    otherVoice.counter -= 1000;
    otherVoice.index++;
    otherVoice.index %= SIZE_OF_LUT;
  }
  thirdVoice.counter += thirdVoice.counter_step;
  if(thirdVoice.counter>= 1000){
    thirdVoice.counter -= 1000;
    thirdVoice.index++;
    thirdVoice.index %= SIZE_OF_LUT;
  }
  // we want the same frenquency as previously when simpleVoice.index = (simpleVoice.counter * 2) /3;
  // our maximum is 1000 so we update our counter every 1000 * (2/3) -= 667 to have 625Hz.
  // To create the 625Hz frequency of 625Hz, we use 4 samples (size of LUT), so the signal frequency is divived by four.
  // So we have counter_sig  = 1000 * Nech * (Fsig/Fpwm) = (2/3) * 1000 ~=667.
  // To retrieve Fsig we have Fsig = (Fpwn * counter_sig ) / (1000 * Ntech)) = ((2/3) *1000*37.5Hz)
  // But, as we use the approximate value 667, we have Fsig = (667*37.5Hz) / (1000 * 4) = 6253.125Hz.
  PINB = (1 << PB1); //toggle the pin on overflow
}


