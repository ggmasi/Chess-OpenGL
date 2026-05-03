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
#include <cmath>

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
    //lê os vértices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //lê as normais
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    //desconexão para evitar que codigos futuros alterem o VAO/VBO acidentalmente
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);   
}

bool LoadOBJ(const char* path, vector<float>& outVertices){
    vector<glm::vec3> verticesTemporarios;
    vector<glm::vec3> normaisTemporarias;

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
        }else if(tipo == "vn"){
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normaisTemporarias.push_back(normal);
        } else if(tipo == "f"){
            // lê até 4 tokens (suporta triângulos e quads)
            vector<int> indicesVertices;
            vector<int> indicesNormais;
            string token;
            while(iss >> token){
                //acha as barras
                size_t primeiraBarra = token.find('/');
                size_t segundaBarra = token.find('/', primeiraBarra+1);

                //pega o vertice
                indicesVertices.push_back(stoi(token.substr(0, primeiraBarra)) - 1);

                //se houver, pega a normal
                if(segundaBarra != string::npos && segundaBarra + 1 < token.length()){
                    indicesNormais.push_back(stoi(token.substr(segundaBarra+1))-1);
                }else{
                    indicesNormais.push_back(-1);
                }
            }

            // triangula: funciona para triângulos (3) e quads (4)
            for(int i = 1; i + 1 < (int)indicesVertices.size(); i++){
                int v1 = indicesVertices[0];
                int n1 = indicesNormais[0];

                outVertices.push_back(verticesTemporarios[v1].x);
                outVertices.push_back(verticesTemporarios[v1].y);
                outVertices.push_back(verticesTemporarios[v1].z);
               
                //se existir a normal, coloca-a. Se não, adiciona uma normal apontando para Y
                if(n1 != -1){
                    outVertices.push_back(normaisTemporarias[n1].x);
                    outVertices.push_back(normaisTemporarias[n1].y);
                    outVertices.push_back(normaisTemporarias[n1].z);
                }else{
                    outVertices.push_back(0.0f);
                    outVertices.push_back(1.0f);
                    outVertices.push_back(0.0f);
                }

                int v2 = indicesVertices[i];
                int n2 = indicesNormais[i];

                outVertices.push_back(verticesTemporarios[v2].x);
                outVertices.push_back(verticesTemporarios[v2].y);
                outVertices.push_back(verticesTemporarios[v2].z);
               
                //se existir a normal, coloca-a. Se não, adiciona uma normal apontando para Y
                if(n2 != -1){
                    outVertices.push_back(normaisTemporarias[n2].x);
                    outVertices.push_back(normaisTemporarias[n2].y);
                    outVertices.push_back(normaisTemporarias[n2].z);
                }else{
                    outVertices.push_back(0.0f);
                    outVertices.push_back(1.0f);
                    outVertices.push_back(0.0f);
                }


                int v3 = indicesVertices[i + 1];
                int n3 = indicesNormais[i + 1];

                outVertices.push_back(verticesTemporarios[v3].x);
                outVertices.push_back(verticesTemporarios[v3].y);
                outVertices.push_back(verticesTemporarios[v3].z);
               
                //se existir a normal, coloca-a. Se não, adiciona uma normal apontando para Y
                if(n3 != -1){
                    outVertices.push_back(normaisTemporarias[n3].x);
                    outVertices.push_back(normaisTemporarias[n3].y);
                    outVertices.push_back(normaisTemporarias[n3].z);
                }else{
                    outVertices.push_back(0.0f);
                    outVertices.push_back(1.0f);
                    outVertices.push_back(0.0f);
                }

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
    glUniform3f(corLoc, 0.85f, 0.85f, 0.85f);
    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i+0.5f, 0.2f, 6.5f));
        modelPeao = glm::scale(modelPeao, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/6);
    }

    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);
    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i+0.5f, 0.2f, 1.5f));
        modelPeao = glm::scale(modelPeao, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/6);
    }

    return true;
}

bool DrawBishop(int modelLoc, int corLoc, int shaderProgram, int tam){
    //bispos brancos
    glUniform3f(corLoc, 0.85f, 0.85f, 0.85f);

    glm::mat4 modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.5f, 0.2f, 7.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(1.35f, 1.35f, 1.35f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.5f, 0.2f, 7.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(1.35f, 1.35f, 1.35f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    //bispos pretos
    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.5f, 0.2f, 0.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(1.35f, 1.35f, 1.35f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.5f, 0.2f, 0.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(1.35f, 1.35f, 1.35f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    return true;
}

bool DrawRook(int modelLoc, int corLoc, int shaderProgram, int tam){
    
    glUniform3f(corLoc, 0.85f, 0.85f, 0.85f);
    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i+0.5f, 0.2f, 7.5f));
        modelTorre = glm::scale(modelTorre, glm::vec3(1.15f, 1.15f, 1.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/6);
    }

    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);
    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i+0.5f, 0.2f, 0.5f));
        modelTorre = glm::scale(modelTorre, glm::vec3(1.15f, 1.15f, 1.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/6);
    }

    return true;
}

bool DrawQueen(int modelLoc, int corLoc, int shaderProgram, int tam){
    //dama branca
    glUniform3f(corLoc, 0.85f, 0.85f, 0.85f);

    glm::mat4 modelDama = glm::mat4(1.0f);
    modelDama = glm::translate(modelDama, glm::vec3(3.5f, 0.2f, 7.5f));
    modelDama = glm::scale(modelDama, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDama));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    //dama preta
    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);

    modelDama = glm::mat4(1.0f);
    modelDama = glm::translate(modelDama, glm::vec3(3.5f, 0.2f, 0.5f));
    modelDama = glm::scale(modelDama, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDama));
    glDrawArrays(GL_TRIANGLES, 0, tam/6);

    return true;
}

bool DrawKing(int modelLoc, int corLoc, int shaderProgram, int tam){
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);
    glm::mat4 modelRei = glm::mat4(1.0f);
    modelRei = glm::translate(modelRei, glm::vec3(4.5f, 0.2f, 7.5f));
    modelRei = glm::scale(modelRei, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRei));
    glDrawArrays(GL_TRIANGLES, 0, tam / 6);

    glUniform3f(corLoc, 0.1f, 0.1f, 0.1f);
    modelRei = glm::mat4(1.0f);
    modelRei = glm::translate(modelRei, glm::vec3(4.5f, 0.2f, 0.5f));
    modelRei = glm::scale(modelRei, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRei));
    glDrawArrays(GL_TRIANGLES, 0, tam / 6);

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

    //adicao de anti aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    

    //vertices das faces do cubo e suas normais
    float verticesCubo[] = {
        // Face Trás 
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        // Face Frente 
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,

        // Face Esquerda 
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        // Face Direita 
        1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,

        // Face Baixo 
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,

        // Face Cima
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f
    };

    //estruturas para desenhar os objetos
    unsigned int VBO_Tabuleiro;
    unsigned int VAO_Tabuleiro;
    SetupGPUModel(verticesCubo, sizeof(verticesCubo), VAO_Tabuleiro, VBO_Tabuleiro);
    
   
   
    vector<float> verticesPeao;
    LoadOBJ("models/pawn.obj", verticesPeao);
    unsigned int VAO_Peao, VBO_Peao;
    SetupGPUModel(verticesPeao.data(), verticesPeao.size()*sizeof(float), VAO_Peao, VBO_Peao);

    vector<float> verticesBispo;
    LoadOBJ("models/bishop.obj", verticesBispo);
    unsigned int VAO_Bispo, VBO_Bispo;
    SetupGPUModel(verticesBispo.data(), verticesBispo.size()*sizeof(float), VAO_Bispo, VBO_Bispo);

    vector<float> verticesTorre;
    LoadOBJ("models/rook.obj", verticesTorre);
    unsigned int VAO_Torre, VBO_Torre;
    SetupGPUModel(verticesTorre.data(), verticesTorre.size()*sizeof(float), VAO_Torre, VBO_Torre);

    vector<float> verticesDama;
    LoadOBJ("models/queen.obj", verticesDama);
    unsigned int VAO_Dama, VBO_Dama;
    SetupGPUModel(verticesDama.data(), verticesDama.size()*sizeof(float), VAO_Dama, VBO_Dama);

    vector<float> verticesRei;
    LoadOBJ("models/king.obj", verticesRei);
    unsigned int VAO_Rei, VBO_Rei;
    SetupGPUModel(verticesRei.data(), verticesRei.size() * sizeof(float), VAO_Rei, VBO_Rei);





    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    bool cameraBrancas = true;
    bool teclaCApertada = false;

    float deltaTime = 0.0f; //tempo entre o frame atual e o último frame
    float lastFrame = 0.0f; //tempo do último frame

    //posicao inicial (90 graus)
    float anguloAtual = 3.14159f / 2.0f;
    float anguloAlvo = 3.14159f / 2.0f;

    //raio do circulo que a camera vai fazer
    float raioCamera = 10.0f;

    //o loop de renderização (Roda até a janela ser fechada)
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame-lastFrame;
        lastFrame = currentFrame;

        //entrada (ex: se apertar ESC, fecha a janela)
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !teclaCApertada){
            cameraBrancas = !cameraBrancas;
            if(cameraBrancas){
                anguloAlvo = 3.14159f / 2.0f;
            }else{
                anguloAlvo = -3.14159f / 2.0f;
            }
            teclaCApertada = true;
        }
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE){
            teclaCApertada = false;
        }
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

        anguloAtual = anguloAtual + (anguloAlvo-anguloAtual)*1.5f*deltaTime;

        //cria e envia uma matriz de câmera
        float camX = 4.0f + cos(anguloAtual)*raioCamera;
        float camZ = 4.0f + sin(anguloAtual)*raioCamera;
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, 8.0f, camZ), //olho atrás das brancas
            glm::vec3(4.0f, 0.0f, 4.0f),  //centro do tabuleiro
            glm::vec3(0.0f, 1.0f, 0.0f)   //vetor para cima
        );;
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        //pega as localizações dos uniforms
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int corLoc = glGetUniformLocation(shaderProgram, "corCasa");

        int ligthPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3f(ligthPosLoc, 4.0f, 10.0f, 4.0f);

        int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3f(viewPosLoc, camX, 8.0f, camZ);


        //desenha o tabuleiro
        int brilhoLocal = glGetUniformLocation(shaderProgram, "brilhoMaterial");
        glUniform1f(brilhoLocal, 0.1f);
        glBindVertexArray(VAO_Tabuleiro);
        DrawBoard(modelLoc, corLoc, shaderProgram);

        //decide o brilho ao bater a luz no material desenhado
        glUniform1f(brilhoLocal, 0.6f);

        //desenho dos peões
        glBindVertexArray(VAO_Peao);
        DrawPawn(modelLoc, corLoc, shaderProgram, verticesPeao.size());

        //desenho dos bispos
        glBindVertexArray(VAO_Bispo);
        DrawBishop(modelLoc, corLoc, shaderProgram, verticesBispo.size());

        //desenho das torres
        glBindVertexArray(VAO_Torre);        
        DrawRook(modelLoc, corLoc, shaderProgram, verticesTorre.size());

        //desenho das damas
        glBindVertexArray(VAO_Dama);
        DrawQueen(modelLoc, corLoc, shaderProgram, verticesDama.size());

        //desenho dos reis
        glBindVertexArray(VAO_Rei);
        DrawKing(modelLoc, corLoc, shaderProgram, verticesRei.size());

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
