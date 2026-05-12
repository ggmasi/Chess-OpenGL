# Chess 3D — Computação Gráfica

Visualização de uma cena 3D de xadrez implementada em C++ com OpenGL 3.3 Core Profile.

## Tecnologias

- **Linguagem:** C++
- **API Gráfica:** OpenGL 3.3 Core Profile
- **Bibliotecas:** GLEW, GLFW, GLM

## Como compilar e executar

**Instalar dependências (Ubuntu/Debian):**
```bash
sudo apt install libglew-dev libglfw3-dev libglu1-mesa-dev
```

**Compilar:**
```bash
g++ main.cpp -o chess -lGL -lGLEW -lglfw -lGLU
```

**Executar:**
```bash
./chess
```

## Interação

| Tecla | Ação |
|-------|------|
| `C` | Alterna a câmera entre a perspectiva das brancas e das pretas |
| `ESC` | Fecha a aplicação |

## Especificações atendidas

### Objetos 3D com posicionamento e escala individuais

O tabuleiro é construído com cubos unitários redimensionados via `glm::scale` e posicionados via `glm::translate`. As peças são carregadas de arquivos `.obj` e cada uma tem sua própria matriz Model com posição e escala aplicadas individualmente:

### Shader próprio

Os shaders GLSL estão em `vertex_shader.glsl` e `fragment_shader.glsl`, implementados do zero sem uso de pipeline de função fixa. O fragment shader aplica o modelo de iluminação **Phong** com três componentes: ambiente, difusa e especular.

### Duas câmeras

Duas posições de câmera são definidas via `glm::lookAt` e alternadas com a tecla `C`:

- **Câmera das brancas:** posição `(4, 8, 14)`, olhando para o centro do tabuleiro
- **Câmera das pretas:** posição `(4, 8, -6)`, olhando para o centro do tabuleiro pelo lado oposto

### Iluminação

Descrever iluminacao

---

## Especificações pendentes

As seções abaixo devem ser preenchidas pelos membros do grupo conforme forem implementando:

### Movimento de objeto
Descreva aqui qual objeto se move, como ele se move e qual tecla/evento dispara o movimento._

### Textura
Descreva aqui em qual objeto a textura foi aplicada, qual arquivo de imagem foi usado e como foi feito o mapeamento UV._

---

## Estrutura do projeto

```
├── main.cpp
├── vertex_shader.glsl
├── fragment_shader.glsl
└── models/
    ├── pawn.obj
    ├── bishop.obj
    ├── rook.obj
    ├── queen.obj
    ├── king.obj
    ├── knight.obj
    └── chessKitExport.obj
```


![Gif mostrando a execução do código](jogo.gif)

