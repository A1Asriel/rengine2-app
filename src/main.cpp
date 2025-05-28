#include <filesystem>
#include <iostream>

#include <Window.h>
#include <Logging.h>
#include <InputHandler.h>

#include "SceneLoader.h"

int W_WIDTH = 800;
int W_HEIGHT = 600;
const char* W_TITLE = "REngine";

int main(int argc, char* argv[]) {
    std::string scenePath = "scene.rem";
    char* vertexPath = NULL;
    char* fragmentPath = NULL;
    if (argc > 1) {
        for (int i = 0; i < argc - 1; i++) {
            if (std::string(argv[i]) == "-scene") {
                scenePath = argv[i + 1];
            } else if (std::string(argv[i]) == "-v") {
                vertexPath = argv[i + 1];
            } else if (std::string(argv[i]) == "-f") {
                fragmentPath = argv[i + 1];
            } else if (std::string(argv[i]) == "-w") {
                W_WIDTH = std::stoi(argv[i + 1]);
            } else if (std::string(argv[i]) == "-h") {
                W_HEIGHT = std::stoi(argv[i + 1]);
            }
        }
    }

    if (REngine::createWindow(W_TITLE, W_WIDTH, W_HEIGHT) != 0) {
        FATAL("Couldn't initialize window");
        return -1;
    }

    REngine::Scene scene;
    if (!SceneLoader::load(scenePath, &scene)) {
        FATAL("Couldn't load the scene");
        REngine::destroyWindow();
        return -1;
    }
    REngine::renderer->setScene(&scene);
    REngine::renderer->setShader(vertexPath, fragmentPath);

    bool wireframeMode = false;

    // Переключение режима отображения вайрфрейма по F1
    REngine::InputHandler::setKeyDownCallback(SDLK_F1, [&wireframeMode]() {
        wireframeMode = !wireframeMode;
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            INFO("Wireframe mode: ON");
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            INFO("Wireframe mode: OFF");
        }
    });

    // Обработка нажатия левой кнопки мыши
    REngine::InputHandler::setMouseButtonDownCallback(SDL_BUTTON_LEFT, [](int x, int y, int button) {
        INFO("Left mouse button clicked at: (" << x << ", " << y << ")");
    });

    // Обработка прокрутки колесика мыши
    REngine::InputHandler::setMouseWheelCallback([](int x, int y) {
        float newFov = REngine::renderer->camera.fov + y;
        if (newFov > 179)      REngine::renderer->camera.fov = 179;
        else if (newFov < 1)   REngine::renderer->camera.fov = 1;
        else                   REngine::renderer->camera.fov += y;
    });
    
    try {
        REngine::mainLoop();
    } catch (...) {
        REngine::destroyWindow();
        return -1;
    }
    
    REngine::destroyWindow();
    DEBUG("Goodnight");

    return 0;
}