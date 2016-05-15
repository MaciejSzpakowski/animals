// main.cpp
// author:  Maciej Szpakowski
// main, keyboard and mouse handlers

#include "proto.h"

// keyboard control, space for pausing and arrows for scrolling
void keyboard(sf::Event::KeyEvent event, sf::Text& txt)
{
    if (event.code == sf::Keyboard::Space)
        World::GetWorld()->TogglePlay();
    else if (event.code == sf::Keyboard::Left)
        World::GetWorld()->Back();
    else if (event.code == sf::Keyboard::Right)
        World::GetWorld()->Forward();    
}

// generatees statistics for the field
// this will deserialize a field
// format:
//       [ grass food ] [ type ] [ animal food ] [ age ] [ id ] [ wheat level ]
//       0            6 7      9 10           19 20   27 28  43 44           50
std::string stat(sf::Event::MouseMoveEvent& mouse)
{
    auto cur = sf::Vector2i(mouse.x, mouse.y);

    FBITS bits = World::GetWorld()->GetInt({ cur.x / CELL,cur.y / CELL });

    std::string format(
        "Pos[%d,%d]\n"
        "Grass: %d\n");

    format += "Wheat: %d\n";

    if (bits.type != 0)
        format +=
        "%s #%d\n"
        "Age: %d\n"
        "Food: %d";

    char cres[1000];
    const char* names[] = { "Wolf","Raptor","Weasel","Rabbit","???" };
    const char* animalName;

    switch (bits.type)
    {
    case 1:
        animalName = names[0];
        break;
    case 2:
        animalName = names[1];
        break;
    case 3:
        animalName = names[2];
        break;
    case 4:
        animalName = names[3];
        break;
    default:
        animalName = names[4];
        break;
    }

    sprintf(cres, format.c_str(), cur.x / CELL, cur.y / CELL, bits.grass, 
        bits.wheat, animalName, bits.id, bits.age, bits.food);
    
    return std::string(cres);
}

// draws the text in the info box
void drawText(const sf::Text& txt, sf::RenderWindow& wnd)
{
    static sf::RectangleShape rect;
    rect.setFillColor({ 0,0,0 });
    auto bnds = txt.getGlobalBounds();
    rect.setSize({ bnds.width + 20,bnds.height + 20 });
    rect.setPosition({ bnds.left - 10,bnds.top - 10 });

    if (txt.getString()[0] != 0)
    {
        wnd.draw(rect);
        wnd.draw(txt);
    }
}

// draw info box for field when world is paused
void mouse(sf::Event::MouseMoveEvent& event, sf::Text& txt, sf::RenderWindow& wnd)
{
    if (World::GetWorld()->IsPaused() && World::GetWorld()->IsStarted())
    {
        txt.setString(stat(event));

        sf::Vector2f pos = { (float)event.x + 20,(float)event.y + 20 };

        if (pos.x + txt.getGlobalBounds().width + 40 > wnd.getSize().x)
            pos.x = event.x - txt.getGlobalBounds().width - 20;
        if (pos.y + txt.getGlobalBounds().height + 40 > wnd.getSize().y)
            pos.y = event.y - txt.getGlobalBounds().height - 20;

        txt.setPosition(pos);
    }
}

int main()
{
    try
    {
        // create the window
        sf::RenderWindow window(sf::VideoMode(WIDTH * CELL, HEIGHT * CELL), "Wildlife");
        window.setPosition({ 0,0 });

        auto res = Resources::GetResources();
        res->AddTexture(FRABBIT);
        res->AddTexture(FWOLF);
        res->AddTexture(FDINO);
        res->AddTexture(FWEASEL);
		res->AddTexture(FWHEAT);

        World* world = World::GetWorld();

        sf::Text text;
        sf::Font font;
        font.loadFromFile("consola.ttf");
        text.setCharacterSize(14);
        text.setFont(font);
        text.setPosition({ 100,100 });
        text.setColor({ 255,255,255 });
        text.setString("Wild Life by Maciej Szpakowski\n"
                       "\n"
                       "Space:            Pause/Play\n"
                       "Left/Right arrow: next/previous generation\n"
                       "                  (in pause mode only)\n\n"
                       "             ***REMARK***\n"
                       "every generation costs ~10KB to store\n\n"
                       "          Press SPACE to start");

        // run the program as long as the window is open
        while (window.isOpen())
        {
            // check all the window's events that were triggered since the last iteration of the loop
            if (POLL)
            {
                sf::Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                        window.close();

                    // process key strokes
                    else if (event.type == sf::Event::KeyPressed)
                        keyboard(event.key, text);

                    // process mouse
                    else if (event.type == sf::Event::MouseMoved)
                        mouse(event.mouseMove, text, window);
                }
            }

            // clear the window with black color
            window.clear(sf::Color::Black);
                        
            world->Activity();
            world->DrawAll(window);
            if (!world->IsPaused())
                text.setString("");
            drawText(text,window);

            // end the current frame
            window.display();
        }
    }
    /*catch (std::runtime_error& e)
    {
        printf("%s\n", e.what());
        getchar();
    }*/
    catch (int)
    {
    }

    return 0;
}