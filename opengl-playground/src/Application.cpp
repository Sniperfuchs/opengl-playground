#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

struct ShaderProgramSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;

    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        // If line contains #shader, we set our type appropriately
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            // We cast our type to an index for our stream array and append the line
            ss[(int)type] << line << '\n';
        }
    }

    // Calls constructor with strings
    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    // Create a shader object of type GL_VERTEX_SHADER and return its id
    unsigned int shader_id = glCreateShader(type);
    const char* src = source.c_str();
    // Replace the source code of shader id with the code in src. 3rd parameter is an array of strings, so the new
    // code can be in multiple strings. The count of strings is in 2nd parameter (here 1). Last parameter is an array
    // containing lengths of the source code strings in 3rd parameter. NULL (or nullptr) means that each string is
    // assumed to be NULL terminated (so a normal C-String).
    glShaderSource(shader_id, 1, &src, nullptr);

    glCompileShader(shader_id);

    int result;

    // Returns a parameter from a shader object. Here we choose the parameter GL_COMPILE_STATUS which is false if
    // there was an error during compilation and true if it was successful.
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

    // If unsuccessful...
    if (result == GL_FALSE)
    {
        int length;

        // Returns the length of the information log which we want to extract and display.
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(shader_id, length, &length, message);
        std::cout << "Failed to compile "
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(shader_id);
        return 0;
    }

    return shader_id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "ERROR" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    /*
    * Create Vertex Buffer
    */

    // Vertex buffer ID
    unsigned int buffer;

    glGenBuffers(1, &buffer);
    // Telling OpenGL which buffer to use (selecting = binding) and what type it is
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Creating and filling buffer with vertex data. Last parameter is a telling
    // OpenGL with the buffer (DRAW) and how to do it (STATIC as in the buffer won't
    // change. The latter is just a hint and might impact performance but still works
    // if buffer changes (DYNAMIC).
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    // HAVE to enable vertex attribute at index 0 (only attribute here, which is vertex position)
    glEnableVertexAttribArray(0);
    // Index of attribute you want to manipulate. Number of components this attribute has. Type of those components.
    // Attributes normalized between 0 and 1 by OpenGL? Stride between data (here: between vertex positions).
    // Offset of different types of attributes (here 0 because we only have 1 type which is position).
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);


    /*
    * Create Index Buffer
    */

    unsigned int indexBufferObject;
    glGenBuffers(1, &indexBufferObject);
    // This time we use GL_ELEMENT_ARAY_BUFFER to show OpenGL that it is an index buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


    // Relative path is our project directory ONLY in VS deb
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    unsigned int shader = CreateShader(source.vertexSource, source.fragmentSource);
    // Load shader after it has been compiled
    glUseProgram(shader);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Second argument is the number of indices. Last argument is NULL because we bound the index buffer already.
        // HAS to be unsigned int.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Clean up shader
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}