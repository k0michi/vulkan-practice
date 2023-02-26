/*
 * バリデーションレイヤーを有効にする
 */

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "console.hh"

// MoltenVKサポート用のコードを有効/無効にする
#define SUPPORT_MOLTENVK 1
// バリエーションレイヤー有効/無効
#define ENABLE_VALIDATION 1

class Application {
 public:
  static constexpr char NAME[] = "02_validation";
  static constexpr uint32_t WIDTH = 960;
  static constexpr uint32_t HEIGHT = 570;

 private:
  SDL_Window* window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

 public:
  void run(const std::vector<std::string>& args) {
    initialize();
    loop();
    finalize();
  }

 private:
  void initialize() {
    initializeSDL();
    initializeVulkan();
  }

  void initializeSDL() {
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
    // 構造体を拡張する際に用いる
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

    VkInstanceCreateFlags instanceFlags = getInstanceFlags();
    showInstanceFlags(instanceFlags);

    showAvailableExtensions(getAvailableExtensions());

    std::vector<const char*> extensionNames = getRequiredExtensions();
    showRequiredExtensions(extensionNames);

    showAvailableLayers(getAvailableLayers());

    std::vector<const char*> layerNames = getRequiredLayers();
    showRequiredLayers(layerNames);

    const void* instanceNext = nullptr;

#if ENABLE_VALIDATION
    // バリデーションレイヤーの出力をコールバックで受け取る為には、VkDebugUtilsMessengerを作らなければならない
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
    messengerInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.pNext = nullptr;
    messengerInfo.flags = 0;
    messengerInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = &messageCallBack;
    messengerInfo.pUserData = nullptr;

    // エクステンションとレイヤーを登録するだけでは、インスタンス生成/削除時のバリデーションが行われない
    // インスタンス生成/削除時のバリデーションを有効にするには、VkInstanceCreateInfoのpNextにVkDebugUtilsMessengerCreateInfoEXTへのポインターを与える
    instanceNext = reinterpret_cast<void*>(&messengerInfo);
#endif

    // インスタンスの設定
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = instanceNext;
    // インスタンスのフラグ
    instanceInfo.flags = instanceFlags;
    // アプリケーションの情報
    instanceInfo.pApplicationInfo = &appInfo;
    // 有効化するレイヤーを登録する
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    instanceInfo.ppEnabledLayerNames = layerNames.data();
    // 有効化するエクステンションを登録する
    instanceInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensionNames.size());
    instanceInfo.ppEnabledExtensionNames = extensionNames.data();

    // vkCreateInstance(pCreateInfo, pAllocator, pInstance)
    // 今回はアロケータを使用しない (nullptr)
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("vkCreateInstance() failed; result: " +
                               std::to_string(result));
    } else {
      std::cout << Console::fgGreen << "# "
                << "vkCreateInstance() succeeded" << Console::fgDefault
                << std::endl;
    }

#if ENABLE_VALIDATION
    // vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator,
    // pMessenger)
    // VkDebugUtilsMessengerを作る
    // この関数は自動的にロードされないので、アプリケーション側でロードしなければならない
    // (vulkan_ext.cc参照)
    vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr,
                                   &debugMessenger);
#endif
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  messageCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {
    const char* color = "";

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      color = Console::fgYellow;
    } else if (messageSeverity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      color = Console::fgRed;
    }

    std::cerr << color << "$ " << pCallbackData->pMessage << Console::fgDefault
              << std::endl;
    return VK_FALSE;
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
    finalizeVulkan();
    finalizeSDL();
  }

  void finalizeVulkan() {
#if ENABLE_VALIDATION
    // vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator)
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

    // vkDestroyInstance(instance, pAllocator)
    vkDestroyInstance(instance, nullptr);
  }

  void finalizeSDL() {
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  VkInstanceCreateFlags getInstanceFlags() {
    VkInstanceCreateFlags flags = 0;

#if SUPPORT_MOLTENVK
    // MoltenVKに対応する場合は、VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHRフラグを指定する
    flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    return flags;
  }

  void showInstanceFlags(VkInstanceCreateFlags flags) {
    std::cout << "# Instance flags: " << flags << std::endl;
  }

  // 利用可能なエクステンションを取得する
  std::vector<VkExtensionProperties> getAvailableExtensions() {
    uint32_t availableExtensionCount;
    // vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount,
    // pProperties)
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount,
                                           nullptr);

    std::vector<VkExtensionProperties> availableExtensions(
        availableExtensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount,
                                           availableExtensions.data());
    return availableExtensions;
  }

  void showAvailableExtensions(
      const std::vector<VkExtensionProperties>& extensions) {
    std::cout << "# Available extensions:" << std::endl;

    for (auto&& ext : extensions) {
      std::cout << "| " << ext.extensionName << " ["
                << versionToString(ext.specVersion) << "]" << std::endl;
    }
  }

  // 必要となるエクステンションを取得する
  std::vector<const char*> getRequiredExtensions() {
    // VulkanとSDLは、エクステンションを通じてやりとりするので、SDLからVulkanインスタンスに登録すべきエクステンションのリストを取得する
    unsigned int sdlExtensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);

    std::vector<const char*> extensionNames(sdlExtensionCount);

    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount,
                                     extensionNames.data());

#if SUPPORT_MOLTENVK
    // MoltenVKに対応する場合は、以下のエクステンションが必要となる
    extensionNames.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensionNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

#if ENABLE_VALIDATION
    extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensionNames;
  }

  void showRequiredExtensions(const std::vector<const char*>& extensions) {
    std::cout << "# Required extensions:" << std::endl;

    for (auto&& ext : extensions) {
      std::cout << "| " << ext << std::endl;
    }
  }

  // 利用可能なレイヤーを取得する
  std::vector<VkLayerProperties> getAvailableLayers() {
    uint32_t layerCount;
    // vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties)
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    return availableLayers;
  }

  void showAvailableLayers(const std::vector<VkLayerProperties>& layers) {
    std::cout << "# Available layers:" << std::endl;

    for (auto&& layer : layers) {
      std::cout << "| " << layer.layerName << " ["
                << versionToString(layer.specVersion) << ", "
                << versionToString(layer.implementationVersion) << ", "
                << layer.description << "]" << std::endl;
    }
  }

  // 必要なレイヤーを取得する
  std::vector<const char*> getRequiredLayers() {
    std::vector<const char*> layerNames;

#if ENABLE_VALIDATION
    layerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif

    return layerNames;
  }

  void showRequiredLayers(const std::vector<const char*>& layers) {
    std::cout << "# Required layers:" << std::endl;

    for (auto&& layer : layers) {
      std::cout << "| " << layer << std::endl;
    }
  }

  std::string versionToString(uint32_t version) {
    return std::to_string(VK_VERSION_MAJOR(version)) + "." +
           std::to_string(VK_VERSION_MINOR(version)) + "." +
           std::to_string(VK_VERSION_PATCH(version));
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
    std::cerr << Console::fgRed << "# " << e.what() << Console::fgDefault
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}