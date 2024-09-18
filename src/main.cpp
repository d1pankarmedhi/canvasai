#include <SFML/Graphics.hpp>
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "base64.h" // Include the base64 library

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const sf::Color BACKGROUND_COLOR = sf::Color::White;
const sf::Color DRAWING_COLOR = sf::Color::Black;
const float BRUSH_SIZE = 2.0f;

class Button
{
public:
    Button(sf::Vector2f position, sf::Vector2f size, const std::string &text)
    {
        shape.setPosition(position);
        shape.setSize(size);
        shape.setFillColor(sf::Color::Green);

        font.loadFromFile("fonts/Arial.ttf"); // Make sure you have this font file or change to an available font
        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::Black);

        sf::FloatRect textBounds = label.getLocalBounds();
        label.setPosition(
            position.x + (size.x - textBounds.width) / 2,
            position.y + (size.y - textBounds.height) / 2);
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(shape);
        window.draw(label);
    }

    bool isMouseOver(sf::RenderWindow &window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        return shape.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

private:
    sf::RectangleShape shape;
    sf::Text label;
    sf::Font font;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Drawing Canvas");
    sf::RenderTexture canvas;
    canvas.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    canvas.clear(BACKGROUND_COLOR);

    Button saveButton(sf::Vector2f(WINDOW_WIDTH - 220, 10), sf::Vector2f(100, 30), "Save PNG");
    Button uploadButton(sf::Vector2f(WINDOW_WIDTH - 110, 10), sf::Vector2f(100, 30), "Upload");

    sf::CircleShape brush(BRUSH_SIZE);
    brush.setFillColor(DRAWING_COLOR);

    bool isDrawing = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (saveButton.isMouseOver(window))
                    {
                        sf::Texture texture = canvas.getTexture();
                        sf::Image screenshot = texture.copyToImage();
                        screenshot.saveToFile("drawing.png");
                        std::cout << "Drawing saved as drawing.png" << std::endl;
                    }
                    else if (uploadButton.isMouseOver(window))
                    {
                        // Read the saved image file
                        std::ifstream file("drawing.png", std::ios::binary);
                        if (!file)
                        {
                            std::cout << "Failed to open drawing.png" << std::endl;
                            continue;
                        }
                        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
                        file.close();

                        // Encode the image to base64 using the base64 library
                        std::string base64_image = base64_encode(buffer.data(), buffer.size());

                        // Prepare form data
                        std::string prompt = "A beautiful drawing"; // You can modify this or get user input
                        std::string post_fields = "image=" + base64_image + "&prompt=" + prompt;

                        // Initialize libcurl
                        CURL *curl = curl_easy_init();
                        if (curl)
                        {
                            // Set the URL to post to
                            curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/upload"); // Replace with your actual URL

                            // Set POST data
                            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

                            // Perform the request
                            CURLcode res = curl_easy_perform(curl);

                            // Check for errors
                            if (res != CURLE_OK)
                            {
                                std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                            }
                            else
                            {
                                std::cout << "Image uploaded successfully!" << std::endl;
                            }

                            // Clean up
                            curl_easy_cleanup(curl);
                        }
                    }
                    else
                    {
                        isDrawing = true;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    isDrawing = false;
                }
            }

            if (event.type == sf::Event::MouseMoved)
            {
                if (isDrawing)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    brush.setPosition(mousePos.x - BRUSH_SIZE, mousePos.y - BRUSH_SIZE);
                    canvas.draw(brush);
                }
            }
        }

        window.clear(BACKGROUND_COLOR);
        window.draw(sf::Sprite(canvas.getTexture()));
        saveButton.draw(window);
        uploadButton.draw(window);
        window.display();
    }

    return 0;
}