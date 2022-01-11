#version 330 core

layout (location = 0) in vec2 in_Quad;
//layout (location = 2) in vec2 in_Centroid;
//layout (location = 1) in vec2 aTexCoord;

//out vec2 TexCoord;
out vec2 ex_Centroid;

uniform float R;
uniform vec2 in_Centroid;
//uniform sampler2D ourTexture;

out vec2 ipos;


float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}



void main(){
  ipos = in_Quad * R + in_Centroid;
 //Compute Color
  //TexCoord = aTexCoord * R + in_Centroid;

  // vec2 cl = in_Centroid + vec2(1.0);
  // cl/=vec2(2.0);
  // colorr = vec3(texture(ourTexture, cl));

  gl_Position = vec4(ipos, 0.0, 1.0);
}