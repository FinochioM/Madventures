#ifndef TILE_H
#define TILE_H

#include <unordered_map>
#include <string>
#include <variant>

class Tile {
public:
    Tile(int gridX, int gridY);
    ~Tile();

    int getGridX() const { return gridX; }
    int getGridY() const { return gridY; }

    int getPixelX() const;
    int getPixelY() const;

    template<typename T>
    void setProperty(const std::string& key, const T& value) {
        properties[key] = value;
    }

    template<typename T>
    T getProperty(const std::string& key, const T& defaultValue) const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            try {
                return std::get<T>(it->second);
            }catch (const std::bad_variant_access&) {
                return defaultValue;
            }
        }

        return defaultValue;
    }

    bool hasProperty(const std::string& key) const {
        return properties.find(key) != properties.end();
    }

private:
    int gridX, gridY;
    std::unordered_map<std::string, std::variant<bool, int, float, std::string>> properties;
};

#endif // TILE_H