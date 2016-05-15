// entity.cpp
// author:  Maciej Szpakowski
// Wheat implementation, several methods for entity and animals

#include "proto.h"

Entity::Entity(int metabolicRate, int initFoodLevel, int maxFoodLevel,
    const sf::Vector2i& loc) :
    age(0), metabolism(initFoodLevel,maxFoodLevel,metabolicRate),
    location(loc)
{
}

void Entity::Grow()
{
    age++;
    metabolism.ProcessFood();
}

std::queue<unsigned short> Animal::ids;

Animal::Animal(int _metabolicRate, int initFoodLevel, int _maxFoodLevel,
    int _matureAge, int _reprocudeChance, int _foodToReproduce,
    int _ageToDie, int meat,
    const sf::Vector2i& loc) :
    Entity(_metabolicRate, initFoodLevel, _maxFoodLevel, loc),
    prevLoc(loc), lastGenActivity(-1), matureAge(_matureAge), 
    reproduceChance(_reprocudeChance), foodToReproduce(_foodToReproduce),
    ageToDie(_ageToDie), meatVal(meat)
{
    if (Animal::ids.size() == 0)
        throw std::runtime_error("ids queue empty");

    id = Animal::ids.front();
    Animal::ids.pop();

    if (id >= WIDTH * HEIGHT)
        throw std::runtime_error("invalid id");
}

void Entity::BuildAdjCross()
{
    adjFields.clear();
    adjFields.push_back({ GetLocation().x - 1, GetLocation().y });
    adjFields.push_back({ GetLocation().x + 1, GetLocation().y });
    adjFields.push_back({ GetLocation().x, GetLocation().y - 1 });
    adjFields.push_back({ GetLocation().x, GetLocation().y + 1 });
}

void Animal::BuildAdjSquare(int edge)
{
    adjFields.clear();
    for (int i = -edge; i <= edge; i++)
        for (int j = -edge; j <= edge; j++)
            adjFields.push_back(GetLocation() + sf::Vector2i(j, i));
}

void Animal::ShuffleAdj()
{
    std::random_shuffle(adjFields.begin(), adjFields.end()); // make it random
}

void Animal::Die()
{
    if (age >= ageToDie)
        throw ExKill();
    else if (metabolism.HasStarved())
        throw ExKill();
}

FBITS Animal::ToBits(unsigned char type)
{
    FBITS bits;
    bits.type = type;
    bits.food = metabolism.GetFoodLevel();
    bits.age = age;
    bits.id = id;
    return bits;
}

sf::Vector2i* Animal::GetBirthPlace()
{
    // not enough food or age
    if (age < matureAge || metabolism.GetFoodLevel() < foodToReproduce)
        return nullptr;

    World* world = World::GetWorld();
    
    if (prevLoc == GetLocation())
        return nullptr;

    // roll die
    if (rand() % 100 + 1 > reproduceChance)
        return nullptr;

    return &prevLoc;
}

void Animal::_PutCreature(Animal* a, sf::Vector2i* birthPlace)
{
    World::GetWorld()->PutCreature(a, *birthPlace);
}

bool Animal::Step(const sf::Vector2i& dst)
{
	if (dst == GetLocation())
		return true;

	World* world = World::GetWorld();

	sf::Vector2i dir(dst - GetLocation()); // displacement vector

	// displacement is diagonal then you can go either up/down or to the sides
	sf::Vector2i stepa(0,0);
	sf::Vector2i stepb(0,0);

	if (dir.x != 0)
		stepa.x = dir.x / (int)labs(dir.x);

	if (dir.y != 0)
		stepb.y = dir.y / (int)labs(dir.y);

	try 	// try moving one way
	{
		world->MoveCreature(this, GetLocation() + stepa);
		return true;
	}
	catch (NotEmpty)
	{
	}

	try 	// try moving another
	{
		world->MoveCreature(this, GetLocation() + stepb);
		return true;
	}
	catch (NotEmpty)
	{
	}

	return false;
}

bool Animal::MoveTo(const sf::Vector2i& dst)
{
	while (dst != GetLocation())
	{
		if (!Step(dst))
			return false;
	}

	return true;
}

void Wheat::Spawn()
{
	World* world = World::GetWorld();

	// roll die
	if (GetFoodLevel() > 50 && rand() % 100 < 5)
	{
		BuildAdjCross();

		auto it = std::find_if(adjFields.begin(), adjFields.end(), [&](auto& e) {
			try
			{
				return !world->GetGrass(e)->HasWheat();
			}
			catch (ArgumentOutOfBounds)
			{
				return false;
			}
		});

		if (it != adjFields.end())
			world->GetGrass(*it)->SpawnWheat();
	}
}