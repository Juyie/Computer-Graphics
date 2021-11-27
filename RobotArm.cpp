#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ROBOT ARM CONTROLS
float BaseTransX = -0.5f;  // 0
float BaseTransZ = 0;
float BaseSpin = 0;        // 1
float ShoulderAng = -10;   // 2
float ElbowAng = -120;
float WristAng = 90;       // 3
float WristTwistAng = 10;
float FingerAng1 = 45;     // 4
float FingerAng2 = -90;

// ROBOT COLORS
GLfloat Ground[] = { 0.5f, 0.5f, 0.5f };
GLfloat Arms[] = { 0.5f, 0.5f, 0.5f };
GLfloat Joints[] = { 0.0f, 0.27f, 0.47f };
GLfloat Fingers[] = { 0.59f, 0.0f, 0.09f };
GLfloat FingerJoints[] = { 0.5f, 0.5f, 0.5f };

// USER INTERFACE GLOBALS
int LeftButtonDown = 0;    // MOUSE STUFF
int RobotControl = 0;

// settings
const unsigned int SCR_WIDTH = 768;
const unsigned int SCR_HEIGHT = 768;

// camera
Camera camera(glm::vec3(0.0f, 1.5f, 2.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.f, -15.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// light information
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

// shader
Shader* PhongShader;
Shader* FloorShader;

// ObjectModel
Model* ourObjectModel;
const char* ourObjectPath = "./teapot.obj";
// translate it so it's at the center of the scene
// it's a bit too big for our scene, so scale it down
glm::mat4 objectXform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)), glm::vec3(0.08f, 0.08f, 0.08f));

// HOUSE KEEPING
void initGL(GLFWwindow** window);
void setupShader();
void destroyShader();
void createGLPrimitives();
void destroyGLPrimitives();

// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mods);

// DRAWING ROBOT PARTS
void DrawGroundPlane(glm::mat4 model);
void DrawJoint(glm::mat4 model);
void DrawBase(glm::mat4 model);
void DrawArmSegment(glm::mat4 model);
void DrawWrist(glm::mat4 model);
void DrawFingerBase(glm::mat4 model);
void DrawFingerTip(glm::mat4 model);

void DrawObject(glm::mat4 model);
bool hasTextures = false; 

void myDisplay()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////

	//Ground
	glm::mat4 model = glm::mat4(1.0f); // initialize matrix to identity matrix first

	DrawGroundPlane(model);

	DrawObject(objectXform);

	// ADD YOUR ROBOT RENDERING STUFF HERE     /////////////////////////////////////////////////////



	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
}

int main()
{
	GLFWwindow* window = NULL;

	initGL(&window);
	setupShader();
	createGLPrimitives();

	glEnable(GL_DEPTH_TEST);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// view/projection transformations
		PhongShader->use();
		PhongShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
		PhongShader->setMat4("view", camera.GetViewMatrix());
		PhongShader->setVec3("viewPos", camera.Position);
		PhongShader->setVec3("lightPos", camera.Position);

		FloorShader->use();
		FloorShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
		FloorShader->setMat4("view", camera.GetViewMatrix());
		FloorShader->setVec3("viewPos", camera.Position);
		FloorShader->setVec3("lightPos", camera.Position);

		// render
		myDisplay();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	destroyGLPrimitives();
	destroyShader();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void initGL(GLFWwindow** window)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Robot Arm", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
	glfwSetCursorPosCallback(*window, mouse_callback);
	glfwSetMouseButtonCallback(*window, mouse_button_callback);
	glfwSetKeyCallback(*window, processInput);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}
}

void setupShader()
{
	PhongShader = new Shader("model_loading.vs", "model_loading.fs");
	PhongShader->use();

	// Light attributes
	PhongShader->setVec3("lightColor", lightColor);

	FloorShader = new Shader("advanced_lighting.vs", "advanced_lighting.fs");
	FloorShader->use();
	FloorShader->setVec3("lightColor", lightColor);
}

void destroyShader()
{
	delete PhongShader;
	delete FloorShader;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5 && action == GLFW_PRESS)
		RobotControl = key - GLFW_KEY_1;
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float) xpos;
		lastY = (float) ypos;
		firstMouse = false;
	}

	float xoffset = (float) (xpos - lastX) / SCR_WIDTH;
	float yoffset = (float) (lastY - ypos) / SCR_HEIGHT; // reversed since y-coordinates go from bottom to top

	lastX = (float) xpos;
	lastY = (float) ypos;

	if (LeftButtonDown)
	{
		switch (RobotControl)
		{
		case 0: BaseTransX += xoffset; BaseTransZ -= yoffset; break;
		case 1: BaseSpin += xoffset * 180 ; break;
		case 2: ShoulderAng += yoffset   * -90; ElbowAng += xoffset  * 90; break;
		case 3: WristAng += yoffset  * -180; WristTwistAng += xoffset  * 180; break;
		case 4: FingerAng1 += yoffset  * 90; FingerAng2 += xoffset * 180; break;
		}
	} 
	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		LeftButtonDown = 1;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		LeftButtonDown = 0;
	}
}

class Primitive {
public:
	Primitive() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);
	}
	~Primitive() {
		if (!ebo) glDeleteBuffers(1, &ebo);
		if (!vbo) glDeleteBuffers(1, &vbo);
		if (!VAO) glDeleteVertexArrays(1, &VAO);
	}
	void Draw() {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLE_STRIP, IndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

protected:
	unsigned int VAO = 0, vbo = 0, ebo = 0;
	unsigned int IndexCount = 0;
	float height = 1.0f;
    float radius[2] = { 1.0f, 1.0f };
};

class Cylinder : public Primitive {
public:
	Cylinder(float bottomRadius = 0.5f, float topRadius = 0.5f, int NumSegs = 16);
};

class Sphere : public Primitive {
public:
	Sphere(int NumSegs = 16);
};

class Plane : public Primitive {
public:
	Plane();
	void Draw() {
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glDrawElements(GL_TRIANGLE_STRIP, IndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
private:
	unsigned int floorTexture;
};

Sphere* unitSphere;
Plane* groundPlane;
Cylinder* unitCylinder;
Cylinder* unitCone;

void createGLPrimitives()
{
	unitSphere = new Sphere();
	groundPlane = new Plane();
	unitCylinder = new Cylinder();
	unitCone = new Cylinder(0.5, 0);

	// Load Object Model
	ourObjectModel = new Model(ourObjectPath);
	hasTextures = (ourObjectModel->textures_loaded.size() == 0) ? 0 : 1;

}
void destroyGLPrimitives()
{
	delete unitSphere;
	delete groundPlane;
	delete unitCylinder;
	delete unitCone;

	delete ourObjectModel;
}

void DrawGroundPlane(glm::mat4 model)
{
	FloorShader->use();
	FloorShader->setMat4("model", model);
	groundPlane->Draw();
}

void DrawJoint(glm::mat4 model)
{
	glm::mat4 Mat1 = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 0.12f));
	Mat1 = glm::rotate(Mat1, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	PhongShader->use();
	Mat1 = model * Mat1;
	PhongShader->setMat4("model", Mat1);
	PhongShader->setVec3("ObjColor", glm::vec3(Joints[0], Joints[1], Joints[2]));
	PhongShader->setInt("hasTextures", false);
	unitCylinder->Draw();
}
void DrawBase(glm::mat4 model)
{
	glm::mat4 Base = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.025f, 0.2f));
	glm::mat4 InBase = glm::inverse(Base);

	PhongShader->use();
	Base = model * Base;
	PhongShader->setMat4("model", Base);
	PhongShader->setVec3("ObjColor", glm::vec3(Joints[0], Joints[1], Joints[2]));
	PhongShader->setInt("hasTextures", false);
	unitCylinder->Draw();

	glm::mat4 Mat1 = glm::translate(InBase, glm::vec3(0.0f, 0.2f, 0.0f));
	Mat1 = glm::scale(Mat1, glm::vec3(0.1f, 0.4f, 0.1f));

	Mat1 = Base * Mat1;
	PhongShader->setMat4("model", Mat1);
	PhongShader->setVec3("ObjColor", glm::vec3(Arms[0], Arms[1], Arms[2]));
	unitCylinder->Draw();

	glm::mat4 Mat2 = glm::translate(InBase, glm::vec3(0.0f, 0.4f, 0.0f));
	Mat2 = Base * Mat2;
	PhongShader->setMat4("model", Mat2);
	DrawJoint(Mat2);
}
void DrawArmSegment(glm::mat4 model)
{
	glm::mat4 Base = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.2f, 0.0f));
	Base = glm::scale(Base, glm::vec3(0.1f, 0.5f, 0.1f));
	glm::mat4 InBase = glm::inverse(Base);

	PhongShader->use();
	Base = model * Base;
	PhongShader->setMat4("model", Base);
	PhongShader->setVec3("ObjColor", glm::vec3(Arms[0], Arms[1], Arms[2]));
	PhongShader->setInt("hasTextures", false);
	unitCylinder->Draw();

	glm::mat4 Mat1 = glm::translate(InBase, glm::vec3(0.0f, 0.5f, 0.0f));;
	Mat1 = Base * Mat1;
	PhongShader->setMat4("model", Mat1);
	DrawJoint(Mat1);
}
void DrawWrist(glm::mat4 model)
{
	glm::mat4 Base = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 0.0f));
	Base = glm::scale(Base, glm::vec3(0.08f, 0.2f, 0.08f));
	glm::mat4 InBase = glm::inverse(Base);

	PhongShader->use();
	Base = model * Base;
	PhongShader->setMat4("model", Base);
	PhongShader->setVec3("ObjColor", glm::vec3(Fingers[0], Fingers[1], Fingers[2]));
	PhongShader->setInt("hasTextures", false);
	unitCylinder->Draw();

	glm::mat4 Mat1 = glm::translate(InBase, glm::vec3(0.0f, 0.2f, 0.0f));
	Mat1 = glm::scale(Mat1, glm::vec3(0.06f, 0.06f, 0.06f));

	Mat1 = Base * Mat1;
	PhongShader->setMat4("model", Mat1);
	PhongShader->setVec3("ObjColor", glm::vec3(FingerJoints[0], FingerJoints[1], FingerJoints[2]));
	unitSphere->Draw(); 
}
void DrawFingerBase(glm::mat4 model)
{
	glm::mat4 Base = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.2f, 0.0f));
	Base = glm::scale(Base, glm::vec3(0.05f, 0.3f, 0.05f));
	glm::mat4 InBase = glm::inverse(Base);

	PhongShader->use();
	Base = model * Base;
	PhongShader->setMat4("model", Base);
	PhongShader->setVec3("ObjColor", glm::vec3(Fingers[0], Fingers[1], Fingers[2]));
	PhongShader->setInt("hasTextures", false);
	unitCylinder->Draw();

	glm::mat4 Mat1 = glm::translate(InBase, glm::vec3(0.0f, 0.35f, 0.0f));
	Mat1 = glm::scale(Mat1, glm::vec3(0.05f, 0.05f, 0.05f));

	Mat1 = Base * Mat1;
	PhongShader->setMat4("model", Mat1);
	PhongShader->setVec3("ObjColor", glm::vec3(FingerJoints[0], FingerJoints[1], FingerJoints[2]));
	unitSphere->Draw();
}
void DrawFingerTip(glm::mat4 model)
{
	glm::mat4 Base = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.25f, 0.05f));
	Base = glm::translate(Base, glm::vec3(0.0f, 0.4f, 0.0f));

	PhongShader->use();
	Base = model * Base;
	PhongShader->setMat4("model", Base);
	PhongShader->setVec3("ObjColor", glm::vec3(Fingers[0], Fingers[1], Fingers[2]));
	PhongShader->setInt("hasTextures", false);
	unitCone->Draw();
}

void DrawObject(glm::mat4 model)
{
	PhongShader->use();
	PhongShader->setMat4("model", model);
	PhongShader->setVec3("ObjColor", glm::vec3(1.0f, 1.0f, 0.0f));
	PhongShader->setInt("hasTextures", hasTextures);
	ourObjectModel->Draw(*PhongShader);
}

/////////////////////////////////////////////////////////////////////////
///// References https://learnopengl.com/Getting-started/Shaders
/////		     https://learnopengl.com/Lighting/Basic-Lighting
/////			 http://www.songho.ca/opengl/gl_cylinder.html
/////////////////////////////////////////////////////////////////////////

Sphere::Sphere(int NumSegs)
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	const unsigned int X_SEGMENTS = NumSegs;
	const unsigned int Y_SEGMENTS = NumSegs;
	const float PI = (float)3.14159265359;

	for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
	{
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			positions.push_back(glm::vec3(xPos, yPos, zPos));
			normals.push_back(glm::vec3(xPos, yPos, zPos));
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	IndexCount = (unsigned int)indices.size();

	std::vector<float> data;
	for (int i = 0; i < positions.size(); ++i)
	{
		data.push_back(positions[i].x);
		data.push_back(positions[i].y);
		data.push_back(positions[i].z);
		if (normals.size() > 0)
		{
			data.push_back(normals[i].x);
			data.push_back(normals[i].y);
			data.push_back(normals[i].z);
		}
	}
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	GLsizei stride = (3 + 3) * sizeof(float);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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


Plane::Plane()
{
	float data[] = {
		// positions           // normals          // texcoords
		-10.0f, 0.0f, -10.0f,  0.0f, 1.0f,  0.0f,  0.0f,  0.0f,
		 10.0f, 0.0f, -10.0f,  0.0f, 1.0f,  0.0f,  10.0f, 0.0f,
		 10.0f, 0.0f,  10.0f,  0.0f, 1.0f,  0.0f,  10.0f, 10.0f,
		-10.0f, 0.0f,  10.0f,  0.0f, 1.0f,  0.0f,  0.0f,  10.0f
	};
	unsigned int indices[] = { 0, 1, 3, 2 };

	IndexCount = sizeof(indices)/sizeof(unsigned int);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GLsizei stride = (3 + 3 + 2) * sizeof(float);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

	glBindVertexArray(0);

	floorTexture = loadTexture("./wood.png");
	FloorShader->use();
	FloorShader->setInt("texture1", floorTexture);

}

Cylinder::Cylinder(float bottomRadius, float topRadius, int NumSegs)
{
	radius[0] = bottomRadius; radius[1] = topRadius;

	std::vector<glm::vec3> base;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	//a circle
	const float PI = (float)3.14159265359;
	float sectorStep = 2 * PI / NumSegs;				// Angle increasing
	float sectorAngle;									// radian

	for (int i = 0; i <= NumSegs; ++i)
	{
		sectorAngle = i * sectorStep;
		float xPos = std::sin(sectorAngle);
		float yPos = 0;
		float zPos = std::cos(sectorAngle);

		base.push_back(glm::vec3(xPos, yPos, zPos));
	}

	//put side of cylinder
	for (int i = 0; i < 2; ++i)
	{
		float h = -height / 2.0f + i * height;			// height from -h/2 to h/2   

		for (int j = 0; j <= NumSegs; ++j)
		{
			positions.push_back(glm::vec3(base[j].x * radius[i], h, base[j].z * radius[i]));
			normals.push_back(glm::vec3(base[j].x, h, base[j].z));
		}
	}

	//the starting index for the base/top surface
	//NOTE: it is used for generating indices later
	int baseCenterIndex = (int)positions.size();
	int topCenterIndex = baseCenterIndex + NumSegs + 1; // include center vertex

	//put base and top circles
	for (int i = 0; i < 2; ++i)
	{
		float h = -height / 2.0f + i * height;
		float ny = (float)-1 + i * 2;

		// center point
		positions.push_back(glm::vec3(0, h, 0));		// height from -h/2 to h/2
		normals.push_back(glm::vec3(0, ny, 0));			// z value of normal; -1 to 1

		for (int j = 0; j < NumSegs; ++j)
		{
			positions.push_back(glm::vec3(base[j].x * radius[i], h, base[j].z * radius[i]));
			normals.push_back(glm::vec3(0, ny, 0));
		}
	}

	//Indexing
	int k1 = 0;											// 1st vertex index at base
	int k2 = NumSegs + 1;								// 1st vertex index at top

	// indices for the side surface
	for (int i = 0; i < NumSegs; ++i, ++k1, ++k2)
	{
		// 2 triangles per sector
		// k1 => k1+1 => k2
		indices.push_back(k1);
		indices.push_back(k1 + 1);
		indices.push_back(k2);

		// k2 => k1+1 => k2+1
		indices.push_back(k2);
		indices.push_back(k1 + 1);
		indices.push_back(k2 + 1);
	}

	//indices for the base surface
	//NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
	//      please see the previous code snippet
	for (int i = 0, k = baseCenterIndex + 1; i < NumSegs; ++i, ++k)
	{
		if (i < NumSegs - 1)
		{
			indices.push_back(baseCenterIndex);
			indices.push_back(k + 1);
			indices.push_back(k);
		}
		else // last triangle
		{
			indices.push_back(baseCenterIndex);
			indices.push_back(baseCenterIndex + 1);
			indices.push_back(k);
		}
	}

	// indices for the top surface
	for (int i = 0, k = topCenterIndex + 1; i < NumSegs; ++i, ++k)
	{
		if (i < NumSegs - 1)
		{
			indices.push_back(topCenterIndex);
			indices.push_back(k);
			indices.push_back(k + 1);
		}
		else // last triangle
		{
			indices.push_back(topCenterIndex);
			indices.push_back(k);
			indices.push_back(topCenterIndex + 1);
		}
	}
	IndexCount = (unsigned int)indices.size();

	std::vector<float> data;
	for (int i = 0; i < positions.size(); ++i)
	{
		data.push_back(positions[i].x);
		data.push_back(positions[i].y);
		data.push_back(positions[i].z);

		if (normals.size() > 0)
		{
			data.push_back(normals[i].x);
			data.push_back(normals[i].y);
			data.push_back(normals[i].z);
		}
	}
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	GLsizei stride = (3 + 3) * sizeof(float);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
}
