#include "MapChooser.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "MapDefaults.h"

namespace {
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}
}  // namespace

std::vector<MapEntry> mapEntriesFrom(const std::vector<std::string>& mapPaths) {
    std::vector<MapEntry> entries;
    for (const std::string& path: mapPaths) {
        std::string display = std::filesystem::path(path).stem().string();
        entries.push_back({display, path, false});
    }
    entries.push_back({"Nuevo mapa", "", true});
    return entries;
}

std::string mapPathForName(const std::string& name) {
    return std::string(MapDefaults::MAPS_DIR) + trim(name) + ".json";
}

std::string newMapError(const std::string& name, const std::vector<std::string>& existingPaths) {
    std::string trimmed = trim(name);
    if (trimmed.empty()) {
        return "el nombre no puede estar vacío";
    }
    for (char c: trimmed) {
        bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
        if (!ok) {
            return "solo letras, números, - y _";
        }
    }
    std::string path = mapPathForName(trimmed);
    for (const std::string& existing: existingPaths) {
        if (existing == path) {
            return "ya existe un mapa con ese nombre";
        }
    }
    return "";
}

std::vector<std::string> listMapFiles(const std::string& dir) {
    std::vector<std::string> paths;
    if (!std::filesystem::exists(dir)) {
        return paths;
    }
    for (const auto& entry: std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            paths.push_back(entry.path().generic_string());
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

std::string readMapFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
