//#define GLFW_INCLUDE_VULKAN
#include <QCoreApplication>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

VkInstance create_instance() {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = nullptr;
    create_info.enabledLayerCount = 0;

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance! error code=" + std::to_string(result));
    }

    return instance;
}

void list_extensions() {
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    std::cout << "available extensions:" << std::endl;
    for (const auto &extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
}

VkPhysicalDevice pick_physical_device(VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);
        std::cout << properties.deviceName << "," << properties.deviceType << std::endl;
        physical_device = device;
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return physical_device;
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
};

VkDevice create_logical_device(VkPhysicalDevice physical_device, QueueFamilyIndices indices) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphicsFamily.value();
    queue_create_info.queueCount = 1;
    float queuePriority = 1.0f;
    queue_create_info.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo dev_create_info = {};
    dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_create_info.pQueueCreateInfos = &queue_create_info;
    dev_create_info.queueCreateInfoCount = 1;
    VkPhysicalDeviceFeatures features = {};
    dev_create_info.pEnabledFeatures = &features;
    dev_create_info.enabledExtensionCount = 0;
    dev_create_info.enabledLayerCount = 0;

    VkDevice device;
    if (vkCreateDevice(physical_device, &dev_create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    return device;
}

VkQueue get_grahpics_queue(VkDevice device, QueueFamilyIndices indices) {
    VkQueue graphics_queue;
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphics_queue);
    return graphics_queue;
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_families.data());
    std::cout << "available queues:" << std::endl;
    int i = 0;
    for (const auto &queue_family : queue_families) {
        std::cout << "\tqueue=" << queue_family.queueFlags << std::endl;
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        i++;
    }
    return indices;
}

class TriangleSampleApplication {
  public:
    void run() {
        this->init();
        this->render();
        this->cleanup();
    }

  private:
    void init() {
        std::cout << "Creating instance..." << std::endl;
        instance = create_instance();
        list_extensions();
        std::cout << "Picking physical device..." << std::endl;
        VkPhysicalDevice phs_dev = pick_physical_device(instance);
        QueueFamilyIndices indices = find_queue_families(phs_dev);
        std::cout << "Creating logical device..." << std::endl;
        device = create_logical_device(phs_dev, indices);
        std::cout << "Obtaining grahpics queue..." << std::endl;
        grahpics_queue = get_grahpics_queue(device, indices);
    }

    void render() {}

    void cleanup() {
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    VkInstance instance;
    VkDevice device;
    VkQueue grahpics_queue;
};

int main() {
    TriangleSampleApplication app;
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
