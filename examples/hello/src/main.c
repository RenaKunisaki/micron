/** This simple example flashes Morse code on the LED.
 */
#include <micron.h>

static const char *msg = "HELLO WORLD";
static const char *morse[] = {
	//Morse codes for characters starting at ASCII 0x30
	"-----", //0
	".----", //1
	"..---", //2
	"...--", //3
	"....-", //4
	".....", //5
	"-....", //6
	"--...", //7
	"---..", //8
	"----.", //9
	"",      //:
	"",      //;
	"",      //<
	"",      //=
	"",      //>
	"",      //?
	"",      //@
	".-",    //A
	"-...",  //B
	"-.-.",  //C
	"-..",   //D
	".",     //E
	"..-.",  //F
	"--.",   //G
	"....",  //H
	"..",    //I
	".---",  //J
	"-.-",   //K
	".-..",  //L
	"--",    //M
	"-.",    //N
	"---",   //O
	".--.",  //P
	"--.-",  //Q
	".-.",   //R
	"...",   //S
	"-",     //T
	"..-",   //U
	"...-",  //V
	".--",   //W
	"-..-",  //X
	"-.--",  //Y
	"--..",  //Z
};

static const int time_unit  = 200; //msec per unit
static const int output_pin =  13;


void outputMorseChar(int pin, const char *chr) {
	for(int i=0; chr[i]; i++) {
		digitalWrite(pin, 1);
		if(chr[i] == '.') delayMS(time_unit);
		else delayMS(time_unit * 3);

		digitalWrite(pin, 0);
		delayMS(time_unit);
	}

	//time between letters = 3 units; we already did one
	delayMS(time_unit * 2);
}


int main() {
	setPinAsOutput(output_pin);
	while(1) {
		for(int i=0; msg[i]; i++) {
			char c = msg[i];
			if(c >= 'a' && c <= 'z') c -= 0x20; //to uppercase
			if(c >= '0' && c <= 'Z') {
				outputMorseChar(output_pin, morse[c - 0x30]);
			}
			else if(c == ' ') {
				//time between words = 7 units; we already did 3
				//(unless there are multiple spaces in a row, but why
				// would you do that?)
				delayMS(time_unit * 4);
			}
			else {
				//invalid character. just ignore it.
			}
		}
		delayMS(time_unit * 20);
	}

	//rebootIntoBootloader();
	return 0;
}
