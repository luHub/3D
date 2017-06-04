#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <chrono>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "objloader.hpp"

using namespace std;

string absPathTim = "C:/Users/Tim/Documents/Uni/CGPracticals/Project/mdi/";

float vertices[] = {
    -0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
     0.0f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f
};

unsigned int instances[] = {
    2, 3
};

unsigned int indices[] = {
    0, 1, 2, 0, 1, 2
};

typedef  struct {
    GLuint  count;
    GLuint  instanceCount;
    GLuint  firstIndex;
    GLuint  baseVertex;
    GLuint  baseInstance;
} DrawElementsIndirectCommand;

DrawElementsIndirectCommand commands[1];

GLchar* loadFile(const string &fileName)
{
    string* result = new string();
    ifstream file(fileName.c_str());
    if (!file.good()) {
        cout << "File does not exist." << endl;
    }
    if (!file) {
        std::cerr << "Cannot open file " << fileName << endl;
        throw exception();
    }
    string line;
    while (getline(file, line)) {
        *result += line;
        *result += '\n';
    }
    file.close();
    return (GLchar*) result->c_str();
}

void printShaderLog(int shaderId)
{
    int logLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        char *log = new char[logLength];
        glGetShaderInfoLog(shaderId, logLength, &logLength, log);
        cout << string(log);
        delete[] log;
    }
}

void specifySceneVertexAttributes(GLuint shaderProgram, GLuint vao, GLuint vbo, GLuint vbo_instances, GLuint ibo, int indices_count)
{
    GLint posAttrib = glGetAttribLocation(shaderProgram, "in_Position");
    GLint norAttrib = glGetAttribLocation(shaderProgram, "in_Normal");
    GLint uvAttrib = glGetAttribLocation(shaderProgram, "in_UV");
    GLint instAttrib = glGetAttribLocation(shaderProgram, "in_Instance");

    glEnableVertexArrayAttrib(vao, posAttrib);
    glEnableVertexArrayAttrib(vao, norAttrib);
    glEnableVertexArrayAttrib(vao, uvAttrib);
    glEnableVertexArrayAttrib(vao, instAttrib);

    // Vertex format
    glVertexArrayAttribFormat(vao, posAttrib, 3, GL_FLOAT, GL_FALSE, 0*sizeof(float));
    glVertexArrayAttribFormat(vao, norAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float));
    glVertexArrayAttribFormat(vao, uvAttrib, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float));
    glVertexArrayAttribBinding(vao, posAttrib, 0);

    glVertexArrayAttribIFormat(vao, instAttrib, 1, GL_INT, 0*sizeof(int));
    glVertexArrayAttribBinding(vao, instAttrib, 1);
    glVertexArrayBindingDivisor(vao, 1, 1);

    // Bind vertex and index buffers
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 8*sizeof(float));
    glVertexArrayVertexBuffer(vao, 1, vbo_instances, 0, 1*sizeof(int));
    glVertexArrayElementBuffer(vao, ibo);

    commands[0].count = indices_count;
    commands[0].instanceCount = 1;
    commands[0].firstIndex = 0;
    commands[0].baseVertex = 0;
    commands[0].baseInstance = 0;

//    commands[1].count = 3;
//    commands[1].instanceCount = 1;
//    commands[1].firstIndex = 3;
//    commands[1].baseVertex = 3;
//    commands[1].baseInstance = 1;
}

static void error_callback(int error, const char* description)
{
    cout << "GLFW: " << description << endl;
}

static void window_close_callback(GLFWwindow* window)
{
    cout << "Closing window" << endl;
}

int main(void)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    // Initialize the library
    if (!glfwInit())
        exit(EXIT_FAILURE);

    // Require OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    const float windowWidth = 500.0f;
    const float windowHeight = 500.0f;

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(500, 500, "MDI", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowCloseCallback(window, window_close_callback);

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    cout << "ignore this error: " << glGetError() << endl;
    cout << "errors: " << glGetError() << endl;

    const GLchar* shaderVSsrc = loadFile("./mdi.vert");
    const GLchar* shaderFSsrc = loadFile("./mdi.frag");

    GLuint shaderVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVS, 1, &shaderVSsrc, NULL);
    glCompileShader(shaderVS);
    printShaderLog(shaderVS);
    cout << "Created VS: " << glGetError() << endl;

    GLuint shaderFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFS, 1, &shaderFSsrc, NULL);
    glCompileShader(shaderFS);
    printShaderLog(shaderFS);
    cout << "Created FS: " << glGetError() << endl;

    // Create program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shaderVS);
    glAttachShader(shaderProgram, shaderFS);
    glBindFragDataLocation(shaderProgram, 0, "out_Color");
    glLinkProgram(shaderProgram);
    cout << "Created program: " << glGetError() << endl;

    // Vertex Array Objects
    GLuint vao;
    glCreateVertexArrays(1, &vao);

    // Vertex Buffer Object
    GLuint vbo, vbo_instances, ibo, cbo;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &vbo_instances);
    glCreateBuffers(1, &ibo);
    glCreateBuffers(1, &cbo);
    cout << "errors: " << glGetError() << endl;

    std::vector<unsigned short> indicesX;
    std::vector<glm::vec3> verticesX;
    std::vector<glm::vec3> normalsX;
    std::vector<glm::vec2> uvsX;
    loadAssImp("./meshes/fish.obj", indicesX, verticesX, uvsX, normalsX);

    printf("vertices: %d, normals: %d, uvs: %d\n", verticesX.size(), normalsX.size(), uvsX.size());

    float mesh_vertices[3*verticesX.size() + 3*normalsX.size() + 2*uvsX.size()] = {};
    for (int i = 0; i < verticesX.size(); i++) {
        mesh_vertices[8*i+0] = verticesX[i].x;
        mesh_vertices[8*i+1] = verticesX[i].y;
        mesh_vertices[8*i+2] = verticesX[i].z;

        mesh_vertices[8*i+3] = normalsX[i].x;
        mesh_vertices[8*i+4] = normalsX[i].y;
        mesh_vertices[8*i+5] = normalsX[i].z;

        mesh_vertices[8*i+6] = uvsX[i].x;
        mesh_vertices[8*i+7] = uvsX[i].y;
    }

    float mesh_indices[indicesX.size()] = {};
    for (int i = 0; i < indicesX.size(); i++) {
        mesh_indices[i] = indicesX[i];
    }

    unsigned int mesh_instances[] = {
        0
    };

    glNamedBufferData(vbo, sizeof(mesh_vertices), mesh_vertices, GL_STATIC_DRAW);
    glNamedBufferData(vbo_instances, sizeof(mesh_instances), mesh_instances, GL_STATIC_DRAW);
    glNamedBufferData(ibo, sizeof(mesh_indices), mesh_indices, GL_STATIC_DRAW);

    // Specify vertex format and bind VBO and IBO
    specifySceneVertexAttributes(shaderProgram, vao, vbo, vbo_instances, ibo, indicesX.size());

    glNamedBufferData(cbo, sizeof(commands), commands, GL_STATIC_DRAW);
    cout << "Created VAO: " << glGetError() << endl;

    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
    cout << "Running: " << glGetError() << endl;

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cbo);

//    glm::mat4 trans;
//    trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    GLint uniTrans = glGetUniformLocation(shaderProgram, "world");
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

//    glm::vec4 result = trans * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
//    printf("glm test: %f, %f, %f\n", result.x, result.y, result.z);

//    glProgramUniformMatrix4fv(shaderProgram, uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

    auto t_start = std::chrono::high_resolution_clock::now();

    // Projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 1.0f, 10.0f);
    glProgramUniformMatrix4fv(shaderProgram, uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // View matrix
        glm::mat4 view = glm::lookAt(
            glm::vec3(1.2f, 1.2f, 1.2f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glProgramUniformMatrix4fv(shaderProgram, uniView, 1, GL_FALSE, glm::value_ptr(view));

        // World matrix
        glm::mat4 trans;
        trans = glm::rotate(trans, time * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glProgramUniformMatrix4fv(shaderProgram, uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, 2, 0);

        // Swap front and back buffers and poll for and process events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(shaderVS);
    glDeleteShader(shaderFS);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vbo_instances);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &cbo);

    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
