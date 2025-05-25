#include <filesystem>
#include <iostream>

#include <SDL.h>

#include <Renderer.h>
#include <SceneLoader.h>
#include <Shader.h>
#include <Logging.h>
#include <SDL_stdinc.h>

const int W_WIDTH = 800;
const int W_HEIGHT = 600;
const std::string W_TITLE = "REngine";

int main(int argc, char* argv[]) {
    std::string scenePath = "scene.rem";
    if (argc > 1) {
        scenePath = argv[1];
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        FATAL("SDL could not initialize! SDL_Error: " << SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* sdl_window = SDL_CreateWindow(
        W_TITLE.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        W_WIDTH,
        W_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!sdl_window) {
        FATAL("Window could not be created! SDL_Error: " << SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
    if (!gl_context) {
        FATAL("OpenGL context could not be created! SDL_Error: " << SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return -1;
    }

    REngine::Renderer* re_window = new REngine::Renderer(W_WIDTH, W_HEIGHT);
    if (re_window && re_window->Init(SDL_GL_GetProcAddress) != 0) {
        FATAL("Couldn't initialize renderer");
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        delete re_window;
        return -1;
    }

    re_window->shader = new REngine::Shader();

    if (SDL_GL_SetSwapInterval(1) != 0) {
        ERROR("Couldn't set up Vsync: " << SDL_GetError());
    }

    REngine::Scene scene;
    if (!REngine::SceneLoader::load(scenePath, &scene)) {
        FATAL("Couldn't load the scene");
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        delete re_window;
        return -1;
    }
    re_window->scene = scene;
    re_window->camera.position = scene.camera.position;
    re_window->camera.front = glm::vec3(sin(glm::radians(scene.camera.rotation.y)),
                                        sin(glm::radians(scene.camera.rotation.x)),
                                        cos(glm::radians(scene.camera.rotation.y)));
    re_window->camera.up = glm::vec3(sin(glm::radians(scene.camera.rotation.z)),
                                     cos(glm::radians(scene.camera.rotation.z)),
                                     0);
    re_window->camera.fov = scene.camera.fov;

    // REngine::Shader* shader;
    // if (!std::filesystem::exists("shaders/vertex.glsl") || !std::filesystem::exists("shaders/fragment.glsl")) {
    //     DEBUG("shaders/ not found");
    //     if (!std::filesystem::exists("../shaders/vertex.glsl") || !std::filesystem::exists("../shaders/fragment.glsl")) {
    //         DEBUG("../shaders/ not found");
    //         FATAL("Shaders not found!");
    //         SDL_DestroyWindow(sdl_window);
    //         SDL_Quit();
    //         delete re_window;
    //         return -1;
    //     }
    //     shader = new REngine::Shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    // } else {
    //     shader = new REngine::Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    // }

    bool quit = false;
    SDL_Event e;

    Uint32 previousTime = SDL_GetTicks();
    double deltaTime = 0.0f;
    int frames = 0;

    while (!quit) {
        // Проверка ввода
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
                    re_window->camera.rotateRelative(e.motion.xrel, e.motion.yrel, 0);
                }
                break;
            default:
                break;
            }
        }
        // Вычислить дельту по времени
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - previousTime) / 1000.0f;

        glm::vec3 viewPos(0.0f);

        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_W]) viewPos.z += deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);
        if (keystate[SDL_SCANCODE_A]) viewPos.x -= deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);
        if (keystate[SDL_SCANCODE_S]) viewPos.z -= deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);
        if (keystate[SDL_SCANCODE_D]) viewPos.x += deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);
        if (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL]) viewPos.y -= deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);
        if (keystate[SDL_SCANCODE_SPACE]) viewPos.y += deltaTime * (keystate[SDL_SCANCODE_LSHIFT]+1);

        if (viewPos.x != 0 || viewPos.y != 0 || viewPos.z != 0)
            re_window->camera.moveRelative(viewPos.x, viewPos.y, viewPos.z);

        // Вывести FPS
        if (currentTime / 1000 - previousTime / 1000 >= 1) {
            INFO("FPS: " << std::to_string(frames));
            frames = 0;
        }
        frames++;
        previousTime = currentTime;
        re_window->Draw(currentTime);
        SDL_GL_SwapWindow(sdl_window);
    }
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    delete re_window;
    DEBUG("Goodnight");

    return 0;
}