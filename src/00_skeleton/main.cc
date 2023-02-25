#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>

class Application {
 public:
  const char* TITLE = "00_skeleton";
  uint32_t WIDTH = 960;
  uint32_t HEIGHT = 570;

 private:
  SDL_Window* window;

 public:
  void run(const std::vector<std::string>& args) {
    initialize();
    loop();
    finalize();
  }

 private:
  void initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }

    window = SDL_CreateWindow(
        TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  void loop() {
    SDL_Event event;
    bool shouldQuit = false;

    while (!shouldQuit) {
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            shouldQuit = true;
            break;
        }
      }

      // Render code here
    }
  }

  void finalize() { SDL_Quit(); }
};

int main(int argc, char* argv[]) {
  Application app;

  try {
    std::vector<std::string> args(argc);

    for (int i = 0; i < argc; i++) {
      args[i] = argv[i];
    }

    app.run(args);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}