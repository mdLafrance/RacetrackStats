#version 330 core

uniform mediump vec3 u_color;

out vec4 FragColor;

void main(){
    FragColor = vec4(u_color.xyz, 1.0f);
};
