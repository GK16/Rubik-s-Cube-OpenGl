//
//  processing.cpp
//  Pro_3
//
//  Created by Asher Huang on 2020/11/30.
//  Copyright © 2020 Asher Huang. All rights reserved.
//
#pragma execution_character_set("utf-8")
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <iostream>
#include <string>
#include <vector>
#include <eigen3/Eigen/Eigen>
#include <mutex>
#include <stdio.h>

using namespace std;
using namespace Eigen;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void curse_poscallback(GLFWwindow *window, double x, double y){
//    cout<<x<<" - "<<y<<endl;
}

void Eigen3Eigen4(Eigen::Matrix3f m3, Eigen::Matrix4f& m4)
{
    m4 << m3(0), m3(1), m3(2), 0,
    m3(3), m3(4), m3(5), 0,
    m3(6), m3(7), m3(8), 0,
    0, 0, 0, 1;
}

void Eigen4Eigen3(Eigen::Matrix4f m4, Eigen::Matrix3f& m3)
{
    m3 << m4(0), m4(1), m4(2),
    m4(4), m4(5), m4(6),
    m4(8), m4(9), m4(10);
}

void Eigen2glm(Eigen::Matrix4f e, glm::mat4& g)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            glm::value_ptr(g)[i * 4 + j] = e(i, j);
        }
    }
}

void show(glm::mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
//            cout << glm::value_ptr(m)[i * 4 + j] << ' ';
        }
//        cout << endl;
    }
}
void showmodel(glm::mat4 m)
{
//    std::cout << glm::value_ptr(m)[12] << ' ' << glm::value_ptr(m)[13] << ' ' << glm::value_ptr(m)[14] << endl;
}


