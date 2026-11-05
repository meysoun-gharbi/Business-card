#include <Arduino.h>
#include <avr/pgmspace.h>


#define SILENCE 0
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define RYTHM_PIN A2
#define NO_MUSIC_DELAY_IN_MS 1000
#define SIZE_OF_LUT 4 //lookup table size 
#define DELAY_UPDATE_TICK_IN_MS 125

//counter_step = 1000 * Nech * (Fsig/Fpwm) = 1000 * Nech * (Fsig/(Fcpu/256))
#define CONVERT_FREQ_TO_COUNTER(NOTE_FREQ) \
 ( (uint16_t)((uint32_t)(NOTE_FREQ) * 1000UL * (uint32_t)(SIZE_OF_LUT) * 256UL / (uint32_t)(F_CPU)) )

const uint8_t square50LUT[SIZE_OF_LUT]= {

  0, 0, 127, 127
};


typedef struct 
{
  const uint8_t * shape;
  uint8_t shapeIndex;
  uint16_t counter;
  uint16_t counter_step;
  uint8_t tempoCounter;
  uint16_t currentNoteDuration;
  const int* melody;                
  const int* noteDurationTable;
  uint8_t noteIndex;


}Voice;

Voice simpleVoice;
Voice otherVoice;

// notes in the melody:
const int melody[] PROGMEM = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  
const int noteDurations[] PROGMEM = {

  4, 2, 2, 4, 4, 4, 4, 4 //Duration: 28
};

// notes in the bassline:
const int bassline[] PROGMEM = {
  NOTE_C3, NOTE_G2, SILENCE, NOTE_G2, NOTE_B2};

// note Durations: 4 = DELAY_UPDATE_TICK_IN_MS * 4 ~= 248 ~= quarter note (Crotchet),
//                 2 = DELAY_UPDATE_TICK_IN_MS * 2 ~= 124 ~= eighth note (Quaver),
//                 1 = DELAY_UPDATE_TICK_IN_MS * 1 ~= 125 ~= sixteenth note (Semiquaver),
  
const int basslineDurations[] PROGMEM = {
  2, 3, 4, 5, 6}; //Duration: 28

void initVoice(Voice * voice, const int* givenMelody, const int* givenNoteDurations){ 
  voice->shape = square50LUT;
  voice->shapeIndex = 0;
  voice->counter = 0; 
  voice->counter_step = 0;
  voice->tempoCounter = 0;
  voice->currentNoteDuration  = 1;
  voice->melody = givenMelody; 
  voice->noteDurationTable = givenNoteDurations;
  voice->noteIndex = 0;


}
void setNote (Voice * voice, uint16_t note_freq){
  voice->counter_step = CONVERT_FREQ_TO_COUNTER(note_freq);
  voice->shapeIndex = 0;
  voice->counter = 0; 
}

void setup() {

  DDRB |= _BV(PB0);
  DDRB |= _BV(PB1);

  //shut down interrupts while we fiddle about with the timer
  cli();

  TCCR0B &= ~0b00000111; //Turn off the timer before changing anything
  TCNT0 = 0; //Timer wounter back to 0

  //set the comparison, we will flip the bit every time this is hit
  // (actually this will set TOP of the timer and we flip on the overflow )

  OCR0A = 200;

  //Enable the overflow interrupt
  TIMSK0 |= _BV(TOIE0);

  //start the timer
  TCCR0A = 0b10000011; 
  TCCR0B = 0b00000000;
  
  sei(); //We have to enable interrupts here even if they were disabled coming in, otherwise its not going to do anything 

}
uint8_t tempo_counter = 0;

void loop() {


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

    initVoice(&simpleVoice, melody, noteDurations);
    initVoice(&otherVoice, bassline, basslineDurations);  

    while(simpleVoice.noteIndex < 8){
      if (simpleVoice.tempoCounter >= simpleVoice.currentNoteDuration){
        uint16_t notePitch =pgm_read_word(&simpleVoice.melody[simpleVoice.noteIndex]);
        simpleVoice.currentNoteDuration = pgm_read_word(&simpleVoice.noteDurationTable[simpleVoice.noteIndex]);
        setNote(&simpleVoice, notePitch);
        simpleVoice.tempoCounter = 0;
        simpleVoice.noteIndex++;
      }
      simpleVoice.tempoCounter++;

       if (otherVoice.tempoCounter >= otherVoice.currentNoteDuration){
        uint16_t notePitch =pgm_read_word(&otherVoice.melody[otherVoice.noteIndex]);
        otherVoice.currentNoteDuration = pgm_read_word(&otherVoice.noteDurationTable[otherVoice.noteIndex]);
        setNote(&otherVoice, notePitch);
        otherVoice.tempoCounter = 0;
        otherVoice.noteIndex++;
      }
      otherVoice.tempoCounter++;      
      delay(DELAY_UPDATE_TICK_IN_MS);
      
    }
   
    

    TCCR0B &= ~0b00000111; 
    delay (NO_MUSIC_DELAY_IN_MS);
}

uint8_t interuptCounter = 0;
ISR(TIM0_OVF_vect){
  uint8_t cumulatedVoices = 0;
  PINB = (1 << PB1); //toggle the pin on overflow
  cumulatedVoices = simpleVoice.shape[simpleVoice.shapeIndex];
  cumulatedVoices += otherVoice.shape[otherVoice.shapeIndex];

  if (cumulatedVoices == 0) //silence
  {
    OCR0A =0;
  }
  else{
    OCR0A = cumulatedVoices;
  }
  simpleVoice.counter+=simpleVoice.counter_step;


  if (simpleVoice.counter>= 1000){
    simpleVoice.counter -= 1000;
    simpleVoice.shapeIndex++;
    simpleVoice.shapeIndex %= SIZE_OF_LUT;
  }
  otherVoice.counter+=otherVoice.counter_step;


  if(otherVoice.counter>= 1000){
    otherVoice.counter -= 1000;
    otherVoice.shapeIndex++;
    otherVoice.shapeIndex %= SIZE_OF_LUT;
  }
  // we want the same frenquency as previously when simpleVoice.index = (simpleVoice.counter * 2) /3;
  // our maximum is 1000 so we update our counter every 1000 * (2/3) -= 667 to have 625Hz.
  // To create the 625Hz frequency of 625Hz, we use 4 samples (size of LUT), so the signal frequency is divived by four.
  // So we have counter_sig  = 1000 * Nech * (Fsig/Fpwm) = (2/3) * 1000 ~=667.
  // To retrieve Fsig we have Fsig = (Fpwn * counter_sig ) / (1000 * Ntech)) = ((2/3) *1000*37.5Hz)
  // But, as we use the approximate value 667, we have Fsig = (667*37.5Hz) / (1000 * 4) = 6253.125Hz.
  PINB = (1 << PB1); //toggle the pin on overflow
}


