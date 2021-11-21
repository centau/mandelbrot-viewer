#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

// Structs
enum fractalType {
      mandelbrot,
      julia
};

struct fractal {
      sf::Sprite frame;
      sf::Texture texture;

      int size;

      long double x = 0.0l;
      long double y = 0.0l;

      long double zr = 0.0l;
      long double zi = 0.0l;

      float magnification = 1.0f;
      int imax = 100;
      float bounds = 2.0f;

      fractal(int x): size(x) {
            texture.create(size, size);
            frame.setTexture(texture);
      }
};

sf::Image colorImage() {
      sf::Image x;
      return x;
}

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

long double frameToComplexCoord(int x0, fractal& fract, long double origin) {
      return ( (long double)x0/fract.size * fract.bounds*2 - fract.bounds ) / fract.magnification + origin;
}

sf::Vector2<long double> screenToComplexCoords(sf::Vector2<int> mousePos, fractal& fract) {
      sf::Rect<int> bounds(fract.frame.getGlobalBounds());
      return sf::Vector2<long double>(
            frameToComplexCoord(mousePos.x - bounds.left, fract, fract.x),
            frameToComplexCoord(mousePos.y - bounds.top, fract, fract.y)
      );      
}

bool isMouseInFrame(sf::Vector2<int> mousePos, sf::Sprite& frame) {
      sf::Rect<int> bounds(frame.getGlobalBounds());
      return bounds.contains(mousePos);
}

int testStability(const long double cr, const long double ci, int imax, long double zr = 0.0l, long double zi = 0.0l) {
      long double ztmp;
      int i = 0;

      while (i < imax && zr*zr + zi*zi < 4.0l) {
            ztmp = zr;
            zr = zr*zr - zi*zi + cr;
            zi = ztmp*zi + zi*ztmp + ci;
            i++;
      }     

      return i; 
}

void renderFractal(fractal& fract, fractalType fract_type) {
      sf::Image image;
	image.create(fract.size, fract.size, sf::Color(0, 0, 0));
      
      for (int screenY = 0; screenY < fract.size; screenY++) {
            long double pi = frameToComplexCoord(screenY, fract, fract.y);

            for (int screenX = 0; screenX < fract.size; screenX++) {
                  long double pr = frameToComplexCoord(screenX, fract, fract.x);

                  int i = fract_type == fractalType::mandelbrot ?
                        testStability(pr, pi, fract.imax) :
                        testStability(fract.zr, fract.zi, fract.imax, pr+fract.x, pi+fract.y);

                  image.setPixel(screenX, screenY, colorPixel(i, fract.imax));
            }
      }

      fract.texture.loadFromImage(image);
}

void resizeFractal(fractal& fract, int newsize) {
      fract.size = newsize;
      fract.texture.create(newsize, newsize);
      fract.frame.setTexture(fract.texture, true);
}

int main() {
      std::cout << "RUNNING\n";

      int width = 1000;
      int height = 500;

      sf::RenderWindow window(sf::VideoMode(width, height),
		"Mandelbrot Visualizer",
		sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize
	);

      // Mandelbrot display
      fractal mandelbrot(height);

      // Julia display
      fractal julia(height);

      // Initializec
      mandelbrot.frame.setPosition(width/2, 0);
      renderFractal(mandelbrot, fractalType::mandelbrot);

      //julia.frame.setOrigin(width, 0);

      // Other
      fractal* activeFractal;

      // Runtime
      while (window.isOpen()) {
            bool draw = false;
            sf::Event event;
            sf::Vector2<int> mouseScreenPos = sf::Mouse::getPosition(window);
            sf::Vector2<long double> mousePlanePos = screenToComplexCoords(mouseScreenPos, mandelbrot);

            julia.zr = mousePlanePos.x;
            julia.zi = mousePlanePos.y;

            renderFractal(julia, fractalType::julia);

            if (isMouseInFrame(mouseScreenPos, mandelbrot.frame)) {
                  activeFractal = &mandelbrot;
            } else if (isMouseInFrame(mouseScreenPos, julia.frame)) {
                  activeFractal = &julia;
            } else {
                  activeFractal = NULL;
            }

            std::cout << width << ", " << height << "\n";

            while (window.pollEvent(event)) {
                  using sf::Event;
                  switch(event.type) {
                        case Event::Closed:
                              window.close();
                              break;
                        case Event::MouseWheelMoved: {
					int delta = event.mouseWheel.delta;
					mandelbrot.magnification *= delta >= 1 ? 1.5l * delta : (1.0l / (1.5l * abs(delta)));
					std::cout << "Magnification: " << mandelbrot.magnification << "\n";
					draw = true;
					break;
                        } case Event::MouseButtonPressed: {
					mandelbrot.x = mousePlanePos.x;
					mandelbrot.y = mousePlanePos.y;
					draw = true;
					break;
                        } case Event::Resized: {
                              width = event.size.width;
                              height = event.size.height;
                              sf::FloatRect view(0, 0, width, height);
                              window.setView(sf::View(view));

                              resizeFractal(mandelbrot, height);
                              resizeFractal(julia, height);

                              mandelbrot.frame.setPosition(width/2, 0);
                              julia.frame.setOrigin(height, 0);
                              julia.frame.setPosition(width/2, 0);

                              draw = true;
                        } default: break;
                  }
            }

            if (draw == true) renderFractal(mandelbrot, fractalType::mandelbrot);

            window.clear();
            window.draw(julia.frame);
            window.draw(mandelbrot.frame);
            window.display();
      }

      // Terminating
      std::cout << "TERMINATING\n";
      return 0;
}