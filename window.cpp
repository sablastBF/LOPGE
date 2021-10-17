#include "window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
namespace logle{

    Window::Window(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }

    void Window::framebuffer_size_callback(GLFWwindow* window, int w, int h){
        glViewport(0, 0, w, h);
    }

    void Window::initWindow(){
        this -> window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);
        glfwSetFramebufferSizeCallback( window, framebuffer_size_callback);
    }

}