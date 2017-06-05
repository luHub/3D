#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define TINYGLTF_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf_loader.h"

using namespace tinygltf;

using namespace std;

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

DrawElementsIndirectCommand commands[2];

// From https://blog.nobel-joergensen.com/2013/02/17/debugging-opengl-part-2-using-gldebugmessagecallback/
void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    cout << "[";
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        cout << "NOTIFICATION";
        break;
    }

    cout << "] [";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            cout << "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            cout << "WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            cout << "SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            cout << "THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            cout << "APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            cout << "OTHER";
            break;
    }
    cout << ":";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << "] " << message << endl;
}

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

void specifySceneVertexAttributes(GLuint shaderProgram, GLuint vao, GLuint vbo, GLuint vbo_instances, GLuint ibo)
{
    GLint posAttrib = glGetAttribLocation(shaderProgram, "in_Position");
    GLint instAttrib = glGetAttribLocation(shaderProgram, "in_Instance");

    glEnableVertexArrayAttrib(vao, posAttrib);
    glEnableVertexArrayAttrib(vao, instAttrib);

    // Vertex format
    glVertexArrayAttribFormat(vao, posAttrib, 3, GL_FLOAT, GL_FALSE, 0*sizeof(float));
    glVertexArrayAttribBinding(vao, posAttrib, 0);

    glVertexArrayAttribIFormat(vao, instAttrib, 1, GL_INT, 0*sizeof(int));
    glVertexArrayAttribBinding(vao, instAttrib, 1);
    glVertexArrayBindingDivisor(vao, 1, 1);

    // Bind vertex and index buffers
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 3*sizeof(float));
    glVertexArrayVertexBuffer(vao, 1, vbo_instances, 0, 1*sizeof(int));
    glVertexArrayElementBuffer(vao, ibo);

    commands[0].count = 3;
    commands[0].instanceCount = 1;
    commands[0].firstIndex = 0;
    commands[0].baseVertex = 0;
    commands[0].baseInstance = 0;

    commands[1].count = 3;
    commands[1].instanceCount = 1;
    commands[1].firstIndex = 3;
    commands[1].baseVertex = 3;
    commands[1].baseInstance = 1;
}

static void error_callback(int error, const char* description)
{
    cout << "GLFW: " << description << endl;
}

static void window_close_callback(GLFWwindow* window)
{
    cout << "Closing window" << endl;
}

static void loadMesh(tinygltf::Scene &scene, GLuint progId) {
    cout << "Loading mesh..." << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout << "Usage: ./mdi path/to/mesh.gltf" << endl;
        exit(EXIT_FAILURE);
    }

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

    // Debug messages
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

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

    //////////////////////////////////////////////////////////////////////
    // Debug callback
    //////////////////////////////////////////////////////////////////////

    cout << "Register OpenGL debug callback " << endl;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction, nullptr);
    GLuint unusedIds = 0;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    cout << "errors: " << glGetError() << endl;

    //////////////////////////////////////////////////////////////////////

    const GLchar* shaderVSsrc = loadFile("mdi.vert");
    const GLchar* shaderFSsrc = loadFile("mdi.frag");

    GLuint shaderVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVS, 1, &shaderVSsrc, NULL);
    glCompileShader(shaderVS);
    printShaderLog(shaderVS);

    GLuint shaderFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFS, 1, &shaderFSsrc, NULL);
    glCompileShader(shaderFS);
    printShaderLog(shaderFS);

    // Create program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shaderVS);
    glAttachShader(shaderProgram, shaderFS);
    glBindFragDataLocation(shaderProgram, 0, "out_Color");
    glLinkProgram(shaderProgram);

    // Vertex Array Objects
    GLuint vao;
    glCreateVertexArrays(1, &vao);

    // Vertex Buffer Object
    GLuint vbo, vbo_instances, ibo, cbo;
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &vbo_instances);
    glCreateBuffers(1, &ibo);
    glCreateBuffers(1, &cbo);

    //////////////////////////////////////////////////////////////////////

    Scene scene;
    TinyGLTFLoader loader;
    string err;
    bool ret = loader.LoadASCIIFromFile(&scene, &err, argv[1]);

    if (!err.empty()) {
        cout << "Error: " << err << endl;
    }

    if (!ret) {
        cout << "Error: failed to parse .gltf model" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "# of meshes = " << scene.meshes.size() << endl;
    loadMesh(scene, shaderProgram);

    //////////////////////////////////////////////////////////////////////

    glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(vbo_instances, sizeof(instances), instances, GL_STATIC_DRAW);
    glNamedBufferData(ibo, sizeof(indices), indices, GL_STATIC_DRAW);

    // Specify vertex format and bind VBO and IBO
    specifySceneVertexAttributes(shaderProgram, vao, vbo, vbo_instances, ibo);

    glNamedBufferData(cbo, sizeof(commands), commands, GL_STATIC_DRAW);

    glBindVertexArray(vao);
    glUseProgram(shaderProgram);

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
