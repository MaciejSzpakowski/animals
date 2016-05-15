// weasel.cpp
// author:  Maciej Szpakowski
// Weasel classs implementation

#include "proto.h"

Weasel::Weasel(const sf::Vector2i& loc) :
    Animal(
        3,  // metabolic rate
        50, // init food 
        70, // max food
        25, // mature age
        7, // reproduce chance
        40, // min food to reproduce
        60, // age to die
		10, // meat value
        loc)
{
}

// routine for weasel is mostly like rabbit
// except that it chases except just eating
void Weasel::Routine(int generation)
{
    if (lastGenActivity != -1 && lastGenActivity == generation)
        return;
    lastGenActivity = generation;

    prevLoc = GetLocation();

    Move();
    if (!Evade())
    {
        Chase();
        Reproduce<Weasel>();
    }
    Grow();
    Die();

    if (age >= ageToDie || metabolism.HasStarved())
        throw std::runtime_error("zombie animal");
}

// move is roaming aimlessly in a bigger square
void Weasel::Move()
{
	World* world = World::GetWorld();

    BuildAdjSquare(2); // move anywhere in 2 radius square
	ShuffleAdj();

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

	auto it = std::find_if(adjFields.begin(), adjFields.end(), predicate);

	if (it != adjFields.end())
        MoveTo(*it);
}

bool Weasel::Eat()
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

    BuildAdjSquare(1);
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

bool Weasel::Evade()
{
    World* world = World::GetWorld();

    BuildAdjSquare(1);
    ShuffleAdj();

    auto predicate = [&](sf::Vector2i& e) // look for predators
    {
        try
        {
            return !world->IsEmpty(e) && world->GetCreature(e)->FearMe(this);
        }
        catch (ArgumentOutOfBounds)
        {
            return false;
        }
    };

    auto it = std::find_if(adjFields.begin(), adjFields.end(), predicate);

    if (it != adjFields.end()) // if predator found
    {        
        try
        {
            // dst is 2 fields in the opposite direction
            sf::Vector2i dst = GetLocation() + (GetLocation() - *it) * 2;
            MoveTo(dst);
        }
        catch (ArgumentOutOfBounds) // prevent rabbit running of the screen
        {
        }
        return true;
    }
    else
        return false;
}

void Weasel::EatWheat()
{
    World* world = World::GetWorld();

    auto predicate = [&](const sf::Vector2i pos)
    {
        try
        {
            return world->IsEmpty(pos) && world->GetGrass(pos)->HasWheat();
        }
        catch (ArgumentOutOfBounds)
        {
            return false;
        }
    };

    // find wheat in 3 space square
    auto it = std::find_if(adjFields.begin(), adjFields.end(), predicate);

    if (it != adjFields.end()) // wheat found
    {
        MoveTo(*it);
        Grass* grassUnderMyFeet = world->GetGrass(GetLocation());
        if (grassUnderMyFeet->HasWheat())
            metabolism.AddFood(grassUnderMyFeet->EatWheat() / 20 + 1);
    }
    else                     // wheat not found
        Move();
}

bool Weasel::Chase()
{
    World* world = World::GetWorld();

    // 3 space square
    BuildAdjSquare(2);
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
    
    if (it != adjFields.end()) // meat found
    {
        MoveTo(*it);
        Eat();
    }
    else                     // meat not found
        EatWheat();

    return true;
}