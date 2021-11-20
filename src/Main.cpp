#include <SFML/Graphics.hpp>
#include <iostream>

// Mandelbrot display
sf::Sprite mandelbrot_frame;

sf::Texture mandelbrot;

mandelbrot_frame.resize();

/*
mandelbrot.create(WIDTH, HEIGHT);
mandelbrot_frame.setTexture(texture);
mandelbrot_frame.setOrigin(WIDTH/2, HEIGHT/2);
mandelbrot_frame.setPosition(WIDTH/2, HEIGHT/2);
mandelbrot_frame.setScale(1, 1);
mandelbrot.loadFromImage(renderImage());
*/

// Julia display
sf::Sprite julia;

// Constants
const int HEIGHT = 1000;

// Mutable
long double originX = 0.0l,
            originY = 0.0l;

float magnification = 1.0f;
float bounds = 2.0f;
int max_iterations = 100;

// Functions
float norm(float min, float max, float z) {
	return (z - min)/(max-min);
}

float d(float min, float max, float z) {
	return z*(max-min) + min;
}

sf::Color colorPixel(int i, int imax) {
	float x = (float)i / imax;
	int c = x*255;

	if (x >= 0.9) {
		float n = norm(0.9, 1, x);
		return sf::Color(d(0, 255, n), c, 255);
	} else if (x > 0.2) {
		float n = norm(0.2, 0.9, x);
		return sf::Color(0, d(51, 229.5, n) , d(204, 255, n));
	} else {
		float n = norm(0, 0.2, x);
		return sf::Color(d(0, 51, 1-n), d(0, 51, n), c*3 +51);
	}
}

long double getRelativeX(int x0) {
      return ( (float)x0/HEIGHT*4 - bounds + originX ) / magnification;
}

long double getRelativeY(int y0) {
      return ( (float)y0/HEIGHT*4 - bounds + originY ) / magnification;
}

// Render
sf::Image renderMandelbrot() {
      sf::Image image;
	image.create(HEIGHT, HEIGHT, sf::Color(0, 0, 0));

      for (int screenY = 0; screenY < HEIGHT; screenY++) {
            long double cr = getRelativeY(screenY);

            for (int screenX = 0; screenX < HEIGHT; screenX++) {
                  long double ci = getRelativeX(screenX);
                  long double zr = 0,
                              zi = 0,
                              ztmp;
                  int i = 0;

                  while (i < 100 && zr*zr + zi*zi < 4.0l) {
                        ztmp = zr;
				zr = zr*zr - zi*zi + cr;
				zi = ztmp*zi + zi*ztmp + ci;
				i++;
                  }

                  image.setPixel(screenX, screenY, colorPixel(i, max_iterations));
            }
      }
}

int main() {
      std::cout << "RUNNING\n";

      sf::RenderWindow window(sf::VideoMode(HEIGHT, HEIGHT),
		"Mandelbrot Visualizer",
		sf::Style::Close | sf::Style::Titlebar
	);

      while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                  using sf::Event;
                  switch(event.type) {
                        case Event::Closed:
                              window.close();
                              break;
                        default: break;
                  }
            }

            window.clear();
            window.display();
      }

      std::cout << "TERMINATING\n";
      return 0;
}