// resources.h
// author:  Maciej Szpakowski
// classes: Resources

#pragma once

class Resources
{
private:
    std::map<std::string, sf::Texture*> textures;
    Resources()
    {
    }

public:
    static Resources* GetResources()
    {
        static Resources* instance = nullptr;
        if (instance == nullptr)
            instance = new Resources();
        return instance;
    }

    void AddTexture(std::string filename)
    {
        textures[filename] = new sf::Texture();
        if (!textures[filename]->loadFromFile(filename))
            throw std::runtime_error("could not open a file");

        textures[filename]->setSmooth(false);
    }

    sf::Texture* GetTexture(std::string filename) const
    {
        auto it = textures.find(filename);
        if (it == textures.end())
            return nullptr;

        return it->second;
    }
};