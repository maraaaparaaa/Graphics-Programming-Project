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
#include <algorithm>
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
GLint shadowMapLoc;

// light shader uniform locations
GLint modelLocL;
GLint viewLocL;
GLint projLocL;
GLint lightDirLocL;
GLint lightColorLocL;
GLint normalMatrixLocL;
GLint lightPosLoc;
GLint pointLightColorLoc;

bool sunOn = true;
glm::vec3 daySunColor = glm::vec3(1.0f, 1.0f, 0.95f);
glm::vec3 nightSunColor = glm::vec3(0.17f);

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
gps::Model3D firePlace;
gps::Model3D tent;

gps::Model3D penguinBody;
gps::Model3D penguinWingL;
gps::Model3D penguinWingR;

GLfloat angle;

// shaders
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader fireShader;
gps::Shader depthMapShader;

bool renderShadows = true;

//textures 
GLuint matterhornTexture, skyTexture, mTexture[N], penguinTexture, astronautTexture;
GLuint fireTexture;
GLuint tentTexture;

// mouse variables
float lastX = 400, lastY = 300; // initial pos, middle
float yaw = -90.0f;             // camera direction -z
float pitch = 0.0f;
bool firstMouse = true;         // avoid jumps at first move

// Shadow mapping - directional light
GLuint shadowMapFBO;
GLuint shadowMapTexture;
const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
glm::mat4 lightSpaceMatrix;

// Shadow mapping - point light (cubemap for all directions)
GLuint pointShadowMapFBO;
GLuint pointShadowCubemap;
const unsigned int POINT_SHADOW_WIDTH = 1024, POINT_SHADOW_HEIGHT = 1024;
std::vector<glm::mat4> pointLightMatrices;

//fire
struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity;
	glm::vec4 color; //RGBA
	float size;
    float life;
	float maxLife;
	bool isFire; // if it is fire or smoke
};

const int MAX_PARTICLES = 1000;
Particle particles[MAX_PARTICLES];

//object positions
glm::vec3 firePos = glm::vec3(-2096.814209f, -980.905457f, 5921.6f);

GLuint particleVAO, particleVBO;

GLuint quadVBO, quadEBO, instanceVBO;

GLfloat quadVertices[] = {
    // (x, y, z)
    -1.0f, -1.0f, 0.0f,  // bottom-left
     1.0f, -1.0f, 0.0f,  // bottom-right
     1.0f,  1.0f, 0.0f,  // top-right
    -1.0f,  1.0f, 0.0f   // top-left
};

GLuint quadIndices[] = {
    0, 1, 2,  // primul triunghi
    2, 3, 0   // al doilea triunghi
};


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

void loadSky();

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

    //sun
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        sunOn = false;
        loadSky();
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        sunOn = true;
        loadSky();
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

    // Toggle shadows
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        renderShadows = !renderShadows;
        std::cout << "Shadows " << (renderShadows ? "ON" : "OFF") << std::endl;
    }

    //sun position
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    lightDir.y += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  lightDir.y -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  lightDir.x -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) lightDir.x += 0.01f;

    // normalizezi mereu
    lightDir = glm::normalize(lightDir);

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

void loadSky() {

    if (sunOn) {
        sky.LoadModel("models/SkyDome/sky.obj");
        skyTexture = sky.ReadTextureFromFile("models/SkyDome/skydomeBIG.png");
    }
    else {
        sky.LoadModel("models/SkyDome/nightSky.obj");
        skyTexture = sky.ReadTextureFromFile("models/SkyDome/nightSky.jpg");
    }
    
}

void initModels() {
	matterhorn.LoadModel("models/Matterhorn/Matterhornbig.obj");
	matterhornTexture = matterhorn.ReadTextureFromFile("models/Matterhorn/Matterhorn.jpg");

    loadSky();

	penguin.LoadModel("models/penguin/penguin1.obj");
	penguinTexture = penguin.ReadTextureFromFile("models/penguin/Penguin Diffuse Color.png");

    penguinBody.LoadModel("models/penguin/penguinBody.obj");
    penguinTexture = penguin.ReadTextureFromFile("models/penguin/Penguin Diffuse Color.png");

    penguinWingL.LoadModel("models/penguin/penguinWingL.obj");
    penguinTexture = penguin.ReadTextureFromFile("models/penguin/Penguin Diffuse Color.png");

    penguinWingR.LoadModel("models/penguin/penguinWingR.obj");
    penguinTexture = penguin.ReadTextureFromFile("models/penguin/Penguin Diffuse Color.png");

	astronaut.LoadModel("models/astronaut/astronaut.obj");
	astronautTexture = astronaut.ReadTextureFromFile("models/astronaut/texture_diffuse.png");

	for (int i = 1; i < N; i++) {
		std::string path = "models/Matterhorn_parts/m" + std::to_string(i) + ".obj";
		m[i].LoadModel(path);
		std::string texturePath = "models/Matterhorn_parts/m" + std::to_string(i) + ".png";
		mTexture[i] = m[i].ReadTextureFromFile(texturePath.c_str());
	}

    tent.LoadModel("models/Tent/tent.obj");
    tentTexture = tent.ReadTextureFromFile("models/Tent/tentTexture.jpg");

	firePlace.LoadModel("models/Fireplace/fire_place.obj");
	fireTexture = firePlace.ReadTextureFromFile("models/Fireplace/texture/fireTex.png");

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].life = 0.0f;
    }

    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(particleVAO);

    // ===== QUAD VERTICES (acelasi pentru toate particulele) =====
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // ===== INDICES =====
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // ===== INSTANCE DATA (diferit pentru fiecare particula) =====
    particleVBO = instanceVBO;  // salvam pentru update-uri
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 9 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Layout: pos(3) + color(4) + size(1) + life(1) = 9 floats

    // Particle position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);  // o instance per particula

    // Particle color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(2, 1);

    // Particle size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // Particle life
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void initShaders() {
    try {
        myCustomShader.loadShader(
            "shaders/shaderStart.vert",
            "shaders/shaderStart.frag");
        std::cout << "myCustomShader loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load myCustomShader: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        lightShader.loadShader(
            "shaders/lightShader.vert",
            "shaders/lightShader.frag");
        std::cout << "lightShader loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load lightShader: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        fireShader.loadShader("shaders/fire_instanced.vert", "shaders/fire_instanced.frag");
        std::cout << "fireShader loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load fireShader: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
        std::cout << "depthMapShader loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load depthMapShader: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

	/*myCustomShader.loadShader(
        "shaders/shaderStart.vert", 
        "shaders/shaderStart.frag"); */   
    /*lightShader.loadShader(
        "shaders/lightShader.vert",
		"shaders/lightShader.frag");*/
    /*fireShader.loadShader("shaders/fire_instanced.vert", "shaders/fire_instanced.frag"); */ 
    //depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
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
            // directional light
    lightDirLoc = glGetUniformLocation(lightShader.shaderProgram, "lightDirEye");
    lightColorLoc = glGetUniformLocation(lightShader.shaderProgram, "lightColor");
    normalMatrixLocL = glGetUniformLocation(lightShader.shaderProgram, "normalMatrix");

    lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, -0.3f));
    lightColor = glm::vec3(1.0f, 1.0f, 0.95f); // soare

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

            // point light
    lightPosLoc = glGetUniformLocation(lightShader.shaderProgram, "lightPosEye");
    pointLightColorLoc = glGetUniformLocation(lightShader.shaderProgram, "pointLightColor");

    glm::vec3 pointLightColor = glm::vec3(3.0f, 2.0f, 0.8f); // fire color
    glm::vec4 lightPosEye = view * glm::vec4(firePos, 1.0f);

    glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(lightPosEye)));
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    // shadow map shader uniforms
    lightShader.useShaderProgram();
    GLint shadowMapLoc = glGetUniformLocation(lightShader.shaderProgram, "shadowMap");
    glUniform1i(shadowMapLoc, 1); // texture unit 1

	//fire shader uniforms
    fireShader.useShaderProgram();

    GLint viewLocFire = glGetUniformLocation(fireShader.shaderProgram, "view");
    GLint projLocFire = glGetUniformLocation(fireShader.shaderProgram, "projection");

    glUniformMatrix4fv(viewLocFire, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocFire, 1, GL_FALSE, glm::value_ptr(projection));

}

void initShadowMap() {
    // ===== DIRECTIONAL LIGHT SHADOW MAP =====
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ===== POINT LIGHT SHADOW MAP (CUBEMAP) =====
    /*glGenFramebuffers(1, &pointShadowMapFBO);

    glGenTextures(1, &pointShadowCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            POINT_SHADOW_WIDTH, POINT_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadowCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}

void calculateLightSpaceMatrix() {

	// ligth position far away in the light direction
	glm::vec3 lightPos = -lightDir * 15000.0f; // ajust distance as needed

    glm::mat4 lightProjection = glm::ortho(-20000.0f, 20000.0f, -20000.0f, 20000.0f, 1.0f, 50000.0f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    lightSpaceMatrix = lightProjection * lightView;
}

void renderDepthMap(gps::Shader& shader, bool isPointLight = false) {

    if (!isPointLight) {
        shader.useShaderProgram();
        GLint lightSpaceMatrixLoc = glGetUniformLocation(shader.shaderProgram, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    }

    GLint modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	// render scene objects to depth map
    glm::mat4 objectModel = glm::mat4(1.0f);

    // Penguin
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objectModel));
    penguin.Draw(shader);

    // Astronaut
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objectModel));
    astronaut.Draw(shader);

    // Tent
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objectModel));
    tent.Draw(shader);

    // FirePlace
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objectModel));
    firePlace.Draw(shader);

    // HiPenguin parts
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objectModel));
    penguinBody.Draw(shader);
    penguinWingL.Draw(shader);
    penguinWingR.Draw(shader);

    float t = glfwGetTime();
    float wingAngle = sin(t * 4.0f) * glm::radians(30.0f);

    glm::vec3 wingLPivot = glm::vec3(-2068.25f, -884.082f, 5489.76f);
    glm::mat4 wingLModel = objectModel *
        glm::translate(glm::mat4(1.0f), wingLPivot) *
        glm::rotate(glm::mat4(1.0f), wingAngle, glm::vec3(0, 0, 1)) *
        glm::translate(glm::mat4(1.0f), -wingLPivot);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wingLModel));
    penguinWingL.Draw(shader);

    glm::vec3 wingRPivot = glm::vec3(-2008.24f, -877.652f, 5558.12f);
    glm::mat4 wingRModel = objectModel *
        glm::translate(glm::mat4(1.0f), wingRPivot) *
        glm::rotate(glm::mat4(1.0f), -wingAngle, glm::vec3(0, 0, 1)) *
        glm::translate(glm::mat4(1.0f), -wingRPivot);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wingRModel));
    penguinWingR.Draw(shader);

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

void renderHiPenguin(gps::Shader shader) {
    shader.useShaderProgram();

    // lighting 
    GLint objLightLoc =
        glGetUniformLocation(lightShader.shaderProgram, "objectLightMultiplier");
    glUniform1f(objLightLoc, 3.0f);

    GLint shininessLoc =
        glGetUniformLocation(lightShader.shaderProgram, "shininess");
    GLint specStrengthLoc =
        glGetUniformLocation(lightShader.shaderProgram, "specularStrength");

    glUniform1f(shininessLoc, 6.0f);
    glUniform1f(specStrengthLoc, 0.15f);

    // texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, penguinTexture);

	// time for animation
    float t = glfwGetTime();
    float wingAngle = sin(t * 4.0f) * glm::radians(30.0f);

    
    glm::vec3 penguinPos = glm::vec3(0.0f, 0.0f, 0.0f);

    // ===== BODY =====
    glm::mat4 bodyModel = glm::translate(glm::mat4(1.0f), penguinPos);

    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(bodyModel));
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * bodyModel)));
    glUniformMatrix3fv(normalMatrixLocL, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    penguinBody.Draw(shader);

    // ===== WING LEFT =====
    glm::vec3 wingLPivot = glm::vec3(-2068.25f, -884.082f, 5489.76f);
    glm::mat4 wingLModel = bodyModel *
		glm::translate(glm::mat4(1.0f), wingLPivot) *   // move pivot to origin
        glm::rotate(glm::mat4(1.0f), wingAngle, glm::vec3(0, 0, 1)) *  // rotate
		glm::translate(glm::mat4(1.0f), -wingLPivot);  // move back

    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(wingLModel));
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * wingLModel)));
    glUniformMatrix3fv(normalMatrixLocL, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    penguinWingL.Draw(shader);

    // ===== WING RIGHT =====
    glm::vec3 wingRPivot = glm::vec3(-2008.24f, -877.652f, 5558.12f);

    glm::mat4 wingRModel = bodyModel *
        glm::translate(glm::mat4(1.0f), wingRPivot) *
        glm::rotate(glm::mat4(1.0f), -wingAngle, glm::vec3(0, 0, 1)) *
        glm::translate(glm::mat4(1.0f), -wingRPivot);

    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(wingRModel));
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * wingRModel)));
    glUniformMatrix3fv(normalMatrixLocL, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    penguinWingR.Draw(shader);
}

void renderTent(gps::Shader shader) {
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

    glBindTexture(GL_TEXTURE_2D, tentTexture);
    tent.Draw(shader);
}

void renderFirePlace(gps::Shader shader) {
    shader.useShaderProgram();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, fireTexture);
    firePlace.Draw(shader);
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

void renderParticles(gps::Shader& shader, glm::vec3 firePos, GLuint particleVAO) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending pentru foc
    glDepthMask(GL_FALSE);  // Nu scrie er
    glDisable(GL_CULL_FACE);

    shader.useShaderProgram();

    // Trimite uniform-urile
    GLint viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    GLint camPosLoc = glGetUniformLocation(shader.shaderProgram, "cameraPos");

    glm::vec3 cameraPos = myCamera.getCameraPosition();

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(camPosLoc, 1, glm::value_ptr(cameraPos));

    struct ParticleDistance {
        int index;
        float distance;
    };

    std::vector<ParticleDistance> distances;
    distances.reserve(MAX_PARTICLES);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0.0f) {
            float dist = glm::length(cameraPos - particles[i].pos);
            distances.push_back({ i, dist });
        }
    }

    // Sortare: cel mai departe primul
    std::sort(distances.begin(), distances.end(),
        [](const ParticleDistance& a, const ParticleDistance& b) {
            return a.distance > b.distance;
        });

    std::vector<float> instanceData;
    instanceData.reserve(distances.size() * 9);

    for (const auto& pd : distances) {
        int i = pd.index;
        const Particle& p = particles[i];

        // Position (3 floats)
        instanceData.push_back(p.pos.x);
        instanceData.push_back(p.pos.y);
        instanceData.push_back(p.pos.z);

        // Color (4 floats)
        instanceData.push_back(p.color.r);
        instanceData.push_back(p.color.g);
        instanceData.push_back(p.color.b);
        instanceData.push_back(p.color.a);

        // Size (1 float)
        instanceData.push_back(p.size);

        // Life (1 float)
        instanceData.push_back(p.life / p.maxLife);
    }

    if (!instanceData.empty()) {
        glBindVertexArray(particleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            instanceData.size() * sizeof(float),
            instanceData.data());

        // Instanced rendering
        int numParticles = instanceData.size() / 9;
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numParticles);

        glBindVertexArray(0);
    }

    // Cleanup
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void respawnParticle(Particle& particle, glm::vec3 firePos) {

    // Decide dac e foc sau fum (70% foc, 30% fum)
    particle.isFire = ((float)rand() / RAND_MAX) < 0.8f;

    float angle = ((float)rand() / RAND_MAX) * 20.0f * 3.14159f;

    // VELOCITATE
    if (particle.isFire) {

        float radius = sqrt((float)rand() / RAND_MAX) * 85.0f;
        float yOffset = ((float)rand() / RAND_MAX) * 150.0f;

        particle.pos = firePos + glm::vec3(
            radius * cos(angle),
            yOffset,
            radius * sin(angle)
        );

        particle.velocity = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * 4.0f,  // x
            15.0f + ((float)rand() / RAND_MAX) * 10.0f,   // y ( sus)
            (((float)rand() / RAND_MAX) - 0.5f) * 4.0f   // z
        );

        // CULOARE BAZAT PE DISTAN DE LA CENTRU
        float distanceRatio = radius / 150.0f;  // 0.0 = centru, 1.0 = margine

        if (distanceRatio < 0.1f) {
            // CENTRU - GALBEN
            particle.color = glm::vec4(1.0f, 1.0f, 0.4f, 1.0f);
        }
        else if (distanceRatio < 0.65f) {
            // MIJLOC - PORTOCALIU
            particle.color = glm::vec4(1.0f, 0.5f, 0.1f, 0.9f);
        }
        else {
            // MARGINE - ROsU
            particle.color = glm::vec4(0.95f, 0.2f, 0.05f, 0.8f);
        }

        particle.size = 20.0f + ((float)rand() / RAND_MAX) * 30.0f;  // 20-50 units
        particle.maxLife = 0.5f + ((float)rand() / RAND_MAX) * 0.4f;  // 0.5-0.9 sec
    }
    else {

        float radius = sqrt((float)rand() / RAND_MAX) * 150.0f;
        float yOffset = 200 + ((float)rand() / RAND_MAX) * 1000.0f;

        particle.pos = firePos + glm::vec3(
            radius * cos(angle),
            yOffset,
            radius * sin(angle)
        );

        particle.velocity = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * 10.0f,
            8.0f + ((float)rand() / RAND_MAX) * 25.0f,
            (((float)rand() / RAND_MAX) - 0.5f) * 12.0f
        );

        // Culoare gri cu alpha redus
        float darkness = 0.1f + ((float)rand() / RAND_MAX) * 0.2f;
        particle.color = glm::vec4(darkness, darkness, darkness, 0.3f);

        particle.size = 70.0f + ((float)rand() / RAND_MAX) * 70.0f;  // 40-80 units
        particle.maxLife = 2.5f + ((float)rand() / RAND_MAX) * 1.5f;  // 1.0-1.8 sec
    }

    particle.life = particle.maxLife;
}

void updateParticles(float deltaTime, glm::vec3 firePos) {

    int newParticles = (int)(deltaTime * 200.0f); // 500 particule/secund
    int particlesSpawned = 0;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle& p = particles[i];

        if (p.life > 0.0f) {
            p.life -= deltaTime;

            p.pos += p.velocity * deltaTime;

            if (p.isFire) {
                p.velocity.x += ((float)rand() / RAND_MAX - 0.5f) * 10.0f * deltaTime;
                p.velocity.z += ((float)rand() / RAND_MAX - 0.5f) * 10.0f * deltaTime;
                p.velocity.y -= 3.0f * deltaTime; // gravitate

                float lifeRatio = p.life / p.maxLife;

                if (lifeRatio > 0.7f) {
                    p.color.r = 1.0f;
                    p.color.g = 1.0f;
                    p.color.b = 0.4f;
                    p.color.a = 1.0f;
                }
                else if (lifeRatio > 0.4f) {
                    p.color.r = 1.0f;
                    p.color.g = 0.5f + (lifeRatio - 0.4f) * 1.67f; // 0.5 -> 1.0
                    p.color.b = 0.1f;
                    p.color.a = 0.9f;
                }
                else {
                    p.color.r = 1.0f;
                    p.color.g = 0.2f * (lifeRatio / 0.4f); // 0.2 -> 0
                    p.color.b = 0.0f;
                    p.color.a = lifeRatio * 2.0f; // fade out rapid
                }
            }
            else {
                // FUM - se extinde si rise up
                p.velocity.x *= 0.97f;  // friction
                p.velocity.z *= 0.97f;
                p.velocity.y += 2.0f * deltaTime;  // continurce

                p.size += 60.0f * deltaTime;  // se extinde mai rapud

                float lifeRatio = p.life / p.maxLife;

                // Fumul devine mai deschis si transparent gradual
                float brightness = 0.4f + ((1.0f - lifeRatio) * 0.3f);  // se deschide
                p.color.r = brightness;
                p.color.g = brightness;
                p.color.b = brightness;
                p.color.a = lifeRatio * 0.3f;  // fade out, max 60% opacity
                //p.velocity.x *= 0.98f; // friction 
                //p.velocity.z *= 0.98f;
                //p.velocity.y += 0.5f * deltaTime; // conti urce

                //p.size += 30.0f * deltaTime; // se extinde

                //float lifeRatio = p.life / p.maxLife;
                //p.color.a = lifeRatio * 0.3f; // max 30% opacity
            }
        }
        else {
            if (particlesSpawned < newParticles) {
                respawnParticle(p, firePos);
                particlesSpawned++;
            }
        }
    }
}

void renderScene() {
	

    if (renderShadows) {
        // ===== SHADOW PASS - render depth map =====
        calculateLightSpaceMatrix();

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

		// render scene from light's point of view
        depthMapShader.useShaderProgram();
        renderDepthMap(depthMapShader, false);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
        glViewport(0, 0, retina_width, retina_height);
    }


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

    // Bind shadow map la texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);

    // Trimite lightSpaceMatrix la shader
    GLint lightSpaceMatrixLoc = glGetUniformLocation(lightShader.shaderProgram, "lightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // transfrom light position from world space to view space
    glm::vec4 lightPosEye = view * glm::vec4(firePos, 1.0f);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(lightPosEye)));

    // time for light shader
    float currentTime = glfwGetTime();
    GLint timeLoc = glGetUniformLocation(lightShader.shaderProgram, "time");
    glUniform1f(timeLoc, currentTime);

    // update model rotation (Matterhorn)
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLocL, 1, GL_FALSE, glm::value_ptr(model));
	//renderMatterhorn(myCustomShader);

	//update normal matrix for lighting
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(view * model)));
    glUniformMatrix3fv(normalMatrixLocL, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//light color based on time of day
    glm::vec3 currentLightColor = sunOn ? daySunColor : nightSunColor;
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(currentLightColor));

	// render objects
	renderMatterhornParts(lightShader);
	renderPenguin(lightShader);
	renderAstronaut(lightShader);
    renderTent(lightShader);
    renderFirePlace(lightShader);
    renderHiPenguin(lightShader);

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
	initShadowMap();
	initUniforms();

	glCheckError();

	float lastFrame = 0.0f;

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {

		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        processMovement();

		updateParticles(deltaTime, firePos);

	    renderScene();
		renderParticles(fireShader, firePos, particleVAO); // render fire particles

        //printf("Camerapos = %f %f %f \n", myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z);

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());


		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
