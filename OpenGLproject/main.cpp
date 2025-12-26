#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include <string>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#define N 35

// window
gps::Window myWindow;
WindowDimensions retina_dimensions = myWindow.getWindowDimensions();
int retina_height;
int retina_width;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// light shader uniform locations
GLint modelLocL;
GLint viewLocL;
GLint projLocL;
GLint lightDirLocL;
GLint lightColorLocL;
GLint normalMatrixLocL;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 5.0f, 15.0f),
    glm::vec3(0.0f, 5.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 10.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D matterhorn;
gps::Model3D sky; 
gps::Model3D m[N];
gps::Model3D penguin;
gps::Model3D astronaut;

GLfloat angle;

// shaders
gps::Shader myCustomShader;
gps::Shader lightShader;

//textures 
GLuint matterhornTexture, skyTexture, mTexture[N], penguinTexture, astronautTexture;

// mouse variables
float lastX = 400, lastY = 300; // initial pos, middle
float yaw = -90.0f;             // camera direction -z
float pitch = 0.0f;
bool firstMouse = true;         // avoid jumps at first move


//fire
struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity;
    float life;
};

const int MAX_PARTICLES = 500;
Particle particles[MAX_PARTICLES];


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invert y
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // limit pitch-ul not to flip
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_Q]) {
        angle += 1.0f;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    //scene view 
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // solid mode
    }

    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
    }

    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // poligonal mode
	}
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
	retina_width = myWindow.getWindowDimensions().width;
	retina_height = myWindow.getWindowDimensions().height;
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    //glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
	matterhorn.LoadModel("models/Matterhorn/Matterhornbig.obj");
	matterhornTexture = matterhorn.ReadTextureFromFile("models/Matterhorn/Matterhorn.jpg");

	sky.LoadModel("models/SkyDome/sky.obj");
	skyTexture = sky.ReadTextureFromFile("models/SkyDome/skydomeBIG.png");

	penguin.LoadModel("models/penguin/penguin1.obj");
	penguinTexture = penguin.ReadTextureFromFile("models/penguin/Penguin Diffuse Color.png");

	astronaut.LoadModel("models/astronaut/astronaut.obj");
	astronautTexture = astronaut.ReadTextureFromFile("models/astronaut/texture_diffuse.png");

	for (int i = 1; i < N; i++) {
		std::string path = "models/Matterhorn_parts/m" + std::to_string(i) + ".obj";
		m[i].LoadModel(path);
		std::string texturePath = "models/Matterhorn_parts/m" + std::to_string(i) + ".png";
		mTexture[i] = m[i].ReadTextureFromFile(texturePath.c_str());
	}
}

void initShaders() {
	myCustomShader.loadShader(
        "shaders/shaderStart.vert", 
        "shaders/shaderStart.frag");    
    lightShader.loadShader(
        "shaders/lightShader.vert",
		"shaders/lightShader.frag");
}

void initUniforms() {
    myCustomShader.useShaderProgram();

	//initialize the model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //initialize the projection matrix
    projection = glm::perspective(glm::radians(55.0f), (float)retina_width / (float)retina_height, 0.1f, 1000000.0f);
    GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //create rotation matrix
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //send matrix data to vertex shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


    //light

	lightShader.useShaderProgram();

	model = glm::mat4(1.0f);

    modelLocL = glGetUniformLocation(lightShader.shaderProgram, "model");
    viewLocL = glGetUniformLocation(lightShader.shaderProgram, "view");
    projLocL = glGetUniformLocation(lightShader.shaderProgram, "projection");

    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocL, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocL, 1, GL_FALSE, glm::value_ptr(projection));

    lightDirLoc = glGetUniformLocation(lightShader.shaderProgram, "lightDirEye");
    lightColorLoc = glGetUniformLocation(lightShader.shaderProgram, "lightColor");
    normalMatrixLocL = glGetUniformLocation(lightShader.shaderProgram, "normalMatrix");

    glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, -0.3f));
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 0.95f); // soare

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

}

void renderMatterhorn(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, matterhornTexture);

	// draw matterhorn
	matterhorn.Draw(shader);
}

void renderMatterhornParts(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();

	// light multiplier uniform 
    GLint objLightLoc =
        glGetUniformLocation(lightShader.shaderProgram, "objectLightMultiplier");
    glUniform1f(objLightLoc, 1.0f);
    GLint shininessLoc =
        glGetUniformLocation(lightShader.shaderProgram, "shininess");
    GLint specStrengthLoc =
        glGetUniformLocation(lightShader.shaderProgram, "specularStrength");

    glUniform1f(shininessLoc, 32.0f);          
    glUniform1f(specStrengthLoc, 0.3f);     

	for (int i = 1; i < N; i++) {
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);
		glBindTexture(GL_TEXTURE_2D, mTexture[i]);

		// draw matterhorn part
		m[i].Draw(shader);
	}
}

void renderSkyDome(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    // model matrix for skydome
    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::rotate(skyModel, glm::radians(180.0f), glm::vec3(1, 0, 0));

	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(skyModel));


    glBindTexture(GL_TEXTURE_2D, skyTexture);
    sky.Draw(shader);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void renderPenguin(gps::Shader shader) {
	shader.useShaderProgram();

	//penguin light multiplier uniform
    GLint objLightLoc =
        glGetUniformLocation(lightShader.shaderProgram, "objectLightMultiplier");
    glUniform1f(objLightLoc, 3.0f);
    GLint shininessLoc =
        glGetUniformLocation(lightShader.shaderProgram, "shininess");
    GLint specStrengthLoc =
        glGetUniformLocation(lightShader.shaderProgram, "specularStrength");

    glUniform1f(shininessLoc, 6.0f);          // smooth
    glUniform1f(specStrengthLoc, 0.15f);      // subtil

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, penguinTexture);
	penguin.Draw(shader);
}

void renderAstronaut(gps::Shader shader) {
	shader.useShaderProgram();

    GLint objLightLoc =
        glGetUniformLocation(lightShader.shaderProgram, "objectLightMultiplier");
    glUniform1f(objLightLoc, 1.0f);
    GLint shininessLoc =
        glGetUniformLocation(lightShader.shaderProgram, "shininess");
    GLint specStrengthLoc =
        glGetUniformLocation(lightShader.shaderProgram, "specularStrength");

    glUniform1f(shininessLoc, 32.0f);
    glUniform1f(specStrengthLoc, 0.3f);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);

	glBindTexture(GL_TEXTURE_2D, astronautTexture);
	astronaut.Draw(shader);
}

void respawnParticle(Particle& particle, glm::vec3 firePos) {
    particle.pos = firePos + glm::vec3(((rand() % 100) / 500.0f) - 0.1f, 0.0f, ((rand() % 100) / 500.0f) - 0.1f);
    particle.velocity = glm::vec3(0.0f, 1.0f + ((rand() % 100) / 200.0f), 0.0f);
	particle.life = 1.0f; // 1 second life
}

void updateParticles(float deltaTime, glm::vec3 firePos) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0.0f) {
            particles[i].life -= deltaTime;
            particles[i].pos += particles[i].velocity * deltaTime;
        }
        else {
            respawnParticle(particles[i], firePos);
        }
    }
}

void renderScene() {
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    ///////////// render sky /////////////////
    myCustomShader.useShaderProgram();

    // update view matrix from camera
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	renderSkyDome(myCustomShader);

	/////////////// render matterhorn and other objects /////////////////
    glDisable(GL_CULL_FACE);

	lightShader.useShaderProgram();
    glUniformMatrix4fv(viewLocL, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocL, 1, GL_FALSE, glm::value_ptr(projection)); //for window resize

    // update model rotation (Matterhorn)
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(model));
	//renderMatterhorn(myCustomShader);

	//update normal matrix for lighting
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * model)));
    glUniformMatrix3fv(normalMatrixLocL, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	renderMatterhornParts(lightShader);
	renderPenguin(lightShader);
	renderAstronaut(lightShader);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    setWindowCallbacks();
    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
