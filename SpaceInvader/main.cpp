// The Complete Guide Doesn't Even Run. Guide Problem.

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <GL/glew.h> // Drawing Shaders
#include <GLFW/glfw3.h> // Window and Accept Input

// Declaring The Error Function -> Takes in An Int and a String
typedef void (*GFLWerrorfun)(int, const char *);

void error_callback(int error,const char* description){
    // Print As Error, Define Variable, Insert Variable
    fprintf(stderr, "Error %s\n", description);
}

// Texture and Colours
// Space to Hold Pixel Data
struct Buffer{
    size_t width,height;
    uint32_t* data; // holds 32 bits per array item
};

// Convert RGB To UINT32 Array
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b){
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

// Fills All Pixels with A 32-bit Color
void buffer_clear(Buffer* buffer, uint32_t color){
    for (size_t i=0; i< buffer->width * buffer->height; i++){
        buffer -> data[i] = color;
    }
}

// Validators (Prints OpenGL Outputs)
void validate_shader(GLuint shader, const char* file=0){
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE]; // Make Space for The Error Log
    GLsizei length = 0;

    // Logs Warning or Errors
    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if (length>0){
        printf("Shader %d(%s) compile error: %s\n", shader, (file? file: ""), buffer);
    }
}

bool validate_program(GLuint program){
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(program, BUFFER_SIZE, &length, buffer);
    if (length>0){
        printf("Program %d is invalid. Error: %s", program, buffer);
        return false;
    }

    return true;
}



int main(){
    //Initialise Height and Width of Buffer (Number of Pixel)
    const size_t buffer_width = 224;
    const size_t buffer_height = 256;

    glfwSetErrorCallback(error_callback);

    //Ensure Library is Initialised
    if(!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);

    if (!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK){
        fprintf(stderr, "Error initialising GLEW \n");
        glfwTerminate();
        return -1;
    }

    // Querying OpenGL Functions
    int glVersion[2] = {-1,1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(1.0, 0.0, 0.0, 1.0);

    uint32_t clear_color = rgb_to_uint32(0, 255, 0);

    // Initialise Buffer Item (Number of Pixels)
    Buffer buffer;
    buffer.width  = buffer_width;
    buffer.height = buffer_height;
    buffer.data   = new uint32_t[buffer.width * buffer.height];

    buffer_clear(&buffer, 0);

    // Create Texture and Buffer for OpenGL
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);

    // Defines Texture
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // Create VAO for Generating Fullscreen Triangle
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);

    // Drawing Vertex (Structure) and Fragment (Details) Shaders
    const char* vertex_shader =
    "\n"
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "\n"
    "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
    "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
    "    \n"
    "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";

    const char* fragment_shader =
    "\n"
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;\n"
    "noperspective in vec2 TexCoord;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void){\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";


    GLuint shader_id = glCreateProgram();

    // Rendering the Shaders
    {
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

        // Use the Empty Shader, Replace with the Vertex Shader String
        glShaderSource(shader_vp,1,&vertex_shader,0);
        glCompileShader(shader_vp);
        validate_shader(shader_vp,vertex_shader);
        glAttachShader(shader_id, shader_vp);

        // Delete After Attaching
        glDeleteShader(shader_vp);
    }

    {
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

        // Use the Empty Shader, Replace with the Vertex Shader String
        glShaderSource(shader_fp,1,&fragment_shader,0);
        glCompileShader(shader_fp);
        validate_shader(shader_fp,fragment_shader);
        glAttachShader(shader_id, shader_fp);

        glDeleteShader(shader_fp);
    }

    // Run the Program Attached By the Shaders
    glLinkProgram(shader_id);

    if (!validate_program(shader_id)){
        fprintf(stderr, "Could Not Validate Shader. \n");
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }

    glUseProgram(shader_id);

    // Retrieve and Set the Value for the Buffer Item (Uniform is The Reference)
    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(fullscreen_triangle_vao);



    //Main Game Loop

    while  (!glfwWindowShouldClose(window)){
        // Clear the RAM for Graphics (Buffer), Then Populate it With the Clear Colour
        glClear(GL_COLOR_BUFFER_BIT);

        // Push the Coloured Frame in Front
        glfwSwapBuffers(window);

        glDrawArrays(GL_TRIANGLES,0,3);

        // Listen to User Activity
        glfwPollEvents();
    }

    std::cout << "Destroyed Window";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
