//biblioteca que carrega funções modernas do OpenGL
#include <GL/glew.h>
//biblioteca para criar a janela de visualização
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

int main() {

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    //criação da estrutura para armazenar grande números de vértices na memória da GPU (vertex buffer object)
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    //para qualquer atualização no GL_ARRAY_BUFFER, será atualizado o VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //copia os valores de "vertices[]" para o VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //inicializa a biblioteca da janela (GLFW)
    if (!glfwInit()) {
        cerr << "Falha ao inicializar o GLFW" << endl;
        return -1;
    }

    //configura o OpenGL para a versão 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //cria a janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "Chess 3D", NULL, NULL);
    if (!window) {
        cerr << "Falha ao criar a janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //inicializa o GLEW (carrega as funções do OpenGL)
    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        cerr << "Falha ao inicializar o GLEW" << endl;
        return -1;
    }

    //o Loop de Renderização (Roda até a janela ser fechada)
    while (!glfwWindowShouldClose(window)) {
        //entrada (ex: se apertar ESC, fecha a janela)
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //renderização: Pinta a tela com uma cor de fundo (Azul escuro)
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //troca os buffers e verifica eventos do sistema (mouse, teclado)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //limpa a memória e fecha
    glfwTerminate();
    return 0;
}
