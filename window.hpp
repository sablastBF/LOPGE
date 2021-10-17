#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

namespace logle{

  class Window {
        public:
        Window(int w, int h, std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

        void initWindow();

        int width;
        int height;

        std::string windowName;
        GLFWwindow *window;
};

}