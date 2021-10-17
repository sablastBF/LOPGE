#include "app.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>




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

        while (!window -> shouldClose()) {
            glClearColor(1.0f, 0.3f, 1.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window -> window);
            glfwPollEvents();
        }
    }

}