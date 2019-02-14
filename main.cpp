#include <glad/glad.h> //include glad before everything else, glad will have all the openGL headers included already
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <list>
#include <stdlib.h> //rand
#include <time.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Cube.h"

using namespace std;

//stuff to manage our cubes
list<Cube> cubes;
unsigned int diffuseMapTextureID;
unsigned int specularMapTextureID;
Shader *cubeShader;
bool cubeButtonPressed = false;

//window resize call back
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//mouse move callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//wheel scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//user inputs
void processInput(GLFWwindow *window);

unsigned int loadTexture(char const * path);


//Camera Details
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;
float fov = 45.0f;

//My lamps position
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColour(1.0f, 1.0f, 1.0f);

//mouse details
float lastX = 400, lastY = 300;

//time management
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //core means ignore all the backwards compat crap before
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); replace above line with thise to make it work


	//build window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Video Games?", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//initialise GLAD (openGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//setup openGL viewport x,y,w,h
	glViewport(0, 0, 800, 600);

	//hide cursor but also capture it inside this window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//NOTE: openGL viewPort coordinates range -1 to 1 on x and y axis and transform to window size. E.G viewport(0,0) is windows(400,300)
//register callback with window resize func
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//setup mouse move callback
	glfwSetCursorPosCallback(window, mouse_callback);

	//scroll wheel callback
	glfwSetScrollCallback(window, scroll_callback);

	//Configure global opengl state
	glEnable(GL_DEPTH_TEST);//Z buffer depth testing on!



	//Using our shader class
	Shader objectLightShader("projectionVertexShader2.txt", "objectLightFragmentShader3.txt");
	cubeShader = &objectLightShader;
	Shader lightShader("modelShader.vs", "modelShader.fs");

	//setup light and object colours
	glm::vec3 lightColour(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColour(1.0f, 0.5f, 0.31f);

	// load models
	// -----------
	Model ourModel("assets/12140_Skull_v3_L2.obj");
	Model model2("assets/nanosuit.obj");

	//set cursor position
	glfwSetCursorPos(window, lastX, lastY);

	//load up our container texture
	diffuseMapTextureID = loadTexture("container2.png");
	specularMapTextureID = loadTexture("container2_specular.png");



	//glfw game loop
	float movement = 0;
	while (!glfwWindowShouldClose(window))
	{
		//time management
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		

		processInput(window);

		//RENDER
		//set draw colour
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		//view matrix
		//glm::mat4 view;
		// note that we're translating the scene in the reverse direction of where we want to move
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); //push all world objects away on z axis
		glm::mat4 view = camera.GetViewMatrix();


		//projection matrix
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		glm::vec3 lightPos = glm::vec3(5.0f, 0.0f, 5.0f);
		//DRAW TEXTURED CUBES
		for (Cube cube : cubes){
			glUniform3fv(glGetUniformLocation(objectLightShader.ID, "objectColour"), 1, &objectColour[0]);
			glUniform3fv(glGetUniformLocation(objectLightShader.ID, "lightColour"), 1, &lightColour[0]);
			glUniform3fv(glGetUniformLocation(objectLightShader.ID, "lightPos"), 1, &lightPos[0]);
			glUniform3fv(glGetUniformLocation(objectLightShader.ID, "viewPos"), 1, &camera.Position[0]);

			//transform local model space to world space
			glm::mat4 model = glm::mat4(1.0f);
			//move our rect on x axis
			model = glm::translate(model, cube.pos);
			//rotate on y axis
			model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));


			//apply all these matrices onto our shader
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(cube.shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			//make first texture slot active(0-15 slots)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cube.diffuseMapTextureID);

			//set uniform value for fragmentShader's texture1:
			//1. get uniforms location
			int texture1UniformLocation = glGetUniformLocation(cube.shader->ID, "diffuseMap");
			//2. set texture1 uniform to use texture in slot 0
			glUniform1i(texture1UniformLocation, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, cube.specularMapTextureID);
			//tell fragment shader that its texture2 variable should reference texture slot 1
			glUniform1i(glGetUniformLocation(cube.shader->ID, "specularMap"), 1);

			cube.draw();
		}

		//CUBES
		
			lightShader.use();
			lightShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			lightShader.setVec3("lightColor", lightColour);
			lightShader.setVec3("lightPos", lightPos);
			lightShader.setVec3("viewPos", camera.Position);


			glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			// render the loaded model
			glm::mat4 model = glm::mat4(1.0f);
			glm::mat4 modelB = glm::mat4(1.0f);
			
			model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));	// it's a bit too big for our scene, so scale it down
			model = glm::rotate(model,glm::radians( 270.0f), glm::vec3(1.0f, 0, 0));	
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, movement)); // translate it down so it's at the center of the scene
			lightShader.setMat4("model", model);
			ourModel.Draw(lightShader);

			modelB = glm::scale(modelB, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
			modelB = glm::translate(modelB, glm::vec3(0.0f, -12.0f, 0.0f)); // translate it down so it's at the center of the scene
			lightShader.setMat4("model", modelB);
			model2.Draw(lightShader);
			

			

			if (movement < 15)
			{
				movement += deltaTime;
			}
			else
			{
				movement = 0;
			}



		//Input
		glfwPollEvents();
		//swap buffers to show completed rendered screen
		glfwSwapBuffers(window);
	}


	//kill window propers
	glfwTerminate();

	//system("pause");

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);



	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		if (!cubeButtonPressed)
		{
			cubeButtonPressed = true;

			Cube cube;

			cube.pos.x = rand() % 21 - 10;
			cube.pos.y = rand() % 21 - 10;
			cube.pos.z = rand() % 21 - 10;
			cube.shader = cubeShader;
			cube.diffuseMapTextureID = diffuseMapTextureID;
			cube.specularMapTextureID = specularMapTextureID;

			//add cube to cube list
			cubes.push_back(cube);
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_C) != GLFW_PRESS){
		cubeButtonPressed = false;
	}


}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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