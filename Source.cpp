/*
* Author: Brigitte Rollain
* Date: November 12, 2023
* Reference: Brian Battersby meshes.h, meshes.cpp source.cpp
*
* Updated: November 19,2023
*
* Updated: November 27, 2023
*
* Updated: November 29 - December 3, 2023
*/


#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

#include <camera.h>     // camera class

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshes.h"
#include "../includes/learnOpengl/camera.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Mod 7 Final"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 800;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	//GLMesh gMesh;
	// Texture ids
	GLuint gTexture1Id;
	GLuint gTexture2Id;
	GLuint gTexture3Id;
	GLuint gTexture4Id;
	GLuint gTexture5Id;
	GLuint gTexture6Id;
	GLuint gTexture7Id;

	/*glm::vec2 gUVScale(5.0f, 5.0f);
	GLint gTexWrapMode = GL_REPEAT;*/
	// Shader program
	GLuint gProgramId;
	GLuint gLampProgramId;

	//Shape Meshes from Professor Brian
	Meshes meshes;

	// camera
	Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));

	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	//timing
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// Cube and light color
	glm::vec3 gLightColor(0.7f, 1.0f, 0.0f);

	// Light position and scale
	glm::vec3 gLightPosition(0.0f, 4.5f, 4.0f);
	glm::vec3 gLightScale(1.0f);

	// variable to handle ortho change
	bool perspective = false;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos); // Change the orientation of the camera
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset); // Adjust speed of movement
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods); // Get the input for mouse button use
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//Make texture
bool UCreateTexture(const char* filename, GLuint& textureId);
//destroy texture
void UDestroyTexture(GLuint textureId);

////////////////////////////////////////////////////////////////////////////////////////
// SHADER CODE
/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 vertexPosition; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec3 vertexNormal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexFragmentNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;
//out vec4 vertexColor; // variable to transfer color data to the fragment shader
//out vec2 vertexTextureCoordinate;

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(vertexPosition, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexFragmentNormal = mat3(transpose(inverse(model))) * vertexNormal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec3 vertexFragmentNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec4 objectColor;
uniform vec3 ambientColor;
uniform vec3 light1Color = vec3(0.8f, 0.7f, 0.3f);;
uniform vec3 light1Position;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform bool ubHasTexture;
uniform float ambientStrength = 0.1f; // Set ambient or global lighting strength
uniform float specularIntensity1 = 0.8f;
uniform float highlightSize1 = 16.0f;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting
	vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

	//**Calculate Diffuse lighting**
	vec3 norm = normalize(vertexFragmentNormal); // Normalize vectors to 1 unit
	vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color

	//**Calculate Specular lighting**
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
	vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;

	//**Calculate phong result**
	//Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate);
	vec3 phong1;

	if (ubHasTexture == true)
	{
		phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
	}
	else
	{
		phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
	}

	fragmentColor = vec4(phong1, 1.0); // Send lighting results to GPU
	//fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
);
///////////////////////////////////////////////////////////////////////////////////////

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

		//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing lamp color (smaller mesh) to the GPU

void main()
{
	fragmentColor = vec4(0.8f, 0.7f, 0.4f, 1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	//UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
	meshes.CreateMeshes();

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	if(!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
		return EXIT_FAILURE;

	// Load textures
	const char* texFilename = "CubeTexture1.jpg";
	if (!UCreateTexture(texFilename, gTexture1Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "CylinderCapTexture1.jpg";
	if (!UCreateTexture(texFilename, gTexture2Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "CoinPurse.jpg";
	if (!UCreateTexture(texFilename, gTexture3Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "CylinderBaseBorder.jpg";
	if (!UCreateTexture(texFilename, gTexture4Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "whitemarble.jpg";
	if (!UCreateTexture(texFilename, gTexture5Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "TorusTex.jpg";
	if (!UCreateTexture(texFilename, gTexture6Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	texFilename = "cPurse.png";
	if (!UCreateTexture(texFilename, gTexture7Id))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexture1Id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTexture2Id);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gTexture3Id);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gTexture4Id);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gTexture5Id);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gTexture6Id);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gTexture7Id);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gCamera.Position = glm::vec3(0.0f, 1.0f, 16.0f);
	gCamera.Front = glm::vec3(0.0, 0.0, -1.0f);
	gCamera.Up = glm::vec3(0.0, 1.0, 0.0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	meshes.DestroyMeshes();

	// Release shader program
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gLampProgramId);

	// Release texture
	UDestroyTexture(gTexture1Id);
	UDestroyTexture(gTexture2Id);
	UDestroyTexture(gTexture3Id);
	UDestroyTexture(gTexture4Id);
	UDestroyTexture(gTexture5Id);
	UDestroyTexture(gTexture6Id);


	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	// Added:
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// Tells GLFW to capture the mouse:
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	float velocity = gCamera.MovementSpeed * gDeltaTime;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.Position += gCamera.Up * velocity;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.Position -= gCamera.Up * velocity;
	

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		perspective = false;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		perspective = true;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//glfw: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (gFirstMouse) {
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX; // x-coordinates go top to bottom
	float yoffset = gLastY - ypos; // reversed b/c y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw:: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	gCamera.ProcessMouseScroll(yoffset);
}

//glfw: handle mouse button events
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed!" << endl;
		else
			cout << "Left mouse button released!" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed!" << endl;
		else
			cout << "Middle mouse button released!" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed!" << endl;
		else
			cout << "Right mouse button released!" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event, FOOL!" << endl;
		break;

	}
}


// Functioned called to render a frame
void URender()
{
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint viewPosLoc;
	GLint ambStrLoc;
	GLint ambColLoc;
	GLint light1ColLoc;
	GLint light1PosLoc;
	GLint objColLoc;
	GLint specInt1Loc;
	GLint highlghtSz1Loc;
	GLint uHasTextureLoc;
	bool ubHasTextureVal;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 rotation1;
	glm::mat4 rotation2;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//camera/view transformation
	view = gCamera.GetViewMatrix();

	if (!perspective)
	{
		// P for Perspective 
		//projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
		//projection = glm::perspective(glm::radians(60.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else {
		// O for Orthographic
		view = glm::translate(glm::vec3(0.0f, -3.8f, -12.0f)); // To not view plane the camera has to be adjusted 
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.9f, 100.0f);
	}

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	viewPosLoc = glGetUniformLocation(gProgramId, "viewPosition");
	ambStrLoc = glGetUniformLocation(gProgramId, "ambientStrength");
	ambColLoc = glGetUniformLocation(gProgramId, "ambientColor");
	light1ColLoc = glGetUniformLocation(gProgramId, "light1Color");
	light1PosLoc = glGetUniformLocation(gProgramId, "light1Position");
	objColLoc = glGetUniformLocation(gProgramId, "objectColor");
	specInt1Loc = glGetUniformLocation(gProgramId, "specularIntensity1");
	highlghtSz1Loc = glGetUniformLocation(gProgramId, "highlightSize1");
	uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the camera view location
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);
	//set ambient lighting strength
	glUniform1f(ambStrLoc, 2.4f);
	//set ambient color
	glUniform3f(ambColLoc, 0.2f, 0.2f, 0.2f);
	glUniform3f(light1ColLoc, 0.8f, 0.7f, 0.6f);
	glUniform3f(light1PosLoc, 0.0f, 2.0f, 4.0f);
	//set specular intensity
	glUniform1f(specInt1Loc, .6f);
	//set specular highlight size
	glUniform1f(highlghtSz1Loc, 12.0f);


	// ----- Start Plane ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(6.0f, 1.0f, 6.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	

	// Make false for coin purse & use: glUniform4f(objColLoc, 1.0f, 1.0f, 1.0f, 1.0f);
	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	// Point to the texture variable to texture unit 4 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 4);

	// Draws the triangles for the plane
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ----- End Plane ------

	

	// ----- Start Cube ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
	// 2. Rotate the object
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.5f, 0.759f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.7f, 0.5f, 0.0f, 1.0f);

	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(1);
	// ----- End Cube ------

	


	// Lip Balm Cap:
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.3f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(90.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 0.55f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));



	// Draws the triangles
	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	// Use colors: 
	ubHasTextureVal = false;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	glUniform4f(objColLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom

	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	//GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	//glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	// Use colors: 
	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);

	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 1);
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top

	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	// Use colors: 
	ubHasTextureVal = false;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	glUniform4f(objColLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// --------------------END Lip Balm Cap--------------------------------

	// ------- Lip Balm center: -------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.4f, 0.2f, 0.4f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 0.35f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Use colors: 
	ubHasTextureVal = false;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	glUniform4f(objColLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// --------------------END Lip Balm center-------------------------------- 

	// ----- Start Lip Balm Base: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 0.001f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	
	// Draws the triangles
	// Use colors: 
	ubHasTextureVal = false;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	glUniform4f(objColLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);

	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 3);
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ------------------- END Lip Balm Base:--------------------------------- 




	//For the fidget Toy
	// ----- Torus Start: -----
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.5f, 1.5f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.8f, 0.2f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ----- END Torus -----

	// ----- Start Left Cylinder: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.09f, 0.4f, 0.09f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.9f, 0.2f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ------------------- END  Left Cylinder:--------------------------------- 

	// ----- Start Right Cylinder: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.09f, 0.4f, 0.09f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.3f, 0.2f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ------------------- END Right Cylinder:--------------------------------- 

	// ----- Start Left Tapered Cylinder: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.2003f, 0.06f, 0.2f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.87f, 0.2f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ------------------- END Left Tapered Cylinder:--------------------------------- 

	// ----- Start Right Tapered Cylinder: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.2003f, 0.06f, 0.2f));
	// 2. Rotate the object
	rotation = glm::rotate(99.0f, glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.74f, 0.2f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	//glBindVertexArray(0);
	// ------------------- END Right Tapered Cylinder:--------------------------------- 
	


	// COIN PURSE:
	// // ------------------- START Left Pyramid:--------------------------------- 
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPyramid4Mesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.8f, 1.8f, 0.2f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.0f, 0.901f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	// Point to the texture variable to texture unit 4 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 6);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ---------------------- END Left Pyramid -------------------------------------

	// ----- Start Back Plane ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.8f, 0.8f, 0.83f));
	// 2. Rotate the object
	rotation = glm::rotate(30.064f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.8f, 0.8f, -0.222f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));



	// Draws the triangles for the plane
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ----- End Back Plane ------

	// ----- Start Front Plane ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.8f, 0.8f, 0.83f));
	// 2. Rotate the object
	rotation = glm::rotate(70.468f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.8f, 0.8f, 0.226f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	// Point to the texture variable to texture unit 4 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 2);

	// Draws the triangles for the plane
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ----- End Front Plane ------

	// ----- Start Top Cylinder: ------
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.155f, 1.6f, 0.06f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(2.59f, 1.679f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);
	// Point to the texture variable to texture unit 4 before drawing the shape mesh
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 6);

	// Point to the texture variable to texture unit 0 before drawing the shape mesh
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ------------------- END Top Cylinder:--------------------------------- 

	// ------------------- START Right Pyramid:--------------------------------- 
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPyramid4Mesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.8f, 1.8f, 0.2f));
	// 2. Rotate the object
	rotation = glm::rotate(33.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(2.58f, 0.901f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// ---------------------- END Right Pyramid -------------------------------------

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)

	

	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}