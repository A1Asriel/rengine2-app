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

void destroyApp(SDL_Window* window, SDL_GLContext context, REngine::Renderer* renderer) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    delete renderer;
}

int main(int argc, char* argv[]) {
    std::string scenePath = "scene.rem";
    char* vertexPath = NULL;
    char* fragmentPath = NULL;
    if (argc > 1) {
        for (int i = 0; i < argc - 1; i++) {
            if (std::string(argv[i]) == "-scene") {
                scenePath = argv[i + 1];
            }
            else if (std::string(argv[i]) == "-v") {
                vertexPath = argv[i + 1];
            }
            else if (std::string(argv[i]) == "-f") {
                fragmentPath = argv[i + 1];
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        FATAL("SDL could not initialize! SDL_Error: " << SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        W_TITLE.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        W_WIDTH,
        W_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        FATAL("Window could not be created! SDL_Error: " << SDL_GetError());
        destroyApp(window, NULL, NULL);
        return -1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        FATAL("OpenGL context could not be created! SDL_Error: " << SDL_GetError());
        destroyApp(window, glContext, NULL);
        return -1;
    }

    REngine::Renderer* renderer = new REngine::Renderer(W_WIDTH, W_HEIGHT);
    if (renderer && renderer->Init(SDL_GL_GetProcAddress) != 0) {
        FATAL("Couldn't initialize renderer");
        destroyApp(window, glContext, renderer);
        return -1;
    }

    renderer->shader = new REngine::Shader(vertexPath, fragmentPath);

    if (SDL_GL_SetSwapInterval(1) != 0) {
        ERROR("Couldn't set up Vsync: " << SDL_GetError());
    }

    REngine::Scene scene;
    if (!REngine::SceneLoader::load(scenePath, &scene)) {
        FATAL("Couldn't load the scene");
        destroyApp(window, glContext, renderer);
        return -1;
    }
    renderer->setScene(&scene);

    bool quit = false;  // Флаг выхода из цикла
    SDL_Event e;

    Uint32 previousTime = SDL_GetTicks();  // Последнее время в миллисекундах
    double deltaTime = 0.0f;  // Время с последнего кадра в секундах
    int frames = 0;  // Количество кадров за секунду

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
                    float dx = e.motion.xrel / 5.0f;  // Вращение камеры по X в градусах
                    float dy = e.motion.yrel / 5.0f;  // Вращение камеры по Y в градусах
                    renderer->camera.rotateRelative(dx, dy, 0);
                }
                break;
            default:
                break;
            }
        }

        Uint32 currentTime = SDL_GetTicks();  // Текущее время в миллисекундах
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
            renderer->camera.moveRelative(viewPos.x, viewPos.y, viewPos.z);

        // Вывести FPS
        if (currentTime / 1000 - previousTime / 1000 >= 1) {
            INFO("FPS: " << std::to_string(frames));
            frames = 0;
        }
        frames++;
        previousTime = currentTime;
        renderer->Draw(currentTime);
        SDL_GL_SwapWindow(window);
    }
    destroyApp(window, glContext, renderer);
    DEBUG("Goodnight");

    return 0;
}