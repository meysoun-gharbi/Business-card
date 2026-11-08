#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#define SILENCE 0
#define TIMING_OFFSET 2
#define TIMING_STEP   2
#define NOTE_OFFSET   7
#define OCTAVE_OFFSET 3

#define NOTE_C0 0xFF
#define NOTE_CS0 0xFF
#define NOTE_D0 0xFF
#define NOTE_DS0 0xFF
#define NOTE_E0 0xFF
#define NOTE_F0 0xFF
#define NOTE_FS0 0xFF
#define NOTE_G0 0xFF
#define NOTE_GS0 0xFF
#define NOTE_A0 0xFF
#define NOTE_AS0 0xFF
#define NOTE_B0 0xFF
#define NOTE_C1 0xFF
#define NOTE_CS1 0xFF
#define NOTE_D1 0xFF
#define NOTE_DS1 0xFF
#define NOTE_E1 0xFF
#define NOTE_F1 0xFF
#define NOTE_FS1 0xFF
#define NOTE_G1 0xFF
#define NOTE_GS1 0xFF
#define NOTE_A1 0xFF
#define NOTE_AS1 0xFF
#define NOTE_B1 0xFF
#define NOTE_C2 0xFF
#define NOTE_CS2 0xFF
#define NOTE_D2 0xFF
#define NOTE_DS2 0xFF
#define NOTE_E2 0xFF
#define NOTE_F2 0xFF
#define NOTE_FS2 0xFF
#define NOTE_G2 0xFF
#define NOTE_GS2 0xFF
#define NOTE_A2 0xFF
#define NOTE_AS2 0xFF
#define NOTE_B2 0xFF
#define NOTE_C3 0xFF
#define NOTE_CS3 0xFF
#define NOTE_D3 0xFF
#define NOTE_DS3 0xFF
#define NOTE_E3 0xFF
#define NOTE_F3 0xFF
#define NOTE_FS3 0xFF
#define NOTE_G3 0x00
#define NOTE_GS3 0x10
#define NOTE_A3 0x20
#define NOTE_AS3 0x30
#define NOTE_B3 0x40
#define NOTE_C4 0x50
#define NOTE_CS4 0x60
#define NOTE_D4 0x70
#define NOTE_DS4 0x80
#define NOTE_E4 0x90
#define NOTE_F4 0xA0
#define NOTE_FS4 0xB0
#define NOTE_G4 0x04
#define NOTE_GS4 0x14
#define NOTE_A4 0x24
#define NOTE_AS4 0x34
#define NOTE_B4 0x44
#define NOTE_C5 0x54
#define NOTE_CS5 0x64
#define NOTE_D5 0x74
#define NOTE_DS5 0x84
#define NOTE_E5 0x94
#define NOTE_F5 0xA4
#define NOTE_FS5 0xB4
#define NOTE_G5 0x08
#define NOTE_GS5 0x18
#define NOTE_A5 0x28
#define NOTE_AS5 0x38
#define NOTE_B5 0x48
#define NOTE_C6 0x58
#define NOTE_CS6 0x68
#define NOTE_D6 0x78
#define NOTE_DS6 0x88
#define NOTE_E6 0x98
#define NOTE_F6 0xA8
#define NOTE_FS6 0xB8
#define NOTE_G6 0x0C
#define NOTE_GS6 0x1C
#define NOTE_A6 0x2C
#define NOTE_AS6 0x3C
#define NOTE_B6 0x4C
#define NOTE_C7 0x5C
#define NOTE_CS7 0x6C
#define NOTE_D7 0x7C
#define NOTE_DS7 0x8C
#define NOTE_E7 0x9C
#define NOTE_F7 0xAC
#define NOTE_FS7 0xBC
#define NOTE_G7 0xFF
#define NOTE_GS7 0xFF
#define NOTE_A7 0xFF
#define NOTE_AS7 0xFF
#define NOTE_B7 0xFF


#define RYTHM_PIN A2
#define NO_MUSIC_DELAY_IN_MS 1000
#define SIZE_OF_LUT 4 //lookup table size 
#define DELAY_UPDATE_TICK_IN_MS 125

#define PRECALCULATED_COUNTER_OCTAVE 6
#define TEMPO_FROM_NOTE_VALUE(Value)(((Value & 0x003) * TIMING_STEP) + TIMING_OFFSET)

#define NB_OF_VOICES 3

const uint8_t precalculated_counter [] PROGMEM = {
  112, 118, 125, 133, 141, 149, 158, 167, 177, 188, 199, 211,
};

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

  NOTE_E5 | 0, NOTE_E5 | 0, SILENCE | 0, NOTE_E5 | 0, SILENCE | 0, NOTE_C5 | 0, NOTE_E5 | 1, NOTE_G5 | 1, SILENCE | 1, NOTE_G4 | 1, 
  SILENCE | 1, NOTE_C4 | 1, SILENCE | 0, NOTE_G4 | 1, SILENCE | 0, NOTE_E4 | 1, SILENCE | 0, NOTE_A4 | 1, NOTE_B4 | 1, NOTE_AS4| 0,
  NOTE_A4 | 1, NOTE_G4 | 0, NOTE_E5 | 0, NOTE_G5 | 1, NOTE_A5 | 1, NOTE_F5 | 0, NOTE_G5 | 0, SILENCE | 0, NOTE_E5 | 1, NOTE_C5 | 0,
  NOTE_D5 | 0, NOTE_B4 | 1, SILENCE | 0, SILENCE | 1, NOTE_G5 | 0, NOTE_FS5| 0, NOTE_F5 | 0, SILENCE | 1, NOTE_E5 | 1, SILENCE | 0,
  NOTE_GS5| 0, NOTE_A4 | 0, NOTE_C5 | 0, SILENCE | 0, NOTE_A4 | 0, NOTE_C5 | 0, NOTE_D5 | 0, SILENCE | 1, NOTE_G5 | 1, NOTE_FS5 | 0,
  NOTE_F5 | 0, NOTE_DS5| 1, NOTE_E5 | 0, SILENCE | 0, NOTE_C6 | 1, NOTE_C6 | 0, NOTE_C6 | 1, SILENCE | 1,         
  SILENCE | 1, NOTE_G5 | 0, NOTE_FS5| 0, NOTE_F5 | 0, NOTE_DS6| 1, NOTE_E5 | 0, SILENCE | 0, SILENCE | 0, NOTE_A4 | 1, NOTE_C5 | 0,
  SILENCE | 0, NOTE_A4 | 0, NOTE_C5 | 0, NOTE_D5 | 0, SILENCE | 1, NOTE_G5 | 0, NOTE_FS5| 0, SILENCE | 0, NOTE_DS5| 1, NOTE_E5 | 0,
  SILENCE | 0, NOTE_E5 | 1, SILENCE | 1, SILENCE | 1, SILENCE | 1, SILENCE | 1, SILENCE | 1, SILENCE | 1, SILENCE | 1, 
};

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  

#if NB_OF_VOICES >=2
// notes in the bassline:
  
const uint8_t bassline[] PROGMEM = {
    NOTE_FS4, 0, NOTE_FS4, 0, SILENCE, 0, NOTE_FS4, 0, SILENCE, 0, NOTE_FS4, 0, NOTE_FS4, 1, NOTE_B4, 1, SILENCE, 1, NOTE_B4, 1,
    SILENCE, 1, NOTE_E4, 1, SILENCE, 0, NOTE_C4, 1, SILENCE, 0, NOTE_G3, 1, SILENCE, 0, NOTE_C4, 1, NOTE_D4, 1, NOTE_CS4, 0,
    NOTE_C4, 1, NOTE_C4, 0, NOTE_G4, 0, NOTE_B4, 1, NOTE_C5, 1, NOTE_A4, 0, NOTE_B4, 0, SILENCE, 0, NOTE_A4, 1, NOTE_E4, 0,
    NOTE_F4, 0, NOTE_D4, 1, SILENCE, 0, SILENCE, 1, NOTE_E5, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_B4, 1, NOTE_C5, 0, SILENCE, 0,
    NOTE_E4, 0, NOTE_F4, 0, NOTE_G4, 0, SILENCE, 0, NOTE_C4, 0, NOTE_E4, 0, NOTE_F4, 0, SILENCE, 1, NOTE_E5, 0, NOTE_DS5, 0,
    NOTE_D5, 0, NOTE_B4, 1, NOTE_C5, 0,
    SILENCE, 0, NOTE_G5, 1, NOTE_G5, 0, NOTE_G5, 1, SILENCE, 1, SILENCE, 0, NOTE_D5, 0, NOTE_DS5, 0, NOTE_D5,0,  NOTE_B4, 1,
    NOTE_C5, 0, SILENCE, 0, NOTE_E4, 0, NOTE_F4, 0, NOTE_G4, 0, SILENCE, 0, NOTE_C4, 0, NOTE_E4, 0, NOTE_F4, 0, SILENCE, 1,
    NOTE_GS4, 1, SILENCE, 0, NOTE_F4, 1, SILENCE, 0, NOTE_E4, 1, SILENCE, 3, SILENCE, 3, SILENCE, 3, SILENCE, 3
};
#endif

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  





#if NB_OF_VOICES >=3
const uint8_t chord[] PROGMEM = {
    NOTE_D3, 0, NOTE_D3, 0, SILENCE, 0, NOTE_D3, 0, SILENCE, 0, NOTE_D3, 0, NOTE_D3, 1, NOTE_G4, 1, SILENCE, 1, NOTE_G3, 1,
    SILENCE, 1, NOTE_G3, 1, SILENCE, 0, NOTE_E3, 1, SILENCE, 0, NOTE_C3, 0, SILENCE, 0, NOTE_F3, 1, NOTE_G3, 1, NOTE_FS3, 0,
    NOTE_G3, 1, NOTE_E3, 0, NOTE_C4, 0, NOTE_E4, 1, NOTE_F4, 1, NOTE_D4, 1, NOTE_E4, 0, SILENCE, 0, NOTE_C4, 1, NOTE_A3, 0,
    NOTE_B3, 0, NOTE_G3, 1, SILENCE, 0, NOTE_C3, 1, SILENCE, 0, NOTE_G3, 0, SILENCE, 1, NOTE_C4, 1, NOTE_F3, 1, SILENCE, 0,
    NOTE_C4, 0, NOTE_C4, 1, NOTE_F3, 1, NOTE_C3, 1, SILENCE, 0, NOTE_G3, 0, SILENCE, 1,
    NOTE_G3, 0, NOTE_C4, 0, SILENCE, 0, NOTE_F5, 1, NOTE_F5, 0, NOTE_F5, 0, NOTE_G3, 1, NOTE_C3, 1, SILENCE, 1, NOTE_G3, 0,
    SILENCE, 1, NOTE_C4, 1, NOTE_F3, 1, SILENCE, 0, NOTE_C4, 0, NOTE_C4, 0, NOTE_F3, 1, NOTE_C3, 1, NOTE_GS3, 1, SILENCE, 0,
    NOTE_AS3, 1, SILENCE, 0, NOTE_C4, 1, SILENCE, 0, NOTE_G3, 0, NOTE_G3, 0, NOTE_C3, 1, SILENCE, 1, SILENCE, 3, SILENCE, 3
};

#endif

const uint8_t chordDurations[] PROGMEM = {
  8, 2, 6, 4, 8}; //Duration: 28


void initVoice(Voice * voice, uint8_t note_value){ 
  voice->index = 0;
  voice->counter = 0; 

  if((note_value & SILENCE ) == SILENCE){
    voice->counter_step = 0;
  }
  else{
    uint8_t absolute_note_value = ((note_value & 0xF0) >> 4) + NOTE_OFFSET;
    uint8_t absolute_octave_value = ((note_value & 0x0C) >> 2) + OCTAVE_OFFSET;
    if(absolute_note_value >= 12){
      absolute_note_value %=12;
      absolute_octave_value++;

    }
    if(absolute_octave_value >= PRECALCULATED_COUNTER_OCTAVE){
       voice->counter_step =(uint16_t)pgm_read_byte(&precalculated_counter[absolute_note_value]) << (absolute_octave_value-PRECALCULATED_COUNTER_OCTAVE);
    }
    else {
      voice->counter_step =(uint16_t)pgm_read_byte(&precalculated_counter[absolute_note_value]) >> (PRECALCULATED_COUNTER_OCTAVE-absolute_octave_value);
    }
  }
}

int main(void) {

  DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);

  PORTB |= _BV(PB3);
 

 

  //set the comparison, we will flip the bit every time this is hit
  // (actually this will set TOP of the timer and we flip on the overflow )


  //Enable the overflow interrupt
  TIMSK0 |= _BV(TOIE0);

  //start the timer
  TCCR0A = 0b10000011; 
 
  
  sei(); //We have to enable interrupts here even if they were disabled coming in, otherwise its not going to do anything 





  while(1) {

    if(!(PINB & _BV(PB3))){
        TCCR0B |= 0b00000001;

        uint16_t melodyNoteDuration = 1; 
        uint8_t melodyNote = 0;
        uint8_t melodyTempoCounter = 0;
#if NB_OF_VOICES >=2
        uint16_t basslineNoteDuration = 1;
        uint8_t basslineNote = 0;
        uint8_t basslineTempoCounter = 0;
#endif
#if NB_OF_VOICES >=3
        uint16_t chordNoteDuration = 1;
        uint8_t chordNote = 0;
        uint8_t chordTempoCounter = 0; 
#endif

        while(melodyNote < sizeof(melody)){
          if (melodyTempoCounter  % melodyNoteDuration ==0 ){
            uint8_t noteValue = pgm_read_byte(&melody[melodyNote]);
            melodyNoteDuration = TEMPO_FROM_NOTE_VALUE(noteValue);
            initVoice(&simpleVoice, noteValue);
            melodyTempoCounter = 0;
            melodyNote++;
          }melodyTempoCounter++;

#if NB_OF_VOICES >=2
          if (basslineTempoCounter % basslineNoteDuration ==0 ){
            uint8_t noteValue =pgm_read_byte(&melody[basslineNote]);
            basslineNoteDuration = TEMPO_FROM_NOTE_VALUE(noteValue);
            initVoice(&otherVoice, noteValue);
            basslineTempoCounter = 0;
            basslineNote++;
          }basslineTempoCounter++;
#endif
#if NB_OF_VOICES >=2
          if (chordTempoCounter % chordNoteDuration == 0){
            uint8_t noteValue =pgm_read_byte(&chord[chordNote]);
            basslineNoteDuration = TEMPO_FROM_NOTE_VALUE(noteValue);
            initVoice(&thirdVoice, noteValue);
            chordTempoCounter = 0;
            chordNote++;
          }chordTempoCounter++;
#endif
          PINB = (1 << PB2);

          _delay_ms(DELAY_UPDATE_TICK_IN_MS - 22);
#if NB_OF_VOICES >=2
          _delay_ms(DELAY_UPDATE_TICK_IN_MS - 36);
#endif
#if NB_OF_VOICES >=3
          _delay_ms(DELAY_UPDATE_TICK_IN_MS - 48);
#endif 
          
        
        
      }
    }

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
      
      TCCR0B &= ~0b00000111;
  }
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


