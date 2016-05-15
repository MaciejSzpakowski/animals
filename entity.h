// entity.h
// author:  Maciej Szpakowski
// classes: Metabolism
//          Entity
//          Grass
//          IHerbivore
//          ICarnivore
//          Animal
//          Wolf
//          Weasel
//          Dino
//          Rabbit


#pragma once

class Metabolism
{
private:
    int food;
    int maxFood;
    int metabolicRate;

    Metabolism() {}
public:
    Metabolism(int initFood, int _maxFood, int _metabolicRate) :
        food(initFood), maxFood(_maxFood), metabolicRate(_metabolicRate) {}

    void AddFood(int val)
    {
        food = food + val > maxFood ? maxFood : food + val;
    }

    bool HasStarved() const
    {
        return food <= 0;
    }

    // adjust food level based on metabolic rate
    void ProcessFood()
    {
        AddFood(-metabolicRate);
    }

    int GetFoodLevel() const
    {
        return food;
    }

    // how much food to be full
    int UntilFull() const
    {
        return maxFood - food;
    }
};

// the purpose of this class is
// to make world be able to access entities location
class LocKey
{
};

// purpose of this class is to make location unaccessible to entity
// itself since only the world can change it
class EntityLoc
{
private:
	sf::Vector2i location;
	EntityLoc() {}
public:
	EntityLoc(int x, int y) :location(x, y) {}
	EntityLoc(sf::Vector2i&& loc) :location(loc) {}
	EntityLoc(const sf::Vector2i& loc) :location(loc) {}

	const sf::Vector2i& GetLocation() const
	{
		return location;
	}

	void SetLocation(const sf::Vector2i& loc, const LocKey& key)
	{
		location = loc;
	}
};

class Entity
{
protected:
    int age;
    Metabolism metabolism;
    EntityLoc location;
	std::vector<sf::Vector2i> adjFields; // adj fields

    // make entity age and starve
    void Grow();

	// make adj fields up,down,left,right
	void BuildAdjCross();
public:
    Entity(int _metabolicRate, int initFoodLevel, int _maxFoodLevel,
        const sf::Vector2i& loc);

    // routine that encapsulates everything animal might do during the day
    virtual void Routine(int generation) = 0;    

    const sf::Vector2i& GetLocation() const
    {
        return location.GetLocation();
    }    

    int GetFoodLevel() const
    {
        return metabolism.GetFoodLevel();
    }
};

class Wheat : public Entity
{
private:
	// spawns a neigbor (in other words, tries to spread)
	void Spawn();
public:
	Wheat(const sf::Vector2i& loc) :
		Entity(-10, 30, 100, loc) {}

	void Routine(int generation) override
	{
		Spawn();
		Grow();
	}
};

class Grass : public Entity
{
private:
	Wheat* wheat;
public:
    Grass(const sf::Vector2i& loc) :
        Entity(-1, 20, 100, loc),wheat(nullptr) {}

    void Routine(int generation) override
    {
		if (metabolism.GetFoodLevel() > 50 && rand() % 100 < 5) // conditions for wheat to spawn
			SpawnWheat();

		if (wheat != nullptr)
			wheat->Routine(generation);

        Grow();
    }

	// creates wheat for this grass
	void SpawnWheat()
	{
		if (wheat != nullptr)
			return;

		wheat = new Wheat(GetLocation());
	}

	bool HasWheat() const
	{
		return wheat != nullptr;
	}

	// returns wheat food level
	int EatWheat()
	{
		int food = wheat->GetFoodLevel();
		delete wheat;
		wheat = nullptr;
		return food;
	}

	Wheat* GetWheat() const
	{
		return wheat;
	}

    // needed by rabbit
    void SubFood(int val)
    {
        metabolism.AddFood(-val);
    }

	FBITS ToBits()
	{
        FBITS bits;
        bits.grass = GetFoodLevel();
        if (wheat != nullptr)
            bits.wheat = wheat->GetFoodLevel();

		return bits;
	}
};

// interafce for herbivore behaviors
class IHerbivore
{
protected:
    virtual bool Evade() = 0;
};

// interface for carnivore behaviors
class ICarnivore
{
protected:
    virtual bool Chase() = 0;
};

class Rabbit;
class Wolf;
class Dino;
class Weasel;

class Animal : public Entity
{
protected:
    static std::queue<unsigned short> ids; // available ids
    unsigned short id;   // id
	int meatVal;         // how much food it gives when eaten
    int lastGenActivity; // used to prevent more than one routine per generation    
    int matureAge;       // minimum age to reproduce
    int reproduceChance; // 0 or less never, 100 or more always
    int foodToReproduce; // food level required to reproduce
    int ageToDie;        // will die when reached that age
    sf::Vector2i prevLoc; // location that it moved from, used for spawning

    virtual void Move() = 0;
    virtual bool Eat() = 0;
    void Die();
	
    // make adj fields all fields in a square around animal
    void BuildAdjSquare(int edge);

    // make adj appear random in the list
    void ShuffleAdj();

    // checks if conditions are satisfied and returns where new
    // animal will be spawned
    // if conditions are not satisifed, returns null
    sf::Vector2i* GetBirthPlace();

    // serializes animal to binary
    // type must be 4 bit
    FBITS ToBits(unsigned char type);

    // tries to spawn a new animal
    // returns wheter a new animal has been spawned
    template <typename T>
    bool Reproduce()
    {
        sf::Vector2i* birthPlace = GetBirthPlace();

        if (birthPlace != nullptr)
        {
            Animal* newAnimal = new T(*birthPlace);
            _PutCreature(newAnimal, birthPlace);
            return true;
        }
        else
            return false;
    }

    // helper function
    void _PutCreature(Animal* a, sf::Vector2i* birthPlace);
		
public:
    Animal(int _metabolicRate, int initFoodLevel, int _maxFoodLevel,
        int _matureAge, int _reprocudeChance, int _foodToReproduce,
        int _ageToDie, int meat, const sf::Vector2i& loc);

    void SetLocation(const sf::Vector2i loc, const LocKey& key)
    {
		location.SetLocation(loc, key);
    }

    // initializes ids queue
    // this should be called only once
    static void InitIds()
    {
        static bool initialized = false;

        if (initialized)
            return;

        for (unsigned short i = 0; i < 1200; i++)
            Animal::ids.push(i);

        initialized = true;
    }

    // when animal is being destroyed, it should return its id
    // to id pool
    static void ReturnId(unsigned short val)
    {
        if (val > 1200)
            throw std::runtime_error("id > 1200");

        Animal::ids.push(val);
    }

	int GetMeatValue() const
	{
		return meatVal;
	}

    unsigned short GetId()
    {
        return id;
    }

	// takes only one step towards the dst
	// returns false if its blocked and cannot get there
	bool Step(const sf::Vector2i& dst);

	// move to dst step by step
	// returns false if its blocked at some point
	bool MoveTo(const sf::Vector2i& dst);

    virtual FBITS ToBits() = 0;

	// returns true if animal evades this
	virtual bool FearMe(Wolf* animal) = 0;
	virtual bool FearMe(Rabbit* animal) = 0;
    virtual bool FearMe(Dino* animal) = 0;
    virtual bool FearMe(Weasel* animal) = 0;

	// return true if animal can eat this
	virtual bool EatMe(Wolf* animal) = 0;
	virtual bool EatMe(Rabbit* animal) = 0;
    virtual bool EatMe(Dino* animal) = 0;
    virtual bool EatMe(Weasel* animal) = 0;
};

class Wolf :public Animal, public ICarnivore
{
private:
	bool Eat() override;
	void Move() override;
	bool Chase() override;

	bool FearMe(Wolf* w) override
	{
		return false;
	}

	bool FearMe(Rabbit* r) override
	{
		return true;
	}

    bool FearMe(Dino* w) override
    {
        return false;
    }

    bool FearMe(Weasel* r) override
    {
        return true;
    }

	bool EatMe(Rabbit* r) override
	{
		return false;
	}

	bool EatMe(Wolf* w) override
	{
		return false;
	}

    bool EatMe(Dino* r) override
    {
        return true;
    }

    bool EatMe(Weasel* w) override
    {
        return false;
    }
public:
    Wolf(const sf::Vector2i& loc);
    
	void Routine(int generation) override;

    FBITS ToBits() override
    {
        return Animal::ToBits(TWOLF);
    }
};

class Dino :public Animal, public ICarnivore
{
private:
	bool Eat() override;
    void Move() override;
    bool Chase() override;

    bool FearMe(Wolf* w) override
    {
        return false;
    }

    bool FearMe(Rabbit* r) override
    {
        return true;
    }

    bool FearMe(Dino* w) override
    {
        return false;
    }

    bool FearMe(Weasel* r) override
    {
        return true;
    }

    bool EatMe(Rabbit* r) override
    {
        return false;
    }

    bool EatMe(Wolf* w) override
    {
        return false;
    }

    bool EatMe(Dino* r) override
    {
        return false;
    }

    bool EatMe(Weasel* w) override
    {
        return false;
    }
public:
    Dino(const sf::Vector2i& loc);

    void Routine(int generation) override;

    FBITS ToBits() override
    {
        return Animal::ToBits(TDINO);
    }
};

class Weasel :public Animal, public ICarnivore, public IHerbivore
{
private:
	bool Eat() override;
    void Move() override;
    bool Chase() override;
    bool Evade() override;

    void EatWheat();

    bool FearMe(Wolf* w) override
    {
        return false;
    }

    bool FearMe(Rabbit* r) override
    {
        return true;
    }

    bool FearMe(Dino* w) override
    {
        return false;
    }

    bool FearMe(Weasel* r) override
    {
        return false;
    }

    bool EatMe(Rabbit* r) override
    {
        return false;
    }

    bool EatMe(Wolf* w) override
    {
        return true;
    }

    bool EatMe(Dino* r) override
    {
        return true;
    }

    bool EatMe(Weasel* w) override
    {
        return false;
    }
public:
    Weasel(const sf::Vector2i& loc);

    void Routine(int generation) override;

    FBITS ToBits() override
    {
        return Animal::ToBits(TWEASEL);
    }
};

class Rabbit :public Animal, public IHerbivore
{
protected:
	bool Eat() override;
    void Move() override;
	bool Evade() override;

	bool FearMe(Wolf* w) override
	{
		return false;
	}

	bool FearMe(Rabbit* r) override
	{
		return false;
	}

    bool FearMe(Dino* w) override
    {
        return false;
    }

    bool FearMe(Weasel* r) override
    {
        return false;
    }

	bool EatMe(Rabbit* r) override
	{
		return false;
	}

	bool EatMe(Wolf* w) override
	{
		return true;
	}

    bool EatMe(Dino* w) override
    {
        return true;
    }

    bool EatMe(Weasel* w) override
    {
        return true;
    }
public:
    Rabbit(const sf::Vector2i& loc);

    void Routine(int generation) override;

    FBITS ToBits() override
    {
        return Animal::ToBits(TRABBIT);
    }
};