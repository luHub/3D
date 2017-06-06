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

#include "gltf.h"

using namespace tinygltf;

using namespace std;

/*float vertices[] = {
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
};*/

typedef struct {
    GLuint  count;
    GLuint  instanceCount;
    GLuint  firstIndex;
    GLuint  baseVertex;
    GLuint  baseInstance;
} DrawElementsIndirectCommand;

//DrawElementsIndirectCommand commands[2];

// vbo contains the vertex attributes
// ibo contains the indices
// cbo contains an array of draw commands (DrawElementsIndirectCommand)
// dbo contains an array of draw ID's (increasing numbers 0, 1, 2, etc. These are used
// to index into a TBO for the matrix transforms and material data)
typedef struct { GLuint vbo, ibo, cbo, dbo; GLsizei count; } GLMDIBuffers;

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

void specifySceneVertexAttributes(GLuint shaderProgram, GLuint vao)
{
    GLint posAttrib = glGetAttribLocation(shaderProgram, "in_Position");
    GLint nomAttrib = glGetAttribLocation(shaderProgram, "in_Normal");
    GLint texAttrib = glGetAttribLocation(shaderProgram, "in_UV");

    GLint instAttrib = glGetAttribLocation(shaderProgram, "in_InstanceID");

    cout << "in_Position: " << posAttrib << endl;
    cout << "in_Normal: " << nomAttrib << endl;
    cout << "in_UV: " << texAttrib << endl;
    cout << "in_InstanceID: " << instAttrib << endl;

    glEnableVertexArrayAttrib(vao, posAttrib);
    glEnableVertexArrayAttrib(vao, nomAttrib);
    glEnableVertexArrayAttrib(vao, texAttrib);

    glEnableVertexArrayAttrib(vao, instAttrib);

    // Vertex format
    glVertexArrayAttribFormat(vao, posAttrib, 3, GL_FLOAT, GL_FALSE, 0*sizeof(float));
    glVertexArrayAttribFormat(vao, nomAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float));
    glVertexArrayAttribFormat(vao, texAttrib, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float));
    glVertexArrayAttribBinding(vao, posAttrib, 0);
    glVertexArrayAttribBinding(vao, nomAttrib, 0);
    glVertexArrayAttribBinding(vao, texAttrib, 0);

    glVertexArrayAttribIFormat(vao, instAttrib, 1, GL_INT, 0*sizeof(int));
    glVertexArrayAttribBinding(vao, instAttrib, 1);
    glVertexArrayBindingDivisor(vao, 1, 1);
}

static void loadMesh(tinygltf::Scene &scene, GLuint progId, GLMDIBuffers &mdiBuffers) {
    cout << "Loading mesh..." << endl;

    vector<DrawElementsIndirectCommand> commands;
    vector<unsigned int> instances;
    map<string, int> viewsCommands; // maps bufferViews to index in the commands vector
    map<string, int> viewsIndices;

    map<string, tinygltf::BufferView>::const_iterator it(scene.bufferViews.begin());
    map<string, tinygltf::BufferView>::const_iterator itEnd(scene.bufferViews.end());

    int vboSize = 0;
    int iboSize = 0;

    int vboCount = 0;
    int iboCount = 0;

    // vec3 + vec3 + vec2 = 32 bytes
    const int totalAttributesByteSize = 4 * 3 + 4 * 3 + 4 * 2;
    const int elementsPerVertex = 3 + 3 + 2;

    // Assuming indices are always unsigned ints
    const int bytesPerIndex = 4;

    for (; it != itEnd; it++) {
        const tinygltf::BufferView &bufferView = it->second;
        if (bufferView.target == 0) {
            cout << "Warning: bufferView.target is zero" << endl;
            continue;
        }

        const tinygltf::Buffer &buffer = scene.buffers[bufferView.buffer];
        if (bufferView.target == ELEMENT_ARRAY_BUFFER) {
            iboSize += bufferView.byteLength;
            iboCount++;
        }
        else if (bufferView.target == ARRAY_BUFFER) {
            vboSize += bufferView.byteLength;
            vboCount++;
        }
    }

    cout << "bytes vbo: " << vboSize << endl;
    cout << "bytes ibo: " << iboSize << endl;

    if (vboCount != iboCount) {
        cout << "Error: expected equal number of vertex and index bufferViews" << endl;
    }

    mdiBuffers.count = vboCount;

    glCreateBuffers(1, &mdiBuffers.vbo);
    glCreateBuffers(1, &mdiBuffers.ibo);
    glCreateBuffers(1, &mdiBuffers.cbo);
    glCreateBuffers(1, &mdiBuffers.dbo);

    // Create 1 big buffer for all the vertex data (attributes like position, normals, UV)
    // and 1 buffer for all the indices, and 1 buffer for the draw commands
    // Note: assuming here that vertex attributes are interleaved
    glNamedBufferStorage(mdiBuffers.vbo, vboSize, NULL, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(mdiBuffers.ibo, iboSize, NULL, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(mdiBuffers.cbo, vboCount * sizeof(DrawElementsIndirectCommand), NULL, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(mdiBuffers.dbo, vboCount * sizeof(unsigned int), NULL, GL_DYNAMIC_STORAGE_BIT);

    map<string, tinygltf::BufferView>::const_iterator it2(scene.bufferViews.begin());
    map<string, tinygltf::BufferView>::const_iterator it2End(scene.bufferViews.end());

    int vboOffset = 0;
    int iboOffset = 0;

    vboCount = 0;
    iboCount = 0;

    // Copy data from buffer views to the VBO or IBO
    // Note: assuming here that vertex attributes are interleaved
    // (which means that for each mesh part, all its attributes fit in a single bufferView)
    for (; it2 != it2End; it2++) {
        const string &name = it2->first;
        const tinygltf::BufferView &bufferView = it2->second;

        const tinygltf::Buffer &buffer = scene.buffers[bufferView.buffer];
        if (bufferView.target == ELEMENT_ARRAY_BUFFER) {
            // Copy data to the next free location in the IBO
            glNamedBufferSubData (mdiBuffers.ibo, iboOffset, bufferView.byteLength,
                &buffer.data.at(0) + bufferView.byteOffset);

            // We cannot assume bufferViews for vertex attributes and indices
            // are adjacent (in practice they are). So lets just store
            // the current iboOffset / bytesPerIndex and then look up the correct command later
            viewsIndices.insert(pair<string,int>(name, iboOffset / bytesPerIndex));

            iboOffset += bufferView.byteLength;
            iboCount++;
        }
        else if (bufferView.target == ARRAY_BUFFER) {
            // Copy data to the next free location in the VBO 
            glNamedBufferSubData (mdiBuffers.vbo, vboOffset, bufferView.byteLength,
                &buffer.data.at(0) + bufferView.byteOffset);

            DrawElementsIndirectCommand command;

            // Fill in count and firstIndex later
            command.count = 0;
            command.instanceCount = 1;
            command.firstIndex = 0;
            command.baseVertex = vboOffset / totalAttributesByteSize * elementsPerVertex;
            command.baseInstance = vboCount;

            commands.push_back(command);
            cout << "Insert command for " << name << " at " << commands.size() - 1 << endl;
            viewsCommands.insert(pair<string,int>(name, commands.size() - 1));

            instances.push_back(vboCount);

            vboOffset += bufferView.byteLength;
            vboCount++;
        }
    }

    std::map<std::string, tinygltf::Mesh>::const_iterator itMesh(scene.meshes.begin());
    std::map<std::string, tinygltf::Mesh>::const_iterator itMeshEnd(scene.meshes.end());

    for (; itMesh != itMeshEnd; itMesh++) {
        const tinygltf::Mesh &mesh = itMesh->second;
        cout << mesh.name << " has " << mesh.primitives.size() << " primitives" << endl;

        if (mesh.primitives.size() != 1) {
            // We expect 1 primitive for simplicity
            // If a mesh has multiple primitives, then you need to disable
            // the non-active UV's in Blender
            cout << "Error: expected 1 primitive for mesh " << mesh.name << endl;
        }

        for (size_t primId = 0; primId < mesh.primitives.size(); primId++) {
            const tinygltf::Primitive &primitive = mesh.primitives[primId];

            // Verify primitive has indices
            if (primitive.indices.empty()) {
                cout << "Error: expected primitive to have indices for mesh " << mesh.name << endl;
            }

            // Verify mode is 4 (triangles)
            if (primitive.mode != 4) {
                cout << "Error: expected primitive with mode 4 for mesh " << mesh.name << endl;
            }

            // Verify attributes are interleaved by checking all accessors
            // point to the same bufferView
            const tinygltf::Accessor &posAccessor =
                scene.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::Accessor &nomAccessor =
                scene.accessors[primitive.attributes.find("NORMAL")->second];
            const tinygltf::Accessor &texAccessor =
                scene.accessors[primitive.attributes.find("TEXCOORD_0")->second];

            if (nomAccessor.bufferView != posAccessor.bufferView
                || texAccessor.bufferView != posAccessor.bufferView) {
                cout << "Error: vertex attributes not interleaved for mesh " << mesh.name << endl;
            }

            // Verify that POSITION attribute is vec3
            if (posAccessor.componentType != 5126 || posAccessor.type != TINYGLTF_TYPE_VEC3) {
                cout << "Expected POSITION to be vec3" << mesh.name << endl;
            }
            // Verify that NORMAL attribute is vec3
            if (nomAccessor.componentType != 5126 || nomAccessor.type != TINYGLTF_TYPE_VEC3) {
                cout << "Expected NORMAL to be vec3" << mesh.name << endl;
            }
            // Verify that TEXCOORD_0 attribute is vec2
            if (texAccessor.componentType != 5126 || texAccessor.type != TINYGLTF_TYPE_VEC2) {
                cout << "Expected TEXCOORD_0 to be vec2" << mesh.name << endl;
            }

            // Build commands array here
            // Now we need to look at the bufferView and figure out the baseVertex (vertex offset)
            // Look at primitive.indices's bufferView to figure out count and firstIndex
            const tinygltf::Accessor &indexAccessor =
                scene.accessors[primitive.indices];

            // Verify indices are unsigned int
            if (indexAccessor.componentType != 5125 || indexAccessor.type != TINYGLTF_TYPE_SCALAR) {
                cout << "Error: expected unsigned int indices for mesh " << mesh.name << endl;
            }

            cout << mesh.name << " is OK " << posAccessor.bufferView << " " << viewsCommands[posAccessor.bufferView] << endl;

            DrawElementsIndirectCommand &command = commands.at(viewsCommands[posAccessor.bufferView]);
            command.firstIndex = viewsIndices[indexAccessor.bufferView];
            command.count = indexAccessor.count;
        }
    }

    // Debug: print commands

    vector<DrawElementsIndirectCommand>::const_iterator itCommand(commands.begin());
    vector<DrawElementsIndirectCommand>::const_iterator itCommandEnd(commands.end());

    for (; itCommand != itCommandEnd; itCommand++) {
        const DrawElementsIndirectCommand &command = *itCommand;

        cout << "count: " << command.count << " instanceCount: " << command.instanceCount << " firstIndex: " << command.firstIndex << " baseVertex: " << command.baseVertex << " baseInstance: " << command.baseInstance << endl;
    }

    // Copy commands to CBO
    glNamedBufferSubData(mdiBuffers.cbo, 0,
        commands.size() * sizeof(DrawElementsIndirectCommand), &commands.at(0));

    // Copy instances vector to DrawID buffer
    // This buffer is needed if GL_ARB_shader_draw_parameters is
    // unavailable or buggy (on Mesa for Ivy Bridge)
    // Separate buffer is also a little bit faster according to Nvidia
    glNamedBufferSubData(mdiBuffers.dbo, 0, instances.size() * sizeof(unsigned int), &instances.at(0));
}

void loadTexture(GLuint shaderProgram)
{
    float pixels[] = {
        0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f
    };

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const auto depth = 1;
    glTextureStorage3D(texture, 1, GL_RGBA8, 4, 4, depth);
    glTextureSubImage3D(texture, 0, 0, 0, 0, 4, 4, depth, GL_RGB, GL_FLOAT, pixels);

    GLint uniTexture = glGetUniformLocation(shaderProgram, "diffuseTexture");
    glProgramUniform1i(shaderProgram, uniTexture, texture);
}

////////////////////////////////////////////////////////////////////////////////
// GLFW callbacks
////////////////////////////////////////////////////////////////////////////////

static void error_callback(int error, const char* description)
{
    cout << "GLFW: " << description << endl;
}

float z_rotate = 0.0f;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_LEFT and action != GLFW_RELEASE)
        z_rotate += 2.0f;
    else if (key == GLFW_KEY_RIGHT and action != GLFW_RELEASE)
        z_rotate -= 2.0f;
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    cout << "with: " << width << " height: " << height << endl;
}

static void window_close_callback(GLFWwindow* window)
{
    cout << "Closing window" << endl;
}

bool moving_camera = false;
double mouse_x = 0.0;
double mouse_y = 0.0;
double mouse_offset_x = 0.0;
double mouse_offset_y = 0.0;
double prev_mouse_offset_x = 0.0;
double prev_mouse_offset_y = 0.0;

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (moving_camera)
    {
        mouse_offset_x = (xpos - mouse_x) / 2.0 + prev_mouse_offset_x;
        mouse_offset_y = ypos - mouse_y + prev_mouse_offset_y;
    }
    else
    {
        mouse_x = xpos;
        mouse_y = ypos;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        moving_camera = (action == GLFW_PRESS);
        if (moving_camera)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            prev_mouse_offset_x = mouse_offset_x;
            prev_mouse_offset_y = mouse_offset_y;
        }
    }
}

double mouse_scroll_y = 2.0;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "scroll x: " << xoffset << " y: " << yoffset << endl;
    mouse_scroll_y = std::max(2.0, mouse_scroll_y - yoffset / 2);
}

////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////

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

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

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
    GLMDIBuffers mdiBuffers;
    loadMesh(scene, shaderProgram, mdiBuffers);

    //////////////////////////////////////////////////////////////////////

    loadTexture(shaderProgram);

    //////////////////////////////////////////////////////////////////////

    // Specify vertex format
    specifySceneVertexAttributes(shaderProgram, vao);

    glBindVertexArray(vao);
    glUseProgram(shaderProgram);

    GLint uniWorld = glGetUniformLocation(shaderProgram, "world");
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

    auto t_start = std::chrono::high_resolution_clock::now();

    // START BIND
        // Bind vertex and index buffers
        glVertexArrayVertexBuffer(vao, 0, mdiBuffers.vbo, 0, 8*sizeof(float));
        glVertexArrayVertexBuffer(vao, 1, mdiBuffers.dbo, 0, 1*sizeof(int));
        glVertexArrayElementBuffer(vao, mdiBuffers.ibo);

        // Bind command buffer and do MDI
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mdiBuffers.cbo);
    // END BIND

//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
//        auto t_now = std::chrono::high_resolution_clock::now();
//        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        float time = 0.0;

    float ratio;
    int width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;

        // View matrix
//        glm::mat4 view = glm::lookAt(
//            glm::vec3(1.0f, 1.0f, 10.0f),
//            glm::vec3(0.0f, 0.0f, 0.0f),
//            glm::vec3(0.0f, 0.0f, 1.0f)
//        );
        glm::mat4 view;
        view = glm::translate(view, glm::vec3(0, 0, -mouse_scroll_y));
        view = glm::rotate(view, glm::radians((float) mouse_offset_y), glm::vec3(1, 0, 0));
        view = glm::rotate(view, glm::radians((float) mouse_offset_x), glm::vec3(0, 0, 1));
        glProgramUniformMatrix4fv(shaderProgram, uniView, 1, GL_FALSE, glm::value_ptr(view));

        // Projection matrix
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) (width / height), 0.1f, 100.0f);
        glProgramUniformMatrix4fv(shaderProgram, uniProj, 1, GL_FALSE, glm::value_ptr(proj));

        // World matrix
        glm::mat4 trans;
        trans = glm::rotate(trans, time * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glProgramUniformMatrix4fv(shaderProgram, uniWorld, 1, GL_FALSE, glm::value_ptr(trans));

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, mdiBuffers.count, 0);

        // Swap front and back buffers and poll for and process events
        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(shaderVS);
    glDeleteShader(shaderFS);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vbo_instances);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &cbo);

    glDeleteBuffers(1, &mdiBuffers.vbo);
    glDeleteBuffers(1, &mdiBuffers.ibo);

    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
