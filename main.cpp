//biblioteca que carrega funções modernas do OpenGL
#include <GL/glew.h>
//biblioteca para criar a janela de visualização
#include <GLFW/glfw3.h>
//biblioteca para aplicações matemáticas
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

//função para ler o arquivo vertex_shader.gsls
string ReadShaderFile(const char* filePath){
    ifstream arquivo(filePath);
    if(!arquivo.is_open()){
        cerr << "Não foi possível abrir o arquivo " << filePath << endl;
        return ""; 
    }

    stringstream buffer;
    buffer << arquivo.rdbuf(); //coloca todo o conteúdo do arquivo no buffer
    arquivo.close();

    return buffer.str();
}

//função para criar o VBO e VAO
//VBO: estrutura para armazenar grande números de vértices na memória da GPU (vertex buffer object)
//VAO: estrutura que indica à GPU como o VBO está estruturado
void SetupGPUModel(float* vertices, size_t tam, unsigned int& VAO, unsigned int& VBO){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //para qualquer atualização no GL_ARRAY_BUFFER, será atualizado o VBO

    glBufferData(GL_ARRAY_BUFFER, tam, vertices, GL_STATIC_DRAW); //copia os valores de "vertices[]" para o VBO

    //instruções para a GPU conseguir ler o VBO corretamente
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //desconexão para evitar que codigos futuros alterem o VAO/VBO acidentalmente
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool LoadOBJ(const char* path, vector<float>& outVertices){
    vector<glm::vec3> verticesTemporarios;

    ifstream arquivo(path);
    if(!arquivo.is_open()){
        cerr << "Falha ao abrir o modelo: " << path << endl;
        return false;
    }

    string linha;
    while(getline(arquivo, linha)){
        istringstream iss(linha);
        string tipo;
        iss >> tipo;

        if(tipo == "v"){
            glm::vec3 vertice;
            iss >> vertice.x >> vertice.y >> vertice.z;
            verticesTemporarios.push_back(vertice);
        } else if(tipo == "f"){
            // lê até 4 tokens (suporta triângulos e quads)
            vector<int> indices;
            string token;
            while(iss >> token){
                // pega só o primeiro número antes de '/' (ex: "3/1/2" → 3)
                int idx = stoi(token.substr(0, token.find('/')));
                indices.push_back(idx - 1); // converte para index 0
            }

            // triangula: funciona para triângulos (3) e quads (4)
            for(int i = 1; i + 1 < (int)indices.size(); i++){
                auto push = [&](int i){
                    outVertices.push_back(verticesTemporarios[i].x);
                    outVertices.push_back(verticesTemporarios[i].y);
                    outVertices.push_back(verticesTemporarios[i].z);
                };
                push(indices[0]);
                push(indices[i]);
                push(indices[i+1]);
            }
        }
    }

    arquivo.close();
    return true;
}


bool DrawBoard(int modelLoc, int corLoc, int shaderProgram){

    for (int x = 0; x < 8; x++){
            for (int z = 0; z < 8; z++){
                //logica para alterar cores
                glm::vec3 cor;
                if((x+z)%2 == 0){
                    cor = glm::vec3(0.9f, 0.9f, 0.8f); //bege claro
                }else{
                    cor = glm::vec3(0.4f, 0.2f, 0.1f); //marrom escuro
                }
                glUniform3f(corLoc, cor.r, cor.g, cor.b);
                
                //calculo da matriz Model, utilizando x e z para transladar. y = o, já que o tabuleiro está no plano XZ
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
                model = glm::scale(model, glm::vec3(1.0f, 0.2f, 1.0f)); //achata os cubos
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            
        }
        
        glUniform3f(corLoc, 0.3f, 0.15f, 0.05f); // Cor sólida para a madeira

        glm::mat4 modelBorda;

        // Borda Esquerda (Esticada no eixo Z)
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(-0.5f, 0.0f, -0.5f)); 
        modelBorda = glm::scale(modelBorda, glm::vec3(0.5f, 0.25f, 9.0f));       
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Direita (Esticada no eixo Z)
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(8.0f, 0.0f, -0.5f));
        modelBorda = glm::scale(modelBorda, glm::vec3(0.5f, 0.25f, 9.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Fundo / Superior (Esticada no eixo X)
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(0.0f, 0.0f, -0.5f));
        modelBorda = glm::scale(modelBorda, glm::vec3(8.0f, 0.25f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Frente / Inferior (Esticada no eixo X)
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(0.0f, 0.0f, 8.0f));
        modelBorda = glm::scale(modelBorda, glm::vec3(8.0f, 0.25f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        return true;
}

bool DrawPawn(int modelLoc, int corLoc, int shaderProgram, int tam){
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i+0.5f, 0.2f, 6.5f));
        modelPeao = glm::scale(modelPeao, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/3);
    }

    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);
    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i+0.5f, 0.2f, 1.5f));
        modelPeao = glm::scale(modelPeao, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/3);
    }

    return true;
}

<<<<<<< HEAD
bool DrawBishop(int modelLoc, int corLoc, int shaderProgram, int tam){
    //bispos brancos
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

    glm::mat4 modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.5f, 0.2f, 7.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/3);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.5f, 0.2f, 7.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/3);

    //bispos pretos
    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.5f, 0.2f, 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/3);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.5f, 0.2f, 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/3);
=======
bool DrawRook(int modelLoc, int corLoc, int shaderProgram, int tam){
    
    glUniform3f(corLoc, 0.95f, 0.95f, 0.85f);
    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i+0.5f, 0.2f, 7.5f));
        modelTorre = glm::scale(modelTorre, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/3);
    }

    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);
    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i+0.5f, 0.2f, 0.5f));
        modelTorre = glm::scale(modelTorre, glm::vec3(0.6f, 0.6f, 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/3);
    }
>>>>>>> ecb278f00ab156ee31d51d913efcf067511ad02d

    return true;
}

int main() {

    
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


    string codigoVertexShader = ReadShaderFile("vertex_shader.glsl");

    //converte a string para um ponteiro de caracteres
    const char* vertexShaderSource = codigoVertexShader.c_str();

    //declaração do vertexShader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //atribuição do codigo fonte do shader ao objeto
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);


    //verificação se a compilação ocorreu corretamente
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }


    string codigoFragmentShader = ReadShaderFile("fragment_shader.glsl");
    //converte a string para um ponteiro de caracteres
    const char* fragmentShaderSource = codigoFragmentShader.c_str();
    
    //declaração do fragmentShader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //atribuição do codigo fonte do shader ao objeto
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    //verificação se a compilação ocorreu corretamente
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //declaracao do shaderProgram
    unsigned int shaderProgram = glCreateProgram();

    //atribuição dos shaders anteriores ao shaderProgram
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //verificação se a compilação ocorreu corretamente
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //com o shaderProgram já compilado, tanto o vertex quanto o fragmente podem ser deletados
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    


    float verticesCubo[] = {
        // Face de trás
        0.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 
        1.0f, 1.0f, 0.0f, 
        1.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 0.0f, 
        0.0f, 0.0f, 0.0f, 

        // Face da frente
        0.0f, 0.0f, 1.0f, 
        1.0f, 0.0f, 1.0f, 
        1.0f, 1.0f, 1.0f, 
        1.0f, 1.0f, 1.0f, 
        0.0f, 1.0f, 1.0f, 
        0.0f, 0.0f, 1.0f, 

        // Face da esquerda
        0.0f, 1.0f, 1.0f, 
        0.0f, 1.0f, 0.0f, 
        0.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 1.0f, 
        0.0f, 1.0f, 1.0f, 

        // Face da direita
        1.0f, 1.0f, 1.0f, 
        1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 1.0f, 
        1.0f, 1.0f, 1.0f, 

        // Face de baixo
        0.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 1.0f, 
        1.0f, 0.0f, 1.0f, 
        0.0f, 0.0f, 1.0f, 
        0.0f, 0.0f, 0.0f, 

        // Face de cima
        0.0f, 1.0f, 0.0f, 
        1.0f, 1.0f, 0.0f, 
        1.0f, 1.0f, 1.0f, 
        1.0f, 1.0f, 1.0f, 
        0.0f, 1.0f, 1.0f, 
        0.0f, 1.0f, 0.0f
    };

    //estruturas para desenhar os objetos
    unsigned int VBO_Tabuleiro;
    unsigned int VAO_Tabuleiro;
    SetupGPUModel(verticesCubo, sizeof(verticesCubo), VAO_Tabuleiro, VBO_Tabuleiro);
    
   
   
    vector<float> verticesPeao;
    LoadOBJ("models/pawn.obj", verticesPeao);
    unsigned int VAO_Peao, VBO_Peao;
    SetupGPUModel(verticesPeao.data(), verticesPeao.size()*sizeof(float), VAO_Peao, VBO_Peao);

<<<<<<< HEAD
    vector<float> verticesBispo;
    LoadOBJ("models/bishop.obj", verticesBispo);
    unsigned int VAO_Bispo, VBO_Bispo;
    SetupGPUModel(verticesBispo.data(), verticesBispo.size()*sizeof(float), VAO_Bispo, VBO_Bispo);
=======
    vector<float> verticesTorre;
    LoadOBJ("models/rook.obj", verticesTorre);
    unsigned int VAO_Torre, VBO_Torre;
    SetupGPUModel(verticesTorre.data(), verticesTorre.size()*sizeof(float), VAO_Torre, VBO_Torre);

>>>>>>> ecb278f00ab156ee31d51d913efcf067511ad02d





    glEnable(GL_DEPTH_TEST);


    //o loop de renderização (Roda até a janela ser fechada)
    while (!glfwWindowShouldClose(window)) {
        //entrada (ex: se apertar ESC, fecha a janela)
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //pega a largura e altura dinâmicas da janela
        int largura, altura;
        glfwGetFramebufferSize(window, &largura, &altura);

        //prevenção de divisão por zero (minimização da tela)
        if(altura == 0) altura = 1;
        
        //atualiza a área de desenho para ocupar a tela toda
        glViewport(0, 0, largura, altura);

        //calcula a proporção da tela em determinado instante
        float proporcaoTela = (float)largura/(float)altura;
        
        //renderização: Pinta a tela com uma cor de fundo (Azul escuro)
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        //cria a matriz de projeção e envia para a GPU
        glm::mat4 projecao = glm::perspective(glm::radians(45.0f), proporcaoTela, 0.1f, 100.0f);
        int projecaoLoc = glGetUniformLocation(shaderProgram, "projecao");
        glUniformMatrix4fv(projecaoLoc, 1, GL_FALSE, glm::value_ptr(projecao));

        //cria e envia uma matriz de câmera
        glm::mat4 view = glm::lookAt(
            glm::vec3(4.0f, 8.0f, 14.0f), //olho da câmera
            glm::vec3(4.0f, 0.0f, 4.0f), //olhando para o centro do tabuleiro
            glm::vec3(0.0f, 1.0f, 0.0f) //vetor para cima
        );
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        //pega as localizações dos uniforms
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int corLoc = glGetUniformLocation(shaderProgram, "corCasa");

        
        glBindVertexArray(VAO_Tabuleiro);
        //desenha o tabuleiro
        DrawBoard(modelLoc, corLoc, shaderProgram);


        //desenho dos peões
        glBindVertexArray(VAO_Peao);
        DrawPawn(modelLoc, corLoc, shaderProgram, verticesPeao.size());

<<<<<<< HEAD
        //desenho dos bispos
        glBindVertexArray(VAO_Bispo);
        DrawBishop(modelLoc, corLoc, shaderProgram, verticesBispo.size());

        

=======
        //desenho das torres
        glBindVertexArray(VAO_Torre);        
        DrawRook(modelLoc, corLoc, shaderProgram, verticesTorre.size());
>>>>>>> ecb278f00ab156ee31d51d913efcf067511ad02d

        //troca os buffers e verifica eventos do sistema (mouse, teclado)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //limpa a memória e fecha
    glDeleteBuffers(1, &VBO_Tabuleiro);
    glDeleteBuffers(1, &VBO_Bispo);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
