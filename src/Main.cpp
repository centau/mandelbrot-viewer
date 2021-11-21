#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>

const int THREAD_COUNT = 24;

/*
void render(fractal* fract, fractalType fract_type, sf::Image*, int startRows, int endRows) {
      sf::Image image;
	image.create(fract.size, fract.size, sf::Color(0, 0, 0));
      
      for (int screenY = 0; screenY < fract.size; screenY++) {
            long double pi = frameToComplexCoord(screenY, fract, fract.y);

            for (int screenX = 0; screenX < fract.size; screenX++) {
                  long double pr = frameToComplexCoord(screenX, fract, fract.x);

                  int i = fract_type == fractalType::mandelbrot ?
                        testStability(pr, pi, fract.imax) :
                        testStability(fract.zr, fract.zi, fract.imax, pr, pi);

                  image.setPixel(screenX, screenY, colorPixel(i, fract.imax));
            }
      }

      fract.texture.loadFromImage(image);
}
*/

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

void render(fractal* fract, fractalType fract_type, sf::Image* image, int startRows, int endRows) {  
      for (int screenY = startRows; screenY < endRows; screenY++) {
            long double pi = frameToComplexCoord(screenY, *fract, fract -> y);

            for (int screenX = 0; screenX < fract -> size; screenX++) {
                  long double pr = frameToComplexCoord(screenX, *fract, fract -> x);

                  int i = fract_type == fractalType::mandelbrot ?
                        testStability(pr, pi, fract -> imax) :
                        testStability(fract -> zr, fract -> zi, fract -> imax, pr, pi);

                  image -> setPixel(screenX, screenY, colorPixel(i, fract -> imax));
            }
      }
}

void renderFractal(fractal& fract, fractalType type) {
      sf::Image image;
	image.create(fract.size, fract.size, sf::Color(0, 0, 0));

      std::thread threads[THREAD_COUNT] = {};
	int rowsPerThread = fract.size/THREAD_COUNT;

	for (int i = 0; i < THREAD_COUNT; i++) {
		int targetRows = (i+1)*rowsPerThread;

		if (i == THREAD_COUNT-1) {
			targetRows = fract.size;
		}

		threads[i] = std::thread(render, &fract, type, &image, i*rowsPerThread, targetRows);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
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

      bool paused = false;

      sf::RenderWindow window(sf::VideoMode(width, height),
		"Mandelbrot Visualizer",
		sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize
	);

      window.setFramerateLimit(60);

      // Mandelbrot display
      fractal mandelbrot(height);

      // Julia display
      fractal julia(height);

      // Initialize
      mandelbrot.frame.setPosition(width/2, 0);
      renderFractal(mandelbrot, fractalType::mandelbrot);

      fractal* activefractal = &mandelbrot;

      sf::Vector2<int> mouseScreenPos(0, 0);
      sf::Vector2<int> mouseScreenPos0;

      // Runtime
      while (window.isOpen()) {
            bool draw_all = false;
            bool draw = false;
            bool inFocus = window.hasFocus();
            sf::Event event;
            mouseScreenPos0 = mouseScreenPos;
            mouseScreenPos = sf::Mouse::getPosition(window);
            
            sf::Vector2<long double> mousePlanePos = screenToComplexCoords(mouseScreenPos, activefractal? *activefractal : mandelbrot);

            std::cout << mouseScreenPos.x << ", " << mouseScreenPos.y << "\n";

            if (paused == false) {
                  julia.zr = mousePlanePos.x;
                  julia.zi = mousePlanePos.y;    
            }

            if (isMouseInFrame(mouseScreenPos, mandelbrot.frame)) {
                  activefractal = &mandelbrot;
            } else if (isMouseInFrame(mouseScreenPos, julia.frame)) {
                  activefractal = &julia;
            } else {
                  activefractal = NULL;
            }

            while (window.pollEvent(event)) {
                  using sf::Event;
                  switch(event.type) {
                        case Event::Closed:
                              window.close();
                              break;
                        case Event::MouseWheelMoved: {
                              if (activefractal == NULL) break;
					int delta = event.mouseWheel.delta;
					activefractal -> magnification *= delta >= 1 ? 1.5l * delta : (1.0l / (1.5l * abs(delta)));
					draw = true;
					break;
                        } case Event::MouseButtonPressed: {
                              if (activefractal == NULL) break;
					activefractal -> x = mousePlanePos.x;
					activefractal -> y = mousePlanePos.y;
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

                              draw_all = true;
                              break;
                        } case Event::KeyPressed: {
                              if (activefractal == NULL) break;
                              using sf::Keyboard;
                              switch(event.key.code) {
                                    case Keyboard::Key::Space:
                                          paused = !paused;
                                          draw_all = true;
                                          break;
                                    case Keyboard::Key::R:
                                          activefractal -> x = 0.0l;
                                          activefractal -> y = 0.0l;
                                          activefractal -> magnification = 1.0f;
                                          activefractal -> imax = 100;
                                          draw = true;
                                          break;
                                    case Keyboard::Key::Z:
                                          activefractal -> imax += 1;
                                          activefractal -> imax *= 1.1;
                                          draw = true;
                                          break;
                                    case Keyboard::Key::X: 
                                          if (activefractal -> imax > 1)
                                                activefractal -> imax /= 1.1;
                                          draw = true;
                                          break;
                                    default: break;
                              }   
                        } default: break;
                  }
            }

            if (draw_all == true || (draw == true and activefractal == &mandelbrot)) 
                  renderFractal(mandelbrot, fractalType::mandelbrot);
            if (draw_all == true || (draw == true and activefractal == &julia) || (mouseScreenPos != mouseScreenPos0 and paused == false))
                  renderFractal(julia, fractalType::julia);

            // Render
            window.clear();
            window.draw(julia.frame);
            window.draw(mandelbrot.frame);
            window.display();
      }

      // Terminating
      std::cout << "TERMINATING\n";
      return 0;
}