// wolf.cpp
// author:  Maciej Szpakowski
// Wolf classs implementation

#include "proto.h"

Wolf::Wolf(const sf::Vector2i& loc) :
    Animal(
        2,   // metabolic rate
        150, // init food 
        200, // max food
        30,  // mature age
        7,  // reproduce chance
        120, // min food to reproduce
        70,  // age to die
		60,  // meat value
        loc)
{
}

void Wolf::Routine(int generation)
{
	if (lastGenActivity != -1 && lastGenActivity == generation)
		return;
	lastGenActivity = generation;

	prevLoc = GetLocation();

    if (metabolism.GetFoodLevel() < 40) // run chase routine
        Chase();
    else if (metabolism.UntilFull() <= 10) // roam
        Move();
    else if (!Eat()) // eat
        Move();

	Reproduce<Wolf>();
	Grow();
	Die();

	if (age >= ageToDie || metabolism.HasStarved())
		throw std::runtime_error("zombie animal");
}

// in case of wolf move just moved randomly one step
// up,down,right or left
void Wolf::Move()
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

// eat prey right next to it
// returns true if there was something to eat
bool Wolf::Eat()
{
	World* world = World::GetWorld();

	auto predicate = [&](const sf::Vector2i pos)
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

    BuildAdjCross();
	ShuffleAdj();
	auto dst = std::find_if(adjFields.begin(), adjFields.end(), predicate); // find meat

	if (dst != adjFields.end()) // meat found so consume it
	{
		metabolism.AddFood(world->GetCreature(*dst)->GetMeatValue());
		world->Kill(*dst);
		world->MoveCreature(this, *dst);
		return true;
	}
	else
		return false;
}

// increase search area and give more moves
// return no longer used
bool Wolf::Chase()
{
	World* world = World::GetWorld();

    // 3 space square
	BuildAdjSquare(3);
    ShuffleAdj();

    auto predicate = [&](const sf::Vector2i pos)
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

    // find meat in 3 space square
    auto it = std::find_if(adjFields.begin(), adjFields.end(), predicate);
    sf::Vector2i dst;
        
    if (it != adjFields.end()) // meat found
    {
        dst = *it;
        Step(dst); //take first step
        if (!Eat())
        {
            Step(dst); //take second step
            Eat();
        }
    }
    else                     // meat not found
        Move();

    return true;
}