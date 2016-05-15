// world.cpp
// author:  Maciej Szpakowski
// World classs implementation

#include "proto.h"

World::World():
    tickCount(milliseconds(TICKMS)), curGeneration(0),
    paused(true), started(false), lastTick(std::chrono::duration_cast<milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()))
{
    srand((int)time(0));

    for (auto it = grass.begin(); it != grass.end(); ++it)
        *it = new Grass(it.getpos());

    for (auto it = creatures.begin(); it != creatures.end(); ++it)
        *it = nullptr;

    // add ids to Animal::ids
    Animal::InitIds();

    SpawnCreatures<Rabbit>(RABBIT_COUNT);
    SpawnCreatures<Wolf>(WOLF_COUNT);
    SpawnCreatures<Weasel>(WEASEL_COUNT);
    SpawnCreatures<Dino>(DINO_COUNT);

    // serialize initial state
    AddGeneration();
}

// this will deserialize a field
// format:
//       [ grass food ] [ type ] [ animal food ] [ age ] [ id ] [ wheat level ]
//       0            6 7      9 10           19 20   27 28  43 44           50
void World::DrawAll(sf::RenderWindow& wnd)
{
    sf::RectangleShape grass({ 32,32 });
	sf::RectangleShape wheat({ 32,32 });
    sf::RectangleShape animal({ 32,32 });

    sf::Vector2f pos;
    int index = 0;

    // deserialize and draw grass and wheat
    for (FBITS bits : allGenerations[curGeneration])
    {
        pos.x = (float)(index % (WIDTH)*CELL);
        pos.y = (float)(index / WIDTH*CELL);

        // the more food grass has the greener the field
        grass.setFillColor(sf::Color(0, 55 + bits.grass * 2, 0));
        grass.setPosition({ pos.x,pos.y });
        wnd.draw(grass);

        if (bits.wheat > 0)
        {
            wheat.setPosition({ pos.x,pos.y });
            wheat.setTexture(Resources::GetResources()->GetTexture(FWHEAT));
            wnd.draw(wheat);
        }

        index++;
    }

    index = 0;

    // deserialize and draw animals
    for (FBITS bits : allGenerations[curGeneration])
    {
        pos.x = (float)(index % (WIDTH)*CELL);
        pos.y = (float)(index / WIDTH*CELL);

        // 0 means there's no animal here
        if (bits.type == 0)
        {
            index++;
            continue;
        }

        switch (bits.type)
        {            
        case TWOLF:
        {
            animal.setTexture(Resources::GetResources()->GetTexture(FWOLF));
            if(bits.food < 40)
                animal.setFillColor({ 255,100,100 });  // angry wolf
            else
                animal.setFillColor({ 170,170,200 });  // normal wolf
            break;
        }
        case TDINO:
        {
            animal.setTexture(Resources::GetResources()->GetTexture(FDINO));
            animal.setFillColor({ 255, 0, 0 }); // dino color
            break;
        }
        case TWEASEL:
        {
            animal.setTexture(Resources::GetResources()->GetTexture(FWEASEL));
            animal.setFillColor({ 0, 100, 150 });  // weasel color
            break;
        }
        case TRABBIT:
        {
            animal.setTexture(Resources::GetResources()->GetTexture(FRABBIT));
            animal.setFillColor({ 255,255,255 });   // rabbit color
            break;
        }
        default: // error occurred
            throw std::runtime_error("unknown animal type");
        }

        animal.setPosition({ pos.x,pos.y });
        wnd.draw(animal);

        index++;
    }

    // print info in window's title
    std::string str("Wildlife. Generation ");
    str += std::to_string(curGeneration);
    str += "/";
    str += std::to_string(allGenerations.size() - 1);
    if (paused)
        str += " ***PAUSED***";
    wnd.setTitle(str);
}

// puts animal on the grid
void World::PutCreature(Animal* animal, const sf::Vector2i& dst)
{
    if (creatures.at(dst) != nullptr)
		throw NotEmpty();

    creatures.at(dst) = animal;
}

// moves animal from where it is to dst
void World::MoveCreature(Animal* animal, const sf::Vector2i& dst)
{
	if (creatures.at(dst) != nullptr)
		throw NotEmpty();

    auto src = animal->GetLocation();
    creatures.at(src) = nullptr;
    creatures.at(dst) = animal;
    animal->SetLocation(dst,lockey);
}

void World::AddGeneration()
{
    allGenerations.push_back(std::array<FBITS, WIDTH*HEIGHT>());
    int i = 0;

    auto& last = *(allGenerations.end() - 1);

    for (Grass* g : grass)
        last[i++] = g->ToBits();

    i = 0;
    for (Animal* a : creatures)
    {
        if (a != nullptr)
        {
            FBITS aniBits = a->ToBits();
            last[i].age = aniBits.age;
            last[i].food = aniBits.food;
            last[i].id = aniBits.id;
            last[i].type = aniBits.type;
        }
        i++;
    }

    curGeneration = (int)allGenerations.size() - 1;
}

void World::NextGeneration()
{
    for (Grass* g : grass)
        g->Routine(curGeneration);

    for (Animal* a : creatures)
    {
        if (a != nullptr)
        {
            try
            {
                a->Routine(curGeneration);
            }
            catch (ExKill)
            {
                Kill(a);
            }
        }
    }    
}

void World::Activity()
{
    auto now = std::chrono::duration_cast<milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    if (paused)
        lastTick = now; // if paused then synchornize so next generation won't be generated

    // if enough time has passed since last update
    if (now - lastTick > tickCount)
    {
        lastTick = now;
        NextGeneration();
        AddGeneration();
    }

}