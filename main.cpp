#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "stb_image.h"

using namespace std;

double xpos;
double ypos;
double initialXpos;
double initialYpos;
double finalXpos;
double finalYpos;

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;
const GLfloat CAMERA_MOVEMENT_STEP = 0.4;
const float GRID_UNIT_LENGTH = 1.0f;//0.1f;

GLfloat z_camera_rotation;
GLfloat y_camera_rotation;
GLfloat x_camera_rotation;
glm::vec3 cameraPosition;
glm::vec3 scale;

// Horse transformation variables
float torsoScaleX = 1.5f;
float torsoScaleY = 0.75f;
float torsoScaleZ = 0.75f;

//float torsoLocationX = 0.0f;
//float torsoLocationY = 1.0f;
//float torsoLocationZ = 0.0f;

float neckScaleX = 0.75f;
float neckScaleY = 0.25f;
float neckScaleZ = 0.25f;

float neckLocationX = neckScaleX;
float neckLocationY = neckScaleY;
float neckLocationZ = neckScaleZ;

float headLocationX = 0.5f-torsoScaleX/2-neckScaleX/2;
float headLocationY = -2.0f*0*torsoScaleY/2+0*neckScaleY/2;
float headLocationZ = 0.0f;

float headScaleX = 0.75f;
float headScaleY = 0.5f;
float headScaleZ = 0.5f;

float leftUpperArmScaleX = 0.25f;
float leftUpperArmScaleY = 0.5f;
float leftUpperArmScaleZ = 0.25;

float leftUpperArmLocationX = leftUpperArmScaleX/2 - torsoScaleX/2;
float leftUpperArmLocationY = -torsoScaleY/2-leftUpperArmScaleY/2;
float leftUpperArmLocationZ = torsoScaleZ/2 - leftUpperArmScaleZ/2;

float leftLowerArmScaleX = leftUpperArmScaleX;
float leftLowerArmScaleY = leftUpperArmScaleY;
float leftLowerArmScaleZ = leftUpperArmScaleZ;

float leftLowerArmLocationX = 0.0f;
float leftLowerArmLocationY = -leftUpperArmScaleY;
float leftLowerArmLocationZ = 0.0f;

float rightUpperArmScaleX = leftUpperArmScaleX;
float rightUpperArmScaleY = leftUpperArmScaleY;
float rightUpperArmScaleZ = leftUpperArmScaleZ;

float rightUpperArmLocationX = rightUpperArmScaleX/2 - torsoScaleX/2;
float rightUpperArmLocationY = -torsoScaleY/2-rightUpperArmScaleY/2;
float rightUpperArmLocationZ = -torsoScaleZ/2 + rightUpperArmScaleZ/2;

float rightLowerArmScaleX = rightUpperArmScaleX;
float rightLowerArmScaleY = rightUpperArmScaleY;
float rightLowerArmScaleZ = rightUpperArmScaleZ;

float rightLowerArmLocationX = 0.0f;
float rightLowerArmLocationY = -rightUpperArmScaleY;
float rightLowerArmLocationZ = 0.0f;

float leftUpperLegScaleX = leftUpperArmScaleX;
float leftUpperLegScaleY = leftUpperArmScaleY;
float leftUpperLegScaleZ = leftUpperArmScaleZ;

float leftUpperLegLocationX = -leftUpperArmScaleX/2 + torsoScaleX/2;
float leftUpperLegLocationY = leftUpperArmLocationY;
float leftUpperLegLocationZ = leftUpperArmLocationZ;

float leftLowerLegScaleX = leftUpperLegScaleX;
float leftLowerLegScaleY = leftUpperLegScaleY;
float leftLowerLegScaleZ = leftUpperLegScaleZ;

float leftLowerLegLocationX = 0.0f;
float leftLowerLegLocationY = -leftUpperLegScaleY;
float leftLowerLegLocationZ = 0.0f;

float rightUpperLegScaleX = rightUpperArmScaleX;
float rightUpperLegScaleY = rightUpperArmScaleY;
float rightUpperLegScaleZ = rightUpperArmScaleZ;

float rightUpperLegLocationX = -rightUpperLegScaleX/2 + torsoScaleX/2;
float rightUpperLegLocationY = -torsoScaleY/2-rightUpperArmScaleY/2;
float rightUpperLegLocationZ = -torsoScaleZ/2 + rightUpperArmScaleZ/2;

float rightLowerLegScaleX = rightUpperArmScaleX;
float rightLowerLegScaleY = rightUpperArmScaleY;
float rightLowerLegScaleZ = rightUpperArmScaleZ;

float rightLowerLegLocationX = 0.0f;
float rightLowerLegLocationY = -rightUpperArmScaleY;
float rightLowerLegLocationZ = 0.0f;

struct IndividualTransformations {
    GLfloat rot_x;
    GLfloat rot_y;
    GLfloat rot_z;
    glm::vec3 translation;
    glm::vec3 scale;

    IndividualTransformations* parent;

    public: IndividualTransformations() {
      rot_x = rot_y = rot_z = 0.0f;
      translation = glm::vec3(0.0f);
      scale = glm::vec3(1.0f, 1.0f, 1.0f);
      parent = NULL;
    }
};

IndividualTransformations userInputtedTransformations;
IndividualTransformations torsoPivot;
IndividualTransformations leftUpperArmPivot;

glm::mat4 individualTransformationsToMat4(IndividualTransformations transformations)
{
    //start with an identity matrix
    glm::mat4 result = glm::mat4(1.0f);

    //then finally the translation
    result = glm::translate(result, transformations.translation);

    //then the rotation
    result = glm::rotate(result, transformations.rot_x, glm::vec3(1.0f, 0.0f, 0.0f));
    result = glm::rotate(result, transformations.rot_y, glm::vec3(0.0f, 1.0f, 0.0f));
    result = glm::rotate(result, transformations.rot_z, glm::vec3(0.0f, 0.0f, 1.0f));

    //apply the scale first
    result = glm::scale(result, transformations.scale);

  	if (transformations.parent != NULL)
      result = individualTransformationsToMat4(*(transformations.parent)) * result;

    return result;
}

void DrawCube(GLuint transformLoc, glm::mat4 compoundedTransformations, GLuint cubeVAO) {

    glm::mat4 model_matrix = /*startingPosition * */ compoundedTransformations; // It is assumed that the startingPosition is the origin
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));

	glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 12*3);
}

class Horse {

    public:
        Horse(IndividualTransformations userInputtedTransformations, GLuint transformLoc, glm::mat4 world_mat, GLuint cubeVAO) {
                    //Cube Transformations
            //HORSE

            //Head

            //Torso
            float torsoScaleX = 1.5f;
            float torsoScaleY = 0.75f;
            float torsoScaleZ = 0.75f;

            torsoPivot.translation.y = 1.375f;
            torsoPivot.parent = &userInputtedTransformations;

            IndividualTransformations torsoTransform;
            torsoTransform.scale = glm::vec3(torsoScaleX, torsoScaleY, torsoScaleZ);	//only change scale
            torsoTransform.parent = &torsoPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(torsoTransform), cubeVAO);

            //Neck
            IndividualTransformations neckPivot;
            neckPivot.translation = glm::vec3(-0.25f-torsoScaleX/2, -0.25f+torsoScaleY/2, 0.0f);
            neckPivot.rot_z = glm::radians(-45.0f);
            neckPivot.parent = &torsoPivot;

    //      	float neckScaleX = 0.75f;
    //      	float neckScaleY = 0.25f;
    //      	float neckScaleZ = 0.25f;
    //
    //      	float neckLocationX = neckScaleX;
    //      	float neckLocationY = neckScaleY;
    //      	float neckLocationZ = neckScaleZ;

            IndividualTransformations neckTransform;
            neckTransform.scale = glm::vec3(neckLocationX, neckLocationY, neckLocationZ);
            neckTransform.parent = &neckPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(neckTransform), cubeVAO);

            //Head
    //        float headLocationX = 0.5f-torsoScaleX/2-neckScaleX/2;
    //        float headLocationY = -2.0f*0*torsoScaleY/2+0*neckScaleY/2;
    //        float headLocationZ = 0.0f;

            IndividualTransformations headPivot;
            headPivot.translation = glm::vec3(headLocationX, headLocationY, headLocationZ);
            headPivot.rot_z = glm::radians(240.0f);
            headPivot.parent = &neckPivot;

    //      	float headScaleX = 0.75f;
    //      	float headScaleY = 0.5f;
    //      	float headScaleZ = 0.5f;

            IndividualTransformations headTransform;
            headTransform.scale = glm::vec3(headScaleX, headScaleY, headScaleZ);
            headTransform.parent = &headPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(headTransform), cubeVAO);

            // Left upper arm (from horse's point of view)
    //      	float leftUpperArmScaleX = 0.25f;
    //      	float leftUpperArmScaleY = 0.5f;
    //      	float leftUpperArmScaleZ = 0.25;
    //
    //      	float leftUpperArmLocationX = leftUpperArmScaleX/2 - torsoScaleX/2;
    //      	float leftUpperArmLocationY = -torsoScaleY/2-leftUpperArmScaleY/2;
    //      	float leftUpperArmLocationZ = torsoScaleZ/2 - leftUpperArmScaleZ/2;

    //        IndividualTransformations leftUpperArmPivot;
            leftUpperArmPivot.translation = glm::vec3(leftUpperArmLocationX, leftUpperArmLocationY, leftUpperArmLocationZ);
            leftUpperArmPivot.rot_z = glm::radians(0.0f);
            leftUpperArmPivot.parent = &torsoPivot;

            IndividualTransformations leftUpperArmTransform;
            leftUpperArmTransform.scale = glm::vec3(leftUpperArmScaleX, leftUpperArmScaleY, leftUpperArmScaleZ);
            leftUpperArmTransform.parent = &leftUpperArmPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(leftUpperArmTransform), cubeVAO);

            // Left lower arm (from horse's point of view)
    //        float leftLowerArmScaleX = leftUpperArmScaleX;
    //        float leftLowerArmScaleY = leftUpperArmScaleY;
    //        float leftLowerArmScaleZ = leftUpperArmScaleZ;
    //
    //        float leftLowerArmLocationX = 0.0f;
    //        float leftLowerArmLocationY = -leftUpperArmScaleY;
    //        float leftLowerArmLocationZ = 0.0f;

            IndividualTransformations leftLowerArmPivot;
            leftLowerArmPivot.translation = glm::vec3(leftLowerArmLocationX, leftLowerArmLocationY, leftLowerArmLocationZ);
            leftLowerArmPivot.rot_z = glm::radians(0.0f);
            leftLowerArmPivot.parent = &leftUpperArmPivot;

            IndividualTransformations leftLowerArmTransform;
            leftLowerArmTransform.scale = glm::vec3(leftLowerArmScaleX, leftLowerArmScaleY, leftLowerArmScaleZ);
            leftLowerArmTransform.parent = &leftLowerArmPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(leftLowerArmTransform), cubeVAO);

            // Right upper arm (from the horse's point of view)
    //      	float rightUpperArmScaleX = leftUpperArmScaleX;
    //      	float rightUpperArmScaleY = leftUpperArmScaleY;
    //      	float rightUpperArmScaleZ = leftUpperArmScaleZ;
    //
    //      	float rightUpperArmLocationX = rightUpperArmScaleX/2 - torsoScaleX/2;
    //      	float rightUpperArmLocationY = -torsoScaleY/2-rightUpperArmScaleY/2;
    //      	float rightUpperArmLocationZ = -torsoScaleZ/2 + rightUpperArmScaleZ/2;

            IndividualTransformations rightUpperArmPivot;
            rightUpperArmPivot.translation = glm::vec3(rightUpperArmLocationX, rightUpperArmLocationY, rightUpperArmLocationZ);
            rightUpperArmPivot.rot_z = glm::radians(0.0f);
            rightUpperArmPivot.parent = &torsoPivot;

            IndividualTransformations rightUpperArmTransform;
            rightUpperArmTransform.scale = glm::vec3(rightUpperArmScaleX, rightUpperArmScaleY, rightUpperArmScaleZ);
            rightUpperArmTransform.parent = &rightUpperArmPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(rightUpperArmTransform), cubeVAO);

            // Right lower arm (from the horse's point of view)
    //        float rightLowerArmScaleX = rightUpperArmScaleX;
    //        float rightLowerArmScaleY = rightUpperArmScaleY;
    //        float rightLowerArmScaleZ = rightUpperArmScaleZ;
    //
    //        float rightLowerArmLocationX = 0.0f;
    //        float rightLowerArmLocationY = -rightUpperArmScaleY;
    //        float rightLowerArmLocationZ = 0.0f;

            IndividualTransformations rightLowerArmPivot;
            rightLowerArmPivot.translation = glm::vec3(rightLowerArmLocationX, rightLowerArmLocationY, rightLowerArmLocationZ);
            rightLowerArmPivot.rot_z = glm::radians(0.0f);
            rightLowerArmPivot.parent = &rightUpperArmPivot;

            IndividualTransformations rightLowerArmTransform;
            rightLowerArmTransform.scale = glm::vec3(rightLowerArmScaleX, rightLowerArmScaleY, rightLowerArmScaleZ);
            rightLowerArmTransform.parent = &rightLowerArmPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(rightLowerArmTransform), cubeVAO);

            // Left upper leg (from the horse's point of view)
    //        float leftUpperLegScaleX = leftUpperArmScaleX;
    //      	float leftUpperLegScaleY = leftUpperArmScaleY;
    //      	float leftUpperLegScaleZ = leftUpperArmScaleZ;
    //
    //      	float leftUpperLegLocationX = -leftUpperArmScaleX/2 + torsoScaleX/2;
    //      	float leftUpperLegLocationY = leftUpperArmLocationY;
    //      	float leftUpperLegLocationZ = leftUpperArmLocationZ;

            IndividualTransformations leftUpperLegPivot;
            leftUpperLegPivot.translation = glm::vec3(leftUpperLegLocationX, leftUpperLegLocationY, leftUpperLegLocationZ);
            leftUpperLegPivot.rot_z = glm::radians(0.0f);
            leftUpperLegPivot.parent = &torsoPivot;

            IndividualTransformations leftUpperLegTransform;
            leftUpperLegTransform.scale = glm::vec3(leftUpperLegScaleX, leftUpperLegScaleY, leftUpperLegScaleZ);
            leftUpperLegTransform.parent = &leftUpperLegPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(leftUpperLegTransform), cubeVAO);

            // Left lower leg (from the horse's point of view)
    //        float leftLowerLegScaleX = leftUpperLegScaleX;
    //      	float leftLowerLegScaleY = leftUpperLegScaleY;
    //      	float leftLowerLegScaleZ = leftUpperLegScaleZ;
    //
    //      	float leftLowerLegLocationX = 0.0f;
    //      	float leftLowerLegLocationY = -leftUpperLegScaleY;
    //      	float leftLowerLegLocationZ = 0.0f;

            IndividualTransformations leftLowerLegPivot;
            leftLowerLegPivot.translation = glm::vec3(leftLowerLegLocationX, leftLowerLegLocationY, leftLowerLegLocationZ);
            leftLowerLegPivot.rot_z = glm::radians(0.0f);
            leftLowerLegPivot.parent = &leftUpperLegPivot;

            IndividualTransformations leftLowerLegTransform;
            leftLowerLegTransform.scale = glm::vec3(leftLowerLegScaleX, leftLowerLegScaleY, leftLowerLegScaleZ);
            leftLowerLegTransform.parent = &leftLowerLegPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(leftLowerLegTransform), cubeVAO);

            // Right upper leg (from the horse's point of view)
    //        float rightUpperLegScaleX = rightUpperArmScaleX;
    //      	float rightUpperLegScaleY = rightUpperArmScaleY;
    //      	float rightUpperLegScaleZ = rightUpperArmScaleZ;
    //
    //      	float rightUpperLegLocationX = -rightUpperLegScaleX/2 + torsoScaleX/2;
    //      	float rightUpperLegLocationY = -torsoScaleY/2-rightUpperArmScaleY/2;
    //      	float rightUpperLegLocationZ = -torsoScaleZ/2 + rightUpperArmScaleZ/2;

            IndividualTransformations rightUpperLegPivot;
            rightUpperLegPivot.translation = glm::vec3(rightUpperLegLocationX, rightUpperLegLocationY, rightUpperLegLocationZ);
            rightUpperLegPivot.rot_z = glm::radians(0.0f);
            rightUpperLegPivot.parent = &torsoPivot;

            IndividualTransformations rightUpperLegTransform;
            rightUpperLegTransform.scale = glm::vec3(rightUpperLegScaleX, rightUpperLegScaleY, rightUpperLegScaleZ);
            rightUpperLegTransform.parent = &rightUpperLegPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(rightUpperLegTransform), cubeVAO);

            // Right lower leg (from the horse's point of view)
    //        float rightLowerLegScaleX = rightUpperArmScaleX;
    //      	float rightLowerLegScaleY = rightUpperArmScaleY;
    //      	float rightLowerLegScaleZ = rightUpperArmScaleZ;
    //
    //      	float rightLowerLegLocationX = 0.0f;
    //      	float rightLowerLegLocationY = -rightUpperArmScaleY;
    //      	float rightLowerLegLocationZ = 0.0f;

            IndividualTransformations rightLowerLegPivot;
            rightLowerLegPivot.translation = glm::vec3(rightLowerLegLocationX, rightLowerLegLocationY, rightLowerLegLocationZ);
            rightLowerLegPivot.rot_z = glm::radians(0.0f);
            rightLowerLegPivot.parent = &rightUpperLegPivot;

            IndividualTransformations rightLowerLegTransform;
            rightLowerLegTransform.scale = glm::vec3(rightLowerLegScaleX, rightLowerLegScaleY, rightLowerLegScaleZ);
            rightLowerLegTransform.parent = &rightLowerLegPivot;	//pivot is the parent
            DrawCube(transformLoc, world_mat * individualTransformationsToMat4(rightLowerLegTransform), cubeVAO);
        }
};

unsigned char* getTextureImageData(std::string fileName) {
    int width, height, numComponents;

    unsigned char* imageData = stbi_load(fileName.c_str(), &width, &height, &numComponents, 4);

    return imageData;
}

/*glm::mat4 netTransformation(IndividualTransformations transformations) {

//  glm::mediump_float x_rot = transformations.rotation.x;
//  glm::mediump_float y_rot = transformations.rotation.y;
//  glm::mediump_float z_rot = transformations.rotation.z;

  //glm::mat4 complete_rotation(1.0f);
  glm::mat4 rx = glm::rotate(glm::mat4(1.0f), transformations.rot_x, glm::vec3(1.0f, 0.0f, 0.0f));	//rotate around x axis
  glm::mat4 ry = glm::rotate(glm::mat4(1.0f), transformations.rot_y, glm::vec3(0.0f, 1.0f, 0.0f));	//rotate around y axis
  glm::mat4 rz = glm::rotate(glm::mat4(1.0f), transformations.rot_z, glm::vec3(0.0f, 1.0f, 0.0f));	//rotate around z axis

  glm::mat4 complete_rotation = rz * ry * rx;

  return glm::translate(glm::mat4(1.0f),transformations.translation) * complete_rotation * glm::scale(glm::mat4(1.0f), transformations.scale);//glm::rotate(glm::mat4(1.0f), transformations.rotation)
  //return individualTransformations.rotation * individualTransformations.translation * individualTransformations.scale;
}*/

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    // pan = yaw = rotate about y axis
    if(GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

        glfwGetCursorPos(window, &xpos, &ypos);

        initialXpos = xpos;
        initialYpos = ypos;

        std::cout << "outer xpos: " << xpos << std::endl;
        std::cout << "outer ypos: " << ypos << std::endl;
    }
    if(GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {

        glfwGetCursorPos(window, &xpos, &ypos);
        //cameraPosition.x += CAMERA_MOVEMENT_STEP; // NEED THIS (SOMEWHERE ELSE), I THINK

        finalXpos = xpos;
        finalYpos = ypos;

        if(finalXpos-initialXpos > 0) {
            //cameraPosition.x += CAMERA_MOVEMENT_STEP;

            y_camera_rotation += 10.0f;
        }
        else if(finalXpos-initialXpos < 0) {
            //cameraPosition.x -= CAMERA_MOVEMENT_STEP;
            y_camera_rotation -= 10.0f;
        }

        std::cout << "inner xpos: " << xpos << std::endl;
        std::cout << "inner ypos: " << ypos << std::endl;
    }

    // tilt = pitch = rotate about z axis
    if(GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {

        glfwGetCursorPos(window, &xpos, &ypos);

        initialXpos = xpos;
        initialYpos = ypos;

        std::cout << "outer xpos: " << xpos << std::endl;
        std::cout << "outer ypos: " << ypos << std::endl;
    }
    if(GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {

        glfwGetCursorPos(window, &xpos, &ypos);
        //cameraPosition.x += CAMERA_MOVEMENT_STEP; // NEED THIS (SOMEWHERE ELSE), I THINK

        finalXpos = xpos;
        finalYpos = ypos;

        if(finalYpos-initialYpos > 0) {
            //cameraPosition.y += CAMERA_MOVEMENT_STEP;
            x_camera_rotation += 10.0f;
        }
        else if(finalYpos-initialYpos < 0) {
            //cameraPosition.y -= CAMERA_MOVEMENT_STEP;
            x_camera_rotation -= 10.0f;
        }

        std::cout << "inner xpos: " << xpos << std::endl;
        std::cout << "inner ypos: " << ypos << std::endl;
    }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {

        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {

        cameraPosition.z += CAMERA_MOVEMENT_STEP;
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS) {

        cameraPosition.z -= CAMERA_MOVEMENT_STEP;
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS) {

        cameraPosition.x -= CAMERA_MOVEMENT_STEP;
    }
    else if(key == GLFW_KEY_A && action == GLFW_PRESS) {

        userInputtedTransformations.translation.x -= GRID_UNIT_LENGTH;
    }
    else if(key == GLFW_KEY_W && action == GLFW_PRESS) {

        //cameraPosition.y -= CAMERA_MOVEMENT_STEP; // NEED THIS (SOMEWHERE ELSE), I THINK
        userInputtedTransformations.rot_z -= glm::radians(5.0f);
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {

        //cameraPosition.y += CAMERA_MOVEMENT_STEP; // NEED THIS (SOMEWHERE ELSE), I THINK
        userInputtedTransformations.rot_z += glm::radians(5.0f);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {

        y_camera_rotation += 10.0f;
    }
    else if (key == GLFW_KEY_T && action == GLFW_PRESS) {

        y_camera_rotation -= 10.0f;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS) {

        x_camera_rotation += 10.0f;
    }
    else if (key == GLFW_KEY_G && action == GLFW_PRESS) {

        x_camera_rotation -= 10.0f;
    }
    else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (key == GLFW_KEY_X && action == GLFW_PRESS) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {

        //userInputtedTransformations.translation.x -= 0.1f;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {

        userInputtedTransformations.translation.x += GRID_UNIT_LENGTH;
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {

        userInputtedTransformations.translation.z -= GRID_UNIT_LENGTH;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {

        userInputtedTransformations.translation.z += GRID_UNIT_LENGTH;
    }
    else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS) {

        int randomX = rand() % 101 - 50;
        int randomZ = rand() % 101 - 50;

        userInputtedTransformations.translation.x = randomX;
        userInputtedTransformations.translation.z = randomZ;
    }
    else if(key == GLFW_KEY_R && action == GLFW_PRESS) {

        // TODO complete a run or walk cycle
    }
    else if(key == GLFW_KEY_U && action == GLFW_PRESS) {

        float commonScalingFactor = 1.05;

        userInputtedTransformations.scale *= commonScalingFactor;
    }
    else if(key == GLFW_KEY_J && action == GLFW_PRESS) {

        float commonScalingFactor = 1.05;

        userInputtedTransformations.scale /= commonScalingFactor;
    }
}

int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Horse Program", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }


    int screenWidth, screenHeight;
    glfwGetFramebufferSize( window, &screenWidth, &screenHeight );

    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;

    // Initialize GLEW to setup the OpenGL Function pointers
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);


    // Build and compile our shader program

    // Vertex shader

    // Read the Vertex Shader code from the file
    string vertex_shader_path = "vertex_shader.glsl";
    string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_shader_path, ios::in);

    if (VertexShaderStream.is_open()) {

        string Line = "";

        while (getline(VertexShaderStream, Line)) {
            VertexShaderCode += "\n" + Line;
        }

        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory ?\n", vertex_shader_path.c_str());
        getchar();
        exit(-1);
    }

    // Read the Fragment Shader code from the file
    string fragment_shader_path = "fragment_shader.glsl";
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);

    if (FragmentShaderStream.is_open()) {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
        FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory?\n", fragment_shader_path.c_str());
        getchar();
        exit(-1);
    }
    //Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(vertexShader, 1, &VertexSourcePointer, NULL);
    glCompileShader(vertexShader);
    // Check for compile time errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
    glCompileShader(fragmentShader);
    // Check for compile time errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Link shaders
    GLuint shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);
    // Check for linking errors
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgramID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader); //free up memory
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgramID);

    //----------------------------------------------------------------------------------------------------------


    vector <glm::vec3> gridVert;


    //Grid
    // Inner part of grid
    for(float i = -50*GRID_UNIT_LENGTH; i < 50*GRID_UNIT_LENGTH; i+=GRID_UNIT_LENGTH) {

        gridVert.push_back( glm::vec3(i,0.0,+50) );
        gridVert.push_back( glm::vec3(i,0.0,-50) );

        gridVert.push_back( glm::vec3(-50,0.0,i) );
        gridVert.push_back( glm::vec3(+50,0.0,i) );
    }

    // Outer boundary of grid
    gridVert.push_back( glm::vec3(-50,0.0,+50) );
    gridVert.push_back( glm::vec3(+50,0.0,+50) );

    gridVert.push_back( glm::vec3(-50,0.0,-50) );
    gridVert.push_back( glm::vec3(+50,0.0,-50) );

    gridVert.push_back( glm::vec3(-50,0.0,-50) );
    gridVert.push_back( glm::vec3(-50,0.0,+50) );

    gridVert.push_back( glm::vec3(+50,0.0,-50) );
    gridVert.push_back( glm::vec3(+50,0.0,+50) );

    //Cube
    vector<glm::vec3> vertCube =
    {
        // Front face triangle 1
        glm::vec3(0.5,-0.5,0.5), // position
        glm::vec3(0.05, 0.74, 0.5), // Colour (Teal)
        glm::vec3(0,0,1), // normal vector
        glm::vec3(-0.5,0.5,0.5), // position
        glm::vec3(0.05, 0.74, 0.5), // Colour (Teal)
        glm::vec3(0,0,1), // normal vector
        glm::vec3(-0.5,-0.5,0.5), // position
        glm::vec3(0.05, 0.74, 0.5), // Colour (Teal)
        glm::vec3(0,0,1), // normal vector

        // Front face triangle 2
        glm::vec3(0.5,-0.5,0.5), // position
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,1), // normal vector
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,1), // normal vector
        glm::vec3(-0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,1), // normal vector

        // Back face triangle 1
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector
        glm::vec3(-0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector

        // Back face triangle 2
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector
        glm::vec3(0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,0,-1), // normal vector

        // Left face triangle 1
        glm::vec3(-0.5,-0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector
        glm::vec3(-0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector

        // Left face triangle 2
        glm::vec3(-0.5,-0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector
        glm::vec3(-0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(-1,0,0), // normal vector

        // Right face triangle 1
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector
        glm::vec3(0.5,-0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector

        // Right face triangle 2
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector
        glm::vec3(0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(1,0,0), // normal vector

        // Top face triangle 1
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector
        glm::vec3(-0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector

        // Top face triangle 2
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector
        glm::vec3(0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,1,0), // normal vector

        // Bottom face triangle 1
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector
        glm::vec3(-0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector
        glm::vec3(-0.5,-0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector

        // Bottom face triangle 2
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector
        glm::vec3(-0.5,-0.5,-0.5),
        glm::vec3(0.05, 0.74, 0.5), // Teal
        glm::vec3(0,-1,0), // normal vector
    };

    //Coordinate Axes

    vector <glm::vec3> axesVert;
    //x-axis
    axesVert.push_back(glm::vec3(0,0.0,0));
    axesVert.push_back(glm::vec3(1.0,0.0,0.0)); //Red
    axesVert.push_back(glm::vec3(5,0.0,0));
    axesVert.push_back(glm::vec3(1.0, 0.0, 0.0)); //Red
    //y-axis
    axesVert.push_back(glm::vec3(0,0.0,0));
    axesVert.push_back(glm::vec3(0.0, 1.0, 0.0)); //Green
    axesVert.push_back(glm::vec3(0,0.0,0));
    axesVert.push_back(glm::vec3(0.0, 1.0, 0.0)); //Green
    //z-axis
    axesVert.push_back(glm::vec3(0,0.0,0));
    axesVert.push_back(glm::vec3(0.0, 0.0, 1.0)); //Blue
    axesVert.push_back(glm::vec3(0, 0.0, 5));
    axesVert.push_back(glm::vec3(0.0, 0.0, 1.0)); //Blue






    //------ MODEL MATRIX ---------
    //glm::mat4 mm;
    //glm::mat4 scale;
    //glm::mat4 translate;
    //glm::mat4 rotate;

    //------ VIEW MATRIX ---------

    //view_matrix = glm::lookAt(c_pos, c_pos + c_dir, c_up);

    //------ PROJECTION MATRIX -------
    //glm::mat4 pm = glm::perspective(45.f, 800.f / 600.f, 0.1f, 100.f);

    //GLuint mm_addr = glGetUniformLocation(shaderProgram, "model_matrix");
    //GLuint vm_addr = glGetUniformLocation(shaderProgram, "view_matrix");
    //GLuint pm_addr = glGetUniformLocation(shaderProgram, "projection_matrix");
    //glUniformMatrix4fv(mm_addr, 1, false, glm::value_ptr(mm));
    //glUniformMatrix4fv(pm_addr, 1, false, glm::value_ptr(pm));



    GLuint cubeVAO;
    GLuint gridVAO;
    GLuint cubeVBO;
    GLuint gridVBO;
    GLuint axeVAO;
    GLuint axeVBO;
    GLuint colorVBO;


    //Cube
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);


    glBufferData(GL_ARRAY_BUFFER, vertCube.size() * sizeof(glm::vec3), &vertCube.front(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(glm::vec3), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(glm::vec3), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3*sizeof(glm::vec3), (GLvoid*)(6*sizeof(GLfloat)));


    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(glm::uvec3), &ind.front(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

    //Grid
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, gridVert.size() * sizeof(glm::vec3), &gridVert.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);


    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

    //Axes
    glGenVertexArrays(1, &axeVAO);
    glGenBuffers(1, &axeVBO);

    glBindVertexArray(axeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axeVBO);


    glBufferData(GL_ARRAY_BUFFER, axesVert.size() * sizeof(glm::vec3), &axesVert.front(), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(axeColor) * sizeof(GLfloat), &axeColor[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(axesVert), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(axesVert), (void*)12);


    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

    //Color
    /*glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axeColor), &axeColor[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 0, (void*)0); */

    scale = glm::vec3(1.0f);

    GLuint projectionLoc = glGetUniformLocation(shaderProgramID, "projection_matrix");
    GLuint viewMatrixLoc = glGetUniformLocation(shaderProgramID, "view_matrix");
    GLuint transformLoc = glGetUniformLocation(shaderProgramID, "model_matrix");
    GLuint cameraPositionLoc = glGetUniformLocation(shaderProgramID, "cameraPosition");

    //double xpos, ypos;
    //glfwGetCursorPos(window, &xpos, &ypos);

    glm::mat4 scal;
    glm::mat4 translat;
    glm::mat4 rotate;


    // Game loop


    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer

        glClearColor(0.3f, 0.2f, 0.1f, 1.0f); // Light brown
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);



        glm::mat4 view_matrix;
        view_matrix = glm::translate(view_matrix, glm::vec3(0.0f, 0.0f, -3.0f));
        view_matrix = translate(view_matrix, cameraPosition);
        //view_matrix = glm::lookAt(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model_matrix;
        model_matrix = glm::scale(model_matrix, glm::vec3(1.0f));
        model_matrix = glm::rotate(model_matrix, glm::radians(z_camera_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, glm::radians(y_camera_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, glm::radians(x_camera_rotation), glm::vec3(1.0f, 0.0f, 0.0f));
        //model_matrix = glm::rotate(model_matrix, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);

        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glUniform1fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));


        //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        /*glBindVertexArray(cubeVAO);

        glDrawArrays(GL_TRIANGLES, 0, 12*3);*/

        glBindVertexArray(gridVAO);

        glDrawArrays(GL_LINES, 0, 2*2*100+2*4);

        glBindVertexArray(axeVAO);

        glDrawArrays(GL_LINE_STRIP, 0, 6);

      	//bit of a rename so it makes more sense
      	glm::mat4 world_mat = model_matrix;

// PUT_BACK?:
        Horse h1(userInputtedTransformations, transformLoc, world_mat, cubeVAO);

//      	std::cout << torsoPivot.translation.y << std::endl;
//        std::cout << leftUpperArmPivot.translation.y << std::endl;
//        std::cout << leftLowerArmPivot.translation.y << std::endl;

        glfwSetMouseButtonCallback(window, mouse_button_callback); // YOYO

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.

    glfwTerminate();
    return 0;
}
