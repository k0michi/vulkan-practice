/*
 * バリデーションレイヤーを有効にする
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#define VULKAN_HPP_TYPESAFE_CONVERSION 0
#include <vulkan/vulkan_raii.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "console.hh"

// MoltenVKサポート用のコードを有効/無効にする
#define SUPPORT_MOLTENVK 1
// バリエーションレイヤー有効/無効
#define ENABLE_VALIDATION 1

class SDLApplication {
 public:
  SDLApplication() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  ~SDLApplication() { SDL_Quit(); }
};

class Application : public SDLApplication {
 public:
  static constexpr char NAME[] = "04_surface_hpp";
  static constexpr uint32_t WIDTH = 960;
  static constexpr uint32_t HEIGHT = 570;

 private:
  std::shared_ptr<SDL_Window> window;
  vk::raii::Context context;
  std::shared_ptr<vk::raii::Instance> instance;
  std::shared_ptr<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
  std::shared_ptr<vk::raii::SurfaceKHR> surface;
  std::shared_ptr<vk::raii::PhysicalDevice> physicalDevice;
  std::shared_ptr<vk::raii::Device> device;
  std::shared_ptr<vk::raii::Queue> graphicsQueue;
  std::shared_ptr<vk::raii::Queue> presentQueue;

 public:
  void run(const std::vector<std::string>& args) {
    initialize();
    loop();
    finalize();
  }

 private:
  void initialize() {
    initializeWindow();
    initializeInstance();
    initializeSurface();
    initializeDevice();
  }

  void initializeWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error(SDL_GetError());
    }

    window = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow(
            NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI),
        [](auto window) { SDL_DestroyWindow(window); });

    if (window == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  void initializeInstance() {
    // *Infoと名付けられた構造体は、言わば関数への引数をまとめたもの
    // vk::ApplicationInfoは、アプリケーションの情報を指定する構造体
    vk::ApplicationInfo appInfo{
        // アプリケーション名 (任意)
        /* pApplicationName = */ NAME,
        // アプリケーションのバージョン (任意)
        /* applicationVersion = */ VK_MAKE_VERSION(1, 0, 0),
        // エンジン名 (任意)
        /* pEngineName = */ "None",
        // エンジンバージョン (任意)
        /* engineVersion = */ VK_MAKE_VERSION(1, 0, 0),
        // 使用するAPIのバージョン
        /* apiVersion = */ VK_API_VERSION_1_0};

    vk::InstanceCreateFlags instanceFlags = getInstanceFlags();
    showInstanceFlags("Instance flags", instanceFlags);

    showExtensions("Available instance extensions",
                   context.enumerateInstanceExtensionProperties());

    std::vector<const char*> extensionNames = getRequiredExtensions();
    showExtensions("Required instance extensions", extensionNames);

    showLayers("Available instance layers",
               context.enumerateInstanceLayerProperties());

    std::vector<const char*> layerNames = getRequiredLayers();
    showLayers("Required instance layers", layerNames);

    // インスタンスの設定
    vk::InstanceCreateInfo instanceInfo{
        // インスタンスのフラグ
        /* flags = */ instanceFlags,
        // アプリケーションの情報
        /* pApplicationInfo = */ &appInfo,
        // 有効化するレイヤーを登録する
        /* pEnabledLayerNames = */ layerNames,
        // 有効化するエクステンションを登録する
        /* pEnabledExtensionNames = */ extensionNames};

#if ENABLE_VALIDATION
    // バリデーションレイヤーの出力をコールバックで受け取る為には、vk::DebugUtilsMessengerを作らなければならない
    vk::DebugUtilsMessengerCreateInfoEXT messengerInfo{
        /* flags = */ {},
        /* messageSeverity = */
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            /*vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |*/
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        /* messageType = */ vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        /* pfnUserCallback = */ &messageCallBack,
        /* pUserData = */ nullptr};
#endif

    // エクステンションとレイヤーを登録するだけでは、インスタンス生成/削除時のバリデーションが行われない
    // インスタンス生成/削除時のバリデーションを有効にするには、vk::InstanceCreateInfoのpNextにvk::DebugUtilsMessengerCreateInfoEXTへのポインターを与える必要がある
    // Vulkan-Hppでは、vk::StructureChainによってpNextを設定する
    vk::StructureChain<vk::InstanceCreateInfo,
                       vk::DebugUtilsMessengerCreateInfoEXT>
        instanceChain{instanceInfo, messengerInfo};

    // vkCreateInstance(pCreateInfo, pAllocator, pInstance)に相当
    instance = std::make_shared<vk::raii::Instance>(
        context, instanceChain.get<vk::InstanceCreateInfo>());

    std::cout << Console::fgGreen << "# "
              << "vkCreateInstance() succeeded" << Console::fgDefault
              << std::endl;

#if ENABLE_VALIDATION
    // vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator,
    // pMessenger)に相当
    // vk::DebugUtilsMessengerを作成する
    debugMessenger = std::make_shared<vk::raii::DebugUtilsMessengerEXT>(
        *instance, messengerInfo);
#endif
  }

  void initializeSurface() {
    VkSurfaceKHR cSurface;
    // ウィンドウの描画サーフェイスを作成する
    SDL_bool sResult =
        SDL_Vulkan_CreateSurface(window.get(), **instance, &cSurface);
    surface = std::make_shared<vk::raii::SurfaceKHR>(*instance, cSurface);

    if (sResult != SDL_TRUE) {
      throw std::runtime_error("SDL_Vulkan_CreateSurface() failed");
    } else {
      std::cout << Console::fgGreen << "# "
                << "SDL_Vulkan_CreateSurface() succeeded" << Console::fgDefault
                << std::endl;
    }
  }

  void initializeDevice() {
    vk::raii::PhysicalDevices devices(*instance);

    showPhysicalDevices("Available physical devices", devices);

    // 使用する物理デバイスを選択する
    physicalDevice = std::make_shared<vk::raii::PhysicalDevice>(
        selectPhysicalDevice(devices, *surface));
    showPhysicalDevice("Selected physical device", *physicalDevice);
    showExtensions("Available device extensions",
                   physicalDevice->enumerateDeviceExtensionProperties());

    showQueueFamilies("Queue families",
                      physicalDevice->getQueueFamilyProperties());

    // 有効化するキューファミリーのインデックスを格納する
    // セットに格納することで、インデックスは重複しない
    std::set<uint32_t> queueFamilyIndices;
    // グラフィックス命令をサポートするキューファミリーのインデックスを取得する
    uint32_t graphicsQueueFamilyIndex =
        *findQueueFamilyIndex(*physicalDevice, vk::QueueFlagBits::eGraphics);
    queueFamilyIndices.insert(graphicsQueueFamilyIndex);
    // プレゼンテーションをサポートするキューファミリーのインデックスを取得する
    uint32_t presentQueueFamilyIndex =
        *findPresentQueueFamilyIndex(*physicalDevice, *surface);
    queueFamilyIndices.insert(presentQueueFamilyIndex);

    // vk::DeviceQueueCreateInfoには論理デバイスと共に作成するキューの情報を格納する
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    std::vector<float> queuePriorities{1.0f};

    for (auto&& index : queueFamilyIndices) {
      vk::DeviceQueueCreateInfo queueInfo{
          /* flags = */ {},
          // キューをどのキューファミリーから作成するかを指定する
          // vkGetPhysicalDeviceQueueFamilyProperties()が返すpQueueFamilyPropertiesへのインデックスである
          /* queueFamilyIndex = */ index,
          // キューの優先度を指定する
          // 優先度は0から1であり、1の時に最高
          // 優先度が高いキューの命令が優先されて処理される場合がある
          /* pQueuePriorities = */ queuePriorities};
      queueCreateInfos.push_back(queueInfo);
    }

    std::vector<const char*> deviceExtensionNames =
        getRequiredDeviceExtensions(*physicalDevice);
    showExtensions("Required device extensions", deviceExtensionNames);

    vk::PhysicalDeviceFeatures enabledFeatures{};

    vk::DeviceCreateInfo deviceInfo{
        /* flags = */ {},
        // デバイスと同時に作成するキューの情報を登録する
        /* pQueueCreateInfos = */ queueCreateInfos,
        // enabledLayerCount, ppEnabledLayerNamesは非推奨であり無視される
        /* ppEnabledLayerNames = */ nullptr,
        // 有効化するデバイスエクステンションを登録する
        /* ppEnabledExtensionNames = */ deviceExtensionNames,
        // 有効化するデバイスフィーチャー
        /* pEnabledFeatures = */ &enabledFeatures};

    // vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice)に相当
    // 論理デバイスを作成する
    device = std::make_shared<vk::raii::Device>(*physicalDevice, deviceInfo);

    std::cout << Console::fgGreen << "# "
              << "vkCreateDevice() succeeded" << Console::fgDefault
              << std::endl;

    // vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue)に相当
    // 作成したキューを取得する
    // queueIndexはキューファミリー内のキューのインデックス
    graphicsQueue =
        std::make_shared<vk::raii::Queue>(*device, graphicsQueueFamilyIndex, 0);
    std::cout << "# "
              << "Obtained graphics queue: " << **graphicsQueue << std::endl;

    presentQueue =
        std::make_shared<vk::raii::Queue>(*device, presentQueueFamilyIndex, 0);
    std::cout << "# "
              << "Obtained present queue: " << **presentQueue << std::endl;
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

  void finalize() {}

  vk::InstanceCreateFlags getInstanceFlags() {
    vk::InstanceCreateFlags flags{};

#if SUPPORT_MOLTENVK
    // MoltenVKに対応する場合は、VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHRフラグを指定する
    flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    return flags;
  }

  static void showInstanceFlags(const char* message,
                                vk::InstanceCreateFlags flags) {
    std::cout << "# " << message << ": " << static_cast<uint32_t>(flags)
              << std::endl;
  }

  static void showExtensions(
      const char* message,
      const std::vector<vk::ExtensionProperties>& extensions) {
    std::cout << "# " << message << ":" << std::endl;

    for (auto&& ext : extensions) {
      std::cout << "| " << ext.extensionName << " ["
                << versionToString(ext.specVersion) << "]" << std::endl;
    }
  }

  static void showExtensions(const char* message,
                             const std::vector<const char*>& extensions) {
    std::cout << "# " << message << ":" << std::endl;

    for (auto&& ext : extensions) {
      std::cout << "| " << ext << std::endl;
    }
  }

  // 必要となるエクステンションを取得する
  std::vector<const char*> getRequiredExtensions() {
    // VulkanとSDLは、エクステンションを通じてやりとりするので、SDLからVulkanインスタンスに登録すべきエクステンションのリストを取得する
    unsigned int sdlExtensionCount;
    SDL_Vulkan_GetInstanceExtensions(window.get(), &sdlExtensionCount, nullptr);

    std::vector<const char*> extensionNames(sdlExtensionCount);

    SDL_Vulkan_GetInstanceExtensions(window.get(), &sdlExtensionCount,
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

  static void showLayers(const char* message,
                         const std::vector<vk::LayerProperties>& layers) {
    std::cout << "# " << message << ":" << std::endl;

    for (auto&& layer : layers) {
      std::cout << "| " << layer.layerName << " ["
                << versionToString(layer.specVersion) << ", "
                << versionToString(layer.implementationVersion) << ", "
                << layer.description << "]" << std::endl;
    }
  }

  static void showLayers(const char* message,
                         const std::vector<const char*>& layers) {
    std::cout << "# " << message << ":" << std::endl;

    for (auto&& layer : layers) {
      std::cout << "| " << layer << std::endl;
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

  static std::string versionToString(uint32_t version) {
    return std::to_string(VK_VERSION_MAJOR(version)) + "." +
           std::to_string(VK_VERSION_MINOR(version)) + "." +
           std::to_string(VK_VERSION_PATCH(version));
  }

  static void showPhysicalDevices(const char* message,
                                  const vk::raii::PhysicalDevices& devices) {
    std::cout << "# " << message << ":" << std::endl;

    for (auto&& dev : devices) {
      vk::PhysicalDeviceProperties props = dev.getProperties();

      std::cout << "| " << props.deviceName << " ["
                << versionToString(props.apiVersion) << ", "
                << props.driverVersion << ", " << props.vendorID << ", "
                << props.deviceID << ", "
                << static_cast<uint32_t>(props.deviceType) << "]" << std::endl;
    }
  }

  static void showPhysicalDevice(const char* message,
                                 vk::raii::PhysicalDevice& device) {
    vk::PhysicalDeviceProperties props = device.getProperties();

    std::cout << "# " << message << ": " << props.deviceName << " ["
              << versionToString(props.apiVersion) << ", "
              << props.driverVersion << ", " << props.vendorID << ", "
              << props.deviceID << ", "
              << static_cast<uint32_t>(props.deviceType) << "]" << std::endl;
  }

  static void showQueueFamilies(
      const char* message,
      const std::vector<vk::QueueFamilyProperties>& families) {
    std::cout << "# " << message << ":" << std::endl;

    for (uint32_t i = 0; i < families.size(); i++) {
      auto&& family = families[i];

      std::cout << "| # Queue family " << i << ":" << std::endl;
      std::cout << "| | Count: " << family.queueCount << std::endl;
      std::cout << "| | Flags: ";
      showFlags(queueFlagsToString(family.queueFlags));
      std::cout << std::endl;
    }
  }

  static void showFlags(const std::vector<std::string>& flags) {
    if (flags.size() > 0) {
      std::cout << flags[0];

      for (size_t i = 1; i < flags.size(); i++) {
        std::cout << " | " << flags[i];
      }
    } else {
      std::cout << "0";
    }
  }

  static std::vector<std::string> queueFlagsToString(vk::QueueFlags flags) {
    std::vector<std::string> strings;

    if ((flags & vk::QueueFlagBits::eGraphics) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_GRAPHICS_BIT");
    }

    if ((flags & vk::QueueFlagBits::eCompute) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_COMPUTE_BIT");
    }

    if ((flags & vk::QueueFlagBits::eTransfer) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_TRANSFER_BIT");
    }

    if ((flags & vk::QueueFlagBits::eSparseBinding) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_SPARSE_BINDING_BIT");
    }

    if ((flags & vk::QueueFlagBits::eProtected) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_PROTECTED_BIT");
    }

    if ((flags & vk::QueueFlagBits::eVideoDecodeKHR) != vk::QueueFlags{}) {
      strings.push_back("VK_QUEUE_VIDEO_DECODE_BIT_KHR");
    }

    return strings;
  }

  // 条件を満たす一番最初のデバイスを返す
  vk::raii::PhysicalDevice& selectPhysicalDevice(
      vk::raii::PhysicalDevices& devices,
      vk::raii::SurfaceKHR& surface) {
    for (auto&& dev : devices) {
      if (isSuitablePhysicalDevice(dev, surface)) {
        return dev;
      }
    }

    throw std::runtime_error("No suitable physical device");
  }

  bool isSuitablePhysicalDevice(vk::raii::PhysicalDevice& device,
                                vk::raii::SurfaceKHR& surface) {
    std::optional<uint32_t> graphicsQueueFamily =
        findQueueFamilyIndex(device, vk::QueueFlagBits::eGraphics);
    std::optional<uint32_t> presentQueueFamily =
        findPresentQueueFamilyIndex(device, surface);
    return graphicsQueueFamily.has_value() && presentQueueFamily.has_value();
  }

  // フラグの条件を満たす一番最初のキューファミリーのインデックスを返す
  std::optional<uint32_t> findQueueFamilyIndex(vk::raii::PhysicalDevice& device,
                                               vk::QueueFlags flags) {
    std::vector<vk::QueueFamilyProperties> families =
        device.getQueueFamilyProperties();

    for (size_t i = 0; i < families.size(); i++) {
      auto&& family = families[i];

      if ((family.queueFlags & flags) != vk::QueueFlags{}) {
        return static_cast<uint32_t>(i);
      }
    }

    return std::nullopt;
  }

  // プレゼンテーションをサポートする一番最初のキューファミリーのインデックスを返す
  std::optional<uint32_t> findPresentQueueFamilyIndex(
      vk::raii::PhysicalDevice& device,
      vk::raii::SurfaceKHR& surface) {
    std::vector<vk::QueueFamilyProperties> families =
        device.getQueueFamilyProperties();

    for (size_t i = 0; i < families.size(); i++) {
      vk::Bool32 supported =
          device.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface);

      if (supported) {
        return static_cast<uint32_t>(i);
      }
    }

    return std::nullopt;
  }

  std::vector<const char*> getRequiredDeviceExtensions(
      const vk::raii::PhysicalDevice& physicalDevice) {
    std::vector<const char*> extensions;

#if SUPPORT_MOLTENVK
    extensions.push_back("VK_KHR_portability_subset");
#endif

    return extensions;
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