#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <Scene.h>

/// @brief Класс для загрузки и сохранения сцен
/// @details Предоставляет статические методы для работы с файлами сцены
class SceneLoader {
public:
    /// @brief Загрузка сцены из файла
    /// @param file Путь к файлу
    /// @param scene Указатель на сцену для загрузки
    /// @return true при успешной загрузке, false в противном случае
    static bool load(const std::string& file, REngine::Scene* scene);

    /// @brief Сохранение сцены в файл
    /// @param file Путь к файлу
    /// @param scene Указатель на сцену для сохранения
    /// @return true при успешном сохранении, false в противном случае
    static bool save(const std::string& file, REngine::Scene* scene);
};

#endif
