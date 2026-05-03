#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;


uniform vec3 corCasa;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float brilhoMaterial;

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
    vec3 diffuse = diff*corLuz*0.7;

    //brilho especular
    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 reflectDir = reflect(-ligthDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64); //64 = foco do brilho
    vec3 corBrilho = mix(vec3(1.0), corLuz, 0.5);
    vec3 specular = brilhoMaterial * spec * corBrilho;

    vec3 resultado = (ambient+diffuse+specular)*corCasa;
    FragColor = vec4(resultado, 1.0);
}
