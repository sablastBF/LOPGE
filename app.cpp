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
#include <set>
#include <utility>
#include <stb_image.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

 unsigned int SCR_WIDTH = 1199;
 unsigned int SCR_HEIGHT = 958;
const float TWO_PI = 6.28318530718;
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


class vornoi_centroid{
    public:
    vornoi_centroid(){
    }
    vornoi_centroid(glm::vec2 pos_){
        pos = pos_;
    }
    glm::vec2 pos;
    glm::vec3 boja;
    double r;
    double RR;
};

// void get_hexagon_distribudion(vector<glm::vec2> &v, int p = 50){
//     float j;
//     int k = 0;

//     for (float i = -p; i <= p; i+=0.866025404){
//         if (k % 2) j = -p + 0.5; else j = -p;
//         k++;
//         for (; j <= p; j+=1){
//             v.push_back(glm::vec2(i/p,j/p));
//         }
//     }
// }

// void get_kvadratic_distribudion( vector<glm::vec2> &v, int p){
//     for (float i = -p; i <= p; i+=1){
//         for (float j = -p; j <= p; j+=1){
//             v.push_back(glm::vec2(i/p,j/p));
//         }
//     }
// }

void get_hexagon_distribudion(vector<vornoi_centroid *> &v, int p = 50){
    float j;
    int k = 0;

    for (float i = -p; i <= p+0.2; i+=0.866025404){
        if (k % 2) j = -p + 0.5; else j = -p;
        k++;
        for (; j <= p+0.2; j+=1){
            v.push_back(new vornoi_centroid(glm::vec2(i/p,j/p)));
        }
    }
}

void get_kvadratic_distribudion( vector<vornoi_centroid *> &v, int p){
    for (float i = -p; i <= p; i+=1){
        for (float j = -p; j <= p; j+=1){
            v.push_back(new vornoi_centroid(glm::vec2(i/p,j/p)));
        }
    }
}

void get_kvad_2( vector<vornoi_centroid *> &v, int p){
    for (float i = -p; i <= p; i+=1){
        for (float j = -p; j <= p; j+=1){
            v.push_back(new vornoi_centroid(glm::vec2(i/p,j/p)));
            v.back()->r = 0.5;
            v.back() -> RR = 0.4;
        }
    }

}



void get_hex_2( vector<vornoi_centroid *> &v, int p){
    float j;
    int k = 0;

    for (float i = -p; i <= p+0.2; i+=0.866025404){
        if (k % 2) j = -p + 0.5; else j = -p;
        k++;
        for (; j <= p+0.2; j+=1){
            v.push_back(new vornoi_centroid(glm::vec2(i/p,j/p)));
            v.back() -> r = 0.5;
            v.back() -> RR = 0.5;
        }
    }

}

void get_kvad( vector<vornoi_centroid *> &v){
    v.push_back(new vornoi_centroid(glm::vec2(-0.5,-0.5)));
    v[0] -> r = 0.8;
    v.push_back(new vornoi_centroid(glm::vec2(-0.5,0.5)));
    v[1] -> r = 0.8;
    v.push_back(new vornoi_centroid(glm::vec2(0.5,-0.5)));
    v[2] -> r = 0.8;
    v.push_back(new vornoi_centroid(glm::vec2(0.5,0.5)));
    v[3] -> r = 0.8;

    v[0] -> RR = 0.8;
    v[1] -> RR = 0.8;
    v[2] -> RR = 0.8;
    v[3] -> RR = 0.8;

}

void get_dot( vector<vornoi_centroid *> &v){
    v.push_back(new vornoi_centroid(glm::vec2(0,0)));
    v[0] -> r = 1;
    v[0] -> RR = 0.8;
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
Mat originalna;


cv::Mat get_ocv_img_from_gl_img(GLuint ogl_texture_id)
{
   
    glBindTexture(GL_TEXTURE_2D, ogl_texture_id);

    GLenum gl_texture_width, gl_texture_height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);
    
    unsigned char* gl_texture_bytes = (unsigned char*) malloc(sizeof(unsigned char)*gl_texture_width*gl_texture_height*3);
    

    
    cv::Mat img(gl_texture_height, gl_texture_width, CV_8UC3);
    //cout << img.step<<endl;
    glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
    glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);
    img =cv::Mat(gl_texture_height, gl_texture_width, CV_8UC3, gl_texture_bytes);
    cv::flip(img,img,0);
    return img;
}

void uzorkuj_boje_iz_piksela(vector<vornoi_centroid *> &v,Mat &image,const unsigned int SCR_WIDTH,const unsigned int SCR_HEIGHT){
    
    for (int i = 0 ; i < v.size(); i++){
        v[i] -> boja = pixe(image,v[i] -> pos.x, v[i] -> pos.y, SCR_WIDTH, SCR_HEIGHT);
    }
}

void uzorkuj_boje_iz_piksela_median(vector<vornoi_centroid *> &v,Mat &image,const unsigned int SCR_WIDTH,const unsigned int SCR_HEIGHT, int d = 5){
    vector<glm::vec3> medijan;
    int nx = 0, ny = 0, X= 0, Y=0;
    glm::vec3 color;
    Vec3b vv;
    
    for (int i = 0; i < v.size(); i++){
        //v[i] -> boja = ;  
         X =  double(SCR_WIDTH)*(v[i] -> pos.x + 1)/2.0;
    Y =  double(SCR_HEIGHT)*(v[i] -> pos.y + 1)/2.0;
        for (int x = -d; x <= d; x++){
            for (int y = -d; y <= d; y++){
                
               
                nx = X + x;
                ny = Y+ y;
                if (nx >= 0 && nx < SCR_WIDTH && ny >= 0 && ny < SCR_HEIGHT){
                   //originalna.at<Vec3b>(Point(nx,ny)) = color;
                    vv =image.at<Vec3b>(SCR_HEIGHT- ny, nx);

                    color= glm::vec3(vv[2]/255.0,vv[1]/255.0,vv[0]/255.0);                  
                    medijan.push_back(color);
                }
            }
        }
        // cout << medijan.size()<<endl;
        // cout << medijan[0].x<<" "<<medijan[0].y<<" "<<medijan[0].z<<endl;
        v[i] -> boja = color;
        medijan.clear();
    }

    cout<<"ASDAD"<<endl;
}

double get_local_error(Mat &image,glm::vec2 &pos,const unsigned int SCR_WIDTH,const unsigned int SCR_HEIGHT, double R = 0.1){
    int X =  double(SCR_WIDTH)*(pos.x + 1)/2.0;
    int Y =  double(SCR_HEIGHT)*(pos.y + 1)/2.0;
    double dx = R*SCR_WIDTH/2;
    double dy = R*SCR_HEIGHT/2;
    Vec3b color(255,0,255);
    int nx = 0,ny = 0;
    double MSE = 0.0;
    double p = 0;
    for (int i = -dx; i < dx; i++){
        for (int j = -dy; j< dy; j++){
            nx = i + X;
            ny = j + Y;
            if (nx >= 0 && nx < SCR_WIDTH && ny >= 1 && ny < SCR_HEIGHT){
                //cout << SCR_HEIGHT- ny<<" "<<nx<<endl;
                   // originalna.at<Vec3b>(Point(SCR_HEIGHT- ny, nx)) = color;
                    for (int z = 0;z < 3;z++){
                    //originalna.at<Vec3b>(SCR_HEIGHT- ny, nx)[z] = 100;
                    MSE += pow((image.at<Vec3b>(SCR_HEIGHT -ny, nx)[z] - originalna.at<Vec3b>(SCR_HEIGHT -ny, nx)[z]),2.0);
                   
                    }
                    p++;
            }
        }
    }
    if (p == 0) return 0;
    MSE /= p;
    return MSE;
}


double diference(Mat &a){
    double err = 0;
    for (int i = 0; i < a.cols; i++){
        for (int j = 0; j< a.rows;j++){
            for (int z = 0; z< 3;z++)
            err += abs(a.at<Vec3b>(j, i)[z] - originalna.at<Vec3b>(j,i)[z]);
        }
    }
    return  err;
}
vector<vornoi_centroid *> novi(vector<vornoi_centroid *> &v){
    vector<vornoi_centroid *> no;
    for (int i = 0; i < v.size(); i++){
        no.push_back(new vornoi_centroid(v[i] -> pos));
        no.back() -> r = v[i] -> r;
        no.back() -> boja = v[i] -> boja;
    }
    return no;
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
        string ime_slika = "lp_0.jpg";
        if (sl != "0") ime_slika = sl;
        originalna = imread(ime_slika);
        SCR_WIDTH= originalna.cols;
        SCR_HEIGHT = originalna.rows;
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
    vector<vornoi_centroid *> v;

    
    if (fun == 0)
    get_kvadratic_distribudion(v, p);
    if (fun == 1)
    get_hexagon_distribudion(v, p);
    if (fun == 2)
    get_kvad(v);
    if (fun == 3)
    get_dot(v);
     if (fun == 4)
    get_kvad_2(v,p);
     if (fun == 5)
    get_hex_2(v,p);


    ourShader2.use();
    ourShader2.setFloat("R", 0.1);
    ourShader2.setBool("drawcenter", 0);
    ourShader2.setBool("style", 1);


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
    uzorkuj_boje_iz_piksela(v, image, width,height);

    //uzorkuj_boje_iz_piksela_median(v, image, width,height);
    // width+=5;
    // height+=5;
    // for (int i = 0 ; i < v.size(); i++){
    //     v[i] -> boja = pixe(image,v[i] -> pos.x, v[i] -> pos.y, SCR_WIDTH, SCR_HEIGHT);
    // }
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
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)


    // for (int i = 0; i < v.size(); i++)
    // get_local_error(v[i]->pos, width, height);

    //cv::imshow("Originalna_2", originalna);

    // crta u texturu

    //main loop
    double err =0.0;
    vector<vornoi_centroid *> update, novi_vv;
    vector<vector<vornoi_centroid *>> update_render;
    double treshold = 2000;
    cv::Mat vornoi_image;
   // set<pair<float,float>> st;
    update_render.push_back(novi(v));

    for (int i = 0; i < 8; i++){
        ourShader2.use();
        ourShader2.setBool("style", 0);
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        glViewport(0, 0, width, height);
        glClear(GL_DEPTH_BUFFER_BIT| GL_COLOR_BUFFER_BIT);
        glClear(GL_FRAMEBUFFER);
        cout << "Broj centroida: "<<v.size()<< endl;
        for (int k = 0; k < v.size(); k++){
            ourShader2.setVec2("in_Centroid", v[k]->pos);
            ourShader2.setVec3("colorr", v[k]->boja);
            ourShader2.setFloat("R", v[k]-> r);
            quad->Draw();
        }

        vornoi_image = get_ocv_img_from_gl_img(renderedTexture); 
         double err =0.0;   
        for (int k = 0; k < v.size(); k++){
            err = get_local_error(vornoi_image,v[k] -> pos, width, height, v[k] -> r);
            //err = get_local_error(vornoi_image,v[k] -> pos, width, height);
            if (err > treshold){
                update.push_back(v[k]);
            }
           // cout << "err: "<<err << endl;
        }

        if (update.size() == 0) break;
        // radimo update
        for (int j = 0; j < update.size(); j++){
            
            float N = 6;
            update[j] -> r /=2;
            update[j] -> RR /=1.6;

            for (float z = 0.0; z < N;z++){
                glm::vec2 pomak = float(update[j] -> RR)*glm::vec2(cos(z/N *TWO_PI), sin(z/N*TWO_PI));
                // pozicija
                glm::vec2 nov =glm::vec2(pomak + update[j] -> pos);
                if (nov.x < -1 || nov.y < -1 ||  nov.x >= width || nov.y >= height) continue;
                v.push_back(new vornoi_centroid(nov));
                //boja
                // if (st.count({nov.x,nov.y})) continue;
                // st.insert({nov.x,nov.y});
                v.back() -> boja = pixe(originalna, v.back() -> pos.x, v.back() -> pos.y, width, height);
                //
                v.back() -> r = update[j] -> r;
                v.back() -> RR = update[j] -> RR;
                novi_vv.push_back(v.back());
            }
        }

        update_render.push_back(novi(novi_vv));
        novi_vv.clear();
        update.clear();

        // ourShader2.setBool("style", 0);
        // ourShader2.setBool("drawcenter", 1);
        // glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        // glViewport(0, 0, width, height);
        // glClear(GL_DEPTH_BUFFER_BIT| GL_COLOR_BUFFER_BIT);
        // glClear(GL_FRAMEBUFFER);
        // cout << "Broj centroida: "<<v.size()<< endl;
        // for (int k = 0; k < v.size(); k++){
        //     ourShader2.setVec2("in_Centroid", v[k]->pos);
        //     ourShader2.setVec3("colorr", v[k]->boja);
        //     ourShader2.setFloat("R", v[k]-> r);
        //     quad->Draw();
        // }

        // vornoi_image = get_ocv_img_from_gl_img(renderedTexture); 

        // cv::imshow("Image", vornoi_image);
        // cout << "Broj centroida: "<<v.size()<<endl;
        // cv::waitKey(0);
       
    }

    //Mat flipHorizontal; 
 
    // cv::imshow("Originalna", originalna);
    // cv::waitKey(0);
    // cv::destroyAllWindows();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    cout << "Razlika: "<<diference(originalna)<<endl;
    int w = 0;
    cout << update_render.size()<<endl;
    double global_r = 0;
      float dod = 0.005;
    //   glClear(GL_DEPTH_BUFFER_BIT);
    //     glfwSwapBuffers(window -> window);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    //       glfwSwapBuffers(window -> window);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    while (!glfwWindowShouldClose(window -> window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window -> window);
       // glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT );
    
        // ourShader2.use();
        int p = 0;

        for (int k = 0; k < update_render[w].size(); k++){
            ourShader2.setVec2("in_Centroid", update_render[w][k] -> pos);
            ourShader2.setVec3("colorr",update_render[w][k] -> boja);
            if (global_r >  update_render[w][k]-> r) p++;
            ourShader2.setFloat("R", min(global_r, update_render[w][k]-> r));
            quad->Draw();
        }
    
        global_r += dod;
        if (p ==  update_render[w].size()) {
            global_r = 0;
            w++;
            dod /= 1.4;
        }

        if (w >= update_render.size()){
            w--;
            dod = 0;
        }
        // for (int k = 0; k < v.size(); k++){
        //     v[k] += glm::vec2(0.001 * (int(rand()%3) - 1.0) ,0.001 * (int(rand()%3) - 1.0) );
        // }
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