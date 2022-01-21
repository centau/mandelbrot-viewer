#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <stdlib.h>
#include <Complex.hpp>

const int THREAD_COUNT = 24;

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

// Functions
float norm(float min, float max, float z) {
	return (z - min)/(max-min);
}

float d(float min, float max, float z) {
	return z*(max-min) + min;
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

const int mapN = 2;
sf::Color (*colormaps[mapN])(float i, int imax) = {
      [](float i, int imax) -> sf::Color {
            float x = i / imax;
            x = x > 1 ? 1 : x;
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
      },
      [](float i, int imax) -> sf::Color {
            float x = i/imax;
            int c = x*255;

            if (x >= 1) {
                  return sf::Color(0, 0, 0);
            } else if (x > 0.5) {
                  return sf::Color(c, 255, c);
            } else {
                  return sf::Color(0, c*2, 0);
            }
      },
};

const long double euler = 2.71828182845904523536028l;
const int escapeN = 3;
float (*escapeTests[escapeN])(const long double cr, const long double ci, int imax, long double zr, long double zi) = {
      [](const long double cr, const long double ci, int imax, long double zr = 0.0l, long double zi = 0.0l) -> float { // standard mandelbrot set
            register int i = 0;

            register long double zr2 = zr*zr,
                        zi2 = zi*zi;

            while (zr2 + zi2 <= 4.0l and i < imax) {
                  zi = 2*zr*zi + ci;
                  zr = zr2 - zi2 + cr;
                  zr2 = zr*zr;
                  zi2 = zi*zi;
                  i++;
            }     

            return i;
      },
      [](const long double cr, const long double ci, int imax, long double zr = 0.0l, long double zi = 0.0l) -> float { // the burning ship
            int i = 0;

            long double zr2 = zr*zr,
                        zi2 = zi*zi;

            while (zr2 + zi2 < 4.0l and i < imax) {
                  zi = abs(zr*zi*2) + ci;
                  zr = zr2 - zi2 + cr;
                  zr2 = zr*zr;
                  zi2 = zi*zi;
                  i++;
            }     

            return i;       
      },
      [](const long double cr, const long double ci, int imax, long double zr = 0.0l, long double zi = 0.0l) -> float { // 
            Complex c(cr, ci);
            Complex z(zr, zi);
            Complex one(1, 0);
            Complex tmp;
            int i = 0;

            while (z.modulusSqrd() < 4.0l and i < imax) {
                  tmp = z + c;
                  z = one/(tmp*tmp*tmp);
                  i++;
            }     

            return i;       
      },
};

bool isMouseInFrame(sf::Vector2<int> mousePos, sf::Sprite& frame) {
      sf::Rect<int> bounds(frame.getGlobalBounds());
      return bounds.contains(mousePos);
}

void render(fractal* fract, fractalType fract_type, sf::Image* image, int startRows, int endRows, int mapn, int escapen) {  
      for (int screenY = startRows; screenY < endRows; screenY++) {
            long double pi = frameToComplexCoord(screenY, *fract, fract -> y);

            for (int screenX = 0; screenX < fract -> size; screenX++) {
                  long double pr = frameToComplexCoord(screenX, *fract, fract -> x);

                  float i = fract_type == fractalType::mandelbrot ?
                        escapeTests[escapen](pr, pi, fract -> imax, 0.0l, 0.0l) :
                        escapeTests[escapen](fract -> zr, fract -> zi, fract -> imax, pr, pi);

                  image -> setPixel(screenX, screenY, colormaps[mapn](i, fract -> imax));
            }
      }
}

void renderFractal(fractal& fract, fractalType type, int mapn, int escapen) {
      sf::Image image;
	image.create(fract.size, fract.size, sf::Color(0, 0, 0));

      std::thread threads[THREAD_COUNT] = {};
	int rowsPerThread = fract.size/THREAD_COUNT;

	for (int i = 0; i < THREAD_COUNT; i++) {
		int targetRows = (i+1)*rowsPerThread;

		if (i == THREAD_COUNT-1) {
			targetRows = fract.size;
		}

		threads[i] = std::thread(render, &fract, type, &image, i*rowsPerThread, targetRows, mapn, escapen);
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

      bool paused = false;//

      int colormap = 0;
      int escapetest = 0;

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
      renderFractal(mandelbrot, fractalType::mandelbrot, colormap, escapetest);

      fractal* activefractal = &mandelbrot;

      sf::Vector2<int> mouseScreenPos(0, 0);
      sf::Vector2<int> mouseScreenPos0;

      // Runtime
      while (window.isOpen()) {
            bool draw_all = false;
            bool draw = false;
            // bsool inFocus = window.hasFocus();
            sf::Event event;
            mouseScreenPos0 = mouseScreenPos;
            mouseScreenPos = sf::Mouse::getPosition(window);
            
            sf::Vector2<long double> mousePlanePos = screenToComplexCoords(mouseScreenPos, activefractal? *activefractal : mandelbrot);

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
                                    case Keyboard::Key::C:
                                          colormap = (colormap+1)%mapN;
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
                                    case Keyboard::Key::V: 
                                          escapetest = (escapetest+1)%escapeN;
                                          draw_all = true;
                                          break;
                                    default: break;
                              }   
                        } default: break;
                  }
            }

            if (draw_all == true || (draw == true and activefractal == &mandelbrot)) 
                  renderFractal(mandelbrot, fractalType::mandelbrot, colormap, escapetest);
            if (draw_all == true || (draw == true and activefractal == &julia) || (mouseScreenPos != mouseScreenPos0 and paused == false))
                  renderFractal(julia, fractalType::julia, colormap, escapetest);

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