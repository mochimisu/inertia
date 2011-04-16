/*
 * Console.cpp
 * Brandon Wang
 */

#include "Console.h"

Console::Console(){
}

void Console::config(float x, float y, int numLines, float lineInterval) {
	initx = x;
	inity = y;
	length = numLines;
	interval = lineInterval;
}
void Console::output(string s) {
	messages.push_back(s);
	if((int)messages.size() > length) {
		messages.pop_front();
	}
}
void Console::draw() {
	void * font = GLUT_BITMAP_HELVETICA_10;
	float curx = initx;
	float cury = inity - interval;
	for (list<string>::iterator it = messages.begin(); it != messages.end(); it++) {
		glRasterPos2f(curx,cury);
		for (string::iterator i = it->begin(); i != it->end(); ++i)
		{
			char c = *i;
			glutBitmapCharacter(font, c);
		}
		cury -= interval;
	}
}