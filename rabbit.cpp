// rabbit.cpp
// author:  Maciej Szpakowski
// Rabbit classs implementation

#include "proto.h"

Rabbit::Rabbit(const sf::Vector2i& loc) :
    Animal(
        3,  // metabolic rate
        10, // init food 
        45, // max food
        10, // mature age
        70, // reproduce chance 50
        40, // min food to reproduce
        25, // age to die 25
		10, // meat value
        loc)
{
}

void Rabbit::Routine(int generation)
{
	if (lastGenActivity != -1 && lastGenActivity == generation)
		return;
	lastGenActivity = generation;

	prevLoc = GetLocation();

	Move();
	if (!Evade()) // eat and reproduce if it didn't have to evade
	{
		Eat();
		Reproduce<Rabbit>();
	}
	Grow();
	Die();

	if (age >= ageToDie || metabolism.HasStarved())
		throw std::runtime_error("zombie animal");
}

// rabbit looks for highest grass and moves there
void Rabbit::Move()
{
    World* world = World::GetWorld();
    BuildAdjCross();
	ShuffleAdj();

    // find heighest grass
    int max = 0;
    sf::Vector2i* dst = nullptr;
    for (auto& pos : adjFields)
    {
		try
		{
            // break on wheat because wheat is the best
            if (world->IsEmpty(pos) && world->GetGrass(pos)->HasWheat())
            {
                dst = &pos;
                break;
            }
			if (world->IsEmpty(pos) && world->GetGrass(pos)->GetFoodLevel() > max)
			{
				max = world->GetGrass(pos)->GetFoodLevel();
				dst = &pos;
			}
		}
		catch (ArgumentOutOfBounds)
		{
		}
    }

    // not found
    if (dst == nullptr)
        return;

    world->MoveCreature(this, *dst);
}

// eats where it stands
// return value is not used
bool Rabbit::Eat()
{
    Grass* grassUnderMyFeet = World::GetWorld()->GetGrass(GetLocation());

    // check for wheat
    if (grassUnderMyFeet->HasWheat())
        metabolism.AddFood(grassUnderMyFeet->EatWheat());

    // make sure not to eat more than there is and no more than 5
    int food = grassUnderMyFeet->GetFoodLevel();
    if (food > 5)
        food = 5;

    // extra 1 for every 20 levels
    food += grassUnderMyFeet->GetFoodLevel() / 20;

    // dont eat more than you can
    if (metabolism.UntilFull() < food)
        food = metabolism.UntilFull();

    metabolism.AddFood(food);
    grassUnderMyFeet->SubFood(food);

	return true;
}

// evades predator thats is left,roght,up or down from this
bool Rabbit::Evade()
{
    if (metabolism.GetFoodLevel() <= 30)
        return false;

    World* world = World::GetWorld();

	BuildAdjCross();

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
		// dst is 2 fields in the opposite direction
        try
        {
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