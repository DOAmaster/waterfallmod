//modified by: Derrick Alden

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};


#define rnd() (float)rand() / (float)RAND_MAX

void particleVelocity(Particle *p) {

	p->velocity.y = rnd() * 1.0 - .5;
	p->velocity.x = rnd() * 1.0 - .5;

}




