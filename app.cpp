#include "app.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);



// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



namespace logle{
  
    App::App(){
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    App::~App(){ glfwTerminate();}

    void App::run() {
        
        window = new Window(800, 600, "Prvi program");
        glfwMakeContextCurrent(window -> window);
        gladLoadGL(); 

      
        glfwSetFramebufferSizeCallback(window -> window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window -> window, mouse_callback);
        glfwSetScrollCallback(window -> window, scroll_callback);

        glfwSetInputMode(window -> window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glEnable(GL_DEPTH_TEST);



    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");
        glm::mat4 transform = glm::mat4(1.0f);
        ourShader.use();
        ourShader.setMat4("model", transform);
        // render loop
        // -----------
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f); 
        glm::vec3 light = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 objectColor = glm::vec3(0.5f, 0.5f, 0.5f);

        ourShader.setVec3("lightPos", light);
        ourShader.setVec3("lightColor", lightColor);
        ourShader.setVec3("objectColor", objectColor);

        Model *mod = new Model("models/medo2.obj");
        mod->setShader(ourShader);



    Shader ourShader2("shaders/shader2.vert", "shaders/shader2.frag");
    ourShader2.use();
    Model *line = new Model();
    line->setShader(ourShader2);

    vector<glm::vec3> dots = {glm::vec3(1.0f),glm::vec3(2.0f),glm::vec3(3.0f),glm::vec3(5.0f),glm::vec3(0.0f,0.5f,-100.f) };
    line->addDot(dots);


    while (!glfwWindowShouldClose(window -> window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window -> window);
        glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        ourShader.use();
        mod->model = glm::rotate(mod->model, glm::radians(1.0f), glm::vec3(1.0, 0.0, 0.0));;  
            ourShader.setVec3("viewPos", camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        mod->Draw(GL_LINES);

        ourShader2.use();
        ourShader2.setMat4("view", view);
        ourShader2.setMat4("projection", projection);
        ourShader2.setVec3("color", glm::vec3(1.0f, 0.0f, 0.5f));

        line->Draw(GL_LINE_STRIP);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window -> window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    }
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}