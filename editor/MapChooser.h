#ifndef MAP_CHOOSER_H
#define MAP_CHOOSER_H

#include <string>
#include <vector>

struct MapEntry {
    std::string displayName;
    std::string path;
    bool isNew;
};

std::vector<MapEntry> mapEntriesFrom(const std::vector<std::string>& mapPaths);
std::string mapPathForName(const std::string& name);
std::string newMapError(const std::string& name, const std::vector<std::string>& existingPaths);
std::vector<std::string> listMapFiles(const std::string& dir);
std::string readMapFile(const std::string& path);
void writeMapFile(const std::string& path, const std::string& contents);

#endif
