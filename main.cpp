#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>

#define SCALE 2.0f
#define ATTRACTOR_MASS 500.0f
#define ATTRACTOR_RADIUS 10.0f
#define G 100.0f // Gravitational constant in arbitrary units
#define SOFTENING 50.0f // Softening factor to prevent singularity at zero distance




//Calculate the magnitude and return a normalized vector in the same direction as the input vector.
sf::Vector2f normalize(sf::Vector2f vector)
{
    double magnitude = std::sqrt(std::pow(vector.x, 2) + std::pow(vector.y, 2));
    if (magnitude == 0) return {0.f, 0.f};
    return {static_cast<float>(vector.x / magnitude), static_cast<float>(vector.y / magnitude)};
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

template <typename T, typename U>
sf::Vector2f calculateDirection(T object, U point)
{
    sf::Vector2f direction = point.getPosition() - object.getPosition();
    return normalize(direction);
}

template <typename T>
void update(T &object, sf::Vector2f force, float delta_time)
{
    object.setAcceleration(force / static_cast<float>(object.getMass()));
    object.setVelocity(object.getVelocity() + object.getAcceleration() * delta_time);
    object.setPosition(object.getPosition() + object.getVelocity() * delta_time);

}



class body
{
    private:

    double mass;
    sf::Vector2f acceleration;
    sf::Vector2f velocity;
    sf::Vector2f position;
    sf::CircleShape shape;

    public:

    //Constructor for body Class
    body(double bodyMass, sf::Vector2f bodyPosition, sf::Vector2f initialVelocity = {0.0f, 50.0f})
    : mass(bodyMass), position(bodyPosition), acceleration({0.0f, 0.0f}), velocity(initialVelocity)
    {
        shape.setFillColor(sf::Color::White);
        float ratio = mass / ATTRACTOR_MASS; // Calculate the ratio of the body's mass to the attractor's mass
        float radius = ATTRACTOR_RADIUS/std::cbrt(log10(ratio) + 1); // Scale the radius based on the mass, using a logarithmic scale to prevent excessively large or small sizes
        shape.setRadius(radius);
        shape.setOrigin({radius,radius}); // Set the origin to the center of the circle
        shape.setPosition(position);
    }

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

    sf::CircleShape getShape() const
    {
        return shape;
    }

    //setters
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
    sf::Vector2f position = {0.0f, 0.0f};
    sf::CircleShape shape;

    public:

    // Constructor for the attractor class
    attractor(sf::Vector2f pos) : position(pos)
    {   
        shape.setFillColor(sf::Color::Red);
        shape.setRadius(ATTRACTOR_RADIUS);
        shape.setOrigin({ATTRACTOR_RADIUS, ATTRACTOR_RADIUS}); // Set the origin to the center of the circle
        shape.setPosition(position);
    }

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

};

int main()
{
    //Create array for bodies and attractors
    std::vector<body> bodies;
    std::vector<attractor> attractors;

    //Create random number generator for body mass
    //Used logarithmic distribution so big planets are more unlikely to appear
    //logDis will generate numbers between 10^-4 and 10^1, thats why [-6.0, -3.0] is used as parameters, then we will power 10 to the generated number to get the mass of the body
    std::random_device rd;
    std::mt19937 gen(1);
    std::uniform_real_distribution<float> logDis(-2.0f, 1.0f);


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

        for(int j = 0; j < bodies.size(); j++)
        {
            sf::Vector2f totalForce = {0.0f, 0.0f};
            for(int i = 0; i < attractors.size(); i++)
            {
                sf::Vector2f force = calculateForce(bodies[j], attractors[i]);
                totalForce += force;
            }
            update(bodies[j], totalForce, delta_time);
            std::cout << "Body " << j << " acceleration: " << bodies[j].getAcceleration().x << ", " << bodies[j].getAcceleration().y << std::endl;
        }

        for(int i = 0; i < attractors.size(); i++)
        {
            for(int j = 0; j < bodies.size(); j++)
            {
                std::array line ={
                sf::Vertex{sf::Vector2f(bodies[j].getPosition())},
                sf::Vertex{sf::Vector2f(attractors[i].getPosition())}
                };

                window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
            }
        }

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





















/*OG CODE

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <random>

const double G = 5000.0f; // Gravitational constant (arbitrary units)
const float SOFTENING = 200.0f; // Softening factor to prevent singularity at zero distance


template <typename T, typename U>

//Template to facilitate calculation between any given two points regardless of type, as long as they have getPosition() and getMass() methods.
//Template to calculate the direction from a given object to a point, returns a normalized vector.
sf::Vector2f calculateDirection(T object, U point)
{
    sf::Vector2f direction = point.getPosition() - object.getPosition();
    float magnitude = std::sqrt(std::pow(direction.x, 2) + std::pow(direction.y, 2));
    
    if (magnitude == 0) return {0.f, 0.f};
    return direction / magnitude;
}


template <typename T, typename U>
sf::Vector2f calculateForce(T object, U point)
{
    double distance = sqrt(pow(point.getPosition().x - object.getPosition().x, 2) + pow(point.getPosition().y - object.getPosition().y, 2));
    sf::Vector2f force_dir = calculateDirection(object, point);
    float force_magnitude = (G * point.getMass() * object.getMass()) / (distance * distance + SOFTENING);
    std::cout << "force magnitude: " << force_magnitude << " distance: " << distance  << "acceleration: " << force_magnitude / object.getMass() << "mass: " << object.getMass() << std::endl;
    if(distance < 0.1f) return {0.f, 0.f};
    return sf::Vector2f(force_dir.x * force_magnitude, force_dir.y * force_magnitude);
}

//Body class to represent any moving object, planets, meteors, etc.
class body
{
    private:
        float mass;
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f acceleration;
        sf::CircleShape shape;
    
    public: 

    //default constructor
    body(float m = 50, sf::Vector2f pos = {50.0f,50.0f}, sf::Vector2f initial_velocity = {10.0f,10.0f}, sf::Vector2f acc = {0.0f,0.0f}) : mass(m), position(pos), velocity(initial_velocity), acceleration(acc)
    {
        float radius = std::sqrt(mass) / 2.0f; // Scale down the radius for better visualization
        shape.setRadius(radius);
        shape.setPosition(position);
        shape.setOrigin(sf::Vector2f(radius, radius));
        shape.setFillColor(sf::Color::White);
    }

    sf::Vector2f getVelocity()
    {
        return velocity;
    }

    sf::Vector2f getAcceleration()
    {
        return acceleration;
    }

    sf::Vector2f getPosition()
    {
        return position;
    }

    float getMass()
    {
        return mass;
    }

    //method used to update the position and velocity of the body based on the total force applied and the time elapsed since the last update.
    void update(sf::Vector2f force, float delta_time)
    {
        acceleration = force / mass;
        velocity += acceleration * delta_time;
        position += velocity * delta_time;
        shape.setPosition(position);
    }

    void draw(sf::RenderWindow& window) 
    {
    window.draw(shape);
    }
};

//Class to represent places of gravity, such as stars. Represented by a red circle of a given mass.

class gravity_point
{
    private:
    sf::Vector2f position;
    float mass;
    sf::CircleShape shape;

    public: 
    gravity_point(float m = 10.0f, sf::Vector2f pos = {500.0f, 500.0f}) : position(pos), mass(m)
    {
        float radius = std::sqrt(mass) / 2.0f; // Scale down the radius for better visualization
        shape.setRadius(radius);
        shape.setPosition(position);
        shape.setOrigin(sf::Vector2f(radius, radius));
        shape.setFillColor(sf::Color::Red);
    }

    sf::Vector2f getPosition()
    {
        return position;
    }

    float getMass()
    {
        return mass;
    }

    void draw(sf::RenderWindow& window) 
    {
    window.draw(shape);
    }
};



int main()
{
    //Initialization of the window, clock, vectors, and random number generator for the mass of the bodies.
    //Resolution affects simulation 
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Gravity Simulation");
    sf::Clock clock;
    std::vector<body> bodies;
    std::vector<gravity_point> gravity_points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(50.0f, 200.0f);

    while (window.isOpen()) {
        
        //Calculate the time elapsed since the last frame, which is used to ensure that the simulation runs at a consistent speed regardless of the frame rate.
        float delta_time = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            //Check for mouse button press
            //if left click is press, a gravity point appears
            //if right click is press, a body appears with a random mass and an initial velocity of 150 in the y direction.
            if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>()) {

                if(mouseClick->button == sf::Mouse::Button::Left)
                {   
                    gravity_points.push_back(gravity_point(1000.0f, sf::Vector2f(mouseClick->position)));
                }

                if(mouseClick->button == sf::Mouse::Button::Right)
                {
                    sf::Vector2i click_pos = mouseClick->position;
                    bodies.push_back(body(dis(gen), sf::Vector2f(click_pos), {0.0f, 150.0f}, {0.0f, 0.0f}));
                }

            }
        }


        for(int i = 0; i < bodies.size(); i++)
        {
            sf::Vector2f total_force = {0.0f, 0.0f};
            for(int j = 0; j < gravity_points.size(); j++)
            {
                total_force += calculateForce(bodies[i], gravity_points[j]);
            }

            for (int k = i + 1; k < bodies.size(); k++) 
            {
            sf::Vector2f f = calculateForce(bodies[i], bodies[k]);
            bodies[i] += f;
            bodies[k] -= f; // Tercera Ley de Newton: Ahorras el 50% del cálculo
            }

            bodies[i].update(total_force, delta_time);
        }
        
        
        window.clear();
        for(body& first_body : bodies)
        {
            first_body.draw(window);
        }
        for(auto& first_gravity : gravity_points)
        {
            first_gravity.draw(window);
        }
        window.display();

        
    }

    



}

*/