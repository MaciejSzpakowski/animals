// world.h
// author:  Maciej Szpakowski
// classes: World

#pragma once

class World
{
private:
    std::vector<std::array<FBITS,WIDTH*HEIGHT> > allGenerations; // stores all previous generations
    Matrix<Animal*> creatures; // grid for animal
    Matrix<Grass*> grass;      // grid for grass
    milliseconds lastTick;     // when was the last routine played
    milliseconds tickCount;    // how often to play routine
    int curGeneration;         // currently disaplyed generation
    bool paused;               // is time stopped
    bool started;              // has the simulation been started by the user
	LocKey lockey;             // key to animals location

    World();

    // spawns count number of creatures T in empty spaces randomly
    template<typename T>
    void SpawnCreatures(int count)
    {
		int maxIt = 1000;
		int it = 0;

        while (count)
        {
            sf::Vector2i loc = { rand() % WIDTH, rand() % HEIGHT };

            if (creatures.at(loc) == nullptr)
            {
                creatures.at(loc) = new T(loc);
                count--;
            }

			it++;
			if (it > maxIt)
				throw std::runtime_error("max number of iterations in SpawnCreatures exceeded"
					" possibly due to the world being full");
        }
    }
public:
    // returns singleton
    static World* GetWorld()
    {
        static World* instance = nullptr;
        
        if (instance == nullptr)
            instance = new World();

        return instance;
    }

    // draws grass and creatures
    void DrawAll(sf::RenderWindow& wnd);

    // removes the animal from the world
    void Kill(Animal* animal)
    {
        creatures.at(animal->GetLocation()) = nullptr;
        Animal::ReturnId(animal->GetId());
        delete animal;
    }

    // removes animal at postion pos fro the world
    // throws exception if there's no animal at pos
	void Kill(const sf::Vector2i& pos)
	{
		if (creatures.at(pos) == nullptr)
		{
			throw std::runtime_error("no animal at that pos");
		}

		Kill(creatures.at(pos));
	}

    // serializes the last generation (state of grass and animal grids)
    void AddGeneration();

    // play/pause
    void TogglePlay()
    {
        if (!started)
            started = true;

        paused = !paused;
    }

    bool IsPaused()
    {
        return paused;
    }

    bool IsStarted()
    {
        return started;
    }

    // get serialized animal at position pos for the current generation
    FBITS GetInt(sf::Vector2i pos)
    {
        return allGenerations[curGeneration][pos.x % WIDTH + pos.y * WIDTH];
    }

    // if paused, displays prev generation
    void Back()
    {
        if (paused && started)
            curGeneration--;

        if (curGeneration < 0)
            curGeneration = 0;
    }

    // if pasued, displays next generation
    void Forward()
    {
        if (paused && started)
            curGeneration++;

        if (curGeneration >= allGenerations.size())
        {
            NextGeneration();
            AddGeneration();
        }
    }

    // called every frame
    void Activity();

    // animals is not currently on the grid
    // puts animal on the grid at dst
    // it doesnt change animals position
    void PutCreature(Animal* animal, const sf::Vector2i& dst);

    // it moves animal from where it is on the grid to dst
    // it updates animals position
    void MoveCreature(Animal* animal, const sf::Vector2i& dst);

    bool IsEmpty(const sf::Vector2i& p)
    {
        return creatures.at(p) == nullptr;
    }

	Animal* GetCreature(const sf::Vector2i& pos)
	{
		return creatures.at(pos);
	}

    Grass* GetGrass(const sf::Vector2i& p)
    {
        return grass.at(p);
    }

    // plays routines for all entities on the map
    void NextGeneration();
};