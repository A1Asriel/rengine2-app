#include "SceneLoader.h"

#include <fstream>
#include <sstream>
#include <vector>

#include <Logging.h>
#include <Texture.h>
#include <CubeMesh.h>
#include <SphereMesh.h>

bool SceneLoader::load(const std::string& file, REngine::Scene* scene) {
    scene->nodes.clear();

    std::ifstream input(file);
    if (!input.is_open()) {
        return false;
    }

    std::string line;

    std::getline(input, line);
    if (line != "[RENGINE MAP FORMAT V1.0]") {
        ERROR("Invalid file format");
        return false;
    }

    while (std::getline(input, line)) {
        std::istringstream iss(line);
        std::string token;
        if (line.length() < 2 || line.substr(0,2) == "//") {
            continue;
        }

        std::vector<std::string> tokens;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens[0] == "camera") {
            if (tokens.size() < 8) {
                ERROR("Line \"" << line << "\" has incorrect amount of parameters");
                continue;
            }
            try {
                scene->camera.position.x = std::stof(tokens[1]);
                scene->camera.position.y = std::stof(tokens[2]);
                scene->camera.position.z = std::stof(tokens[3]);
                DEBUG("Loaded camera position: " << scene->camera.position.x << ", " << scene->camera.position.y << ", " << scene->camera.position.z);
                scene->camera.rotation.x = std::stof(tokens[4]);
                scene->camera.rotation.y = std::stof(tokens[5]);
                scene->camera.rotation.z = std::stof(tokens[6]);
                DEBUG("Loaded camera rotation: " << scene->camera.rotation.x << ", " << scene->camera.rotation.y << ", " << scene->camera.rotation.z);
                scene->camera.fov = std::stof(tokens[7]);
                DEBUG("Loaded camera fov: " << scene->camera.fov);
            } catch(const std::exception& e) {
                ERROR("Line \"" << line << "\" could not be interpreted");
                continue;
            }
        } else if (tokens[0] == "sky") {
            if (tokens.size() < 4) {
                ERROR("Line \"" << line << "\" has incorrect amount of parameters");
                continue;
            }
            try {
                scene->skyColor.x = std::stof(tokens[1]);
                scene->skyColor.y = std::stof(tokens[2]);
                scene->skyColor.z = std::stof(tokens[3]);
                DEBUG("Loaded sky color: " << scene->skyColor.x << ", " << scene->skyColor.y << ", " << scene->skyColor.z);
            } catch(const std::exception& e) {
                ERROR("Line \"" << line << "\" could not be interpreted");
                continue;
            }
        } else if (tokens[0] == "dirlight") {
            if (tokens.size() < 13) {
                ERROR("Line \"" << line << "\" has incorrect amount of parameters");
                continue;
            }
            try {
                scene->dirLight.direction.x = std::stof(tokens[1]);
                scene->dirLight.direction.y = std::stof(tokens[2]);
                scene->dirLight.direction.z = std::stof(tokens[3]);
                DEBUG("Loaded direction: " << scene->dirLight.direction.x << ", " << scene->dirLight.direction.y << ", " << scene->dirLight.direction.z);
                scene->dirLight.ambient.x = std::stof(tokens[4]);
                scene->dirLight.ambient.y = std::stof(tokens[5]);
                scene->dirLight.ambient.z = std::stof(tokens[6]);
                DEBUG("Loaded directional ambient color: " << scene->dirLight.ambient.x << ", " << scene->dirLight.ambient.y << ", " << scene->dirLight.ambient.z);
                scene->dirLight.diffuse.x = std::stof(tokens[7]);
                scene->dirLight.diffuse.y = std::stof(tokens[8]);
                scene->dirLight.diffuse.z = std::stof(tokens[9]);
                DEBUG("Loaded directional diffuse color: " << scene->dirLight.diffuse.x << ", " << scene->dirLight.diffuse.y << ", " << scene->dirLight.diffuse.z);
                scene->dirLight.specular.x = std::stof(tokens[10]);
                scene->dirLight.specular.y = std::stof(tokens[11]);
                scene->dirLight.specular.z = std::stof(tokens[12]);
                DEBUG("Loaded directional specular color: " << scene->dirLight.specular.x << ", " << scene->dirLight.specular.y << ", " << scene->dirLight.specular.z);
            } catch(const std::exception& e) {
                ERROR("Line \"" << line << "\" could not be interpreted");
                continue;
            }
        } else if (tokens[0] == "pointlight") {
            if (tokens.size() < 16) {
                ERROR("Line \"" << line << "\" has incorrect amount of parameters");
                continue;
            }
            if (scene->pointLights.size() >= POINT_LIGHTS_MAX) {
                ERROR("The scene has exceeded the maximum amount of point lights. Skipping");
                continue;
            }
            try {
                REngine::PointLight pointLight;
                pointLight.position.x = std::stof(tokens[1]);
                pointLight.position.y = std::stof(tokens[2]);
                pointLight.position.z = std::stof(tokens[3]);
                DEBUG("Loaded position: " << pointLight.position.x << ", " << pointLight.position.y << ", " << pointLight.position.z);
                pointLight.constant = std::stof(tokens[4]);
                pointLight.linear = std::stof(tokens[5]);
                pointLight.quadratic = std::stof(tokens[6]);
                DEBUG("Loaded constants: " << pointLight.constant << ", " << pointLight.linear << ", " << pointLight.quadratic);
                pointLight.ambient.x = std::stof(tokens[7]);
                pointLight.ambient.y = std::stof(tokens[8]);
                pointLight.ambient.z = std::stof(tokens[9]);
                DEBUG("Loaded point ambient color: " << pointLight.ambient.x << ", " << pointLight.ambient.y << ", " << pointLight.ambient.z);
                pointLight.diffuse.x = std::stof(tokens[10]);
                pointLight.diffuse.y = std::stof(tokens[11]);
                pointLight.diffuse.z = std::stof(tokens[12]);
                DEBUG("Loaded point diffuse color: " << pointLight.diffuse.x << ", " << pointLight.diffuse.y << ", " << pointLight.diffuse.z);
                pointLight.specular.x = std::stof(tokens[13]);
                pointLight.specular.y = std::stof(tokens[14]);
                pointLight.specular.z = std::stof(tokens[15]);
                DEBUG("Loaded point specular color: " << pointLight.specular.x << ", " << pointLight.specular.y << ", " << pointLight.specular.z);
                scene->pointLights.push_back(pointLight);
            } catch(const std::exception& e) {
                ERROR("Line \"" << line << "\" could not be interpreted");
                continue;
            }
        } else {
            if (tokens.size() < 11) {
                ERROR("Line \"" << line << "\" has incorrect amount of parameters");
                continue;
            }
            REngine::SceneNode node;
            if (tokens[0] == "cube")
                node.mesh = (REngine::Mesh*)new REngine::CubeMesh();
            else if (tokens[0] == "sphere")
                node.mesh = (REngine::Mesh*)new REngine::SphereMesh(40, 40);
            else {
                ERROR("Line \"" << line << "\" has incorrect mesh type");
                continue;
            }
            try {
                DEBUG("Loading node \"" << tokens[0] << "\"");
                node.position.x = std::stof(tokens[1]);
                node.position.y = std::stof(tokens[2]);
                node.position.z = std::stof(tokens[3]);
                DEBUG("Loaded node position: " << node.position.x << ", " << node.position.y << ", " << node.position.z);
                node.rotation.x = std::stof(tokens[4]);
                node.rotation.y = std::stof(tokens[5]);
                node.rotation.z = std::stof(tokens[6]);
                DEBUG("Loaded node rotation: " << node.rotation.x << ", " << node.rotation.y << ", " << node.rotation.z);
                node.scale.x = std::stof(tokens[7]);
                node.scale.y = std::stof(tokens[8]);
                node.scale.z = std::stof(tokens[9]);
                DEBUG("Loaded node scale: " << node.scale.x << ", " << node.scale.y << ", " << node.scale.z);
                node.shininess = std::stof(tokens[10]);
                DEBUG("Loaded node shininess: " << node.shininess);
                node.distort = tokens[11] == "true";
                DEBUG("Loaded node distort: " << node.distort);
                if (tokens.size() > 12) {
                    node.texturePath = tokens[12];
                    DEBUG("Loaded texture path: " << node.texturePath);
                }
                if (tokens.size() > 13) {
                    node.specularPath = tokens[13];
                    DEBUG("Loaded specular path: " << node.specularPath);
                }
            } catch(const std::exception& e) {
                ERROR("Line \"" << line << "\" could not be interpreted");
                continue;
            }
            scene->nodes.push_back(node);
        }
    }

    return true;
}

bool SceneLoader::save(const std::string& file, REngine::Scene* scene) {
    // TODO: To be implemented
    ERROR("SceneLoader::save not implemented");
    return false;
}
