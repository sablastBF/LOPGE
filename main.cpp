#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "app.hpp"

int main(){
    logle::App app{};
    app.run();
    return 0;
}