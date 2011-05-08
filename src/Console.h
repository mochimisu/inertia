/*
 * Console.h
 * Brandon Wang
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "global.h"
#include "freetype.h"

#include <list>

class Console {
public:
	//instantiate.
	Console();
	//destroy.
	~Console();
	//configure.
	void config(float x, float y, int numLines, float lineInterval);
	//output.
	void output(string s);
	//draw.
	void draw();
private:
	int length;
	float initx, inity, interval;
	list<string> messages;
};

#endif /* CONSOLE_H_ */