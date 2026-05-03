#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

//shader que decide as posições

out vec3 FragPos;
out vec3 Normal;

uniform mat4 projecao;
uniform mat4 view;
uniform mat4 model;

//transforma a posição em coordenadas homogêneas (ultimo valor igual a 1 significa que é a coordenada de um ponto)
void main(){
    FragPos = vec3(model * vec4(aPos, 1.0)); //calcula a posição no mundo

    Normal = mat3(transpose(inverse(model)))*aNormal; //calcula para onde a normal aponta

    gl_Position = projecao * view * vec4(FragPos, 1.0);
}