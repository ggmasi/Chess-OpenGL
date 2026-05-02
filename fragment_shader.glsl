#version 330 core
out vec4 FragColor;

//shader que decide as cores

uniform vec3 corCasa;

void main(){
    FragColor = vec4(corCasa, 1.0);
}
