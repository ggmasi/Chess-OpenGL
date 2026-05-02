#version 330 core
layout (location = 0) in vec3 aPos;

//shader que decide as posições

uniform mat4 projecao;
uniform mat4 view;
uniform mat4 model;

void main(){
    //transforma a posição em coordenadas homogêneas (ultimo valor igual a 1 significa que é a coordenada de um ponto)
    gl_Position = projecao * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}