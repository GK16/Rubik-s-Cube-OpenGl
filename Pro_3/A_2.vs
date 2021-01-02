#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 vertexColor;
layout (location = 2) in vec3 aNormal;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentColor;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    
//    fragmentColor = vertexColor;
    fragmentColor = vertexColor;
    
}
