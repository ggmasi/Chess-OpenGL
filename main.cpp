#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

unsigned int CarregarTextura(const char* caminho) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // comportamento nas bordas e filtragem
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //aplicacao do filtro anisotropico
    float maxAnisotropia;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropia); //qual o maximo da placa de video?
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropia);//aplica o maximo!

    int largura, altura, canais;
    stbi_set_flip_vertically_on_load(true); // OpenGL lê de baixo para cima
    unsigned char* dados = stbi_load(caminho, &largura, &altura, &canais, 0);
    if (dados) {
        GLenum formato = (canais == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, formato, largura, altura, 0, formato, GL_UNSIGNED_BYTE, dados);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cerr << "Falha ao carregar textura: " << caminho << endl;
    }
    stbi_image_free(dados);
    return texID;
}

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

bool LoadOBJ(const char* path, vector<float>& outVertices){
    vector<glm::vec3> verticesTemporarios;
    vector<glm::vec3> normaisTemporarias;
    vector<glm::vec2> uvsTemporarias;

    ifstream arquivo(path);
    if (!arquivo.is_open()) { cerr << "Falha ao abrir: " << path << endl; return false; }

    string linha;
    while (getline(arquivo, linha)) {
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
        }else if(tipo == "vt"){
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            uvsTemporarias.push_back(uv);
        }else if(tipo == "f"){
            // lê até 4 tokens (suporta triângulos e quads)
            vector<int> indicesVertices;
            vector<int> indicesNormais;
            vector<int> indicesUvs;
            string token;
            //logica para ler os formatos: v, v//vn e v/vt/vn
            while(iss >> token){
                int vIdx = -1, uvIdx = -1, nIdx = -1;
                size_t p1 = token.find('/');
                if(p1 != string::npos){
                    vIdx = stoi(token.substr(0, p1))-1;
                    size_t p2 = token.find('/', p1+1);
                    if(p2 != string::npos){
                        if(p2 > p1+1) uvIdx = stoi(token.substr(p1+1, p2-p1-1))-1;
                        if(p2 + 1 < token.length()) nIdx = stoi(token.substr(p2+1))-1;
                    }else{
                        uvIdx = stoi(token.substr(p1+1))-1;
                    }
                }else{
                    vIdx = stoi(token)-1;
                }

                indicesVertices.push_back(vIdx);
                indicesUvs.push_back(uvIdx);
                indicesNormais.push_back(nIdx);
            }

            // triangula: funciona para triângulos (3) e quads (4) e garante posicao (3), normal (3) e textura (2)
            for(int i = 1; i + 1 < (int)indicesVertices.size(); i++){
                int v[3]  = {indicesVertices[0], indicesVertices[i], indicesVertices[i+1]};
                int n[3]  = {indicesNormais[0], indicesNormais[i], indicesNormais[i+1]};
                int uv[3] = {indicesUvs[0], indicesUvs[i], indicesUvs[i+1]};

                for (int j = 0; j < 3; j++) {
                    //posição
                    outVertices.push_back(verticesTemporarios[v[j]].x);
                    outVertices.push_back(verticesTemporarios[v[j]].y);
                    outVertices.push_back(verticesTemporarios[v[j]].z);

                    //normal
                    if (n[j] != -1) {
                        outVertices.push_back(normaisTemporarias[n[j]].x);
                        outVertices.push_back(normaisTemporarias[n[j]].y);
                        outVertices.push_back(normaisTemporarias[n[j]].z);
                    } else {
                        outVertices.push_back(0.0f); outVertices.push_back(1.0f); outVertices.push_back(0.0f);
                    }

                    //textura
                    if (uv[j] != -1) {
                        outVertices.push_back(uvsTemporarias[uv[j]].x);
                        outVertices.push_back(uvsTemporarias[uv[j]].y);
                    } else {
                        outVertices.push_back(0.0f); outVertices.push_back(0.0f);
                    }
                }
            }
        }
    }
    return true;
}

//função para criar o VBO e VAO
//VBO: estrutura para armazenar grande números de vértices na memória da GPU (vertex buffer object)
//VAO: estrutura que indica à GPU como o VBO está estruturado
void SetupGPUModel(float* vertices, size_t tam, unsigned int& VAO, unsigned int& VBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, tam, vertices, GL_STATIC_DRAW);

    // atributo 0: posição (xyz)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // atributo 1: iluminação (normal)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //atributo 2: textura (uv)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool DrawBoard(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texMarmore, unsigned int texMarmoreEscuro, unsigned int texMadeira, int shaderProgram, int escalaTexturaLoc){

    glUniform1i(usarTexturaLoc, 1);

    for (int x = 0; x < 8; x++){
            for (int z = 0; z < 8; z++){
                //logica para alterar cores
                glm::vec3 cor;
                if((x+z)%2 == 0){
                    glBindTexture(GL_TEXTURE_2D, texMarmore);
                }else{
                    glBindTexture(GL_TEXTURE_2D, texMarmoreEscuro);
                }
                
                glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

                //calculo da matriz Model, utilizando x e z para transladar. y = o, já que o tabuleiro está no plano XZ
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
                model = glm::scale(model, glm::vec3(1.0f, 0.2f, 1.0f)); //achata os cubos
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            
        }

        
        // glUniform3f(corLoc, 0.3f, 0.15f, 0.05f); // Cor sólida para a MarmoreEscuro
        glUniform2f(escalaTexturaLoc, 1.0f, 1.0f);
        
        glUniform1i(usarTexturaLoc, 1);
        glBindTexture(GL_TEXTURE_2D, texMadeira);
        glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

        glm::mat4 modelBorda;

        // Borda Esquerda (Esticada no eixo Z)
        glUniform2f(escalaTexturaLoc, 0.5f, 9.0f);
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(-0.5f, 0.0f, -0.5f)); 
        modelBorda = glm::scale(modelBorda, glm::vec3(0.5f, 0.25f, 9.0f));       
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Direita (Esticada no eixo Z)
        glUniform2f(escalaTexturaLoc, 0.5f, 9.0f);
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(8.0f, 0.0f, -0.5f));
        modelBorda = glm::scale(modelBorda, glm::vec3(0.5f, 0.25f, 9.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Fundo / Superior (Esticada no eixo X)
        glUniform2f(escalaTexturaLoc, 8.0f, 0.5f);
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(0.0f, 0.0f, -0.5f));
        modelBorda = glm::scale(modelBorda, glm::vec3(8.0f, 0.25f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Borda Frente / Inferior (Esticada no eixo X)
        glUniform2f(escalaTexturaLoc, 8.0f, 0.5f);
        modelBorda = glm::mat4(1.0f);
        modelBorda = glm::translate(modelBorda, glm::vec3(0.0f, 0.0f, 8.0f));
        modelBorda = glm::scale(modelBorda, glm::vec3(8.0f, 0.25f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBorda));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //reseta a escala das texturas
        glUniform2f(escalaTexturaLoc, 1.0f, 1.0f);
        return true;
}
// offset: deslocamento, movimento
bool DrawPawn(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam,
              float offsetPeaoBrancoE4 = 0.0f, float offsetPeaoPretoE5 = 0.0f, bool peaoPretoF7Comido = false,
              float alturaBranco = 0.0f, float alturaPreto = 0.0f){
    
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);
    
    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        float dz = (i == 4) ? offsetPeaoBrancoE4 : 0.0f;
        float dy = (i == 4) ? alturaBranco : 0.0f;
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i - 0.05f + 0.5f, 0.1f+dy, 8.1f + dz));
        modelPeao = glm::scale(modelPeao, glm::vec3(0.125f, 0.125f, 0.125f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/8);
    }
    
    glBindTexture(GL_TEXTURE_2D, texPretas);

    for (int i = 0; i < 8; i++){
        glm::mat4 modelPeao = glm::mat4(1.0f);
        float dz = (i == 4) ? offsetPeaoPretoE5 : 0.0f;
        float dy = (i == 4) ? alturaPreto : 0.0f;
        if (i == 5 && peaoPretoF7Comido) continue;
        modelPeao = glm::translate(modelPeao, glm::vec3((float)i - 0.05f + 0.5f, 0.1f+dy, 3.1f + dz));
        modelPeao = glm::scale(modelPeao, glm::vec3(0.125f, 0.125f, 0.125f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPeao));
        glDrawArrays(GL_TRIANGLES, 0, tam/8);
    }

    return true;
}

bool DrawBishop(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam,
                float offsetBispoF1X = 0.0f, float offsetBispoF1Z = 0.0f, float offsetBispoF1Y = 0.0f){
    //bispos brancos
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

    glm::mat4 modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.45f, 0.1f, 7.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.45f + offsetBispoF1X, 0.1f+offsetBispoF1Y, 7.5f + offsetBispoF1Z));
    modelBispo = glm::scale(modelBispo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    //bispos pretos
    glBindTexture(GL_TEXTURE_2D, texPretas);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(2.45f, 0.1f, 0.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    modelBispo = glm::mat4(1.0f);
    modelBispo = glm::translate(modelBispo, glm::vec3(5.45f, 0.1f, 0.5f));
    modelBispo = glm::scale(modelBispo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBispo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    return true;
}

bool DrawKnight(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam,
                float offsetCavaloPretoC6 = 0.0f, float offsetCavaloPretoC6X = 0.0f,
                float offsetCavaloPretoF6 = 0.0f, float offsetCavaloPretoF6X = 0.0f,
                float offsetCavaloPretoC6Y = 0.0f, float offsetCavaloPretoF6Y = 0.0f){
    //cavalos brancos
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);
    
    glm::mat4 modelCavalo = glm::mat4(1.0f);
    modelCavalo = glm::translate(modelCavalo, glm::vec3(1.45f, 0.1f, 8.0f));
    modelCavalo = glm::scale(modelCavalo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCavalo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    modelCavalo = glm::mat4(1.0f);
    modelCavalo = glm::translate(modelCavalo, glm::vec3(6.45f, 0.1f, 8.0f));
    modelCavalo = glm::scale(modelCavalo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCavalo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    //cavalos pretos
    glBindTexture(GL_TEXTURE_2D, texPretas);

    modelCavalo = glm::mat4(1.0f);
    modelCavalo = glm::translate(modelCavalo, glm::vec3(1.45f + offsetCavaloPretoC6X, 0.1f+offsetCavaloPretoC6Y, 1.0f + offsetCavaloPretoC6));
    modelCavalo = glm::scale(modelCavalo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCavalo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    modelCavalo = glm::mat4(1.0f);
    modelCavalo = glm::translate(modelCavalo, glm::vec3(6.45f + offsetCavaloPretoF6X, 0.1f+offsetCavaloPretoF6Y, 1.0f + offsetCavaloPretoF6));
    modelCavalo = glm::scale(modelCavalo, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCavalo));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    return true;
}

bool DrawRook(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam){
    
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i-0.05f+0.5f, 0.1f, 8.55f));
        modelTorre = glm::scale(modelTorre, glm::vec3(0.125f, 0.125f, 0.125f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/8);
    }

    glBindTexture(GL_TEXTURE_2D, texPretas);

    for (int i : {0, 7}){
        glm::mat4 modelTorre = glm::mat4(1.0f);
        modelTorre = glm::translate(modelTorre, glm::vec3((float)i-0.05f+0.5f, 0.1f, 1.55f));
        modelTorre = glm::scale(modelTorre, glm::vec3(0.125f, 0.125f, 0.125f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTorre));
        glDrawArrays(GL_TRIANGLES, 0, tam/8);
    }

    return true;
}

bool DrawQueen(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam,
               float damaBrancaX = 3.45f, float damaBrancaZ = 6.9f, float damaBrancaY = 0.0f){
    //dama branca
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

    glm::mat4 modelDama = glm::mat4(1.0f);
    modelDama = glm::translate(modelDama, glm::vec3(damaBrancaX, 0.1f+damaBrancaY, damaBrancaZ));
    modelDama = glm::scale(modelDama, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDama));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    //dama preta
    glBindTexture(GL_TEXTURE_2D, texPretas);

    modelDama = glm::mat4(1.0f);
    modelDama = glm::translate(modelDama, glm::vec3(3.45f, 0.1f, -0.1f));
    modelDama = glm::scale(modelDama, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDama));
    glDrawArrays(GL_TRIANGLES, 0, tam/8);

    return true;
}

bool DrawKing(int modelLoc, int corLoc, int usarTexturaLoc, unsigned int texBrancas, unsigned int texPretas, int shaderProgram, int tam, float anguloReiPreto = 0.0f){
    
    glUniform1f(usarTexturaLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texBrancas);
    glUniform3f(corLoc, 1.0f, 1.0f, 1.0f);

    glm::mat4 modelRei = glm::mat4(1.0f);
    modelRei = glm::translate(modelRei, glm::vec3(4.45f, 0.1f, 6.25f));
    modelRei = glm::scale(modelRei, glm::vec3(0.125f, 0.125f, 0.125f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRei));
    glDrawArrays(GL_TRIANGLES, 0, tam / 6);

    glBindTexture(GL_TEXTURE_2D, texPretas);

    modelRei = glm::mat4(1.0f);
    modelRei = glm::translate(modelRei, glm::vec3(4.45f, 0.1f, -0.75f));
    modelRei = glm::rotate(modelRei, anguloReiPreto, glm::vec3(1.0f, 0.0f, 0.0f)); //rotacao para o rei deitar ao tomar mate
    modelRei = glm::scale(modelRei, glm::vec3(0.125f, 0.125f, 0.125f));
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
    GLenum err = glewInit();

    // O "Unknown error" ou "Missing GL version" no Core Profile 
    // muitas vezes pode ser ignorado se as funções básicas forem carregadas.
    if (err != GLEW_OK) {
        string erroStr = (const char*)glewGetErrorString(err);
        if (erroStr != "Unknown error") { // Erro específico do Fedora
            cerr << "Falha ao inicializar o GLEW: " << erroStr << endl;
            return -1;
        }
        // Se for "Unknown error", limpamos o erro do OpenGL que o GLEW costuma deixar no log
        glGetError(); 
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



    codigoVertexShader = ReadShaderFile("bg_vertex_shader.glsl");

    //converte a string para um ponteiro de caracteres
    vertexShaderSource = codigoVertexShader.c_str();

    //declaração do vertexShader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //atribuição do codigo fonte do shader ao objeto
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);


    //verificação se a compilação ocorreu corretamente
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "ERROR::BACKGROUND::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }


    codigoFragmentShader = ReadShaderFile("bg_fragment_shader.glsl");
    //converte a string para um ponteiro de caracteres
    fragmentShaderSource = codigoFragmentShader.c_str();
    
    //declaração do fragmentShader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //atribuição do codigo fonte do shader ao objeto
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    //verificação se a compilação ocorreu corretamente
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERROR::BACKGROUND::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //declaracao do shaderProgram
    unsigned int bgShaderProgram = glCreateProgram();

    //atribuição dos shaders anteriores ao shaderProgram
    glAttachShader(bgShaderProgram, vertexShader);
    glAttachShader(bgShaderProgram, fragmentShader);
    glLinkProgram(bgShaderProgram);

    //verificação se a compilação ocorreu corretamente
    glGetProgramiv(bgShaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "ERROR::BACKGROUND::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //com o shaderProgram já compilado, tanto o vertex quanto o fragmente podem ser deletados
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    //vertices das faces do cubo e suas normais
    float verticesCubo[] = {
        // Posições (3)       // Normais (3)      // UVs (2)
        // Face Trás
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

        // Face Frente
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

        // Face Esquerda
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

        // Face Direita
        1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

        // Face Baixo
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

        // Face Cima 
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };

    //vértices para o background (adição das normais "falsas" para adaptar à utilização do SetupGPUModel())
    float quadVertices[] = { 
        //posições (X, Y, Z) | normais "Falsas" | textura (U, V)
        -1.0f,  1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        1.0f, -1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        1.0f, -1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
        1.0f,  1.0f, 0.99f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f
    };

    unsigned int bgVAO, bgVBO;
    SetupGPUModel(quadVertices, sizeof(quadVertices), bgVAO, bgVBO);


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

    vector<float> verticesCavalo;
    LoadOBJ("models/knight.obj", verticesCavalo);
    unsigned int VAO_Cavalo, VBO_Cavalo;
    SetupGPUModel(verticesCavalo.data(), verticesCavalo.size()*sizeof(float), VAO_Cavalo, VBO_Cavalo);
    
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

    
    unsigned int texMarmore = CarregarTextura("textures/marble.jpg");
    unsigned int texMarmoreEscuro = CarregarTextura("textures/blackmarble.jpg");
    unsigned int texMadeira = CarregarTextura("textures/wood.jpg");
    unsigned int texBrancas = CarregarTextura("textures/onyx.jpg");
    unsigned int texPretas = CarregarTextura("textures/blackmetal.jpg");
    unsigned int texBiblioteca = CarregarTextura("textures/biblioteca.jpg");    

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    
    int projecaoLoc = glGetUniformLocation(shaderProgram, "projecao");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int corLoc = glGetUniformLocation(shaderProgram, "corCasa");
    int ligthPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    int usarTexturaLoc = glGetUniformLocation(shaderProgram, "usarTextura");
    int brilhoLocal = glGetUniformLocation(shaderProgram, "brilhoMaterial");
    int focoLoc = glGetUniformLocation(shaderProgram, "focoBrilho");
    int difusaLoc = glGetUniformLocation(shaderProgram, "intensidadeDifusa");
    int escalaTexturaLoc = glGetUniformLocation(shaderProgram, "escalaTextura");

    bool cameraBrancas = true;
    bool teclaCApertada = false;
    bool teclaRApertada = false;

    int passoAtual = 0;
    bool teclaSpaceApertada = false;
    bool animEmAndamento = false;

    float offsetPeaoBrancoE4 = 0.0f;
    float offsetPeaoPretoE5 = 0.0f;
    float damaBrancaX = 3.45f;
    float damaBrancaZ = 6.9f;
    float offsetCavaloPretoC6 = 0.0f;
    float offsetCavaloPretoC6X = 0.0f;
    float offsetCavaloPretoF6 = 0.0f;
    float offsetCavaloPretoF6X = 0.0f;
    float offsetBispoC4 = 0.0f;  
    float offsetBispoF1X = 0.0f; 
    float offsetBispoF1Z = 0.0f;
    bool  peaoPretoF7Comido  = false;
    float anguloReiPreto = 0.0f;

    float offsetPeaoBrancoE4_Alvo = 0.0f;
    float offsetPeaoPretoE5_Alvo = 0.0f;
    float damaBrancaX_Alvo = 3.45f;
    float damaBrancaZ_Alvo = 6.9f;
    float offsetCavaloPretoC6_Alvo = 0.0f;
    float offsetCavaloPretoC6X_Alvo = 0.0f;
    float offsetCavaloPretoF6_Alvo = 0.0f;
    float offsetCavaloPretoF6X_Alvo = 0.0f;
    float offsetBispoF1X_Alvo = 0.0f; 
    float offsetBispoF1Z_Alvo = 0.0f;
    float anguloReiPreto_Alvo = 0.0f;
    
    float deltaTime = 0.0f; //tempo entre o frame atual e o último frame
    float lastFrame = 0.0f; //tempo do último frame

    //posicao inicial (90 graus)
    float anguloAtual = 3.14159f / 2.0f;
    float anguloAlvo = 3.14159f / 2.0f;
    float proximoAngulo = 3.14159f / 2.0f;

    //raio do circulo que a camera vai fazer
    float raioCamera = 10.0f;

    float alturaAnim = 0.1f; 
    int faseMovimento = 0;
    float delay = 0.0f;

    //o loop de renderização (Roda até a janela ser fechada)
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame-lastFrame;
        lastFrame = currentFrame;

        //entrada (ex: se apertar ESC, fecha a janela)
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        //ao apertar a tecla C e a animação não estiver em andamento, a câmera é movimentada
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !teclaCApertada && !animEmAndamento){
            cameraBrancas = !cameraBrancas;
            if(cameraBrancas){
                anguloAlvo = 3.14159f / 2.0f;
                proximoAngulo = 3.14159f / 2.0f;
            }else{
                anguloAlvo = -3.14159f / 2.0f;
                proximoAngulo = -3.14159f / 2.0f;
            }
            teclaCApertada = true;
        }
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE){
            teclaCApertada = false;
        }
        //ao apertar "espaço" a animação do mate do pastor é disparada
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !teclaSpaceApertada){
            passoAtual++;
            teclaSpaceApertada = true;
            animEmAndamento = true;

            offsetPeaoBrancoE4_Alvo = -2.0f;
            proximoAngulo = -3.14159f / 2.0f;
            cameraBrancas = false;
        
        
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
            teclaSpaceApertada = false;
        }
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !teclaRApertada){
            teclaRApertada = true;

            //reseta os controles da máquina de estados
            passoAtual = 0;
            animEmAndamento = false;
            faseMovimento = 0;
            delay = 0.0f;
            peaoPretoF7Comido = false;
            cameraBrancas = true;

            //reseta a câmera
            anguloAlvo = 3.14159f / 2.0f;
            proximoAngulo = 3.14159f / 2.0f;

            //reseta as posições atuais
            offsetPeaoBrancoE4 = 0.0f;
            offsetPeaoPretoE5 = 0.0f;
            damaBrancaX = 3.45f;
            damaBrancaZ = 6.9f;
            offsetCavaloPretoC6 = 0.0f;
            offsetCavaloPretoC6X = 0.0f;
            offsetCavaloPretoF6 = 0.0f;
            offsetCavaloPretoF6X = 0.0f;
            offsetBispoF1X = 0.0f;
            offsetBispoF1Z = 0.0f;
            alturaAnim = 0.1f; 
            anguloReiPreto = 0.0f;

            //reseta as posições alvo
            offsetPeaoBrancoE4_Alvo = 0.0f;
            offsetPeaoPretoE5_Alvo = 0.0f;
            damaBrancaX_Alvo = 3.45f;
            damaBrancaZ_Alvo = 6.9f;
            offsetCavaloPretoC6_Alvo = 0.0f;
            offsetCavaloPretoC6X_Alvo = 0.0f;
            offsetCavaloPretoF6_Alvo = 0.0f;
            offsetCavaloPretoF6X_Alvo = 0.0f;
            offsetBispoF1X_Alvo = 0.0f;
            offsetBispoF1Z_Alvo = 0.0f;
            anguloReiPreto_Alvo = 0.0f;
        }
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE){
            teclaRApertada = false;
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


        glDepthMask(GL_FALSE);

        glUseProgram(bgShaderProgram);
        glBindVertexArray(bgVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texBiblioteca);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDepthMask(GL_TRUE);


        glUseProgram(shaderProgram);
        
        //cria a matriz de projeção e envia para a GPU
        glm::mat4 projecao = glm::perspective(glm::radians(45.0f), proporcaoTela, 0.1f, 100.0f);
        glUniformMatrix4fv(projecaoLoc, 1, GL_FALSE, glm::value_ptr(projecao));

        float velAnim = 2.0f;
        //calcula o movimento que as peças devem fazer
        offsetPeaoBrancoE4 += (offsetPeaoBrancoE4_Alvo-offsetPeaoBrancoE4)*velAnim*deltaTime;
        offsetPeaoPretoE5 += (offsetPeaoPretoE5_Alvo - offsetPeaoPretoE5) * velAnim * deltaTime;
        
        damaBrancaX += (damaBrancaX_Alvo - damaBrancaX) * velAnim * deltaTime;
        damaBrancaZ += (damaBrancaZ_Alvo - damaBrancaZ) * velAnim * deltaTime;
        
        offsetCavaloPretoC6 += (offsetCavaloPretoC6_Alvo - offsetCavaloPretoC6) * velAnim * deltaTime;
        offsetCavaloPretoC6X += (offsetCavaloPretoC6X_Alvo - offsetCavaloPretoC6X) * velAnim * deltaTime;
        
        offsetBispoF1X += (offsetBispoF1X_Alvo - offsetBispoF1X) * velAnim * deltaTime;
        offsetBispoF1Z += (offsetBispoF1Z_Alvo - offsetBispoF1Z) * velAnim * deltaTime;
        
        offsetCavaloPretoF6 += (offsetCavaloPretoF6_Alvo - offsetCavaloPretoF6) * velAnim * deltaTime;
        offsetCavaloPretoF6X += (offsetCavaloPretoF6X_Alvo - offsetCavaloPretoF6X) * velAnim * deltaTime;

        //verifica a distância que falta apenas para a peça da jogada atual
        float diffAtual = 0.0f;
        
        if (passoAtual == 1) diffAtual = abs(offsetPeaoBrancoE4_Alvo - offsetPeaoBrancoE4);
        else if (passoAtual == 2) diffAtual = abs(offsetPeaoPretoE5_Alvo - offsetPeaoPretoE5);
        else if (passoAtual == 3) diffAtual = abs(damaBrancaX_Alvo - damaBrancaX) + abs(damaBrancaZ_Alvo - damaBrancaZ);
        else if (passoAtual == 4) diffAtual = abs(offsetCavaloPretoC6_Alvo - offsetCavaloPretoC6) + abs(offsetCavaloPretoC6X_Alvo - offsetCavaloPretoC6X);
        else if (passoAtual == 5) diffAtual = abs(offsetBispoF1X_Alvo - offsetBispoF1X) + abs(offsetBispoF1Z_Alvo - offsetBispoF1Z);
        else if (passoAtual == 6) diffAtual = abs(offsetCavaloPretoF6_Alvo - offsetCavaloPretoF6) + abs(offsetCavaloPretoF6X_Alvo - offsetCavaloPretoF6X);
        else if (passoAtual == 7) diffAtual = abs(damaBrancaX_Alvo - damaBrancaX) + abs(damaBrancaZ_Alvo - damaBrancaZ);
        else if (passoAtual == 8) diffAtual = abs(anguloReiPreto_Alvo - anguloReiPreto);
        
        if(animEmAndamento){
            float velSubida = 4.0f;
            float velAnimacao = 2.0f;
            float alturaMax = 1.0;
            //os movimentos são divididos em 4 fases: levantar a peça, mover a peça, abaixar a peça e mover a câmera
            if(faseMovimento == 0){
                if(passoAtual == 8){
                    faseMovimento = 1; //o tombo do rei não necessita subir a peça
                }else{
                    alturaAnim += velSubida*deltaTime;
                    if(alturaAnim >= alturaMax){
                        alturaAnim = alturaMax;
                        faseMovimento = 1;
                    }
                }
            }else if(faseMovimento == 1){
                if (passoAtual == 1) offsetPeaoBrancoE4 += (offsetPeaoBrancoE4_Alvo - offsetPeaoBrancoE4) * velAnimacao * deltaTime;
                else if (passoAtual == 2) offsetPeaoPretoE5 += (offsetPeaoPretoE5_Alvo - offsetPeaoPretoE5) * velAnimacao * deltaTime;
                else if (passoAtual == 3) {
                    damaBrancaX += (damaBrancaX_Alvo - damaBrancaX) * velAnimacao * deltaTime;
                    damaBrancaZ += (damaBrancaZ_Alvo - damaBrancaZ) * velAnimacao * deltaTime;
                }
                else if (passoAtual == 4) {
                    offsetCavaloPretoC6 += (offsetCavaloPretoC6_Alvo - offsetCavaloPretoC6) * velAnimacao * deltaTime;
                    offsetCavaloPretoC6X += (offsetCavaloPretoC6X_Alvo - offsetCavaloPretoC6X) * velAnimacao * deltaTime;
                }
                else if (passoAtual == 5) {
                    offsetBispoF1X += (offsetBispoF1X_Alvo - offsetBispoF1X) * velAnimacao * deltaTime;
                    offsetBispoF1Z += (offsetBispoF1Z_Alvo - offsetBispoF1Z) * velAnimacao * deltaTime;
                }
                else if (passoAtual == 6) {
                    offsetCavaloPretoF6 += (offsetCavaloPretoF6_Alvo - offsetCavaloPretoF6) * velAnimacao * deltaTime;
                    offsetCavaloPretoF6X += (offsetCavaloPretoF6X_Alvo - offsetCavaloPretoF6X) * velAnimacao * deltaTime;
                }
                else if (passoAtual == 7) {
                    damaBrancaX += (damaBrancaX_Alvo - damaBrancaX) * velAnimacao * deltaTime;
                    damaBrancaZ += (damaBrancaZ_Alvo - damaBrancaZ) * velAnimacao * deltaTime;
                }
                else if (passoAtual == 8){
                    anguloReiPreto += (anguloReiPreto_Alvo - anguloReiPreto)*velAnimacao*deltaTime;
                }

                // Se a peça atual chegou no destino horizontal, vai para a descida
                if (diffAtual < 0.05f) {
                    faseMovimento = 2; 
                }
            }else if(faseMovimento == 2){
                if(alturaAnim > 0.1f){
                    alturaAnim -= velSubida * deltaTime;
                    if (alturaAnim <= 0.1f) {
                        alturaAnim = 0.1f; 
                        delay = 0.0f;
                    }
                }else{
                    delay += deltaTime;
                    if(delay >= 0.6f){
                        anguloAlvo = proximoAngulo; 
                        faseMovimento = 3; 
                    }
                }
            }else if(faseMovimento == 3){
                // Checa se a câmera chegou no ângulo alvo
                if (abs(anguloAlvo - anguloAtual) < 0.05f) {
                    passoAtual++;
                    faseMovimento = 0; // Prepara a próxima peça para começar subindo
                    
                    // Configura os alvos horizontais do próximo movimento
                    if (passoAtual == 2) {
                        offsetPeaoPretoE5_Alvo = 2.0f;
                        proximoAngulo = 3.14159f / 2.0f;
                        cameraBrancas = true;
                    }
                    else if (passoAtual == 3) {
                        damaBrancaX_Alvo = 7.45f;
                        damaBrancaZ_Alvo = 2.85f;
                        proximoAngulo = -3.14159f / 2.0f;
                        cameraBrancas = false;
                    }
                    else if (passoAtual == 4) {
                        offsetCavaloPretoC6_Alvo = 2.0f;
                        offsetCavaloPretoC6X_Alvo = 1.0f;
                        proximoAngulo = 3.14159f / 2.0f;
                        cameraBrancas = true;
                    }
                    else if (passoAtual == 5) {
                        offsetBispoF1X_Alvo = -3.0f;
                        offsetBispoF1Z_Alvo = -3.0f;
                        proximoAngulo = -3.14159f / 2.0f;
                        cameraBrancas = false;
                    }
                    else if (passoAtual == 6) {
                        offsetCavaloPretoF6_Alvo = 2.0f;
                        offsetCavaloPretoF6X_Alvo = -1.0f;
                        proximoAngulo = 3.14159f / 2.0f;
                        cameraBrancas = true;
                    }
                    else if (passoAtual == 7) {
                        damaBrancaX_Alvo = 5.45f;
                        damaBrancaZ_Alvo = 0.85f;
                        proximoAngulo = -3.14159f / 2.0f; 
                        cameraBrancas = false;
                    }
                    else if (passoAtual == 8) {
                        anguloReiPreto_Alvo = -1.5708f; //-90 graus em radianos, tomba para trás
                        proximoAngulo = -3.14159f / 2.0f; //mantém a câmera parada
                        cameraBrancas = false;
                    }else if (passoAtual > 8){
                        animEmAndamento = false; //XEQUE-MATE!
                    }
                }
            }
        }else{
            anguloAlvo = proximoAngulo;
        }
    
        anguloAtual = anguloAtual+(anguloAlvo-anguloAtual)*1.5f*deltaTime;
        

        if (passoAtual == 7 && abs(damaBrancaX - 5.45f) < 0.1f && abs(damaBrancaZ - 0.85f) < 0.1f) {
            peaoPretoF7Comido = true;
        }

        //cria e envia uma matriz de câmera
        float camX = 4.0f + cos(anguloAtual)*raioCamera;
        float camZ = 4.0f + sin(anguloAtual)*raioCamera;
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, 8.0f, camZ), //olho atrás das brancas
            glm::vec3(4.0f, 0.0f, 4.0f),  //centro do tabuleiro
            glm::vec3(0.0f, 1.0f, 0.0f)   //vetor para cima
        );
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        //pega as localizações dos uniforms
        glUniform3f(ligthPosLoc, 4.0f, 10.0f, 4.0f);

        glUniform3f(viewPosLoc, camX, 8.0f, camZ);


        //desenha o tabuleiro
        glUniform1f(brilhoLocal, 0.3f);
        glUniform1f(focoLoc, 32.0f);
        glUniform1f(difusaLoc, 0.7f);
        glBindVertexArray(VAO_Tabuleiro);
        DrawBoard(modelLoc, corLoc, usarTexturaLoc, texMarmore, texMarmoreEscuro, texMadeira, shaderProgram, escalaTexturaLoc);

        //decide o brilho ao bater a luz no material desenhado
        glUniform1f(brilhoLocal, 1.2f); 
        glUniform1f(focoLoc, 128.0f); 
        
        glUniform1f(difusaLoc, 0.9f);
        //desenho dos peões
        glBindVertexArray(VAO_Peao);
        DrawPawn(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesPeao.size(),
                 offsetPeaoBrancoE4, offsetPeaoPretoE5, peaoPretoF7Comido,
                 (passoAtual == 1) ? alturaAnim : 0.1f,
                 (passoAtual == 2) ? alturaAnim : 0.1f);
        
        glUniform1i(usarTexturaLoc, 0);
        //desenho dos bispos
        glBindVertexArray(VAO_Bispo);
        DrawBishop(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesBispo.size(),
                   offsetBispoF1X, offsetBispoF1Z, (passoAtual == 5) ? alturaAnim : 0.1f);

        //desenho dos cavalos
        glBindVertexArray(VAO_Cavalo);
        DrawKnight(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesCavalo.size(),
                   offsetCavaloPretoC6, offsetCavaloPretoC6X,
                   offsetCavaloPretoF6, offsetCavaloPretoF6X,
                   (passoAtual == 4) ? alturaAnim : 0.1f,
                   (passoAtual == 6) ? alturaAnim : 0.1f);

        //desenho das torres
        glBindVertexArray(VAO_Torre);        
        DrawRook(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesTorre.size());

        //desenho das damas
        glBindVertexArray(VAO_Dama);
        DrawQueen(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesDama.size(),
                  damaBrancaX, damaBrancaZ, (passoAtual == 3 || passoAtual == 7) ? alturaAnim : 0.1f);

        //desenho dos reis
        glBindVertexArray(VAO_Rei);
        DrawKing(modelLoc, corLoc, usarTexturaLoc, texBrancas, texPretas, shaderProgram, verticesRei.size(), anguloReiPreto);

        //troca os buffers e verifica eventos do sistema (mouse, teclado)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //limpa a memória e fecha
    glDeleteBuffers(1, &VBO_Tabuleiro);
    glDeleteBuffers(1, &VBO_Peao);
    glDeleteBuffers(1, &VBO_Bispo);
    glDeleteBuffers(1, &VBO_Cavalo);
    glDeleteBuffers(1, &VBO_Torre);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
