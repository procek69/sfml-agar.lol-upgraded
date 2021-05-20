#include <iostream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

std::vector<sf::Color> colors;

struct enemy : sf::CircleShape
{
    float dx;
    float dy;
    float velocity = 1.f;
    bool dirty = false;

    enemy()
    {
        resetColor();
    }

    bool hit(const sf::CircleShape& player)
    {
        if (dirty) return false;
        float distance = sqrt(pow(player.getPosition().x + player.getRadius() - getPosition().x - getRadius(), 2) + pow(player.getPosition().y + player.getRadius() - getPosition().y - getRadius(), 2));
        return getRadius() + player.getRadius() > distance;
    }

    void randRadius(const sf::CircleShape& player)
    {
        setRadius(rand() % (int)player.getRadius() + (int)(player.getRadius() * 0.2));
    }

    void move()
    {
        sf::Vector2<float> pos = getPosition();
        pos.x += dx * velocity;
        pos.y += dy * velocity;
        setPosition(pos);
    }

    void resetPosition(int W, int H)
    {
        velocity = 0.7f + (rand() % 20 / 10.f);
        const auto r = getRadius();
        sf::Vector2f start({ 0, 0 });
        sf::Vector2f end({ 0, 0 });
        if (rand() % 2)
        {
            start.x = r + rand() % (int)(W - 2 * r);
            end.x = r + rand() % (int)(W - 2 * r);
            start.y = r;
            end.y = H - r;
            if (rand() % 2)
            {
                std::swap(start.y, end.y);
            }
        }
        else
        {
            start.y = r + rand() % (int)(H - 2 * r);
            end.y = r + rand() % (int)(H - 2 * r);
            start.x = r;
            end.x = W - r;
            if (rand() % 2)
            {
                std::swap(start.x, end.x);
            }
        }
        
        sf::Vector2f dir = end - start;
        float length = sqrt(pow(dir.x, 2) + pow(dir.y, 2));
        sf::Vector2f unit({ dir.x / length, dir.y / length });
        dx = unit.x;
        dy = unit.y;
        setPosition(start);
    }

    void resetColor()
    {
        setFillColor(colors[rand() % colors.size()]);
    }

    bool isOutside(int W, int H) {
        const auto r = getRadius();
        const auto pos = getPosition();
        if (pos.x + 3 * r < 0) return true;
        if (pos.y + 3 * r < 0) return true;
        if (pos.x - 3 * r > W) return true;
        if (pos.y - 3 * r > H) return true;
        return false;
    }
};

template<typename T>
struct deleter : std::unary_function<const T*, void>
{
    void operator() (const T* ptr) const
    {
        delete ptr;
    }
};

int minimumLimit = 10;
int currentLimit = minimumLimit;
int maximumLimit = 20;
int main()
{
    colors.push_back(sf::Color(255, 204, 0));
    colors.push_back(sf::Color(51, 204, 204));
    colors.push_back(sf::Color(153, 51, 102));
    colors.push_back(sf::Color(153, 204, 0));
    colors.push_back(sf::Color(0, 204, 255));
    colors.push_back(sf::Color(255, 128, 128));

    int W = 1024;
    int H = 768;

    srand(time(nullptr));
    sf::RenderWindow window(sf::VideoMode(W, H), "agar.lol clone");
    sf::CircleShape player(10.f);
    player.setFillColor(sf::Color::Green);
    window.setMouseCursorVisible(false);


    std::vector<enemy*> enemies;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        std::cout << "arial.ttf not found" << std::endl;
        return 1;
    }
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(30);
    text.setFillColor(sf::Color::White);
    bool lose = false;
    int points = 0;

    bool backgroundThreadWorking = true;
    sf::Thread thread([=]() {
        while (backgroundThreadWorking)
        {
            if (currentLimit > minimumLimit)
            {
                currentLimit = currentLimit - 1;
                sf::sleep(sf::milliseconds(1000));
            }
        }
    });
    thread.launch();

    int currentEnemies = 0;
    while (window.isOpen())
    {
        std::cout << currentEnemies << "/" << currentLimit << std::endl;
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.key.code == 13)
            {
                lose = false;
                points = 0;
                for (auto e : enemies)
                    delete e;
                enemies.clear();
                currentEnemies = 0;
                player.setRadius(10.f);
                window.setMouseCursorVisible(false);
            }
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
        int radius = player.getRadius();
        worldPos.x -= radius;
        worldPos.y -= radius;
        player.setPosition(worldPos);

        if (currentEnemies < currentLimit)
        {
            enemy* ed = nullptr;
            for (auto e : enemies)
            {
                if (e->dirty)
                    ed = e;
            }
            if (ed != nullptr)
            {
                ed->dirty = false;
                ed->randRadius(player);
                ed->resetPosition(W, H);
            }
            else
            {
                enemy* e = new enemy();
                e->randRadius(player);
                e->resetPosition(W, H);
                enemies.push_back(e);
            }
            currentEnemies++;
        }

        if (lose)
        {
            window.setMouseCursorVisible(true);
            text.setString("You lose: " + std::to_string(points) + " points. Press ENTER to restart");
            sf::FloatRect textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width / 2.f, textRect.top + textRect.height / 2.f);
            text.setPosition(sf::Vector2f(W / 2.f, H / 2.f));
        }
        else
        {
            for (int i = 0; i < enemies.size(); i++)
            {
                enemy*& e = enemies[i];
                e->move();
                if (e->hit(player))
                {
                    
                    if (e->getRadius() >= player.getRadius())
                    {
                        lose = true;
                    }
                    else
                    {
                        points++;
                        currentLimit++;
                        if (currentLimit > maximumLimit)
                        {
                            currentLimit = maximumLimit;
                        }
                        player.setRadius(player.getRadius() + 1);
                    }
                    e->dirty = true;
                    currentEnemies--;
                }
                if (e->isOutside(W, H)) {
                    e->dirty = true;
                    currentEnemies--;
                }
            }
        }


        window.clear();
        for (auto e : enemies) {
            if (!e->dirty)
                window.draw(*e);
        }

        if (lose)
        {
            window.draw(text);
        }
        else
        {
            window.draw(player);
        }
        window.display();
    }
    backgroundThreadWorking = false;
    std::for_each(enemies.begin(), enemies.end(), deleter<enemy>());
    enemies.clear();
    return 0;
}