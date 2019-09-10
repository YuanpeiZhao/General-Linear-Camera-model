#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl_core_3_3.h"
#include <GL/freeglut.h>
#include "util.hpp"
#include "mesh.hpp"
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "sphere.h"
#include "TriangleMesh.h"
#include "hitable_list.h"
#include "float.h"
#include "GLCcamera.h"

using namespace std;
using namespace glm;

GLCcamera *cam;

// Global state
GLint width, height;
GLuint shader;			// Shader program
GLuint uniXform;		// Shader location of xform mtx
GLuint vao;				// Vertex array object
GLuint vbuf;			// Vertex buffer
GLuint EBO;
GLsizei vcount;			// Number of vertices
Mesh* mesh;				// Mesh loaded from .obj file
// Camera state
vec3 camCoords;			// Spherical coordinates (theta, phi, radius) of the camera
bool camRot;			// Whether the camera is currently rotating
vec2 camOrigin;			// Original camera coordinates upon clicking
vec2 mouseOrigin;		// Original mouse coordinates upon clicking

GLuint texture;

// Constants
const int MENU_PIN = 0;		// pinhole/perspective
const int MENU_ORT = 1;		// orthographic
const int MENU_XSL = 2;		// XSlit
const int MENU_REV_X = 3;	// reverse the X-axis
const int MENU_REV_Y = 4;	// reverse the Y-axis
const int MENU_EXIT = 5;			// Exit application

vec3 a1, a2, b1, b2, c1, c2;

// Initialization functions
void initState();
void initGLUT(int* argc, char** argv);
void initOpenGL();
void initViewPlane();

// Callback functions
void display();
void mouseBtn(int button, int state, int x, int y);
void mouseMove(int x, int y);
void menu(int cmd);
void cleanup();

GLuint LoadTexture(const char* filename);

int main(int argc, char** argv) {
	try {
		// Initialize
		initState();
		initGLUT(&argc, argv);
		initOpenGL();
		//initTriangle();
		initViewPlane();

	} catch (const exception& e) {
		// Handle any errors
		cerr << "Fatal error: " << e.what() << endl;
		cleanup();
		return -1;
	}

	// Execute main loop
	glutMainLoop();

	return 0;
}

void initState() {
	// Initialize global state
	width = 0;
	height = 0;
	shader = 0;
	uniXform = 0;
	vao = 0;
	vbuf = 0;
	vcount = 0;
	mesh = NULL;

	camCoords = vec3(0.0, 0.0, -1.0);
	camRot = false;

	a1 = vec3(0.0f, 0.0f, 0.0f);
	b1 = vec3(0.0f, 0.2f, 0.0f);
	c1 = vec3(0.2f, 0.0f, 0.0f);
	a2 = vec3(0.0f, 0.0f, -1.0f);
	b2 = vec3(0.0f, 1.0f, -1.0f);
	c2 = vec3(1.0f, 0.0f, -1.0f);
	cam = new GLCcamera(a1, b1, c1, a2, b2, c2);
}

void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	width = 1024; height = 1024;
	glutInit(argc, argv);
	glutInitWindowSize(width, height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("FreeGlut Window");

	// Create a menu
	glutCreateMenu(menu);
	glutAddMenuEntry("Camera mode : pinhole", MENU_PIN);
	glutAddMenuEntry("Camera mode : orthographic", MENU_ORT);
	glutAddMenuEntry("Camera mode : XSlit", MENU_XSL);

	glutAddMenuEntry("Reverse X-axis", MENU_REV_X);
	glutAddMenuEntry("Reverse Y-axis", MENU_REV_Y);

	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// GLUT callbacks
	glutDisplayFunc(display);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutCloseFunc(cleanup);
}

void initOpenGL() {
	// Set clear color and depth
	glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
	glClearDepth(1.0f);
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Compile and link shader program
	vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "sh_v.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "sh_f.glsl"));
	shader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();
	// Locate uniforms
	uniXform = glGetUniformLocation(shader, "xform");
	assert(glGetError() == GL_NO_ERROR);
}

void initViewPlane() {
	float vertices[] = {
		// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbuf);
	glGenBuffers(1, &EBO);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	texture = LoadTexture("test.jpg");
}

GLuint LoadTexture(const char* filename)
{

	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}

vec3 random_vec() {

	return (2.0f * vec3(rand() / 1.0f / MAXINT16, rand() / 1.0f / MAXINT16, rand() / 1.0f / MAXINT16) - vec3(1, 1, 1)) / 2.0f;
}

vec3 color(const ray& r, hitable *world) {
	hit_record rec;
	if (world->hit(r, 0.01f, FLT_MAX, rec)) {
		vec3 target = rec.p + rec.normal + random_vec();
		return 0.8f * rec.color * color(ray(rec.p, target-rec.p), world);
		//return 0.5f * vec3(rec.normal.x + 1, rec.normal.y + 1, rec.normal.z + 1);
	}
	else
	{
		vec3 unit_direction = normalize(r.direction());
		float t = 0.5f * (unit_direction.y + 1.0f);
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
	}
}

void WriteTexture(const char* filename)
{
	int width = 512;
	int height = 512;
	uint8_t* pixels = new uint8_t[width * height * 3];

	vec3 lower_left_corner(-1.0f, -1.0f, -1.0f);
	vec3 horizontal(2.0f, 0.0f, 0.0f);
	vec3 vertical(0.0f, 2.0f, 0.0f);
	vec3 origin(0.0f, 0.0f, 0.0f);

	hitable* list[6];
	list[0] = new sphere(vec3(-1, 0, -7), 0.5, vec3(1.0f, 1.0f, 0.0f));
	list[1] = new sphere(vec3(-1, 0, -4), 0.5, vec3(0.0f, 1.0f, 1.0f));
	list[2] = new sphere(vec3(1, 0, -7), 0.5, vec3(1.0f, 0.0f, 1.0f));
	list[3] = new sphere(vec3(1, 0, -4), 0.5, vec3(1.0f, 1.0f, 1.0f));
	list[4] = new triangleMesh(vec3(-2, -0.5, -2),vec3(-2, -0.5, -10),vec3(2, -0.5, -2), vec3(1.0f, 0.0f, 0.0f));
	list[5] = new triangleMesh(vec3(2, -0.5, -2),vec3(-2, -0.5, -10),vec3(2, -0.5, -10), vec3(0.0f, 1.0f, 0.0f));

	hitable* world = new hitable_list(list, 6);
	
	cam->setCoords(camCoords);

	int index = 0;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			float u = (float)i / (float)width;
			float v = (float)j / (float)height;

			ray r = cam->genRay(lower_left_corner + u * horizontal + v * vertical);
			vec3 col = color(r, world);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			pixels[index++] = ir;
			pixels[index++] = ig;
			pixels[index++] = ib;
		}
	}

	stbi_write_jpg(filename, width, height, 3, pixels, 100);
	delete[] pixels;
}

void display() {
	try {
		// Clear the back buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get ready to draw
		glUseProgram(shader);

		mat4 xform;
		float aspect = (float)width / (float)height;
		// Create perspective projection matrix
		mat4 proj = perspective(45.0f, aspect, 0.1f, 100.0f);
		// Create view transformation matrix
		mat4 view = translate(mat4(1.0f), vec3(0.0, 0.0, -camCoords.z));
		mat4 rot = rotate(mat4(1.0f), radians(camCoords.y), vec3(1.0, 0.0, 0.0));
		rot = rotate(rot, radians(camCoords.x), vec3(0.0, 1.0, 0.0));
		xform = proj * view * rot;

		WriteTexture("test.jpg");
		texture = LoadTexture("test.jpg");
		// bind Texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// render container
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		assert(glGetError() == GL_NO_ERROR);

		// Revert context state
		glUseProgram(0);

		// Display the back buffer
		glutSwapBuffers();

	} catch (const exception& e) {
		cerr << "Fatal error: " << e.what() << endl;
		glutLeaveMainLoop();
	}
}

void mouseBtn(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		// Activate rotation mode
		camRot = true;
		camOrigin = vec2(camCoords);
		mouseOrigin = vec2(x, y);
	}
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		// Deactivate rotation
		camRot = false;
	}
}

void mouseMove(int x, int y) {
	if (camRot) {
		// Convert mouse delta into degrees, add to rotation
		float rotScale = min(width / 450.0f, height / 270.0f);
		vec2 mouseDelta = vec2(x, y) - mouseOrigin;
		vec2 newAngle = camOrigin + mouseDelta / rotScale;
		newAngle.y = clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.y < -180.0f) newAngle.y += 360.0f;
		if (length(newAngle - vec2(camCoords)) > FLT_EPSILON) {
			camCoords.x = newAngle.x;
			camCoords.y = newAngle.y;
			glutPostRedisplay();
		}
	}
}

void menu(int cmd) {
	switch (cmd) {
	case MENU_PIN:
		a1 = vec3(0.0f, 0.0f, 0.0f);
		b1 = vec3(0.0f, 0.2f, 0.0f);
		c1 = vec3(0.2f, 0.0f, 0.0f);
		a2 = vec3(0.0f, 0.0f, -1.0f);
		b2 = vec3(0.0f, 1.0f, -1.0f);
		c2 = vec3(1.0f, 0.0f, -1.0f);
		cam->reset(a1, b1, c1, a2, b2, c2);
		glutPostRedisplay();	// Tell GLUT to redraw the screen
		break;

	case MENU_ORT:
		a1 = vec3(0.0f, 0.0f, 0.0f);
		b1 = vec3(0.0f, 1.0f, 0.0f);
		c1 = vec3(1.0f, 0.0f, 0.0f);
		a2 = vec3(0.0f, 0.0f, -1.0f);
		b2 = vec3(0.0f, 1.0f, -1.0f);
		c2 = vec3(1.0f, 0.0f, -1.0f);
		cam->reset(a1, b1, c1, a2, b2, c2);
		glutPostRedisplay();	// Tell GLUT to redraw the screen
		break;

	case MENU_XSL:
		a1 = vec3(0.0f, 0.0f, 0.0f);
		b1 = vec3(0.0f, 1.0f, 0.0f);
		c1 = vec3(1.0f, 0.0f, 0.0f);
		a2 = vec3(1.0f, 0.0f, -1.0f);
		b2 = vec3(0.0f, 0.0f, -1.0f);
		c2 = vec3(0.0f, 1.0f, -1.0f);
		cam->reset(a1, b1, c1, a2, b2, c2);
		glutPostRedisplay();	// Tell GLUT to redraw the screen
		break;

	case MENU_REV_X:
		cam->reverseX();
		break;
		
	case MENU_REV_Y:
		cam->reverseY();
		break;

	case MENU_EXIT:
		glutLeaveMainLoop();
		break;
	}
}

void cleanup() {
	// Release all resources
	if (shader) { glDeleteProgram(shader); shader = 0; }
	uniXform = 0;
	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	vcount = 0;
	if (mesh) { delete mesh; mesh = NULL; }
}