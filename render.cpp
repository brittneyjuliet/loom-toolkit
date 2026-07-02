#include <Bela.h>
#include <stdlib.h>
#include <libraries/OscSender/OscSender.h>
#include <math.h>
#include <libraries/BelaLibpd/BelaLibpd.h>

int permabuttons[] = {0, 1, 2, 3};
const int pbcount = sizeof(permabuttons)/sizeof(permabuttons[0]);
int pbstatus[4];
int pbprev[4] = {0, 0, 0, 0};

int knobpin1 = 0;
int knobpin2 = 1;
int filterin1 = 2;
int enca = 5;
int encb = 4;

// patchable
int pdout[] = {10, 11, 12, 13, 14, 15};
const int pdcount = sizeof(pdout)/sizeof(pdout[0]);
int pdstatus[6];
int pdprev[6] = {0, 0, 0, 0, 0, 0};

int count = 0;
float knob1_old;
float knob2_old;
int enccurr;
int encprev;
std::string encdir = "";

// patchable
int ci[] = {6, 7, 8, 9};
const int cicount = sizeof(ci)/sizeof(ci[0]);
int cistatus[4];
int ciprev[4] = {0, 0, 0, 0};
int rememberi = 0;
int rememberj = 0;

OscSender oscSender;
// int localPort = 7562;
int remotePort = 7563;
const char* remoteIp = "192.168.7.1";
// "127.0.0.1";
// "192.168.7.2";

bool setup(BelaContext *context, void *userData)
{
	oscSender.setup(remotePort, remoteIp);
	
	// set permanent buttons
	for(int i = 0; i < pbcount; i++){
		pinMode(context, 0, permabuttons[i], INPUT);
	}

	// set pd pins
	for(int i = 0; i < pdcount; i++){
		pinMode(context, 0, pdout[i], INPUT);
	} 

	// set custom input
	for (int i = 0; i < cicount; i++){
		pinMode(context, 0, ci[i], INPUT);
	}

	// set encoder test
	pinMode(context, 0, enca, INPUT);
	pinMode(context, 0, encb, INPUT);

	encprev = digitalRead(context, 0, enca);
	
	return true;
}


void render(BelaContext *context, void *userData)
{

	// permabuttons
	for (int i = 0; i < pbcount; i++){
		pbstatus[i] = digitalRead(context, 0, permabuttons[i]);

		if (pbstatus[i] == 1 && pbprev[i] == 0)
		{
			oscSender.newMessage("/pb" + std::to_string(i)).add(pbstatus[i]).sendNonRt();
			pbprev[i] =  1;
		
		} else if (pbstatus[i] == 0 && pbprev[i] == 1){
			oscSender.newMessage("/pb" + std::to_string(i)).add(pbstatus[i]).sendNonRt();
			pbprev[i] = 0;
		}
	}

	// custom input
	for (int i = 0; i < cicount; i++){ // 4 total
		cistatus[i] = digitalRead(context, 0, ci[i]);
		// oscSender.newMessage("/ci" + std::to_string(i)).add(cistatus[i]).sendNonRt();

		// read pd out for each button press
		for (int j = 0; j < pdcount; j++){
			pdstatus[j] = digitalRead(context, 0, pdout[j]);

			if (cistatus[i] == 1 && pdstatus[j] == 1 && pdprev[j] == 0){
				pdprev[j] = 1;
				rememberi = i;
				oscSender.newMessage("/ci" + std:: to_string(i)).add(i).add(j).add(pdstatus[j]).sendNonRt();
				// printf("i + j: %d, %d\n", i, j);
			} else if(cistatus[rememberi] == 0 && pdstatus[j] == 0 && pdprev[j] == 1){
				pdprev[j] = 0;
				oscSender.newMessage("/ci" + std:: to_string(rememberi)).add(rememberi).add(j).add(pdstatus[j]).sendNonRt();
			}
		}
	}

	float knob1status = floor(analogRead(context, 0, knobpin1)*1000)/1000;
	float knob2status = floor(analogRead(context, 0, knobpin2)*1000)/1000;
	float filter1status = analogRead(context, 0, filterin1);

	if (knob1status < knob1_old - .018 || knob1status > knob1_old + .018){
		knob1_old = knob1status;
	}

	if (knob2status < knob2_old - .018 || knob2status > knob2_old + .018){
		knob2_old = knob2status;
	}

	// printf("knob1status: %f\n", knob1_old);
	// printf("knob2status: %f\n", knob2_old);

	oscSender.newMessage("/knob1").add(map(knob1_old, 0.0, 0.8, 0.0, 1.0)).sendNonRt();
	oscSender.newMessage("/knob2").add(map(knob2_old, 0.0, 0.8, 0.0, 1.0)).sendNonRt();
	oscSender.newMessage("/filter1").add(filter1status).sendNonRt();

	// encoder test
	enccurr = digitalRead(context, 0, enca);

	if (enccurr != encprev)
	{
		if (digitalRead(context, 0, encb) != enccurr)
		{
			oscSender.newMessage("/e1").add(count).sendNonRt();
			count--;
			encdir="ccw";
		} else 
		{
			oscSender.newMessage("/e1").add(count).sendNonRt();
			count++;
			encdir="cw";
		}
	}

	encprev = enccurr;
	// printf("encoder test: %d\n", count);
	
	for(unsigned int n = 0; n < context->audioFrames; n++)
	{
		
		
	}	
	
}


void cleanup(BelaContext *context, void *userData)
{
	// Nothing to do here
}