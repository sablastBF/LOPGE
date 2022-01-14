#version 330 core

layout (location = 0) in vec2 in_Quad;


out vec2 ex_Centroid;

uniform float R;
uniform vec2 in_Centroid;

out vec2 ipos;



void main(){
  ipos = in_Quad * R + in_Centroid;
 
  gl_Position = vec4(ipos, 0.0, 1.0);
}