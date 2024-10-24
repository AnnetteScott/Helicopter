/******************************************************************************
 *
 * Computer Graphics Programming 2020 Project Template v1.0 (11/04/2021)
 *
 * Based on: Animation Controller v1.0 (11/04/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene,
 * plus keyboard handling for smooth game-like control of an object such as a
 * character or vehicle.
 *
 * A simple static lighting setup is provided via initLights(), which is not
 * included in the animationalcontrol.c template. There are no other changes.
 *
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/******************************************************************************
 * Animation & Timing Setup
 ******************************************************************************/
#pragma warning(disable : 4996)

// Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				
#define M_PI 3.14159f

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;


const float gridSize = 250.0f;
typedef struct {
	float x, y, z;
} Point;

//Helicopter
const float maxRPS = 6.0f * 4; // 4 blades
const float rotorIncreaseRPS = 1.5f * 4;
const float minY = 2.1f;
const float moveSpeed = 8.0f;
const float turnSpeed = 50.0f;
float rotorSpeed = 0.0f;
bool spinning = false;
float rotorAngle = 0.0f;
float tailRotorAngle = 0.0f;
float heliRotation = 0.0f;


Point heliLocation = { 0.0f, 2.1f, 0.0f };

//Camera
const float cameraDistance = 20.0f;
const float cameraHeight = 5.0f;
float cameraX = 0.0f;
float cameraY = 5.0f;
float cameraZ = 20.0f;

// Windmill
float windmillAngle = 0.0f;

#define textureWidth  32
#define textureHeight 32
GLubyte groundTexture[textureWidth][textureHeight][3];
GLubyte buildingTexture[textureWidth][textureHeight][3];

#define buildingNum 10


/******************************************************************************
 * Some Simple Definitions of Motion
 ******************************************************************************/

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.

// Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
// 
// You can use any numeric values, as specified in the comments for each axis. However,
// the MOTION_ definitions offer an easy way to define a "unit" movement without using
// magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
//
typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

// Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
} keystate_t;

// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;

// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_MOVE_FORWARD	'w'
#define KEY_MOVE_BACKWARD	's'
#define KEY_MOVE_LEFT		'a'
#define KEY_MOVE_RIGHT		'd'
#define KEY_RENDER_FILL		'l'
#define KEY_EXIT			27 // Escape key.

// Define all GLUT special keys used for input (add any new key definitions here).

#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);
void drawGrid(float size, int divisions);
void drawHelicopter();
void moveHelicopter();
void createTexture(GLubyte(*myTexture)[textureWidth][textureHeight][3], char fileName[50]);
void drawBuilding(float x, float z, float height);
void drawWindmill(float x, float z, float angle);
void drawTree(float x, float z);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

// Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;

/******************************************************************************
 * Entry Point (don't put anything except the main function here)
 ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

/*
	Called when GLUT wants us to (re)draw the current animation frame.

	Note: This function must not do anything to update the state of our simulated
	world. Animation (moving or rotating things, responding to keyboard input,
	etc.) should only be performed within the think() function provided below.
*/
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(cameraX, cameraY, cameraZ,
		heliLocation.x, heliLocation.y, heliLocation.z,
		0.0, 1.0, 0.0);

	// Switch between filled and wireframe modes
	if (renderFillEnabled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// make sure the current drawing color is white for texturing
	glColor3f(1, 1, 1);

	// Draw the filled grid
	drawGrid(gridSize, 1);

	drawHelicopter();


	// Draw buildings
	for(int i = 0; i < buildingNum; i++){
		drawBuilding(25.0f, -10.0f + (i * 5), 10.0f);
	}
	for(int i = 0; i < buildingNum; i++){
		drawBuilding(15.0f, -10.0f + (i * 5), 10.0f);
	}

	drawWindmill(60.0f, 50.0f, windmillAngle);
	drawWindmill(40.0f, 50.0f, windmillAngle);
	drawWindmill(20.0f, 50.0f, windmillAngle);
	drawWindmill(0.0f, 50.0f, windmillAngle);
	drawWindmill(-20.0f, 50.0f, windmillAngle);

	// Draw trees
	for (float x = -50.0f; x <= 50.0f; x += 5.0f) {
		for (float z = -60.0f; z <= -20.0f; z += 5.0f) {
			drawTree(x, z);
		}
	}

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)h, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is pressed, we do two things:
			(1) Update motionKeyStates to record that the key is held down. We use
				this later in the keyReleased callback.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. The most recent key always "wins" (e.g. if
				you're holding down KEY_MOVE_LEFT then also pressed KEY_MOVE_RIGHT,
				our object will immediately start moving right).
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_FORWARD;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_BACKWARD;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_LEFT;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_RIGHT;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Rather than using literals (e.g. "t" for spotlight), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
			For example, refer to the existing keys used here (KEY_MOVE_FORWARD,
			KEY_MOVE_LEFT, KEY_EXIT, etc).
		*/
	case KEY_RENDER_FILL:
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyPressed.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_UP;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_DOWN;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_CLOCKWISE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			Rather than directly using the GLUT constants (e.g. GLUT_KEY_F1), create
			a new SP_KEY_ definition in the "Keyboard Input Handling Setup" section of
			this file. For example, refer to the existing keys used here (SP_KEY_MOVE_UP,
			SP_KEY_TURN_LEFT, etc).
		*/
	}
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is released.
*/
void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is released, we do two things:
			(1) Update motionKeyStates to record that the key is no longer held down;
				we need to know when we get to step (2) below.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. This gets a little complicated to ensure
				the controls work smoothly. When the user releases a key that moves
				in one direction (e.g. KEY_MOVE_RIGHT), we check if its "opposite"
				key (e.g. KEY_MOVE_LEFT) is pressed down. If it is, we begin moving
				in that direction instead. Otherwise, we just stop moving.
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveBackward == KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ? MOTION_FORWARD : MOTION_NONE;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ? MOTION_RIGHT : MOTION_NONE;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ? MOTION_LEFT : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Note: If you only care when your key is first pressed down, you don't have to
			add anything here. You only need to put something in keyReleased if you care
			what happens when the user lets go, like we do with our movement keys above.
			For example: if you wanted a spotlight to come on while you held down "t", you
			would need to set a flag to turn the spotlight on in keyPressed, and update the
			flag to turn it off in keyReleased.
		*/
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ? MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ? MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ? MOTION_CLOCKWISE : MOTION_NONE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ? MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			As per keyReleased, you only need to handle the key here if you want something
			to happen when the user lets go. If you just want something to happen when the
			key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

/*
	Called by GLUT when it's not rendering a frame.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

/*
	Initialise OpenGL and set up our scene before we begin the render loop.
*/
void init(void)
{
	glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // RGBA for a sky blue color
	glEnable(GL_DEPTH_TEST); // Enable depth testing
	initLights();

	createTexture(groundTexture, "Moss.ppm");
	createTexture(buildingTexture, "Brick.ppm");
	// Anything that relies on lighting or specifies normals must be initialised after initLights.
}

/*
	Advance our animation by FRAME_TIME milliseconds.
*/
void think(void)
{
	// 3 Blades
	windmillAngle += (5.0f * 3) * FRAME_TIME_SEC;
	if (windmillAngle >= 360.0) {
		windmillAngle -= 360.0;
	}


	// Spin the rotors
	if (spinning) {
		if (rotorSpeed < maxRPS) {
			rotorSpeed += rotorIncreaseRPS * FRAME_TIME_SEC;
		}
		if (rotorSpeed > maxRPS) {
			rotorSpeed = maxRPS;
		}

		
	}
	else if (rotorSpeed > 0) {
		rotorSpeed -= rotorIncreaseRPS * FRAME_TIME_SEC;
		if (rotorSpeed < 0) {
			rotorSpeed = 0;
		}

	}

	rotorAngle += rotorSpeed;
	tailRotorAngle += (rotorSpeed * 5);
	
	if (rotorAngle >= 360.0) {
		rotorAngle -= 360.0;
	}

	if (tailRotorAngle >= 360.0) {
		tailRotorAngle -= 360.0;
	}

	moveHelicopter();
	
	cameraX = heliLocation.x - cameraDistance * sin(heliRotation * M_PI / 180.0f);
	cameraY = heliLocation.y + cameraHeight;
	cameraZ = heliLocation.z - cameraDistance * cos(heliRotation * M_PI / 180.0f);

	// Position the spotlight at the front of the helicopter
	GLfloat light1_position[] = {
		heliLocation.x + 1.5f * sin(heliRotation * M_PI / 180.0f), // Front of the helicopter
		heliLocation.y, // Use the helicopter's current height
		heliLocation.z + 1.5f * cos(heliRotation * M_PI / 180.0f),
		1.0f
	};

	// Direction vector pointing more downward
	GLfloat light1_direction[] = {
		sin(heliRotation * M_PI / 180.0f), // X direction
		-1.0f,                            // Y direction (more downward)
		cos(heliRotation * M_PI / 180.0f)  // Z direction
	};

	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_direction);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f); // Adjust the cutoff angle if needed
}

void moveHelicopter() {
	float yawRadians = heliRotation * (M_PI / 180.0f);
	/*
		Keyboard motion handler: complete this section to make your "player-controlled"
		object respond to keyboard input.
	*/
	bool rotorsAtSpeed = rotorSpeed >= maxRPS;
	if (keyboardMotion.Yaw != MOTION_NONE && rotorsAtSpeed) {
		if (keyboardMotion.Yaw < 0) {
			// Turn right (clockwise)
			heliRotation -= turnSpeed * FRAME_TIME_SEC;
		}
		else {
			// Turn left (anticlockwise)
			heliRotation += turnSpeed * FRAME_TIME_SEC;
		}
	}
	if (keyboardMotion.Surge != MOTION_NONE && rotorsAtSpeed) {
		if (keyboardMotion.Surge < 0) {
			// Move backward
			heliLocation.z -= moveSpeed * FRAME_TIME_SEC * cos(yawRadians);
			heliLocation.x -= moveSpeed * FRAME_TIME_SEC * sin(yawRadians);
		}
		else {
			// Move forward
			heliLocation.z += moveSpeed * FRAME_TIME_SEC * cos(yawRadians);
			heliLocation.x += moveSpeed * FRAME_TIME_SEC * sin(yawRadians);
		}
	}
	if (keyboardMotion.Sway != MOTION_NONE && rotorsAtSpeed) {
		if (keyboardMotion.Sway < 0) {
			// Strafe left
			heliLocation.x += moveSpeed * FRAME_TIME_SEC * cos(yawRadians);
			heliLocation.z -= moveSpeed * FRAME_TIME_SEC * sin(yawRadians);
		}
		else {
			// Strafe right
			heliLocation.x -= moveSpeed * FRAME_TIME_SEC * cos(yawRadians);
			heliLocation.z += moveSpeed * FRAME_TIME_SEC * sin(yawRadians);
		}
	}
	if (keyboardMotion.Heave != MOTION_NONE) {
		if (keyboardMotion.Heave > 0) {
			// Move up
			spinning = true;
			if (rotorsAtSpeed) {
				heliLocation.y += moveSpeed * FRAME_TIME_SEC;
			}
		}
		else if (keyboardMotion.Heave < 0) {
			// Move down
			heliLocation.y -= moveSpeed * FRAME_TIME_SEC;
		}
	}

	if (heliLocation.y < minY) {
		heliLocation.y = minY;
		spinning = false;
	}
}

/*
	Initialise OpenGL lighting before we begin the render loop.
*/
void initLights(void)
{
	// Enable lighting
	glEnable(GL_LIGHTING);

	/// Directional light
	GLfloat light0_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat light0_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light0_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

	glEnable(GL_LIGHT0);

	// Set up the helicopter spotlight
	GLfloat light1_position[] = { 0.0f, 5.0f, 0.0f, 1.0f };
	GLfloat light1_direction[] = { 0.0f, -1.0f, 0.0f };
	GLfloat light1_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat light1_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light1_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_direction);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

	glEnable(GL_LIGHT1);

	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);
}

void drawGrid(float size, int squareSize) {
	int divisions = size / squareSize;
	float halfGridSize = size / 2.0f;

	GLfloat gridDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gridDiffuse);

	// Set the current texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureWidth, textureHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, groundTexture);
	// Enable texture mapping
	glEnable(GL_TEXTURE_2D);

	for (int i = 0; i < divisions; ++i) {
		for (int j = 0; j < divisions; ++j) {
			float x0 = -halfGridSize + i * squareSize;
			float z0 = -halfGridSize + j * squareSize;
			float x1 = x0 + squareSize;
			float z1 = z0 + squareSize;

			glBegin(GL_QUADS);
			glNormal3f(0.0f, 1.0f, 0.0f);

			glTexCoord2f(0, 0);
			glVertex3f(x0, 0.0f, z0);

			glTexCoord2f(2, 0);
			glVertex3f(x1, 0.0f, z0);

			glTexCoord2f(2, 2);
			glVertex3f(x1, 0.0f, z1);

			glTexCoord2f(0, 2);
			glVertex3f(x0, 0.0f, z1);
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void drawHelicopter() {
	// Define material properties
	GLfloat redDiffuse[] = {1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat yellowDiffuse[] = {1.0f, 1.0f, 0.0f, 1.0f};
	GLfloat blackDiffuse[] = {0.0f, 0.0f, 0.0f, 1.0f};

	glPushMatrix();
		glTranslatef(heliLocation.x, heliLocation.y, heliLocation.z);
		glRotatef(heliRotation, 0.0f, 1.0f, 0.0f);

		// Body
		glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
		glScalef(1.0f, 1.0f, 1.2f);
		glutSolidSphere(1.50f, 10, 10);

		// Main rotor
		glPushMatrix();
			glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowDiffuse);
			glTranslatef(0.0f, 1.50f, 0.0f);
			glScalef(1.0f, 1.0f, 1.0f);
			glRotatef(rotorAngle, 0.0f, 1.0f, 0.0f);

			// Draw four blades
			for (int i = 0; i < 4; ++i) {
				glPushMatrix();
					glRotatef(i * 90.0f, 0.0f, 1.0f, 0.0f);
					glTranslatef(0.0f, 0.0f, 0.0f);
					glScalef(6.0f, 0.1f, 0.2f);
					glutSolidCube(1.0);
				glPopMatrix();
			}
		glPopMatrix();

		// Tail
		glPushMatrix();
			glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
			glTranslatef(0.0f, 0.0f, -3.0f);
			glScalef(0.5f, 0.6f, 6.0f);
			glutSolidCube(1.0);

			glPushMatrix();
				glTranslatef(0.0f, 0.0f, -0.5f);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
				glScalef(1.0f, 1.0f, 0.16f);
				glutSolidSphere(0.5f, 20, 20);
			glPopMatrix();
		glPopMatrix();

		// Tail rotors
		glPushMatrix();
			glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowDiffuse);
			glTranslatef(0.0f, 0.0f, -6.0f);
			glRotatef(tailRotorAngle, 1.0f, 0.0f, 0.0f);
			for (int i = 0; i < 4; ++i) {
				glPushMatrix();
					glRotatef(i * 90.0f, 1.0f, 0.0f, 0.0f);
					glTranslatef(-0.3f, 0.0f, 0.0f);
					glScalef(0.1f, 0.2f, 1.5f);
					glutSolidCube(1.0);
				glPopMatrix();
			}
		glPopMatrix();

		// Landing skids
		glPushMatrix();
			glMaterialfv(GL_FRONT, GL_DIFFUSE, blackDiffuse);
			glTranslatef(0.5f, -1.50f, 0.0f);
			glScalef(0.3f, 1.0f, 0.3f);
			glutSolidCube(1.0);

			glPushMatrix();
				glMaterialfv(GL_FRONT, GL_DIFFUSE, blackDiffuse);
				glTranslatef(0.0f, -0.5f, 0.0f);
				glScalef(1.0f, 0.2f, 10.0f);
				glutSolidCube(1.0);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			glMaterialfv(GL_FRONT, GL_DIFFUSE, blackDiffuse);
			glTranslatef(-0.5f, -1.50f, 0.0f);
			glScalef(0.3f, 1.0f, 0.3f);
			glutSolidCube(1.0);

			glPushMatrix();
				glMaterialfv(GL_FRONT, GL_DIFFUSE, blackDiffuse);
				glTranslatef(0.0f, -0.5f, 0.0f);
				glScalef(1.0f, 0.2f, 10.0f);
				glutSolidCube(1.0);
			glPopMatrix();
		glPopMatrix();

	glPopMatrix();
}

void drawBuilding(float x, float z, float height) {
	glPushMatrix();
		glTranslatef(x, height / 2, z);
		glScalef(4.0f, height, 3.0f);

		GLfloat gridDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
		glMaterialfv(GL_FRONT, GL_DIFFUSE, gridDiffuse);
		// Set the current texture
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureWidth, textureHeight, 0,
			GL_RGB, GL_UNSIGNED_BYTE, buildingTexture);
		glEnable(GL_TEXTURE_2D);

		// Draw the building with tiled texture
		glBegin(GL_QUADS);

		// Front face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, height); glVertex3f(0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, height); glVertex3f(-0.5f, 0.5f, 0.5f);

		// Back face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, height); glVertex3f(0.5f, 0.5f, -0.5f);
		glTexCoord2f(0.0f, height); glVertex3f(-0.5f, 0.5f, -0.5f);

		// Left face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, height); glVertex3f(-0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, height); glVertex3f(-0.5f, 0.5f, -0.5f);

		// Right face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
		glTexCoord2f(1.0f, height); glVertex3f(0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, height); glVertex3f(0.5f, 0.5f, -0.5f);

		// Top face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

		glEnd();

		// Disable texture mapping
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void drawWindmill(float x, float z, float angle) {
	glPushMatrix();
		glTranslatef(x, 10.0f, z); // Position the windmill so that its base is at ground level

		// Draw base (20 meters high)
		GLfloat baseDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f}; // Grey color
		glMaterialfv(GL_FRONT, GL_DIFFUSE, baseDiffuse);
		glPushMatrix();
			glScalef(1.0f, 20.0f, 1.0f); // Scale to 20 meters high
			glutSolidCube(1.0);
		glPopMatrix();

		// Draw blades
		glTranslatef(0.0f, 10.0f, -0.5f); // Move to the top of the base
		glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate the blades

		GLfloat bladeDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; // White color
		glMaterialfv(GL_FRONT, GL_DIFFUSE, bladeDiffuse);
		for (int i = 0; i < 3; ++i) { // Draw 3 blades
			glPushMatrix();
				glRotatef(i * 120.0f, 0.0f, 0.0f, 1.0f); // 120 degrees apart
				glTranslatef(0.0f, 5.0f, 0.0f); // Adjust blade length
				glScalef(1.0f, 10.0f, 0.2f); // Scale blades
				glutSolidCube(1.0);
			glPopMatrix();
		}
	glPopMatrix();

}

void drawTree(float x, float z) {
    glPushMatrix();
        glTranslatef(x, 0, z);

        // Draw trunk (4 meters tall)
        GLfloat trunkDiffuse[] = {0.55f, 0.27f, 0.07f, 1.0f}; // Brown color
        glMaterialfv(GL_FRONT, GL_DIFFUSE, trunkDiffuse);
        glPushMatrix();
            glRotatef(-90, 1.0f, 0.0f, 0.0f); // Rotate cylinder to stand upright
            GLUquadric* quad = gluNewQuadric();
            gluCylinder(quad, 0.5, 0.2, 4.0, 20, 20); // Cylinder with height 4 meters
            gluDeleteQuadric(quad);
        glPopMatrix();

        // Draw leaves (2 meters tall)
        glTranslatef(0.0f, 4.0f, 0.0f); // Position leaves at the top of the trunk
        GLfloat leavesDiffuse[] = {0.0f, 0.8f, 0.0f, 1.0f}; // Green color
        glMaterialfv(GL_FRONT, GL_DIFFUSE, leavesDiffuse);
        glPushMatrix();
            glRotatef(-90, 1.0f, 0.0f, 0.0f); // Rotate cone to stand upright
            glScalef(1.0f, 1.0f, 1.0f); // Uniform scaling
            glutSolidCone(1.0, 2.0, 20, 20); // Adjusted height to 2 meters
        glPopMatrix();
    glPopMatrix();
}


/*
	Read file and create texture
*/
void createTexture(GLubyte(*myTexture)[textureWidth][textureHeight][3], char fileName[50])
{
	char headerLine[100];
	char tempChar;
	int imageWidth, imageHeight, maxValue, totalPixels;
	int red, green, blue;
	float RGBScaling;

	FILE* fileID = fopen(fileName, "r");
	fscanf(fileID, "%[^\n] ", headerLine);

	if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
	{
		printf("This is not a PPM file!\n");
		exit(0);
	}

	printf("This is a PPM file\n");
	fscanf(fileID, "%c", &tempChar);

	// While there is still comment lines
	while (tempChar == '#')
	{
		fscanf(fileID, "%[^\n] ", headerLine);
		printf("%s\n", headerLine);
		fscanf(fileID, "%c", &tempChar);
	}

	ungetc(tempChar, fileID);
	fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);
	// print out the information about the image file
	printf("%d rows  %d columns  max value= %d\n", imageHeight, imageWidth, maxValue);
	totalPixels = imageWidth * imageHeight;
	RGBScaling = 255.0 / maxValue;

	GLubyte *imageData;
	imageData = malloc(3 * sizeof(GLuint) * totalPixels);
	if (maxValue == 255)
	{
		for (int i = 0; i < totalPixels; i++)
		{
			// Read in the current pixel from the file
			fscanf(fileID, "%d %d %d", &red, &green, &blue);

			// Store the red, green, and blue data of the current pixel in the data array
			imageData[3 * i] = (GLubyte)red;
			imageData[3 * i + 1] = (GLubyte)green;
			imageData[3 * i + 2] = (GLubyte)blue;
		}
	}
	else  // Need to scale up the data values
	{
		for (int i = 0; i < totalPixels; i++)
		{
			// Read in the current pixel from the file
			fscanf(fileID, "%d %d %d", &red, &green, &blue);

			// Store the red, green, and blue data of the current pixel in the data array
			imageData[3 * i] = (GLubyte)(red * RGBScaling);
			imageData[3 * i + 1] = (GLubyte)(green * RGBScaling);
			imageData[3 * i + 2] = (GLubyte)(blue * RGBScaling);
		}
	}

	int s, t;

	// enable texture mapping
	glEnable(GL_TEXTURE_2D);

	// for each texture element
	for (s = 0; s < textureWidth; s++)
	{
		for (t = 0; t < textureHeight; t++)
		{
			(*myTexture)[s][t][0] = imageData[3 * (s * textureHeight + t)];
			(*myTexture)[s][t][1] = imageData[3 * (s * textureHeight + t) + 1];
			(*myTexture)[s][t][2] = imageData[3 * (s * textureHeight + t) + 2];
		}
	}

	// tell openGL how to scale the texture image up if needed
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// tell openGL how to scale the texture image down if needed
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// set the wrapping function for the S and T texture directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	free(imageData);
}

/******************************************************************************/