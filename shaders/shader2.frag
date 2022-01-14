#version 330 core

out vec4 fragColor;
uniform float R;
uniform vec2 in_Centroid;
uniform bool drawcenter;
uniform int style;
//uniform sampler2D ourTexture;
//in vec2 TexCoord;
//in vec3 colorr;
in vec2 ipos;
uniform vec3 colorr;


void main(){
    gl_FragDepth = length(ipos-in_Centroid); //Depth Buffer Write
    if(gl_FragDepth > R) discard; //Make them Round

    if(style == 1) //Draw Depth
      fragColor = vec4(vec3(gl_FragDepth/R), 1.0);
    else           //Draw Labeling Map (Normal Voronoi Texture)
      fragColor = vec4(colorr, 1.0);

    if(gl_FragDepth < 0.001 && drawcenter) //Draw Centroids
      fragColor = vec4(1.0, 0.0, 0.0,1.0);


    // vec2 cl = TexCoord + vec2(1.0);
    // cl/=vec2(2.0);
    // fragColor = texture(ourTexture, cl);

}