#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


#include "window.hpp"


namespace logle{

    class App{
        public:
            App();
            ~App();

            App(const App &) = delete;
            App &operator=(const App &) = delete;
            static  void framebuffer_size_callback(GLFWwindow* window, int width, int height){
                glViewport(0, 0, width, height);
            }
            void run();

        private:
            Window *window;
    };


}