// Hello, OpenGL triangle!

#include <glad/glad.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <array>
#include <iostream>
#include <vector>

constexpr char VERTEXT_SHADER_SOURCE[] = R"(#version 330 core
layout (location = 0) in vec3 aPos;
uniform float offset;
void main()
{
   gl_Position = vec4(aPos.x + offset, aPos.y, aPos.z, 1.0);
}
)";

constexpr char FRAGMENT_SHADER_SOURCE[] = R"(#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

static GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,  // left
    0.5f,  -0.5f, 0.0f,  // right
    0.0f,  0.5f,  0.0f   // top
};

static bool CheckCompileErrors(GLuint shader)
{
    GLint success = 0;
    char info_log[512]{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << "\n";
    }
    return success;
}

bool CheckLinkErrors(GLuint program)
{
    GLint success = 0;
    char info_log[512]{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }
    return success;
}

static GLuint CompileShader(const char* vs, const char* fs)
{
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs, nullptr);
    glCompileShader(vertex_shader);
    if (!CheckCompileErrors(vertex_shader)) {
        return 0;
    }

    // fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs, nullptr);
    glCompileShader(fragment_shader);
    if (!CheckCompileErrors(fragment_shader)) {
        return 0;
    }

    // link shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (!CheckLinkErrors(program)) {
        return 0;
    }
    return program;
}

int main(int argc, char* argv[])
{
    bool disable_vsync = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--disable-vsync") == 0) {
            disable_vsync = true;
        }
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "Hello, OpenGL triangle!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(disable_vsync ? 0 : 1);

    if (!gladLoadGL()) {
        SDL_Log("Could not load GLAD");
        return -1;
    }

    GLuint program = CompileShader(VERTEXT_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);
    if (!program) {
        SDL_Log("Could not compile shader program");
        return -1;
    }

    GLuint vbo = 0;
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure
    // vertex attributes(s).
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex
    // attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    uint64_t last_tickets = SDL_GetTicks64();
    GLfloat offset = -0.5f;
    GLfloat speed = 0.5f;
    GLfloat direction = 1.0f;

    GLint offset_location = glGetUniformLocation(program, "offset");

    while (true) {
        SDL_Event event{};
        bool done = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
        if (done) {
            break;
        }

        uint64_t current_ticks = SDL_GetTicks64();
        float delta_time = (current_ticks - last_tickets) / 1000.0f;
        last_tickets = current_ticks;

        offset += delta_time * speed * direction;
        if (offset > 0.5f) {
            direction = -1.0;
        } else if (offset < -0.5f) {
            direction = 1.0;
        }

        glClearColor(0.06f, 0.0f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glUniform1f(offset_location, offset);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}