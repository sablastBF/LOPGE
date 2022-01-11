#define STB_IMAGE_IMPLEMENTATION
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
#include <stb_image.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

const unsigned int SCR_WIDTH = 1199;
const unsigned int SCR_HEIGHT = 958;

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


void get_hexagon_distribudion(vector<glm::vec2> &v, int p = 50){
    float j;
    int k = 0;

    for (float i = -p; i <= p; i+=0.866025404){
        if (k % 2) j = -p + 0.5; else j = -p;
        k++;
        for (; j <= p; j+=1){
            v.push_back(glm::vec2(i/p,j/p));
        }
    }
}

void get_kvadratic_distribudion( vector<glm::vec2> &v, int p){
    for (float i = -p; i <= p; i+=1){
        for (float j = -p; j <= p; j+=1){
            v.push_back(glm::vec2(i/p,j/p));
        }
    }
}

glm::vec3 pixe(Mat &slika, float x, float y, int w, int h){

    x = (x + 1.0)/2.0;
    y = (y + 1.0)/2.0;

    glm::vec3 pix;
    int i = x*w;
    int j = y*h; 
   
    
    Vec3b color = slika.at<Vec3b>(Point(i,h - j));

    pix.x = float(color[2])/255.0;
    pix.y = float(color[1])/255.0;
    pix.z = float(color[0])/255.0;
 
    return pix; 
}

cv::Mat get_ocv_img_from_gl_img(GLuint ogl_texture_id)
{
    glBindTexture(GL_TEXTURE_2D, ogl_texture_id);
    GLenum gl_texture_width, gl_texture_height;

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);

    unsigned char* gl_texture_bytes = (unsigned char*) malloc(sizeof(unsigned char)*gl_texture_width*gl_texture_height*3);
    glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);

    return cv::Mat(gl_texture_height, gl_texture_width, CV_8UC3, gl_texture_bytes);
}



namespace logle{
  
    App::App(){
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    App::~App(){ glfwTerminate();}

    void App::run() {

        cout << "Koji nacin crtanja zelite: "<< endl;
        cout << "Kvadratici(0) "<<endl;
        cout << "Heksagoni(1) "<<endl;
        cout <<"Upisite broj"<< endl;
        int fun = 0;
        cin >> fun;
        cout <<"Upisite broj poligona p: "<< endl;
        int p;
        cin >> p;
        cout <<"Upisite ime slike" << endl;

        string sl;
        cin >> sl;
        string ime_slika = "gogh.jpg";
        if (sl != "0") ime_slika = sl;
        
        window = new Window(SCR_WIDTH, SCR_HEIGHT, "Prvi program");
        glfwMakeContextCurrent(window -> window);
        gladLoadGL(); 

      
        glfwSetFramebufferSizeCallback(window -> window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window -> window, mouse_callback);
        glfwSetScrollCallback(window -> window, scroll_callback);

        //glfwSetInputMode(window -> window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glEnable(GL_DEPTH_TEST);


    Shader ourShader2("shaders/shader2.vert", "shaders/shader2.frag");
    
    Model *quad = new Model();
    quad->setShader(ourShader2);

    //vector<glm::vec3> dots = {glm::vec3(1.0f),glm::vec3(2.0f),glm::vec3(3.0f),glm::vec3(5.0f),glm::vec3(0.0f,0.5f,-100.f) };
    vector<glm::vec3> quad_dots = 
    {glm::vec3(1.0f,  1.0f, 0.0f),
    glm::vec3( 1.0f, -1.0f, 0.0f),
    glm::vec3( -1.0f, -1.0f, 0.0f),
    glm::vec3( -1.0f,  1.0f, 0.0f)
    };

    quad->addDot(quad_dots);
    vector<glm::vec2> v;
    vector<glm::vec3> boja;

    
    if (fun == 0)
    get_kvadratic_distribudion(v, p);
    if (fun == 1)
    get_hexagon_distribudion(v, p);

    ourShader2.use();
    ourShader2.setFloat("R", 0.1);
    ourShader2.setBool("drawcenter", 0);
    ourShader2.setBool("style", 0);


    unsigned int texture1;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(ime_slika.c_str(), &width, &height, &nrChannels, 3);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    //ourShader2.setInt("texture1", 0);

    Mat image = imread(ime_slika);
    for (int i = 0 ; i < v.size(); i++){
        boja.push_back(pixe(image,v[i].x, v[i].y, SCR_WIDTH, SCR_HEIGHT));
    }
    //Shader ourShader("shaders/shader.vert", "shaders/shader.frag");


    ///
    GLuint FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout <<"Gss"<<endl;

    vector<vornoi_centroid *> vornoi_diagram;

    ///
        // crta u texturu

    ourShader2.use();
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0, 0, 1024, 768);
    glClear(GL_DEPTH_BUFFER_BIT);

    
  
    for (int k = 0; k < v.size(); k++){
        ourShader2.setVec2("in_Centroid", v[k]);
        ourShader2.setVec3("colorr", boja[k]);
        quad->Draw();
    }

    Mat texture = get_ocv_img_from_gl_img(renderedTexture);
    cv::cvtColor(texture,texture,cv::COLOR_BGRA2RGB);

    cv::imshow("Unity Texture", texture);
    cv::waitKey(0);
    cv::destroyAllWindows();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window -> window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window -> window);
        glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        // ourShader2.use();
        for (int k = 0; k < v.size(); k++){
            ourShader2.setVec2("in_Centroid", v[k]);
            ourShader2.setVec3("colorr", boja[k]);
            quad->Draw();
        }

        for (int k = 0; k < v.size(); k++){
            v[k] += glm::vec2(0.001 * (int(rand()%3) - 1.0) ,0.001 * (int(rand()%3) - 1.0) );
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window -> window);
        glfwPollEvents();
    }
    
    glfwTerminate();
     }
}

class vornoi_centroid{
    public:
    vornoi_centroid(){
    }
    glm::vec2 pos;
    glm::vec3 boja;
    double r;
};

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