//modified by: Derrick Alden
//date:
//purpose:
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include "fonts.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 1000
#define GRAVITY 0.1


const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 11;



//define types
typedef float Flt;
//typedef float Vec[3];
typedef Flt Matrix[4][4];

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

enum State {
	STATE_NONE,
	STATE_STARTUP,
	STATE_GAMEPLAY,
	STATE_PAUSE,
	STATE_GAMEOVER

};

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

class Player {
public:
	Vec dir;
	Vec pos;
	Vec vel;
	float angle;
	float color[3];
	
	Player() {
		pos.x = (Flt)(WINDOW_WIDTH/2);
		pos.y = (Flt)(WINDOW_HEIGHT/2);
		pos.z = 0.0;
		angle = 0.0;
		color[0] = color[1] = color[2] = 1.0;
	}

};

class Game {
    public:
	Player player;
	Shape box[6];
	Particle particle[MAX_PARTICLES];
	int n;

	State state;

	bool spawner;

	unsigned char keys[65535];

	bool leftUp;

	Game(){
	    	leftUp = 0;
	    	state = STATE_STARTUP;
		spawner = false;
		n = 0;
	}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();

	
	//declare game object
	Game game;
	game.n=0;


	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

//void print(int x, int y, int z, char *string) {
//
//	glRasterPos2f(x,y);
//int len = (int) strlen(string);
//
//for(int i = 0; i < len; i ++) {
//	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, *string);
//}

//}



void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "OpenGL Wars");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	//currently dark blue
	glClearColor(0.1, 0.1, 0.45, 1.0);
	//allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

#define rnd() (float)rand() / (float)RAND_MAX

//external function found in particle.cpp
extern void particleVelocity(Particle *p);

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
//	p->velocity.y = rnd() * 1.0 - .5;
//	p->velocity.x = rnd() * 1.0 - .5;
	particleVelocity(p);
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);	
			makeParticle(game, e->xbutton.x, y);
			makeParticle(game, e->xbutton.x, y);
			makeParticle(game, e->xbutton.x, y);
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
		//	int y = WINDOW_HEIGHT - e->xbutton.y;
			//makeParticle(game, e->xbutton.x, y);
	}
}

void normalize2d(Vec v)
{
	Flt len = v.x*v.x + v.y*v.y;
	if (len == 0.0f) {
		v.x = 1.0;
		v.y = 0.0;
		return;
	}
	len = 1.0f / sqrt(len);
	v.x *= len;
	v.y *= len;
}



void moveRight(Game *game)
{
    //rorate method
    /*
	game->player.angle -= 8.0;
	if (game->player.angle >= 360.0f)
		game->player.angle -= 360.0f;

	*/
    game->player.pos.x += 5;
}

void moveLeft(Game *game)
{
    //rotate method
    /*
	game->player.angle += 8.0;
	if (game->player.angle >= 360.0f)
		game->player.angle -= 360.0f;
	*/
    game->player.pos.x -= 5;
}

void moveUp(Game *game)
{
		/*
		//apply thrust method
		//convert ship angle to radians
		Flt rad = ((game->player.angle+90.0) / 360.0f) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		game->player.vel.x += xdir*0.02f;
		game->player.vel.y += ydir*0.02f;
		Flt speed = sqrt(game->player.vel.x*game->player.vel.x+
				game->player.vel.x*game->player.vel.y);
		if (speed > 5.0f) {
			speed = 5.0f;
			normalize2d(game->player.vel);
			game->player.vel.x *= speed;
			game->player.vel.y *= speed;
		}
		*/
    		if (game->leftUp == 1) {
			game->player.pos.y += 5;
			game->player.pos.x -= 5;

		} else {
		    game->player.pos.y += 5;
		}
}

void moveDown(Game *game)
{
		/*
		//apply thrust method
		//convert ship angle to radians
		Flt rad = ((game->player.angle+90.0) / 360.0f) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		game->player.vel.x += xdir*0.02f;
		game->player.vel.y += ydir*0.02f;
		Flt speed = sqrt(game->player.vel.x*game->player.vel.x+
				game->player.vel.x*game->player.vel.y);
		if (speed > 5.0f) {
			speed = 5.0f;
			normalize2d(game->player.vel);
			game->player.vel.x *= speed;
			game->player.vel.y *= speed;
		}
		*/
		game->player.pos.y -= 5;

}


int check_keys(XEvent *e, Game *game) {

    	//Was W + A pushed?
	static int leftUp = 0;
	if (e->type == KeyRelease) {

		int key = XLookupKeysym(&e->xkey, 0);
		game->keys[key]=0;
		if (key == 'a') {
			leftUp =0;
			game->leftUp = 0;
		}
	}
    	//Was W + A pushed?
//	static int leftUp = 0;
	if (e->type == KeyPress) {

		int key = XLookupKeysym(&e->xkey, 0);
		game->keys[key]=1;
		if (key == 'a') {
			leftUp = 1;
			game->leftUp = 1;
		}
	}

	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		if (key == 'a') { moveLeft(game); }
		if (key == 'w') { moveUp(game); }
		if (key == 'd') { moveRight(game); }
		if (key == 's') { moveDown(game); }
		if (key == 'p') { if(game->state == STATE_PAUSE) {game->state = STATE_GAMEPLAY; } else {game->state = STATE_PAUSE;} }
		if (key == 'o') { game->state = STATE_GAMEPLAY; }
		if (key == 'b') { game->spawner = true;}
	
		}
		//You may check other keys here.



	
	return 0;
}


void movement(Game *game)
{
	Particle *p;

		
	Flt d0,d1,dist;
	//Update ship position
	game->player.pos.x += game->player.vel.x;
	game->player.pos.x += game->player.vel.y;

	if (game->n <= 0)
		return;

	//check keys
	//move with wsad
/*
	if (key == a) {
		g.ship.angle += 4.0;
		if (g.ship.angle >= 360.0f)
			g.ship.angle -= 360.0f;
	}
	if (gl.keys[XK_d]) {
		g.ship.angle -= 4.0;
		if (g.ship.angle < 0.0f)
			g.ship.angle += 360.0f;
	}
	if (gl.keys[XK_w]) {
		//apply thrust
		//convert ship angle to radians
		Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		g.ship.vel[0] += xdir*0.02f;
		g.ship.vel[1] += ydir*0.02f;
		Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
				g.ship.vel[1]*g.ship.vel[1]);
		if (speed > 5.0f) {
			speed = 5.0f;
			normalize2d(g.ship.vel);
			g.ship.vel[0] *= speed;
			g.ship.vel[1] *= speed;
		}
	}
*/
//======================================================================	

	for (int i = 0; i < game->n; i++){

		p = &game->particle[i];
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
	
	//check for collision with shapes...
	//checks for partical hitting the shape ss then reverses the velocity of the partical

	Shape *s;
 
		for (int j = 0; j < 5; j++) {
			s = &game->box[j];

			if(p->s.center.y < s->center.y + s->height &&
				p->s.center.x > s->center.x - s ->width &&
				p->s.center.x < s->center.x + s->width) {
					p->s.center.y = s->center.y + s->height;
					p->velocity.y = -p->velocity.y;
					p->velocity.y *= 0.5;
		}
	}
	

	//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
			//std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[ game->n-1 ];
			game->n--;
		}
	}
}

void setFrame(Game *game)
{

	//draw boxes
	Shape *s;
	float w, h;


	//filling box array 
//	for (int i = 0; i < 6; i++) {
//		game->box[i].width = 100;
//		game->box[i].height = 15;
//		game->box[i].center.x = 120 + 5*65 - ( i * 30 );
//		game->box[i].center.y = 500 - 5*60 + ( i * 50 );
//	}

	//set up box 1
	game->box[0].width = WINDOW_WIDTH;
	game->box[0].height = 15;
	game->box[0].center.x = WINDOW_WIDTH/2;
	game->box[0].center.y = 0;


	//drawing box 1 bottom
		glColor3ub(90,140,90);
		s = &game->box[0];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();	
		glPopMatrix();

	//set up box 2
	game->box[1].width = 15;
	game->box[1].height = WINDOW_HEIGHT;
	game->box[1].center.x = 0;
	game->box[1].center.y = 0;


	//drawing box 2 left
		glColor3ub(90,140,90);
		s = &game->box[1];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();	
		glPopMatrix();


	//set up box 3
	game->box[2].width = 15;
	game->box[2].height = WINDOW_HEIGHT;
	game->box[2].center.x = WINDOW_WIDTH;
	game->box[2].center.y = 0;


	//drawing box 3 right
		glColor3ub(90,140,90);
		s = &game->box[2];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();	
		glPopMatrix();

	//set up box 4
	game->box[3].width = WINDOW_WIDTH;
	game->box[3].height = 15;
	game->box[3].center.x = WINDOW_WIDTH/2;
	game->box[3].center.y = 0;




	//drawing box 4 top
		glColor3ub(90,140,90);
		s = &game->box[3];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();	
		glPopMatrix();




}

void render(Game *game)
{
	if (game->state == STATE_STARTUP) {
		game->state = STATE_GAMEPLAY;

	}

	//pause not working when unpaused
	//could be if statement or state switching does not rerender
/*	if (game->state == STATE_PAUSE) {
	
		Rect newr;
		newr.bot = 100-20;
		newr.left = 10;
		newr.center = 0;

		float nw, nh;
		nh = 100.0;
		nw = 200.0;
	//	glPushMatrix();
	//	glEnable(GL_BLEND);
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glColor4f(1.0, 1.0, 0.0, 0.8);
		glTranslated(WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 0);
		glBegin(GL_QUADS);
			glVertex2i(-nw,  -nh);
			glVertex2i(-nw,   nh);
			glVertex2i( nw,    nh);
			glVertex2i( nw,   -nh);
		glEnd();
	//	glDisable(GL_BLEND);
		glPopMatrix();
		newr.bot = WINDOW_WIDTH/2+80;
		newr.left = WINDOW_HEIGHT/2;
		newr.center = 1;
		ggprint8b(&newr, 16, 0, "PAUSED");
		newr.center = 0;
		newr.left = WINDOW_HEIGHT/2-100;
		ggprint8b(&newr, 16, 0, "P Pause");
		ggprint8b(&newr, 16, 0, "O Play");
		return;		
//	}else {
	*/
	float w, h;
//	Rect r;
	glClear(GL_COLOR_BUFFER_BIT);

//	unsigned int c = 0x00ffff44;
//	r.bot = 100 - 20;
//	r.left = 10;
//	r.center = 0;

//	const char* text[5] = {"Maintence", "Testing", "Coding", "Design", "Requirements"};

	//draw boxes
	Shape *s;


	setFrame(game);
	//filling box array 
	/*
	for (int i = 0; i < 6; i++) {
		game->box[i].width = 100;
		game->box[i].height = 15;
		game->box[i].center.x = 120 + 5*65 - ( i * 30 );
		game->box[i].center.y = 500 - 5*60 + ( i * 50 );
	}
	*/

	//draw player
	glColor3fv(game->player.color);
	glPushMatrix();
	glTranslatef(game->player.pos.x, game->player.pos.y, game->player.pos.z);
	//float angle = atan2(ship.dir[1], ship.dir[0]);
	glRotatef(game->player.angle, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	//glVertex2f(-10.0f, -10.0f);
	//glVertex2f(  0.0f, 20.0f);
	//glVertex2f( 10.0f, -10.0f);
	glVertex2f(-12.0f, -10.0f);
	glVertex2f(  0.0f, 20.0f);
	glVertex2f(  0.0f, -6.0f);
	glVertex2f(  0.0f, -6.0f);
	glVertex2f(  0.0f, 20.0f);
	glVertex2f( 12.0f, -10.0f);
	glEnd();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	glVertex2f(0.0f, 0.0f);
	glEnd();
	glPopMatrix();



	//drawing boxes
	for( int i = 0; i < 5; i++) {
		glColor3ub(90,140,90);
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
	//draw text with ggtext
//	r.bot = s->height - 10;
//	r.left = s->width - 150;
//	r.center = 0;
	
//	ggprint8b(&r, 16, c, text[i]);	
		glPopMatrix();

	}
	
	
	//starts the spawner if b is pushed
	if(game->spawner) {makeParticle(game, 350, 600);}

	//draw all particles here
	for (int i = 0; i < game->n; i++) {
	glPushMatrix();
	glColor3ub(150,160,220);
	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
	}
//	}
	
}
