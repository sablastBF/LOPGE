#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model 
{
public:
    // model data 
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    Shader sh;
    glm::mat4 model = glm::mat4(1.0f);

    // constructor, expects a filepath to a 3D model.
    Model(string const &path,bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
        normalize();
    }

    Model(string const &path, Shader sh,bool gamma = false) : gammaCorrection(gamma)
    {
        set_shader(sh);
        loadModel(path);
        normalize();
    }

   

    // draws the model, and thus all its meshes
    void setModel(glm::mat4 mat){
        model = mat;
    }

    void Draw( GLenum tip = GL_TRIANGLES)
    {   sh.use();
        sh.setMat4("model",model);

        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(sh, tip);
    }

    void set_shader(Shader s){
        sh = s;
    }


private:
    void normalize(){
        glm::vec3 max_vec, min_vec;
        max_vec.x = meshes[0].vertices[0].Position.x;
        max_vec.y = meshes[0].vertices[0].Position.y;
        max_vec.z = meshes[0].vertices[0].Position.z;

        min_vec.x = meshes[0].vertices[0].Position.x;
        min_vec.y = meshes[0].vertices[0].Position.y;
        min_vec.z = meshes[0].vertices[0].Position.z;

        for (int i = 0; i < meshes.size(); i++){
            for(int j = 0; j <meshes[i].vertices.size(); j++){
                max_vec.x = max(meshes[i].vertices[j].Position.x, max_vec.x);
                max_vec.y = max(meshes[i].vertices[j].Position.y, max_vec.y);
                max_vec.z = max(meshes[i].vertices[j].Position.z, max_vec.z);

                min_vec.x = min(meshes[i].vertices[j].Position.x,  min_vec.x);
                min_vec.y = min(meshes[i].vertices[j].Position.y,  min_vec.y);
                min_vec.z = min(meshes[i].vertices[j].Position.z,  min_vec.z);
            }
        }
        glm::vec3 duz = max_vec - min_vec;
        int k = std::max({duz.x, duz.y, duz.z});
        glm::vec3 v = glm::vec3(k);

        for (int i = 0; i < meshes.size(); i++){
            for( int j = 0; j < meshes[i].vertices.size(); j++){
                meshes[i].vertices[j].Position = 2.0f * ( meshes[i].vertices[j].Position - min_vec)/v - glm::vec3(1);
            }
             meshes[i].setupMesh();
        }
    }
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene){
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
           

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
       // normalize(vertices);
        return Mesh(vertices, indices);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
};
#endif