#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>
#include <deque>

#define SCALE 2.0f
#define ATTRACTOR_MASS 500.0f
#define ATTRACTOR_RADIUS 10.0f
#define G 1000.0f // Gravitational constant in arbitrary units
#define SOFTENING 50.0f // Softening factor to prevent singularity at zero distance

//Calculate the magnitude and return a normalized vector in the same direction as the input vector.
sf::Vector2f normalize(sf::Vector2f vector)
{
    double magnitude = std::sqrt(std::pow(vector.x, 2) + std::pow(vector.y, 2));
    if (magnitude == 0) return {0.f, 0.f};
    return {static_cast<float>(vector.x / magnitude), static_cast<float>(vector.y / magnitude)};
}

template <typename T, typename U>
sf::Vector2f calculateDirection(T object, U point)
{
    sf::Vector2f direction = point.getPosition() - object.getPosition();
    return normalize(direction);
}

template <typename T, typename U>
double calculateDistance(const T &object, const U &point)
{
    sf::Vector2f delta = point.getPosition() - object.getPosition();
    double r2 = delta.x * delta.x + delta.y * delta.y;
    r2 += SOFTENING * SOFTENING;

    return r2;
}

template <typename T, typename U>
//template to facilitate calculation between any given point regardless of the type.
sf::Vector2f calculateForce(T &object, U &point)
{
    sf::Vector2f dir = calculateDirection(object, point);

    double r2 = calculateDistance(object, point);

    double forceMagnitude =
        (G * object.getMass() * point.getMass()) / r2;

    return dir * (float)forceMagnitude;
}

template <typename T>
void update(T &object, float delta_time)
{
    object.setAcceleration(object.getForce() / static_cast<float>(object.getMass()));
    object.setVelocity(object.getVelocity() + object.getAcceleration() * delta_time);
    object.setPosition(object.getPosition() + object.getVelocity() * delta_time);
    object.resetForce(); // Reset force accumulator after each update
}

class body
{
    private:

    double mass;
    sf::Vector2f acceleration;
    sf::Vector2f velocity;
    sf::Vector2f position;
    sf::Vector2f force; // Acumulador de fuerza
    sf::CircleShape shape;

    public:

    //Constructor for body Class
    body(double bodyMass, sf::Vector2f bodyPosition, sf::Vector2f initialVelocity = {0.0f, 50.0f})
    : mass(bodyMass), position(bodyPosition), acceleration({0.0f, 0.0f}), velocity(initialVelocity), force({0.0f, 0.0f})
    {
        shape.setFillColor(sf::Color::White);
        float ratio = mass / ATTRACTOR_MASS; // Calculate the ratio of the body's mass to the attractor's mass
        float radius = ATTRACTOR_RADIUS/std::cbrt(log10(ratio) + 1); // Scale the radius based on the mass, using a logarithmic scale to prevent excessively large or small sizes
        shape.setRadius(radius);
        shape.setOrigin({radius,radius}); // Set the origin to the center of the circle
        shape.setPosition(position);
    }

    std::deque<sf::Vector2f> trail;

    //GETTERS

    double getMass() const
    {
        return mass;
    }

    sf::Vector2f getAcceleration() const
    {
        return acceleration;
    }

    sf::Vector2f getVelocity() const
    {
        return velocity;
    }

    sf::Vector2f getPosition() const
    {
        return position;
    }

    sf::Vector2f getForce() const
    {
        return force;
    }

    sf::CircleShape getShape() const
    {
        return shape;
    }

    //setters
    void addForce(sf::Vector2f f)
    {
        force += f;
    }

    void resetForce()
    {
        force = {0.0f, 0.0f};
    }

    void setAcceleration(sf::Vector2f newAcceleration)
    {
        acceleration = newAcceleration;
    }
    
    void setVelocity(sf::Vector2f newVelocity)
    {
        velocity = newVelocity;
    }

    void setPosition(sf::Vector2f newPosition)
    {
        position = newPosition;
        shape.setPosition(position);
    }

};

//Class to represent static attractors, such as stars. Represented by a red circle of a given mass.
class attractor 
{
    private:

    double mass = ATTRACTOR_MASS;
    sf::Vector2f acceleration;
    sf::Vector2f velocity;
    sf::Vector2f position;
    sf::Vector2f force; // Acumulador de fuerza
    sf::CircleShape shape;

    public:

    // Constructor for the attractor class
    attractor(sf::Vector2f pos) : position(pos), acceleration({0.0f, 0.0f}), velocity({0.0f, 0.0f}), force({0.0f, 0.0f})
    {   
        shape.setFillColor(sf::Color::Red);
        shape.setRadius(ATTRACTOR_RADIUS);
        shape.setOrigin({ATTRACTOR_RADIUS, ATTRACTOR_RADIUS}); // Set the origin to the center of the circle
        shape.setPosition(position);
    }

    std::deque<sf::Vector2f> trail;

    //GETTERS

    double getMass() const
    {
        return mass;
    }

    sf::Vector2f getPosition() const
    {
        return position;
    }

    sf::CircleShape getShape() const
    {
        return shape;
    }

    sf::Vector2f getAcceleration() const
    {
        return acceleration;
    }

    sf::Vector2f getVelocity() const
    {
        return velocity;
    }

    sf::Vector2f getForce() const
    {
        return force;
    }

    //SETTERS

    void addForce(sf::Vector2f f)
    {
        force += f;
    }

    void resetForce()
    {
        force = {0.0f, 0.0f};
    }

    void setPosition(sf::Vector2f newPosition)
    {
        position = newPosition;
        shape.setPosition(position);
    }

    void setAcceleration(sf::Vector2f newAcceleration)
    {
        acceleration = newAcceleration;
    }

    void setVelocity(sf::Vector2f newVelocity)
    {
        velocity = newVelocity;
    }

};

int main()
{
    //Create array for bodies and attractors
    std::vector<body> bodies;
    std::vector<attractor> attractors;

    //Create random number generator for body mass
    //Used logarithmic distribution so big planets are more unlikely to appear
    //logDis will generate numbers between 10^-4 and 10^1, thats why [-2.0, 1.0] is used as parameters, then we will power 10 to the generated number to get the mass of the body
    std::random_device rd;
    std::mt19937 gen(1);
    std::uniform_real_distribution<float> logDis(-2.0f, 1.0f);

    const int maxTrail = 2000;


    //Initialize window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Gravity Simulation");

    sf::Clock clock;

    while(window.isOpen())
    {

        float delta_time = clock.restart().asSeconds();

        //Polling events to check for any user input.
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            //Check if event is of type mouse button press, if not, returns nullopt.
            //if it is, checks if the left mouse button is pressed and creates a new attractor at the mouse position.
            if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()){

                if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    attractors.emplace_back(sf::Vector2f(mousePos));
                }

                if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    bodies.emplace_back(pow(10.0f, logDis(gen)), sf::Vector2f(mousePos));
                }
            }
        }

        
        window.clear(sf::Color(0X0B213C));

        
        for(int i = 0; i < bodies.size(); i++)
        {
            for(int j = 0; j < attractors.size(); j++)
            {
                sf::Vector2f f = calculateForce(bodies[i], attractors[j]);
                bodies[i].addForce(f);
                attractors[j].addForce(-f); // Newton's third law
            }
        }

        //Updating state of bodies and attractors
        for(int i = 0; i < bodies.size(); i++)
        {
            update(bodies[i], delta_time);
            bodies[i].trail.push_front(bodies[i].getPosition());
            if (bodies[i].trail.size() > maxTrail){
                bodies[i].trail.pop_back();
            }
        }

        for(int i = 0; i < attractors.size(); i++)
        {
            update(attractors[i], delta_time);
            attractors[i].trail.push_front(attractors[i].getPosition());
            if (attractors[i].trail.size() > maxTrail){
                attractors[i].trail.pop_back();
            }
        }

        //Drawing trails
        for (const auto& obj : bodies)
        {
            for (const auto& pos : obj.trail)
            {
                sf::CircleShape point(0.8f);
                point.setFillColor(sf::Color::Yellow);
                point.setPosition(pos);
                window.draw(point);
            }
        }

        for (const auto& obj : attractors)
        {
            for (const auto& pos : obj.trail)
            {
                sf::CircleShape point(0.8f);
                point.setFillColor(sf::Color::Yellow);
                point.setPosition(pos);
                window.draw(point);
            }
        }

        //Drawing bodies and attractors
        for(int i = 0; i < attractors.size(); i++)
        {
            window.draw(attractors[i].getShape());
        }

        for(int i = 0; i < bodies.size(); i++)
        {
            window.draw(bodies[i].getShape());
        }

        window.display();
    }

}