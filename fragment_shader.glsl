#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;


uniform vec3 corCasa;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float brilhoMaterial;

uniform sampler2D textura;
uniform bool usarTextura;
uniform vec2 escalaTextura;

uniform float focoBrilho;
uniform float intensidadeDifusa;

//shader que decide as cores e aplica o modelo de Phong
void main(){

    vec3 corLuz = vec3(1.0, 0.9, 0.6);

    //luz ambiente minima para a sombra nao ficar preta
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength*corLuz;

    //luz que bate de frente
    vec3 norm = normalize(Normal);
    vec3 ligthDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, ligthDir), 0.0);
    vec3 diffuse = diff*corLuz*intensidadeDifusa;

    //brilho especular
    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 reflectDir = reflect(-ligthDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), focoBrilho); 
    vec3 corBrilho = mix(vec3(1.0), corLuz, 0.5);
    vec3 specular = brilhoMaterial * spec * corBrilho;

    vec3 luzCamera = max(dot(norm, viewDir), 0.0)*vec3(0.2, 0.2, 0.2);
    vec3 resultadoLuz = (ambient+diffuse+specular+luzCamera);
    
    if (usarTextura){
        vec4 corImagem = texture(textura, TexCoord*escalaTextura);
        FragColor = vec4(corImagem.rgb*resultadoLuz, 1.0);
    }else{
        FragColor = vec4(corCasa*resultadoLuz, 1.0);
    }
}
