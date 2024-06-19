#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <glut.h>
#include <cmath>
#include <playsoundapi.h>
#include <mmsystem.h>
#include <mciapi.h>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int WIDTH = 1280;
int HEIGHT = 720;

//timer
int timeLeft = 120;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 1000; // for al heta al sooda

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {};
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

//Vector Eye(20, 5, 20);
//Vector At(0, 0, 0);
//Vector Up(0, 1, 0);

Vector Eye(-10, 10, 10);
Vector At(-10, 8, 0);
Vector Up(0, 1, 0);


int cameraZoom = 0;

/// camera and light things
#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}


};

Camera camera;



// Model Variables
//level 1
Model_3DS model_lamp;//obatacles
Model_3DS model_candy;//collectables
Model_3DS model_ghost;//player 1
Model_3DS model_door;//target

//level 2
Model_3DS model_santa;//main player
Model_3DS model_carrot;//collectables
Model_3DS model_tree;//obatacles
Model_3DS model_present;//level 2 target

// Textures
GLTexture tex_ground;//ground
GLTexture tex_snow; // snow
GLTexture tex_sky;




//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}
// ground boundaries
int xMin = -20;
int xMax = 20;
int zMin = -20;
int zMax = 20;


void RenderSnow() {
	glDisable(GL_LIGHTING); // Disable lighting


	glColor3f(0.6, 0.6, 0.6); // Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D); // Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_snow.texture[0]); // Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0); // Set quad normal direction.
	glTexCoord2f(0, 0); // Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(1, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(1, 1);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 1);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING); // Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1); // Set material back to white instead of grey used for the ground texture.

}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy: Angle between the bottom and top of the projectors, in degrees. //
	// aspectRatio: Ratio of width to height of the clipping plane. //
	// zNear and zFar: Specify the front and back clipping planes distances from camera. //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera. //
	// AT (ax, ay, az): denotes the direction where the camera is aiming at. //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera. //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}




void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}
void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//this is the method used to print text in OpenGL
//there are three parameters,
//the first two are the coordinates where the text is display,
//the third coordinate is the string containing the text to display
void print(int x, int y, int z, char* string)
{
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	//glRasterPos2f(x, y);
	glRasterPos3f(x, y, z);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		//glColor3f(0, 0, 1);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}


void drawCarrot(int x, int y, int z) {
	glPushMatrix();
	//glTranslatef(1, 1, 15);
	glScalef(15, 15, 15);
	model_carrot.Draw();
	glPopMatrix();
}

//player variables

int angle = 0;
int playerDirection = 1;

//player position
double xPosition = 0;
double yPosition = 0;
double zPosition = 0;
double playerSpeed = 0.5;
int score = 0;



//candy Positions

//---collectables
// Add these global variables to store collectible positions
bool initialCollectableGeneration = true;
int collectableX[10];  // Array to store X positions of collectables
int collectableZ[10];  // Array to store Z positions of collectables

bool isCollectable1Visible = true; // Variable to track collectable visibility first enviroment
bool isCollectable2Visible = true; // Variable to track collectable visibility second enviroment



void generateRandomCollectablePositions() {
	for (int i = 0; i < 10; i++) {
		collectableX[i] = rand() % 20 + 1;
		collectableZ[i] = rand() % 20 + 1;

	}

}


void hideCollectable() {
	// Code to hide or remove the collectable
	std::cout << "Collectable disappeared!" << std::endl;
	isCollectable1Visible = false;
}

void handlePlayer1CollectablesCollision() {
	for (int i = 0; i < 10; i++) {
		float distance = sqrt((xPosition - collectableX[i]) * (xPosition - collectableX[i]) + (zPosition - collectableZ[i]) * (zPosition - collectableZ[i]));

		if (distance < 1 && isCollectable1Visible) { // Assuming a collision occurs when the distance is less than a certain threshold
			// Player has collected the collectable
			score += 50; // Increase the score by 20
			collectableX[i] = -100; // Mark the collected collectable as invalid
			collectableZ[i] = -100;

		}
	}
}

void handlePlayer2CollectablesCollision() {
	for (int i = 0; i < 10; i++) {
		float distance = sqrt((xPosition - collectableX[i]) * (xPosition - collectableX[i]) + (zPosition - collectableZ[i]) * (zPosition - collectableZ[i]));

		if (distance < 1 && isCollectable1Visible) { // Assuming a collision occurs when the distance is less than a certain threshold
			// Player has collected the collectable
			score += 10; // Increase the score by 20
			collectableX[i] = -100; // Mark the collected collectable as invalid
			collectableZ[i] = -100;

		}
	}
}

bool isValid = true;

void collectableValid() {
	if (collectableX != 0 && collectableZ != 0)
		isCollectable1Visible = true;
	else
	{
		isCollectable1Visible = false;

	}
}

// this is the door collision part

bool ghostCollidedWithDoor = false;

int currentLevel = 1;




void renderBitmapString(float x, float y, void* font, const char* string) {
	glRasterPos2f(x, y);
	while (*string) {
		glutBitmapCharacter(font, *string);
		string++;
	}
}



void checkCollisionWithDoor() {
	// Threshold distance for collision
	float collisionDistance = 0.5; // Adjust this value based on your scale

	// Ghost position
	float ghostX = xPosition;
	float ghostZ = zPosition;

	// Door position (as per your drawDoor function)
	float doorX = -15;
	float doorZ = 10;

	// Calculate the distance between the ghost and the door
	float distance = sqrt(pow(ghostX - doorX, 2) + pow(ghostZ - doorZ, 2));

	// Check if the distance is less than the threshold
	if (distance < collisionDistance) {
		ghostCollidedWithDoor = true;
	}
	else {
		ghostCollidedWithDoor = false;
	}
}

// lamp collision
// Lamp post positions
// Define lamp positions and collision radius as before
// Define lamp positions and collision radius as before
float lamp1X = -15.0f;
float lamp1Z = 15.0f;
float lamp2X = 15.0f;
float lamp2Z = -2.0f;
float lamp3X = 3.0f;
float lamp3Z = 10.0f;
float lampCollisionRadius = 2.0f;

float tree1X = -15.0f;
float tree1Z = 15.0f;
float tree2X = 15.0f;
float tree2Z = -2.0f;
float tree3X = 3.0f;
float tree3Z = 10.0f;

float presentX = 0.0f;
float presentZ = 0.0f;




// Function to calculate distance between two points

// Function to calculate distance between two points
float calculateDistance(float x1, float z1, float x2, float z2) {
	float dx = x1 - x2;
	float dz = z1 - z2;
	return sqrt(dx * dx + dz * dz);
}

// Function to check collision with a tree
bool checkCollisionWithTree(float treeX, float treeZ) {
	float dx = xPosition - treeX;
	float dz = zPosition - treeZ;
	float distance = sqrt(dx * dx + dz * dz);
	return distance <= lampCollisionRadius; // Assuming the collision radius is the same for trees
}

bool hasCollidedWithPresent = false;


// Function to check collision with a present
bool checkCollisionWithPresent(float presentX, float presentZ) {
	float dx = xPosition - presentX;
	float dz = zPosition - presentZ;
	float distance = sqrt(dx * dx + dz * dz);
	return distance <= 1.0; // Assuming the collision radius is the same for presents
}

// Void function to check all collisions
void checkAllCollisions() {
	float ghostX = xPosition;
	float ghostZ = zPosition;

	// Check collision with each lamp
	if (calculateDistance(ghostX, ghostZ, lamp1X, lamp1Z) <= lampCollisionRadius ||
		calculateDistance(ghostX, ghostZ, lamp2X, lamp2Z) <= lampCollisionRadius ||
		calculateDistance(ghostX, ghostZ, lamp3X, lamp3Z) <= lampCollisionRadius) {
		// Collision detected with a lamp, prevent movement
		// You can set the position to a safe value here, or reset it to previous positions
		xPosition = xPosition - 0.5;
		zPosition = zPosition - 0.5;
	}

	// Check collision with each tree
	if (checkCollisionWithTree(tree1X, tree1Z) ||
		checkCollisionWithTree(tree2X, tree2Z) ||
		checkCollisionWithTree(tree3X, tree3Z)) {
		// Collision detected with a tree, prevent movement
		// You can set the position to a safe value here, or reset it to previous positions
		xPosition = xPosition - 0.5;
		zPosition = zPosition - 0.5;
		//PlaySound(TEXT("sounds/danger.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
	}

	// Check collision with the present


}
float  minutes = 0.0;
float  seconds = 50.0;

float light = 1.0;
float lightPos;

double angbackg;
bool myAnim = true;

void anim() {
	if (myAnim) {
		angbackg += 20;
		glutPostRedisplay();
	}
}

void timer(int value) {
	static bool gameOverDisplayed = false;
	anim();

	// Check if there is time left
	if (minutes > 0 || seconds > 0) {
		// Decrement seconds
		seconds -= 1.0;
		if (seconds < 0.0) {
			seconds = 59.0;
			minutes -= 1.0;
		}

		// Request a redisplay
		glutPostRedisplay();

		light -= 0.1;
		if (light <= 0) {
			light = 0;
		}
		lightPos++;
		// Set the timer to call itself again in 1000 milliseconds (1 second)
		glutTimerFunc(1000, timer, 0);
	}
	else if (!gameOverDisplayed) {
		// Time is up, turn the display black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
		glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

		// Set text color to white
		glColor3f(1.0f, 1.0f, 1.0f);

		// Display "Game Over" text
		renderBitmapString(-0.5f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Game Over");

		// Swap buffers to display the "Game Over" text
		glutSwapBuffers();

		// Set a timer to exit after 2 seconds
		glutTimerFunc(2000, exit, 0);

		gameOverDisplayed = true;
	}
	else if (checkCollisionWithPresent(presentX, presentZ) && currentLevel == 2) {
		// Time is up, turn the display black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
		glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

		// Set text color to white
		glColor3f(1.0f, 1.0f, 1.0f);

		// Display "Game Over" text
		renderBitmapString(-0.5f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Game winnnnnnnn2222");

		// Swap buffers to display the "Game Over" text
		glutSwapBuffers();

		// Set a timer to exit after 2 seconds
		glutTimerFunc(2000, exit, 0);

		gameOverDisplayed = true;

	}
}

void print(int x, int y, char* string)
{
	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}


void renderTime() {
	// Set the color to white
	glColor3f(1.0f, 1.0f, 1.0f);

	// Position to display the time (change the values as needed)
	float posX = 2.0f;
	float posY = 3.0f;
	float posZ = -3.0f;

	glPushMatrix(); // Save the current matrix
	glLoadIdentity();
	glTranslatef(1.0, 1.2, -3.0); // Set the position in 3D space

	// Display the time as text in 3D space
	char timeStr[20];
	sprintf(timeStr, "Time: %.2f seconds", minutes * 60 + seconds); // Convert minutes to seconds
	renderBitmapString(0.0f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, timeStr);

	glPopMatrix(); // Restore the previous matrix
}



void wahooshainput() {
	PlaySound(TEXT("sounds/kaj.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}

void alarmSound() {
	if (checkAllCollisions) {
		PlaySound(TEXT("sounds/danger.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
	}
}

// Movement functions


float cameraRadius = 20.0f;  // Distance of the camera from the target point
float cameraTheta = 0.0f;    // Horizontal angle of the camera
float cameraPhi = 45.0f;      // Vertical angle of the camera
float cameraSpeed = 0.1f;

float lookAtX = 0.0f;   // X-coordinate of the point to look at
float lookAtY = 0.0f;   // Y-coordinate of the point to look at
float lookAtZ = 0.0f;   // Z-coordinate of the point to look at

void updateCameraPosition() {
	float x = cameraRadius * sin(cameraTheta * 3.14 / 180) * cos(cameraPhi * 3.14 / 180);
	float y = cameraRadius * sin(cameraPhi * 3.14 / 180);
	float z = cameraRadius * cos(cameraTheta * 3.14 / 180) * cos(cameraPhi * 3.14 / 180);

	glLoadIdentity();

	if (fabs(cameraPhi) > 89.0) {
		gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);
	}
	else {
		gluLookAt(x, y, z, lookAtX, lookAtY, lookAtZ, 0, 1, 0);
	}
}





//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	setupCamera();
	setupLights();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderTime();
	RenderSnow();



	GLfloat lightIntensity[] = { light,light, light, 1.0f };
	GLfloat lightPosition[] = { lightPos, 100.0f, lightPos, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);


	//print score
	char scoreText[20];
	sprintf(scoreText, "Score: %d", score);
	renderBitmapString(15, 20, GLUT_BITMAP_TIMES_ROMAN_24, scoreText);
	//print(50, 50, 50, scoreText);





	if (checkCollisionWithPresent(presentX, presentZ) && currentLevel == 2 && score >= 100) {
		//// Time is up, turn the display black
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear both color and depth buffers

		//// Set text color to white
		//glColor3f(1.0f, 1.0f, 1.0f);


		//// Display "Game Over" text
		//renderBitmapString(-0.5f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Game winnnnnnnn2222");

		//// Swap buffers to display the "Game Over" text
		//glutSwapBuffers();

		//// Set a timer to exit after 2 seconds
		//glutTimerFunc(2000, exit, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 0.0f, 0.7f, 1.0f);

		//glColor3f(1.0f, 0.0f, 0.7f);
		const char* lostMessage = "Game Won!";

		// Set the raster position for drawing the string
		glRasterPos2f(0.0f, 0.0f);

		// Draw the entire string at once
		for (int i = 0; i < strlen(lostMessage); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, lostMessage[i]);
		}

	}
	else {
		if (score >= 100 && currentLevel == 1) {
			currentLevel = 2;
		}
		if (currentLevel == 1) {

			//draw lamp post(obstacles)
			// First Lamp
			glPushMatrix();
			glTranslatef(lamp1X, 0, lamp1Z);
			glRotatef(angbackg, 0, 1, 0);
			glTranslatef(-lamp1X, 0, -lamp1Z);
			glTranslatef(lamp1X, 0, lamp1Z); // Adjusted position
			glRotatef(90, 1, 0, 0);
			glScalef(0.03, 0.03, 0.03);
			model_lamp.Draw();
			glPopMatrix();

			// Second Lamp
			glPushMatrix();
			glTranslatef(lamp2X, 0, lamp2Z);
			glRotatef(angbackg, 0, 1, 0);
			glTranslatef(-lamp2X, 0, -lamp2Z);
			glTranslatef(lamp2X, 0, lamp2Z); // Adjusted position
			glRotatef(90, 1, 0, 0);
			glScalef(0.03, 0.03, 0.03);
			model_lamp.Draw();
			glPopMatrix();

			// Third Lamp
			glPushMatrix();
			glTranslatef(lamp3X, 0, lamp3Z);
			glRotatef(angbackg, 0, 1, 0);
			glTranslatef(-lamp3X, 0, -lamp3Z);
			glTranslatef(lamp3X, 0, lamp3Z); // Adjusted position
			glRotatef(90, 1, 0, 0);
			glScalef(0.03, 0.03, 0.03);
			model_lamp.Draw();
			glPopMatrix();



			//draw candy (collectables)
			if (initialCollectableGeneration) {
				generateRandomCollectablePositions();
				initialCollectableGeneration = false;

			}

			handlePlayer1CollectablesCollision();

			for (int i = 0; i < 10; i++) {
				if (isCollectable1Visible == true) {
					glPushMatrix();
					glTranslatef(collectableX[i], 0, collectableZ[i]);
					glRotatef(angbackg, 0, 1, 0);
					glTranslatef(-collectableX[i], 0, -collectableZ[i]);
					glTranslatef(collectableX[i], 0, collectableZ[i]);
					glRotatef(90, 1, 0, 0);
					glScalef(0.08, 0.08, 0.08);
					model_candy.Draw();
					glPopMatrix();
				}


			}


			//draw ghost
			glPushMatrix();
			glTranslatef(xPosition, yPosition, zPosition); // Apply the translation to the player
			glRotatef(angle, 0, 1, 0);
			glRotatef(90, 1, 0, 0);
			glScalef(5, 5, 5);
			model_ghost.Draw();
			glPopMatrix();

			//draw door(target)
			glPushMatrix();
			glTranslatef(1, 0, 10);
			glRotatef(90, 1, 0, 0);
			glScalef(5, 5, 5);
			model_door.Draw();
			glPopMatrix();

			checkCollisionWithDoor();
			checkAllCollisions();
		}


		//level 2
		else if (currentLevel == 2) {



			// Draw Tree Model
			glPushMatrix();
			glTranslatef(tree1X, 0, tree1Z);
			glScalef(0.5, 0.5, 0.5);
			model_tree.Draw();
			glPopMatrix();
			checkAllCollisions();
			// Draw Tree Model
			glPushMatrix();
			glTranslatef(tree2X, 0, tree2Z);
			glScalef(0.5, 0.5, 0.5);
			model_tree.Draw();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(tree3X, 0, tree3Z);
			glScalef(0.5, 0.5, 0.5);
			model_tree.Draw();
			glPopMatrix();

			// Draw Tree Model


			//draw carrot (collectables)
			if (initialCollectableGeneration) {
				generateRandomCollectablePositions();
				initialCollectableGeneration = false;

			}

			handlePlayer2CollectablesCollision();

			for (int i = 0; i < 10; i++) {
				if (isCollectable2Visible == true) {
					glPushMatrix();
					glTranslatef(collectableX[i], 0, collectableZ[i]);
					glRotatef(angbackg, 0, 1, 0);
					glTranslatef(-collectableX[i], 0, -collectableZ[i]);
					glTranslatef(collectableX[i], 0, collectableZ[i]);
					glScalef(15, 15, 15);
					model_carrot.Draw();
					glPopMatrix();

				}


			}


			// Draw main player 2 Model
			glPushMatrix();
			glTranslatef(xPosition, yPosition, zPosition); // Apply the translation to the player
			glRotatef(angle, 0, 1, 0);
			glRotatef(90.f, 1, 0, 0);
			glScalef(20, 20, 15);
			model_santa.Draw();
			glPopMatrix();
			checkAllCollisions();


			// DA HOWA AL PRESENTTT ************* Present Model
			glPushMatrix();
			glTranslatef(presentX, 2, presentZ);
			glRotatef(90.f, 1, 0, 0);
			glScalef(3, 3, 3);
			model_present.Draw();
			glPopMatrix();
			checkAllCollisions();




		}


		////sky box
		glPushMatrix();

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);


		glPopMatrix();
	}


	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================




bool firstperson = false;
bool thirdperson = false;

void changeCamera() {
	if (firstperson == true) {
		if (angle == 0) // bottom
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 1;
			camera.center.x = xPosition;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition + 1;
		}
		else if (angle == -90)//left direction
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 1;
			camera.center.x = xPosition - 1;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition;
		}

		else if (angle == 180)//top direction
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 1;
			camera.center.x = xPosition;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition - 1;
		}
		else // right direction
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 1;
			camera.center.x = xPosition + 1;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition;
		}
	}
	if (thirdperson == true) {
		if (angle == 0) // bottom
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition - 5;
			camera.eye.y = yPosition + 13;
			camera.center.x = xPosition;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition + 5;
		}
		else if (angle == -90)//left direction
		{
			camera.eye.x = xPosition + 5;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 13;
			camera.center.x = xPosition - 5;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition;
		}

		else if (angle == 180)//top direction
		{
			camera.eye.x = xPosition;
			camera.eye.z = zPosition + 5;
			camera.eye.y = yPosition + 13;
			camera.center.x = xPosition;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition - 5;
		}
		else // right direction
		{
			camera.eye.x = xPosition - 5;
			camera.eye.z = zPosition;
			camera.eye.y = yPosition + 13;
			camera.center.x = xPosition + 5;
			camera.center.y = yPosition + 1;
			camera.center.z = zPosition;
		}
	}

}

void moveup() {

	zPosition -= playerSpeed;
	changeCamera();

}
void movedown() {
	zPosition += playerSpeed;
	changeCamera();
}

void moveleft() {
	xPosition -= playerSpeed;
	changeCamera();

}

void moveright() {
	xPosition += playerSpeed;
	changeCamera();
}


//playerDirection=1 -- looking down
//playerDirection=2 -- looking right
//playerDirection=3 -- looking up
//playerDirection=4 -- looking left

void rotateDirection() {
	if (angle == 0) // bottom
		playerDirection = 1;
	else if (angle == 90)//left direction
		playerDirection = 2;
	else if (angle == 180)//top direction
		playerDirection = 3;
	else
		playerDirection = 4; // right direction
}
void specialKeyPressed(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		// Move the player up
		if (playerDirection == 1 || playerDirection == 2 || playerDirection == 4) {
			angle = 180;
			moveup();
		}
		break;
	case GLUT_KEY_DOWN:
		// Move the player down
		if (playerDirection == 2 || playerDirection == 3 || playerDirection == 4) {
			angle = 0;
			movedown();
		}
		break;
	case GLUT_KEY_RIGHT:
		// Move the player right
		if (playerDirection == 1 || playerDirection == 3 || playerDirection == 4) {
			angle = 90;
			moveright();
		}
		break;
	case GLUT_KEY_LEFT:
		// Move the player left
		if (playerDirection == 1 || playerDirection == 2 || playerDirection == 3) {
			angle = -90;
			moveleft();
		}
		break;
	}
}


void keepPlayerToBoundary() {
	if (xPosition > xMax || xPosition < xMin || zPosition>zMax || zPosition < zMin) {
		xPosition = 0;
		zPosition = 0;
	}
}



void myKeyboard(unsigned char key, int x, int y) {
	keepPlayerToBoundary();
	rotateDirection();
	float d = 0.01;

	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;

	case 'u': // move player up
		if (playerDirection == 1) {
			angle = 180;
		}
		if (playerDirection == 2) {
			angle = 180;
		}
		if (playerDirection == 4) {
			angle = 180;
		}

		moveup();
		break;

	case 'j': // move player down
		if (playerDirection == 2) {
			angle = 0;
		}
		if (playerDirection == 3) {
			angle = 0;
		}
		if (playerDirection == 4) {

			angle = 0;
		}
		movedown();
		break;
	case 'k': // move player right
		if (playerDirection == 1) {
			angle = 90;
		}
		if (playerDirection == 3) {

			angle = 90;
		}
		if (playerDirection == 4) {
			angle = 90;
		}
		moveright();
		break;
	case 'h': // move player left

		if (playerDirection == 1) {
			angle = -90;
		}
		if (playerDirection == 2) {
			angle = -90;
		}
		if (playerDirection == 3) {
			angle = -90;
		}
		moveleft();
		break;

	case 'x': // for stopping animation
		if (myAnim) {
			myAnim = false;
		}
		else {
			myAnim = true;
		}
		break;

	case'f': // for the front view
		camera.eye = Vector3f(30, 30, 30);
		camera.center = Vector3f(0, 0, 0);
		camera.up = Vector3f(0.0, 1.0, 0.0);
		break;

	case't': // for the top view
		camera.eye = Vector3f(20, 20, 20);
		camera.center = Vector3f(0.5, 0, 0.5);
		camera.up = Vector3f(0, 1, 0);
		break;

	case'i': // for the side view
		camera.eye = Vector3f(3, 0.5, 0.5);
		camera.center = Vector3f(0.5, 0, 0.5);
		camera.up = Vector3f(0, 1, 0);
		break;

	case 'o':
		firstperson = true;
		thirdperson = false;
		changeCamera();
		break;

	case 'l':

		thirdperson = true;
		firstperson = false;
		changeCamera();
		/*Eye.z += 20;
		gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);*/

		break;

	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}

	glutPostRedisplay();
}
void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}


//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity(); //Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z); //Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay(); //Re-draw scene
}

//=======================================================================
// Mouse Function
//=======================================================================

bool isUp = false;

void myMouse(int button, int state, int x, int y)
{
	// Invert the y-coordinate to match the OpenGL coordinate system
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isUp == false)
	{
		// Move the object upwards when the left mouse button is pressed
		yPosition += 10; // You can adjust the value based on how much you want the object to move
		glutPostRedisplay(); // Request a redraw to update the display
	}
	else {
		yPosition -= 10; // You can adjust the value based on how much you want the object to move
		glutPostRedisplay(); // Request a redraw to update the display
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	//level 1
	model_lamp.Load("Models/lamp/lamp.3ds");
	model_candy.Load("Models/candy/candy.3ds");
	model_ghost.Load("Models/ghost/ghost.3ds");
	model_door.Load("Models/door/door.3ds");

	//level 2
	model_santa.Load("Models/santa/santa.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_carrot.Load("Models/carrot/carrot.3ds");
	model_present.Load("Models/present/present.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_snow.Load("Textures/snow.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow("this is the game");


	glutKeyboardFunc(myKeyboard);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutTimerFunc(0, timer, 0);


	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	glutReshapeFunc(myReshape);

	glutDisplayFunc(myDisplay);
	myInit();
	//wahooshainput();
	alarmSound();
	glutMainLoop();
}