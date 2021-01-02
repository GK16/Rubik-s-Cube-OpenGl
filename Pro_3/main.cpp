//
//  main.cpp
//  Pro_3
//
//  Created by Asher Huang on 2020/11/28.
//  Copyright © 2020 Asher Huang. All rights reserved.
//
#pragma execution_character_set("utf-8")
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <eigen3/Eigen/Eigen>
#include <mutex>
#include <cmath>

using namespace std;
using namespace Eigen;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, vector<glm::mat4> &allMats);
void curse_poscallback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
float area_3Points_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
bool point_in_3Points_triangle (float xpos, float ypos, float x1, float y1, float x2, float y2, float x3, float y3);
float getRotatingCubesCoord(char axis, float offsetToCenter);

void show(glm::mat4 m);
void showmodel(glm::mat4 m);

//视角矩阵和投影矩阵
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

//魔方的阶，默认为3
int RANK = 3;
int numOfCubes = 27;

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;
// camera
glm::vec3 cameraPos = glm::vec3(50.f, 50.f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  5.0f;


//整体旋转使用
float view_deltaAlpha = 0.0f;
float view_deltaBeta = 0.0f;
float view_deltaTheta = 0.0f; //左右转

//某一层旋转所用
glm::vec3 cubesSelected = glm::vec3(0.0, 0.0, 0.0);
double allTime = 0;
float angle = 0;
float targetAngle = 90;
glm::vec3 axisVec;
mutex axisMutex;

//初始化为单位矩阵
glm::mat4 currentModel = glm::mat4(1.0f);
glm::mat4 world = glm::mat4(1.f);
vector<glm::vec3> cubePositions;

//鼠标点击的坐标
int x_click = 0;
int y_click = 0;
bool clickChanged = false;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(12.0f, 10.0f, 20.0f);

enum State
{
    RESTING, RUN_X, RUN_Y, RUNZ, UPDATE, OTHER
};
State state = RESTING;

enum Axis{
    X ,Y, Z, NONE
};
Axis rotationAxis = NONE;

int main()
{
    // 获取魔方的阶数
    // ------------------------------
    cout<<"请输入魔方的阶数："<<endl;
    cin>>RANK;
    numOfCubes = pow(RANK,3);
    std::cout << RANK << "阶魔方正在生成" << std::endl;
    vector<glm::mat4> allMats(numOfCubes, glm::mat4(1.f));
    
    //偏移，使得魔方中心为坐标原点
    float offsetToCenter = -(RANK - 1) / 2.0 - (RANK - 1) * 0.05;
    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "魔方", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, curse_poscallback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("/Users/asherhuang/Desktop/Pro_3/Pro_3/A_2.vs", "/Users/asherhuang/Desktop/Pro_3/Pro_3/A_2.fs");
    Shader lightingShader("/Users/asherhuang/Desktop/Pro_3/Pro_3/A_2_lighting.vs", "/Users/asherhuang/Desktop/Pro_3/Pro_3/A_2_lighting.fs");
    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float originCubeVertices[] = {
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,   -0.5f,  0.5f, -0.5f,   -0.5f, -0.5f, -0.5f,
        
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,   -0.5f,  0.5f,  0.5f,   -0.5f, -0.5f,  0.5f,
        
        -0.5f,  0.5f,  0.5f,   -0.5f,  0.5f, -0.5f,   -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,   -0.5f, -0.5f,  0.5f,   -0.5f,  0.5f,  0.5f,
        
        0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
        
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,   -0.5f, -0.5f,  0.5f,   -0.5f, -0.5f, -0.5f,
        
        -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,   -0.5f,  0.5f,  0.5f,   -0.5f,  0.5f, -0.5f
    };
    //各个三角面片的法向量
    float normals[] = {
          0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,
          0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,
          
          0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
          0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  1.0f,
        
         -1.0f,  0.0f,  0.0f,-1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         -1.0f,  0.0f,  0.0f,-1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        
        1.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f,
        
        0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f,
          
        0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f
    };
    
    float surfaceColors[] = {
        //白色
        1.f,1.f,1.f,   1.f,1.f,1.f,   1.f,1.f,1.f,
        1.f,1.f,1.f,   1.f,1.f,1.f,   1.f,1.f,1.f,
        //红色
        1.0f,0.f,0.f,   1.0f,0.f,0.f,   1.0f,0.f,0.f,
        1.0f,0.f,0.f,   1.0f,0.f,0.f,   1.0f,0.f,0.f,
        //绿色
        0.0f,1.f,0.f,   0.0f,1.f,0.f,   0.0f,1.f,0.f,
        0.0f,1.f,0.f,   0.0f,1.f,0.f,   0.0f,1.f,0.f,
        //蓝色
        0.0f,0.f,1.f,   0.0f,0.f,1.f,   0.0f,0.f,1.f,
        0.0f,0.f,1.f,   0.0f,0.f,1.f,   0.0f,0.f,1.f,
        //黑色
        0.f,0.f,0.f,   0.f,0.f,0.f,   0.f,0.f,0.f,
        0.f,0.f,0.f,   0.f,0.f,0.f,   0.f,0.f,0.f,
        //浅蓝
        0.f,1.f,1.f,   0.f,1.f,1.f,   0.f,1.f,1.f,
        0.f,1.f,1.f,   0.f,1.f,1.f,   0.f,1.f,1.f
    };
    
    // world space positions of our cubes
    //一次填充各个小块的位置
    for (int x=0; x<RANK; x++) {
        for (int y=0; y<RANK; y++) {
            for (int z=0; z<RANK; z++) {
                float _x = x * 1.1 + offsetToCenter;
                float _y = y * 1.1 + offsetToCenter;
                float _z = z * 1.1 + offsetToCenter;
                cubePositions.push_back(glm::vec3(_x, _y, _z));
            }
        }
    }

    //绑定顶点数据
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originCubeVertices), originCubeVertices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    unsigned int colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(surfaceColors), surfaceColors, GL_STATIC_DRAW);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    //normal
    unsigned int VBO2, VAO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    
    glBindVertexArray(VAO2);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    
    // normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    
    
    ourShader.use();
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
        // -----------------------------------------------------------------------------------------------------------
        projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window,allMats);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // activate shader
        ourShader.use();
        ourShader.setVec3("light.position", lightPos);
        ourShader.setVec3("viewPos", cameraPos);
        // light properties
        glm::vec3 lightColor;
//        lightColor.x = sin(glfwGetTime() * 2.0f);
//        lightColor.y = sin(glfwGetTime() * 0.7f);
//        lightColor.z = sin(glfwGetTime() * 1.3f);
        lightColor.x = sin(2.0f);
        lightColor.y = sin(0.7f);
        lightColor.z = sin(1.3f);
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        ourShader.setVec3("light.ambient", ambientColor);
        ourShader.setVec3("light.diffuse", diffuseColor);
        ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        // material properties
        ourShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        ourShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        ourShader.setFloat("material.shininess", 32.0f);
        
        
        // camera/view transformation
        view = glm::lookAt(cameraPos, glm::vec3(0.f,0.f,0.f), cameraUp);
        ourShader.setMat4("view", view);
        // calculate the model matrix for each object and pass it to shader before drawing
        //整体视角旋转后的当前model
        currentModel = glm::rotate(currentModel, glm::radians(view_deltaAlpha), glm::vec3(0.0f, 1.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(view_deltaBeta), glm::vec3(1.0f, 0.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(view_deltaTheta), glm::vec3(0.0f, 0.f, 1.f));
        
        //每次循环，都检查鼠标点击是否发生、旋转的目标方块是否改变
        int index_i = -1;
        int index_j = -1;
        float nearest_Z = 1;
        if(clickChanged){
            //遍历每个方块
            for (int i = 0; i < numOfCubes; i++){
                //求各个方块的model矩阵
                glm::mat4 modelNextFrame = currentModel;
                modelNextFrame *= allMats[i];
                glm::mat4 model = glm::translate(modelNextFrame, cubePositions[i]);
                //分别检查方块的12个三角面片
                for (int j = 0; j < 12; j++){
                    //取出三角面片三顶点局部空间坐标
                    glm::vec4 vertex0 = glm::vec4(originCubeVertices[j*9], originCubeVertices[j*9+1], originCubeVertices[j*9+2], 1.0f);
                    glm::vec4 vertex1 = glm::vec4(originCubeVertices[j*9+3], originCubeVertices[j*9+4], originCubeVertices[j*9+5], 1.0f);
                    glm::vec4 vertex2 = glm::vec4(originCubeVertices[j*9+6], originCubeVertices[j*9+7], originCubeVertices[j*9+8], 1.0f);
                    //计算三角面片三顶点剪裁空间坐标
                    
                    glm::vec4 vertexClippedPos_0 = projection * view * model * vertex0;
                    glm::vec4 vertexClippedPos_1 = projection * view * model * vertex1;
                    glm::vec4 vertexClippedPos_2 = projection * view * model * vertex2;
                    Z
                    float vertexScreenPos_1_x =  (vertexClippedPos_1.x / vertexClippedPos_1.w + 1.0) / 2.0 * SCR_WIDTH;
                    float vertexScreenPos_2_x =  (vertexClippedPos_2.x / vertexClippedPos_2.w + 1.0) / 2.0 * SCR_WIDTH;
                    
                    float vertexScreenPos_0_y =  (vertexClippedPos_0.y / vertexClippedPos_0.w + 1.0) / 2.0 * SCR_HEIGHT;
                    float vertexScreenPos_1_y =  (vertexClippedPos_1.y / vertexClippedPos_1.w + 1.0) / 2.0 * SCR_HEIGHT;
                    float vertexScreenPos_2_y =  (vertexClippedPos_2.y / vertexClippedPos_2.w + 1.0) / 2.0 * SCR_HEIGHT;
                    
                    float vertexScreenPos_0_z =  (vertexClippedPos_0.z / vertexClippedPos_0.w);
                    float vertexScreenPos_1_z =  (vertexClippedPos_1.z / vertexClippedPos_1.w);
                    float vertexScreenPos_2_z =  (vertexClippedPos_2.z / vertexClippedPos_2.w);
                    
                    if(point_in_3Points_triangle(x_click, y_click, vertexScreenPos_0_x, vertexScreenPos_0_y, vertexScreenPos_1_x, vertexScreenPos_1_y, vertexScreenPos_2_x, vertexScreenPos_2_y))
                        {
                            //找出每个三角形中Z最大的顶点
                            int nearest_Z_Point_in_triangle = vertexScreenPos_0_z;
                            if(vertexScreenPos_1_z < nearest_Z_Point_in_triangle)
                                nearest_Z_Point_in_triangle = vertexScreenPos_1_z;
                            if(vertexScreenPos_2_z < nearest_Z_Point_in_triangle)
                                nearest_Z_Point_in_triangle = vertexScreenPos_2_z;
                            
                            //找出所有三角形中Z最大的三角形，并记录下i j值
                            if(nearest_Z_Point_in_triangle < nearest_Z){
                                nearest_Z = nearest_Z_Point_in_triangle;
                                index_i = i;
                                index_j = j;
                            }
                        }
                }
            }
            //找到了屏幕选中的那个方块的index
            cout<<"index_i："<<index_i<<endl;
            //修订选中要旋转的方块
            cubesSelected.x = RANK - floor((cubePositions[index_i].x - offsetToCenter) / 1.1) - 1;
            cubesSelected.y = RANK - floor((cubePositions[index_i].y - offsetToCenter) / 1.1) - 1;
            cubesSelected.z = RANK - floor((cubePositions[index_i].z - offsetToCenter) / 1.1) - 1;
            cout<<cubesSelected.x<<" "<<cubesSelected.y<<" "<<cubesSelected.z<<endl;
            clickChanged = false;
        }
        
        
        //__________________________________________________
        //进入魔方渲染状态机
        glm::mat4 thismodel;
        Eigen::Matrix4f rotation;
        glm::mat4 before;
        switch (state)
        {
            case RESTING:
            {
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat4 modelNextFrame = currentModel;
                    modelNextFrame *= allMats[i];
                    thismodel = glm::translate(modelNextFrame, cubePositions[i]);
                    ourShader.setMat4("model", thismodel);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                //每次进入旋转前，重置angle
                angle = 0;
                if (rotationAxis == X)
                {
                    state = RUN_X;
                    axisVec = glm::vec3(1.f, 0.0f, 0.f);
                }
                else if (rotationAxis == Y)
                {
                    state = RUN_Y;
                    axisVec = glm::vec3(0.f, 1.f, 0.f);
                }
                else if (rotationAxis == Z)
                {
                    state = RUNZ;
                    axisVec = glm::vec3(0.f, 0.f, 1.f);
                }
                break;
            }
            case RUN_X:
            {
                angle += deltaTime * 2.0 * targetAngle; //旋转过程中，每一帧转的角度
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat3 thisMatr = allMats[i];
                    glm::vec3 newcube = thisMatr * cubePositions[i];
                    float& x = newcube.x;
                    
                    glm::mat4 modelNextFrame = currentModel;
                    //要旋转的那一层的坐标
                    float RotatingCubesCoord_x = getRotatingCubesCoord('x', offsetToCenter);
                    //确定每一块是不是再要旋转的那一层
                    if (x > RotatingCubesCoord_x-0.01f && x < RotatingCubesCoord_x+0.01f)
                    {
                        modelNextFrame = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    }
                    modelNextFrame *= allMats[i];
                    thismodel = glm::translate(modelNextFrame, cubePositions[i]);
                    ourShader.setMat4("model", thismodel);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    
                }
                if (abs(angle) > abs(targetAngle)) state = UPDATE;
                break;
            }
            case RUN_Y:
            {
                angle += deltaTime * 2.0 * targetAngle;
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat3 thisMatr = allMats[i];
                    glm::vec3 newcube = thisMatr * cubePositions[i];
                    float& y = newcube.y;
                    
                    glm::mat4 modelNextFrame = currentModel;
                    //要旋转的那一层的坐标
                    float RotatingCubesCoord_y = getRotatingCubesCoord('y', offsetToCenter);
                    //确定每一块是不是再要旋转的那一层
                    if (y > RotatingCubesCoord_y-0.01f && y < RotatingCubesCoord_y+0.01f)
                    {
                        modelNextFrame = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    }
                    modelNextFrame *= allMats[i];
                    thismodel = glm::translate(modelNextFrame, cubePositions[i]);
                    ourShader.setMat4("model", thismodel);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                
                if (abs(angle) > abs(targetAngle)) state = UPDATE;
                break;
            }
            case RUNZ:
            {
                angle += deltaTime * 2.0 * targetAngle;
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat3 thisMatr = allMats[i];
                    glm::vec3 newcube = thisMatr * cubePositions[i];
                    float& z = newcube.z;
                    
                    glm::mat4 modelNextFrame = currentModel;
                    //要旋转的那一层的坐标
                    float RotatingCubesCoord_z = getRotatingCubesCoord('z', offsetToCenter);
                    //确定每一块是不是再要旋转的那一层
                    if (z > RotatingCubesCoord_z-0.01f && z < RotatingCubesCoord_z+0.01f)
                    {
                        modelNextFrame = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    }
                    modelNextFrame *= allMats[i];
                    thismodel = glm::translate(modelNextFrame, cubePositions[i]);
                    ourShader.setMat4("model", thismodel);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                if (abs(angle) > abs(targetAngle)) state = UPDATE;
                break;
            }
            case UPDATE:
            {
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat3 thisMatr = allMats[i];
                    glm::vec3 newcube = thisMatr * cubePositions[i];
                    float& x = newcube.x;
                    float& y = newcube.y;
                    float& z = newcube.z;
                    if (rotationAxis == X)
                    {
                        float RotatingCubesCoord_x = getRotatingCubesCoord('x', offsetToCenter);
                        if (x > RotatingCubesCoord_x-0.01f && x < RotatingCubesCoord_x+0.01f)
                        {
                            //先更新旋转
                            glm::mat4 thistime = glm::rotate(world, glm::radians(targetAngle), glm::vec3(1.f, 0.f, 0.f));
                            allMats[i] = thistime * allMats[i];
                        }
                    }
                    else if (rotationAxis == Y)
                    {
                        float RotatingCubesCoord_y = getRotatingCubesCoord('y', offsetToCenter);
                        if (y > RotatingCubesCoord_y-0.01f && y < RotatingCubesCoord_y+0.01f)
                        {
                            glm::mat4 thistime = glm::rotate(world, glm::radians(targetAngle), glm::vec3(0.f, 1.f, 0.f));
                            allMats[i] = thistime * allMats[i];
                        }
                    }
                    else if (rotationAxis == Z)
                    {
                        float RotatingCubesCoord_z = getRotatingCubesCoord('z', offsetToCenter);
                        if (z > RotatingCubesCoord_z-0.01f && z < RotatingCubesCoord_z+0.01f)
                        {
                            glm::mat4 thistime = glm::rotate(world, glm::radians(targetAngle), glm::vec3(0.f, 0.f, 1.f));
                            allMats[i] = thistime * allMats[i];
                        }
                    }
                }
                axisMutex.unlock();
                //更新完点之后，我再重新画这些点
                for (int i = 0; i < numOfCubes; i++)
                {
                    glm::mat4 modelNextFrame = currentModel;
                    modelNextFrame *= allMats[i];
                    thismodel = glm::translate(modelNextFrame, cubePositions[i]);
                    ourShader.setMat4("model", thismodel);
                    showmodel(thismodel);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                state = RESTING;
                targetAngle = 0;
                rotationAxis = NONE;
                break;
            }
            case OTHER:
                break;
        }
        
        //绘制光源小块，（虽然不一定能看见）
        glm::mat4 model2 = glm::mat4(1.0f);
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, lightPos);
        model2 = glm::scale(model2, glm::vec3(0.2f)); // a smaller cube
        lightingShader.setMat4("model", model2);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    
    
    
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, vector<glm::mat4> &allMats)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    view_deltaAlpha = 0;
    view_deltaBeta = 0;
    view_deltaTheta = 0;
    float cameraSpeed = 200.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaBeta -= cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaBeta += cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaAlpha -= cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaAlpha += cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaTheta -= cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) //魔方整体旋转
    {
        view_deltaTheta += cameraSpeed;
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) //魔方整体旋转
    {
        currentModel = glm::mat4(1.0f);
        state = UPDATE;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)//重置魔方到初始状态
    {
        if(axisMutex.try_lock()){
            for(int i=0; i<allMats.size(); i++)
            {
                allMats[i] = glm::mat4(1.f);
            }
            state = RESTING;
            targetAngle = 0;
            rotationAxis = NONE;
            axisMutex.unlock();
            cout<<"重置成功！！！！！！"<<endl;
        }else cout<<"重置失败"<<endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) //魔方指定层围绕X轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = X;
            targetAngle = 90;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) //魔方指定层围绕Y轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = Y;
            targetAngle = 90;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) //魔方指定层围绕Z轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = Z;
            targetAngle = 90;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) //魔方指定层围绕X轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = X;
            targetAngle = -90;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) //魔方指定层围绕Y轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = Y;
            targetAngle = -90;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) //魔方指定层围绕Z轴旋转
    {
        if(axisMutex.try_lock()){
            rotationAxis = Z;
            targetAngle = -90;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);
        x_click = xpos;
        y_click = ypos;
        clickChanged = true;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

float area_3Points_triangle(float x1, float y1, float x2, float y2, float x3, float y3){
    return abs(x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2))/ 2.0;
}

bool point_in_3Points_triangle (float xpos, float ypos, float x1, float y1, float x2, float y2, float x3, float y3){
        float triangleOriginal = area_3Points_triangle(x1, y1, x2, y2, x3, y3);
        
        float triangle1 = area_3Points_triangle(xpos, ypos, x2, y2, x3, y3);
        float triangle2 = area_3Points_triangle(x1, y1, xpos, ypos, x3, y3);
        float triangle3 = area_3Points_triangle(x1, y1, x2, y2, xpos, ypos);
        float triangleSum = triangle1 + triangle2 + triangle3;
    
        return triangleOriginal * 1.001 > triangleSum;
}

float getRotatingCubesCoord(char axis, float offsetToCenter){
    switch (axis) {
        case 'x':
            return cubesSelected.x * 1.1 + offsetToCenter;
            break;
        case 'y':
            return cubesSelected.y * 1.1 + offsetToCenter;
            break;
        case 'z':
            return cubesSelected.z * 1.1 + offsetToCenter;
            break;
            
        default:
            return 0;
            break;
    }
}
