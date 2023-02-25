/*
  Vulkanインスタンスの生成を行う
*/

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

// MoltenVKサポート用のコードを有効/無効にする
#define SUPPORT_MOLTENVK 1

class Application {
 public:
  static constexpr char NAME[] = "01_instance";
  static constexpr uint32_t WIDTH = 960;
  static constexpr uint32_t HEIGHT = 570;

 private:
  SDL_Window* window;
  VkInstance instance;

 public:
  void run(const std::vector<std::string>& args) {
    initialize();
    loop();
    finalize();
  }

 private:
  void initialize() {
    initializeWindow();
    initializeVulkan();
  }

  void initializeWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }

    window = SDL_CreateWindow(
        NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  void initializeVulkan() {
    // *Infoと名付けられた構造体は、言わば関数への引数をまとめたもの
    // VkApplicationInfoは、アプリケーションの情報を指定する構造体
    VkApplicationInfo appInfo{};

    // 構造体には毎度対応するsTypeを指定しなければならない
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    // 構造体を拡張する際に用いる。基本的にはnullptr
    appInfo.pNext = nullptr;
    // アプリケーション名 (任意)
    appInfo.pApplicationName = NAME;
    // アプリケーションのバージョン (任意)
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    // エンジン名 (任意)
    appInfo.pEngineName = "None";
    // エンジンバージョン (任意)
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    // 使用するAPIのバージョン
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // VulkanとSDLは、エクステンションを通じてやりとりするので、SDLからVulkanインスタンスに登録すべきエクステンションのリストを取得する
    unsigned int sdlExtensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);

    std::vector<const char*> extensionNames(sdlExtensionCount);
    VkInstanceCreateFlags flags = 0;

    extensionNames.resize(sdlExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount,
                                     extensionNames.data());

#if SUPPORT_MOLTENVK
    // MoltenVKに対応する場合は、以下のエクステンションとフラグを指定する必要がある
    extensionNames.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensionNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    std::cout << "# Enabled extensions:" << std::endl;

    for (auto&& ext : extensionNames) {
      std::cout << "| " << ext << std::endl;
    }

    // インスタンスの設定
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    // インスタンスのフラグ
    instanceInfo.flags = flags;
    // アプリケーションの情報
    instanceInfo.pApplicationInfo = &appInfo;
    // レイヤーは使用しない
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    // 必要なエクステンションを登録する
    instanceInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensionNames.size());
    instanceInfo.ppEnabledExtensionNames = extensionNames.data();

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("vkCreateInstance() failed; result: " +
                               std::to_string(result));
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

  void finalize() {
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }
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
    std::cerr << "\x1b[31m" << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}