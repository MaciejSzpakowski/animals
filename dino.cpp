// dino.cpp
// author:  Maciej Szpakowski
// Dino classs implementation

#include "proto.h"

Dino::Dino(const sf::Vector2i& loc) :
    Animal(
        6,   // metabolic rate
        250, // init food 
        600, // max food
        40,  // mature age
        3,  // reproduce chance
        350, // min food to reproduce
        180,  // age to die
		300,  // meat value
        loc)
{
}

// dino always chases and has greater range
void Dino::Routine(int generation)
{
    if (lastGenActivity != -1 && lastGenActivity == generation)
        return;
    lastGenActivity = generation;

    prevLoc = GetLocation();

    if (metabolism.UntilFull() > 20)
        Chase();
    else
        Move();

    Reproduce<Dino>();
    Grow();
    Die();

    if (age >= ageToDie || metabolism.HasStarved())
        throw std::runtime_error("zombie animal");
}

// dino move is moving randomly one space
void Dino::Move()
{
	World* world = World::GetWorld();

    auto predicate = [&](sf::Vector2i& e)
    {
        try
        {
            return world->IsEmpty(e);
        }
        catch (ArgumentOutOfBounds)
        {
            return false;
        }
    };

	BuildAdjCross();
	ShuffleAdj();

	auto dstIt = std::find_if(adjFields.begin(), adjFields.end(), predicate);

	if (dstIt != adjFields.end())
	{
		world->MoveCreature(this, *dstIt);
	}
}

bool Dino::Eat()
{
	World* world = World::GetWorld();

	auto predicate = [&](const sf::Vector2i& pos)
	{
        try
        {
            return !world->IsEmpty(pos) && world->GetCreature(pos)->EatMe(this);
        }
        catch(ArgumentOutOfBounds)
        {
            return false;
        }
	};

    BuildAdjCross();
	ShuffleAdj();

	auto dst = std::find_if(adjFields.begin(), adjFields.end(), predicate);

	if (dst != adjFields.end())
	{
		metabolism.AddFood(world->GetCreature(*dst)->GetMeatValue());
		world->Kill(*dst);
		world->MoveCreature(this, *dst);
	}
	else
		Move();

    return true;
}

// return value no longer used
// this function calls eat
bool Dino::Chase()
{
	World* world = World::GetWorld();

    BuildAdjSquare(4);
    ShuffleAdj();

    auto predicate = [&](const sf::Vector2i& pos)
    {
        try
        {
            return !world->IsEmpty(pos) && world->GetCreature(pos)->EatMe(this);
        }
        catch (ArgumentOutOfBounds)
        {
            return false;
        }
    };

    // find meat in square
    auto dst = std::find_if(adjFields.begin(), adjFields.end(), predicate);

    // meat found
    if (dst != adjFields.end())
    {
        MoveTo(*dst);
        Eat();
    }
    else
        Move();

	return true;
}