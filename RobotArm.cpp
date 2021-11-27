// 컴퓨터공학과 1976433 황주이
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

// TEAPOT CONTROLS
float TeaTransX = 0.0f;
float TeaTransZ = 0.0f;
float TeaBaseSpin = 0.0f;
float TeaShoulderAng = 0.0f;
float TeaElbowAng = 0.0f;
float TeaWristAng = 0.0f;
float TeaWristTwistAng = 0.0f;
float TeaFingerAng1 = 0.0f;
float TeaFingerAng2 = 0.0f;

// ROBOT COLORS
GLfloat Ground[] = { 0.5f, 0.5f, 0.5f };
GLfloat Arms[] = { 0.5f, 0.5f, 0.5f };
GLfloat Joints[] = { 0.0f, 0.27f, 0.47f };
GLfloat Fingers[] = { 0.59f, 0.0f, 0.09f };
GLfloat FingerJoints[] = { 0.5f, 0.5f, 0.5f };

// USER INTERFACE GLOBALS
int LeftButtonDown = 0;    // MOUSE STUFF
int RightButtonDown = 0;
int RobotControl = 0;
bool SpaceDown = false;

// settings
const unsigned int SCR_WIDTH = 768;
const unsigned int SCR_HEIGHT = 768;
// HW6
bool radial = true;

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

// HW5 //
const char* ourObjectPath = "./alien.obj";
//const char* ourObjectPath = "./teapot.obj";
// translate it so it's at the center of the scene
// it's a bit too big for our scene, so scale it down
//glm::mat4 objectXform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)), glm::vec3(0.08f, 0.08f, 0.08f));
// 새로운 모델에 맞게 크기, 위치, 각도 조절
glm::mat4 objectXform = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.3f, 0.0f)), glm::vec3(0.0006f, 0.0006f, 0.0006f)), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

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

	/*Extra Credit*/
	// 주전자의 사이즈가 커서 원래 스케일을 0.08배로 줄이고 시작했으나 이 때문에 계산 결과가 의도와 달라지는 것 같아 기본 사이즈로 다시 만들었다.
	// 그리고 원래 처음 위치였던 0,5, 0, 0으로 옮겨주었다.
	//glm::mat4 objectXform = glm::mat4(1.0f);
	//objectXform = glm::translate(objectXform, glm::vec3(0.5f, 0.0f, 0.0f));
	/*************/
	
	// HW5 //
	glm::mat4 objectXform = glm::mat4(1.0f);
	objectXform = glm::translate(objectXform, glm::vec3(0.5f, 0.3f, 0.0f));

	DrawGroundPlane(model);

	// ADD YOUR ROBOT RENDERING STUFF HERE     /////////////////////////////////////////////////////

	// Base => 맨 아래 바닥 + 회색 기둥 + 파란 동그라미
	model = glm::translate(model, glm::vec3(BaseTransX, 0.0f, BaseTransZ));
	model = glm::rotate(model, BaseSpin * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	DrawBase(model); 

	/*Extra Credit*/
	// 마우스가 움직인 만큼 원래 위치에서 이동하면 되므로 TeaTransX와 TeaTransZ를 이용하여 이동시켜준다.
	objectXform = glm::translate(objectXform, glm::vec3(TeaTransX, 0.0f, TeaTransZ));

	// 회전에서 주의해야 할 것은 주전자가 제자리 회전 뿐만 아니라 로봇의 주위를 회전한다는 것이다.
	// 그렇기 때문에 회전 외의 이동도 필요하다.
	// 먼저 주전자를 로봇이 있는 축으로 옮기고 회전한 뒤 다시 원래자리로 옮긴다.
	if (SpaceDown && RobotControl == 1)
	{
		objectXform = glm::translate(objectXform, glm::vec3(-TeaTransX + BaseTransX - 0.5f, 0.0f, -TeaTransX + BaseTransZ));
		objectXform = glm::rotate(objectXform, TeaBaseSpin * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		objectXform = glm::translate(objectXform, glm::vec3(TeaTransX - BaseTransX + 0.5f, 0.0f, TeaTransZ - BaseTransZ));
	}
	/*************/

	// ArmSegment 2개 => 회색 기둥 + 파란 동그라미
	model = glm::translate(model, glm::vec3(0.0f, 0.4f, 0.0f));
	model = glm::rotate(model, ShoulderAng * 0.02f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawArmSegment(model); 

	model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
	model = glm::rotate(model, ElbowAng * 0.016f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawArmSegment(model);

	/*Extra Credit*/
	if (SpaceDown && RobotControl == 2)
	{
		// 축을 첫번째 ArmSegment로 옮긴 뒤 회전하고 다시 원래 자리로 돌아온다.
		objectXform = glm::translate(objectXform, glm::vec3(TeaTransX - 0.5f, 0.4f, TeaTransZ));
		objectXform = glm::rotate(objectXform, TeaShoulderAng * 0.02f, glm::vec3(0.0f, 0.0f, 1.0f));
		objectXform = glm::translate(objectXform, glm::vec3(-TeaTransX + 0.5f, -0.4f, TeaTransZ));

		// 축을 두번째 ArmSegment로 옮긴 뒤 회전하고 다시 원래 자리로 돌아온다.
		objectXform = glm::translate(objectXform, glm::vec3(TeaTransX - 0.5f, 0.9f, TeaTransZ));
		objectXform = glm::rotate(objectXform, TeaElbowAng * 0.016f, glm::vec3(0.0f, 0.0f, 1.0f));
		objectXform = glm::translate(objectXform, glm::vec3(-TeaTransX + 0.5f, -0.9f, TeaTransZ));
	}
	/*************/

	// Wrist => 빨간색 기둥 + 회색 동그라미
	model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
	model = glm::rotate(model, WristAng * 0.015f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, WristTwistAng * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	DrawWrist(model); 

	/*Extra Credit*/
	if (SpaceDown && RobotControl == 3)
	{
		// 축을 Wrist로 옮긴 뒤 회전하고 다시 원래 자리로 돌아온다.
		objectXform = glm::translate(objectXform, glm::vec3(TeaTransX - 0.5f, 1.3f, TeaTransZ));
		objectXform = glm::rotate(objectXform, TeaWristAng * 0.015f, glm::vec3(0.0f, 0.0f, 1.0f));
		objectXform = glm::rotate(objectXform, TeaWristTwistAng * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		objectXform = glm::translate(objectXform, glm::vec3(-TeaTransX + 0.5f, -1.3f, TeaTransZ));
	}
	/*************/
	
	// Right FingerBase, Left FingerBase => 빨간색 얇고 긴 기둥 + 회색 동그라미
	model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f));
	model = glm::rotate(model, FingerAng1 * 0.018f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawFingerBase(model); 

	model = glm::rotate(model, FingerAng1 * -0.036f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawFingerBase(model);

	// Right FingerTip, Left FingerTip => 빨간색 원뿔
	model = glm::rotate(model, FingerAng1 * 0.036f, glm::vec3(0.0f, 0.0f, 1.0f)); // 다시 left 각도로 회전
	model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
	model = glm::rotate(model, FingerAng2 * 0.087f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawFingerTip(model); 

	model = glm::rotate(model, FingerAng2 * -0.087f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(0.0f, -0.35f, 0.0f));
	model = glm::rotate(model, FingerAng1 * -0.036f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
	model = glm::rotate(model, FingerAng2 * -0.087f, glm::vec3(0.0f, 0.0f, 1.0f));
	DrawFingerTip(model);

	/*Extra Credit*/
	//objectXform = glm::scale(objectXform, glm::vec3(100.08f, 100.08f, 100.08f));
	/**************/

	// HW 5 //
	objectXform = glm::scale(objectXform, glm::vec3(0.0006f, 0.0006f, 0.0006f));
	objectXform = glm::rotate(objectXform, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	DrawObject(objectXform);


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
		// HW6
		PhongShader->setBool("radial", radial);
		PhongShader->setVec3("light.position", camera.Position);
		PhongShader->setVec3("light.direction", camera.Front);
		PhongShader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
		PhongShader->setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
		PhongShader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
		PhongShader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
		PhongShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		PhongShader->setFloat("light.constant", 1.0f);
		PhongShader->setFloat("light.linear", 0.09f);
		PhongShader->setFloat("light.quadratic", 0.032f);

		FloorShader->use();
		FloorShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
		FloorShader->setMat4("view", camera.GetViewMatrix());
		FloorShader->setVec3("viewPos", camera.Position);
		FloorShader->setVec3("lightPos", camera.Position);
		// HW6
		FloorShader->setBool("radial", radial);
		FloorShader->setVec3("light.position", camera.Position);
		FloorShader->setVec3("light.direction", camera.Front);
		FloorShader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
		FloorShader->setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
		FloorShader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
		FloorShader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
		FloorShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		FloorShader->setFloat("light.constant", 1.0f);
		FloorShader->setFloat("light.linear", 0.09f);
		FloorShader->setFloat("light.quadratic", 0.032f);

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
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (SpaceDown)
			SpaceDown = false;
		else
			SpaceDown = true;
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// HW4
	float cameraSpeed = 5.0f * deltaTime; // 속도 조절

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, cameraSpeed);

	// HW6
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (radial)
			radial = false;
		else
			radial = true;
	}
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
		// 제공된 실행결과와 유사해지도록 값 조정
		switch (RobotControl)
		{
		case 0: BaseTransX += xoffset; BaseTransZ -= yoffset; break;
		case 1: BaseSpin += xoffset * 270 ; break;
		case 2: ShoulderAng += yoffset   * -90; ElbowAng += xoffset  * 90; break;
		case 3: WristAng += yoffset  * -220; WristTwistAng += xoffset  * 220; break;
		case 4: FingerAng1 += yoffset  * 90; FingerAng2 += xoffset * 60; break;
		}

		if (SpaceDown) 
		{
			switch (RobotControl)
			{
			case 0: TeaTransX += xoffset; TeaTransZ -= yoffset; break;
			case 1: TeaBaseSpin += xoffset * 270;/* TeaTransX += xoffset; TeaTransZ -= yoffset;*/ break;
			case 2: TeaShoulderAng += yoffset * -90; TeaElbowAng += xoffset * 90; break;
			case 3: TeaWristAng += yoffset * -220; TeaWristTwistAng += xoffset * 220; break;
			case 4: TeaFingerAng1 += yoffset * 90; TeaFingerAng2 += xoffset * 60; break;
			}
		}
	} 

	// HW4
	if (RightButtonDown)
	{
		camera.ProcessMouseMovement(xoffset * 700, yoffset * 700);
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


	// HW4
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		RightButtonDown = 1;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		RightButtonDown = 0;
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
