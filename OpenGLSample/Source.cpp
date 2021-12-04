#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Using GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "camera.h"
#include "ShapeData.h"
#include "ShapeGenerator.h"
#include "cylinder.h"
#include "torus.h"
#include "Sphere.h"

// Cylinder Structure for VAO,VBO
struct CylinderMesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int texture;
	unsigned int vertices;
};

// Plane Structure for VAO, VBO and other variables
struct PlaneMesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int texture;
	unsigned int texture2;
	unsigned int vertices;
	unsigned int planeNumIndices;
	unsigned int planeIndexByteOffset;
	unsigned int planeVertexArrayObjectID;
};

// Structure for torus buffers and vertices
struct TorusMesh {
	int nVertices;
	GLfloat* uvData;
	unsigned int vao;
	GLfloat* vertexData;
	unsigned int texture;
	unsigned int uvBuffer;
	unsigned int vertexBuffer;
}; 

struct SphereMesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int texture;
	unsigned int sphereIndexByteOffset;
	unsigned int sphereNumIndices;
};

// Cube Structure for VAO,VBO
struct CubeMesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int texture;
	unsigned int vertices;
};

// User Utility Functions
void windowResize(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadSphereTexture(char const* path);
bool initializeWindow(GLFWwindow** window);

// Plane
void planeMeshCreation(PlaneMesh& mesh, int planeDimension);
void planeMeshDeletion(PlaneMesh& mesh);
void planeRender(PlaneMesh& mesh, unsigned int& shader, glm::mat4 MVP);

// Cube
void containerMeshCreation(CubeMesh& mesh);
void cubeMeshCreation(CubeMesh& mesh);
void cubeMeshDeletion(CubeMesh& mesh);
void cubeRender(CubeMesh& mesh, unsigned int& shader, glm::mat4 MVP);

// Cylinder 
void cylinderMeshCreation(CylinderMesh& mesh); // Create Cylinder vertices
void cylinderMeshDeletion(CylinderMesh& mesh); // Delete cylinder buffers
void cylinderRender(const static_meshes_3D::Cylinder& cylinder, CylinderMesh& mesh, unsigned int& shader, glm::mat4 MVP); // Render the cylinder

// Torus
Torus torusMeshCreation(TorusMesh& mesh, float innerRadius, float outterRadius); // Create vertices for torus
void torusMeshDeletion(TorusMesh& mesh); // Delete buffers and pointers for torus
void torusRender(TorusMesh& mesh, unsigned int& shader, glm::mat4 MVP); // Render Torus

// Shader Functions
bool createShaders(const char* vertexShaderSource, const char* fragmentShaderSource, unsigned int& programID); // Link shaders
void destroyShaderProgram(unsigned int& program); // Used to destroy shader programs

// Mosue and scroll related
void getMousePosition(GLFWwindow* window, double xpos, double ypos);
void scrollMouseWheel(GLFWwindow* window, double xoffset, double yoffset);

// Globala Variables/Constants
const unsigned int WINDOW_WIDTH = 1400;
const unsigned int WINDOW_HEIGHT = 1000;
const char* WINDOW_TITLE = "Datt Patel Milestone 6";

static bool perspectiveVal = true; // For switching perspectives

// Window Pointer
GLFWwindow* window = nullptr;

// Camera Details
Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // time difference between current frame and last frame
float lastFrame = 0.0f;

// Global Variables
// General Shaders
const char* vertexShader = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 textureCoords;\n"
"layout (location = 2) in vec3 aNormal;\n"
"out vec3 Normal;\n"
"out vec3 FragPos;\n"
"uniform mat4 model;\n"
"uniform mat4 MVP;\n"
"out vec2 textCoord;\n"
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(aPos, 1.0));\n"
"   Normal = mat3(transpose(inverse(model))) * aNormal;;\n"
"	gl_Position = MVP * vec4(aPos, 1.0f); \n"
"	textCoord = textureCoords;\n"
"}\0";

const char* fragmentShader = "#version 330 core\n"
"out vec4 FragColor; \n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"uniform vec4 ourColor; \n"
"in vec2 textCoord;"
"struct LightSource {\n"
"    vec3 position;\n"
"    vec3 ambientStr;\n"
"    vec3 specular;\n"
"	 vec3 diffuse;\n"
"	 float constant;\n"
"    float linear;\n"
"    float quadratic;\n"
"};\n"
"uniform float shininess;\n"
"uniform vec3 viewPosition;\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D specularTexture;\n"
"uniform vec2 uvScale;\n"
"uniform LightSource light1;\n"
"void main() {\n"
"vec3 norm = normalize(Normal);\n"
"vec3 viewDir = normalize(viewPosition - FragPos);\n"
"vec3 ambient1 = light1.ambientStr * texture(diffuseTexture, textCoord * uvScale).rgb;\n"
"vec3 lightDir1 = normalize(light1.position - FragPos);\n"
"float diff1 = max(dot(norm, lightDir1), 0.0);\n"
"vec3 diffuse1 = light1.diffuse * diff1 * texture(diffuseTexture, textCoord * uvScale).rgb;\n"
"vec3 reflectDir1 = reflect(-lightDir1, norm);\n"
"float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), shininess);\n"
"vec3 specular1 = light1.specular * specularComponent1 * texture(specularTexture, textCoord * uvScale).rgb;\n"
"float distance = length(light1.position - FragPos);\n"
"float attenuation = 1.0 / (light1.constant + light1.linear * distance + light1.quadratic * (distance * distance));\n"
"vec3 phong = ((ambient1*attenuation) + (diffuse1*attenuation) + (specular1*attenuation));\n"
"FragColor = vec4(phong, 1.0);\n"
"}\0";

// Light Shaders
const char* lightVertexShader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"uniform mat4 MVP;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(aPos, 1.0);\n"
"}\0";

const char* lightFragmentShader = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0);\n"
"}\0";

// Plane variables
const uint NUM_FLOATS_PER_VERTICE = 9;
const uint VERTEX_BYTE_SIZE = NUM_FLOATS_PER_VERTICE * sizeof(float);

float xLight = -0.74f;
float yLight = 0.66f;
float zLight = 1.12f;

// Shader programs
unsigned int shaderProgram;
unsigned int lightShader;

int main()
{
	if (!initializeWindow(&window)) {
		std::cout << "Error in intializing window" << std::endl;
		return -1;
	}

	// Initialize Shaders
	if (!createShaders(vertexShader, fragmentShader, shaderProgram)) {
		std::cout << "Failure in Plane shader creation/compilation/linking." << std::endl;
		return -1;
	}
	if (!createShaders(lightVertexShader, lightFragmentShader, lightShader)) {
		std::cout << "Failure in Light shader creation/compilation/linking." << std::endl;
		return -1;
	}

	// Mesh for plane
	PlaneMesh planeMesh;
	PlaneMesh lightWindow;

	// Cube Mesh
	CubeMesh container;
	CubeMesh containerBump;
	CubeMesh lidBottom;
	CubeMesh lidTop;

	// Create the Cylinder Mesh and Colors
	CylinderMesh cylinderMesh;
	CylinderMesh cylinderTwoMesh;

	// Creating cylinder mesh
	cylinderMeshCreation(cylinderMesh);
	cylinderMeshCreation(cylinderTwoMesh);

	// Creating Torus Mesh
	TorusMesh torusMesh;

	// Creating Sphere
	Sphere ball(0.5, 50, 50);

	// Creating the objects
	planeMeshCreation(planeMesh, 4);
	planeMeshCreation(lightWindow, 2);
	containerMeshCreation(container);
	cubeMeshCreation(containerBump);
	cubeMeshCreation(lidBottom);
	cubeMeshCreation(lidTop);
	Torus aTorus = torusMeshCreation(torusMesh, 0.03f, 0.055f); // FIXME: Torus = No way there is no normal one only UV
	auto cylinder1 = static_meshes_3D::Cylinder(0.15f, 30.0f, 0.5f, true, true, true);
	auto cylinder2 = static_meshes_3D::Cylinder(0.01f, 30.0f, 0.1f, true, true, true);

	// Binding Textures to meshes
	planeMesh.texture = loadTexture("images/table.jpg");
	planeMesh.texture2 = loadTexture("images/tableDark.jpg");
	lidTop.texture = loadTexture("images/lid.jpg");
	lidBottom.texture = loadTexture("images/lid.jpg");
	container.texture = loadTexture("images/container.jpg");
	containerBump.texture = loadTexture("images/container.jpg");
	cylinderMesh.texture = loadTexture("images/candleEdit.jpg");
	cylinderTwoMesh.texture = loadTexture("images/candleLit.jpg");
	torusMesh.texture = loadTexture("images/CandleTop.jpg");
	unsigned int sphereText = loadSphereTexture("images/ball.jpg");
	
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Handle input
		processInput(window);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable Z-depth.
		glEnable(GL_DEPTH_TEST);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Light Shader
		glUseProgram(lightShader);

		glm::mat4 model = glm::mat4(1.0f);// Model
		model = glm::translate(model, glm::vec3(xLight, yLight, zLight));
		model = glm::scale(model, glm::vec3(0.6f, 0.5f, 0.6f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.5f, 0.0f, 0.0f));
		glm::mat4 view = camera.GetViewMatrix(); // View
		glm::mat4 projection = glm::mat4(1.0f); // Projection
		if (perspectiveVal) {
			projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
		}
		else {
			float scale = 350.0f;
			projection = glm::ortho(-((float)WINDOW_WIDTH) / scale, ((float)WINDOW_WIDTH) / scale, -((float)WINDOW_HEIGHT) / scale, ((float)WINDOW_HEIGHT) / scale, 2.0f, 10.0f);
		}
		glm::mat4 MVP = projection * view * model;// Calculate MVP

		// Light
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		planeRender(lightWindow, lightShader, MVP);

		// Activate shader
		glUseProgram(shaderProgram);

		glm::vec2 gUVScale(2.0f, 2.0f);
		glUniform2fv(glGetUniformLocation(shaderProgram, "uvScale"), 1, glm::value_ptr(gUVScale));
		glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.Position));

		// Various Spotlight light info
		glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 32.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "light1.position"),xLight, yLight, zLight);
		glUniform3f(glGetUniformLocation(shaderProgram, "light1.ambientStr"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(shaderProgram, "light1.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(shaderProgram, "light1.specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shaderProgram, "light1.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shaderProgram, "light1.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(shaderProgram, "light1.quadratic"), 0.032f);

		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeMesh.texture);

		glUniform1i(glGetUniformLocation(shaderProgram, "specularTexture"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, planeMesh.texture2);

		model = glm::mat4(1.0f);// Model
		model = glm::translate(model, glm::vec3(-0.38f, -0.26f, -0.3f)); 
		MVP = projection * view * model;// Calculate MVP
		
		// Plane
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		planeRender(planeMesh, shaderProgram, MVP);

		// Container Render
		// Container Lid Top
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, lidTop.texture);
		glm::vec3 lidTopPos = glm::vec3(-1.2f, 0.41f, -0.6f);
		model = glm::mat4(1.0f);// Model
		model = glm::translate(model, lidTopPos);
		model = glm::scale(model, glm::vec3(0.6f, 0.05f, 0.6f));
		MVP = projection * view * model;// Calculate MVP
		cubeRender(lidTop, shaderProgram, MVP);

		// Container Lid Bottom
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, lidBottom.texture);
		glm::vec3 lidBottomPos = glm::vec3(lidTopPos.x, lidTopPos.y - 0.02f, lidTopPos.z);
		model = glm::mat4(1.0f);// Model
		model = glm::translate(model, lidBottomPos);
		model = glm::scale(model, glm::vec3(0.65f, 0.05f, 0.65f));
		MVP = projection * view * model;// Calculate MVP
		cubeRender(lidBottom, shaderProgram, MVP);
		
		// Container Bump
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, containerBump.texture);
		glm::vec3 containerBumpPos = glm::vec3(lidBottomPos.x, lidBottomPos.y - 0.02f, lidBottomPos.z);
		model = glm::mat4(1.0f);// Model
		model = glm::translate(model, containerBumpPos);
		model = glm::scale(model, glm::vec3(0.7f, 0.05f, 0.7f));
		MVP = projection * view * model;// Calculate MVP
		cubeRender(containerBump, shaderProgram, MVP);

		// Container
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, container.texture);
		model = glm::mat4(1.0f);// Model
		glm::vec3 conatinerPos = glm::vec3(containerBumpPos.x, containerBumpPos.y - 0.3, containerBumpPos.z);
		model = glm::translate(model, conatinerPos);
		model = glm::scale(model, glm::vec3(0.48f, 0.65f, 0.55f));
		MVP = projection * view * model;// Calculate MVP
		cubeRender(container, shaderProgram, MVP);

		// Cylinder Model (candel body)
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, cylinderMesh.texture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		MVP = projection * view * model;
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		cylinderRender(cylinder1, cylinderMesh, shaderProgram, MVP); // Renders Cylinder

		// Cylinder two (wix)
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, cylinderTwoMesh.texture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.23f, 0.0f));
		MVP = projection * view * model;
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		cylinderRender(cylinder2, cylinderTwoMesh, shaderProgram, MVP); // Renders Cylinder

		// Torus model (candel bump)
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, torusMesh.texture);
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.88f, 0.45f, 0.88f));
		model = glm::translate(model, glm::vec3(0.0f, 0.55f, 0.0f)); // Translate the torus above cylinder 
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate the torus by 90 degrees		
		MVP = projection * view * model;
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		torusRender(torusMesh, shaderProgram, MVP); // Render Torus

		// Sphere (basketball)
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, sphereText);
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		model = glm::translate(model, glm::vec3(-0.55f, 0.15f, -0.8f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(2.0f, 0.0f, 0.0f));
		MVP = projection * view * model;
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		ball.Draw();
		
		// Swap Buffers
		glfwSwapBuffers(window);
	}
	
	planeMeshDeletion(planeMesh);
	cubeMeshDeletion(container);
	cubeMeshDeletion(lidBottom);
	cubeMeshDeletion(lidTop);
	cylinderMeshDeletion(cylinderMesh);
	cylinderMeshDeletion(cylinderTwoMesh);
	torusMeshDeletion(torusMesh);
	destroyShaderProgram(shaderProgram);
	glfwTerminate();

	return 0;
}

// Initialize Window and GLAD
bool initializeWindow(GLFWwindow** window) {
	// initialize and configure GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Crete GLFW Window
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, windowResize);

	// Cursor movement
	glfwSetCursorPosCallback(*window, getMousePosition);
	glfwSetScrollCallback(*window, scrollMouseWheel);

	// Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	return true;
}
// Process Key Inputs
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// Used to handle only y-axis movement
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera.Position -= Up * (camera.MovementSpeed * deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera.Position += Up * (camera.MovementSpeed * deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		perspectiveVal = !perspectiveVal;
	}
}
// Handle window resize
void windowResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
// Handles shader creation
bool createShaders(const char* vertexShaderSource, const char* fragmentShaderSource, unsigned int& programID) {

	// Create Shader Program Object
	programID = glCreateProgram();

	// Create Vertex object
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Attach source to object and compile
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check if the Vertex Shader has compilied if print error
	int successful;
	char log[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successful);
	if (!successful)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, log);
		std::cout << "ERROR::Vertex Shader compilation failed\n" << log << std::endl;
		return false;
	}

	// Create Fragment Shader object
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Attach source to object and compile
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Check if the Fragment Shader has compilied if print error
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successful);
	if (!successful)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, log);
		std::cout << "ERROR::Fragment Shader compilation failed\n" << log << std::endl;
		return false;
	}

	// Attachment Vertex and Fragment shaders
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);

	// Link the Shader program with Vertex and Fragment Shader
	glLinkProgram(programID);

	// Check if the Shader Program linking has failed
	glGetProgramiv(programID, GL_LINK_STATUS, &successful);
	if (!successful) {
		glGetProgramInfoLog(programID, 512, NULL, log);
		std::cout << "ERROR::Shader program Linking failed\n" << log << std::endl;
		return false;
	}

	// Delete Shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return true;

}
// Handles destroying shaders
void destroyShaderProgram(unsigned int& program) {
	glDeleteProgram(program);
}

// Plane
void planeMeshCreation(PlaneMesh& mesh, int planeDimension) {
	ShapeData planeObj = ShapeGenerator::makePlane(planeDimension);

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, planeObj.vertexBufferSize() + planeObj.indexBufferSize(), 0, GL_STATIC_DRAW);

	GLsizeiptr currentOffset = 0;

	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, planeObj.vertexBufferSize(), planeObj.vertices);
	currentOffset += planeObj.vertexBufferSize();
	mesh.planeIndexByteOffset = currentOffset;
	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, planeObj.indexBufferSize(), planeObj.indices);
	mesh.planeNumIndices = planeObj.numIndices;

	// Position 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)0);
	// Texture
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));

	// Normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo);
}
void planeMeshDeletion(PlaneMesh& mesh) {
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}
void planeRender(PlaneMesh& mesh, unsigned int& shader, glm::mat4 MVP) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(mesh.vao);
	glDrawElements(GL_TRIANGLES, mesh.planeNumIndices, GL_UNSIGNED_SHORT, (void*)mesh.planeIndexByteOffset);
}

// Cube
void containerMeshCreation(CubeMesh& mesh) {
	float extend = 0.05f;
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f,
		 0.6f + extend, 0.5f, -0.5f - extend, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		 0.6f + extend, 0.5f, -0.5f - extend, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		-0.6f - extend, 0.5f, -0.5f - extend, 0.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f,  0.0f,  1.0f,
		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		-0.6f - extend,  0.5f,  0.5f + extend, 0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,

		-0.6f - extend,  0.5f,  0.5f + extend, 1.0f,  0.0f,-1.0f,  0.0f,  0.0f,
		-0.6f - extend,  0.5f, -0.5f - extend, 1.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,-1.0f,  0.0f,  0.0f,
		-0.6f - extend,  0.5f,  0.5f + extend, 1.0f,  0.0f,-1.0f,  0.0f,  0.0f,

		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,
		 0.6f + extend,  0.5f, -0.5f - extend, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0.0f,  0.0f,
		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, -1.0f,  0.0f,

		-0.6f - extend,  0.5f, -0.5f - extend, 0.0f,  1.0f, 0.0f,  1.0f,  0.0f,
		 0.6f + extend,  0.5f, -0.5f - extend, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,
		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		 0.6f + extend,  0.5f,  0.5f + extend, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		-0.6f - extend,  0.5f,  0.5f + extend, 0.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		-0.6f - extend,  0.5f, -0.5f - extend, 0.0f,  1.0f, 0.0f,  1.0f,  0.0f
	};

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(mesh.vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
}
void cubeMeshCreation(CubeMesh& mesh){
	// Vertex Data, Textures and Normals
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, 1.0f,  0.0f,-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f,  0.0f,-1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  1.0f,  0.0f
	};

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(mesh.vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
}
void cubeMeshDeletion(CubeMesh& mesh){
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}
void cubeRender(CubeMesh& mesh, unsigned int& shader, glm::mat4 MVP){
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Cylinder Functions
void cylinderMeshCreation(CylinderMesh& mesh) {
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
}
void cylinderMeshDeletion(CylinderMesh& mesh) {
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}
void cylinderRender(const static_meshes_3D::Cylinder& cylinder, CylinderMesh& mesh, unsigned int& shader, glm::mat4 MVP) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(mesh.vao);
	cylinder.render();
}

// Torus Functions
Torus torusMeshCreation(TorusMesh& mesh, float innerRadius, float outterRadius) {
	Torus aTorus;

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// Get Vertices
	int vertices = aTorus.createObject(innerRadius, outterRadius, 180, 180, &mesh.vertexData, &mesh.uvData);
	mesh.nVertices = vertices;
	
	// Position buffer
	glGenBuffers(1, &mesh.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices * 3 * sizeof(GLfloat), mesh.vertexData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

	// Texture buffer
	glGenBuffers(1, &mesh.uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices * 2 * sizeof(GLfloat),mesh.uvData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

	return aTorus;
}
void torusMeshDeletion(TorusMesh& mesh) {
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vertexBuffer);
	glDeleteBuffers(1, &mesh.uvBuffer);
	delete mesh.uvData,mesh.vertexData;
}
void torusRender(TorusMesh& mesh, unsigned int& shader, glm::mat4 MVP) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, &MVP[0][0]); // Update

	glBindVertexArray(mesh.vao);

	glDrawArrays(GL_TRIANGLES, 0, mesh.nVertices);
}

// Load texture utility
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
unsigned int loadSphereTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// Mouse functions
void getMousePosition(GLFWwindow* window, double xpos, double ypos) {
	// If this is the first mouse call set last positions
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Change in positions
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;

	// update last position
	lastX = xpos;
	lastY = ypos;

	// handle movement
	camera.ProcessMouseMovement(xOffset, yOffset);
}
void scrollMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
	// increase and decrease speed based on scroll wheel input
	if (yoffset >= 1.0f) {
		// Alter camera speed
		camera.MovementSpeed += 0.5f;
	}
	else {
		// prevent speed from hitting below 0
		if (camera.MovementSpeed > 0.0f) {
			camera.MovementSpeed -= 0.5f;
		}
	}
}