#version 330 core
in vec2 texCoord;

out vec4 FragColor;

uniform vec4 color;

void main(){
    FragColor = vec4(1,0,0,1);//color;
};