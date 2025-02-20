/* nuklear - 1.32.0 - public domain */
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_VULKAN_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "../../nuklear.h"
#include "nuklear_sdl_vulkan.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
/* #define INCLUDE_ALL */
/* #define INCLUDE_STYLE */
/* #define INCLUDE_CALCULATOR */
/* #define INCLUDE_CANVAS */
#define INCLUDE_OVERVIEW
/*#define INCLUDE_CONFIGURATOR */
/* #define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_CANVAS
  #define INCLUDE_OVERVIEW
  #define INCLUDE_CONFIGURATOR
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
#include "../../demo/common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
#include "../../demo/common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
#include "../../demo/common/canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
#include "../../demo/common/overview.c"
#endif
#ifdef INCLUDE_CONFIGURATOR
  #include "../../demo/common/style_configurator.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
#include "../../demo/common/node_editor.c"
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/

static const char *validation_layer_name = "VK_LAYER_KHRONOS_validation";

struct queue_family_indices {
    int graphics;
    int present;
};

struct swap_chain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formats_len;
    VkPresentModeKHR *present_modes;
    uint32_t present_modes_len;
};

void swap_chain_support_details_free(
    struct swap_chain_support_details *swap_chain_support) {
    if (swap_chain_support->formats_len > 0) {
        free(swap_chain_support->formats);
        swap_chain_support->formats = NULL;
    }
    if (swap_chain_support->present_modes_len > 0) {
        free(swap_chain_support->present_modes);
        swap_chain_support->present_modes = NULL;
    }
}

struct vulkan_demo {
    SDL_Window *win;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    struct queue_family_indices indices;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSampler sampler;

    VkSwapchainKHR swap_chain;
    VkImage *swap_chain_images;
    uint32_t swap_chain_images_len;
    VkImageView *swap_chain_image_views;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_image_extent;

    VkImage *overlay_images;
    VkImageView *overlay_image_views;
    VkDeviceMemory *overlay_image_memories;

    VkRenderPass render_pass;
    VkFramebuffer *framebuffers;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *descriptor_sets;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer *command_buffers;
    VkSemaphore image_available;
    VkSemaphore render_finished;

    VkImage demo_texture_image;
    VkImageView demo_texture_image_view;
    VkDeviceMemory demo_texture_memory;

    VkFence render_fence;
};

VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                      const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                      void *user_data) {
    (void)message_severity;
    (void)message_type;
    (void)user_data;
    fprintf(stderr, "validation layer: %s\n", callback_data->pMessage);

    return VK_FALSE;
}

bool check_validation_layer_support() {
    uint32_t layer_count;
    bool ret = false;
    VkResult result;
    uint32_t i;
    VkLayerProperties *available_layers = NULL;

    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceLayerProperties failed: %d\n",
                result);
        return ret;
    }

    available_layers = malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceLayerProperties failed: %d\n",
                result);
        goto cleanup;
    }

    printf("Available vulkan layers:\n");
    for (i = 0; i < layer_count; i++) {
        printf("  %s\n", available_layers[i].layerName);
        if (strcmp(validation_layer_name, available_layers[i].layerName) == 0) {
            ret = true;
            break;
        }
    }
cleanup:
    free(available_layers);
    return ret;
}

VkResult create_debug_utils_messenger_ext(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

bool create_debug_callback(struct vulkan_demo *demo) {
    VkResult result;

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    memset(&create_info, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = vulkan_debug_callback;

    result = create_debug_utils_messenger_ext(demo->instance, &create_info,
                                              NULL, &demo->debug_messenger);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "create_debug_utils_messenger_ext failed %d\n", result);
        return false;
    }
    return true;
}

bool create_instance(struct vulkan_demo *demo) {
    uint32_t i;
    uint32_t available_instance_extension_count;
    VkResult result;
    VkExtensionProperties *available_instance_extensions = NULL;
    bool ret = false;
    VkApplicationInfo app_info;
    VkInstanceCreateInfo create_info;
    uint32_t sdl_extension_count;
    uint32_t enabled_extension_count;
    const char **enabled_extensions = NULL;
    bool validation_layers_installed;

    validation_layers_installed = check_validation_layer_support();

    if (!validation_layers_installed) {
        fprintf(stdout,
                "Couldn't find validation layer %s. Continuing without "
                "validation layers.\n",
                validation_layer_name);
    }
    result = vkEnumerateInstanceExtensionProperties(
        NULL, &available_instance_extension_count, NULL);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceExtensionProperties failed %d\n",
                result);
        return ret;
    }

    available_instance_extensions = malloc(available_instance_extension_count *
                                           sizeof(VkExtensionProperties));

    result = vkEnumerateInstanceExtensionProperties(
        NULL, &available_instance_extension_count,
        available_instance_extensions);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceExtensionProperties failed %d\n",
                result);
        goto cleanup;
    }

    printf("available instance extensions:\n");
    for (i = 0; i < available_instance_extension_count; i++) {
        printf("  %s\n", available_instance_extensions[i].extensionName);
    }

    if (SDL_Vulkan_GetInstanceExtensions(demo->win, &sdl_extension_count,
                                         NULL) != SDL_TRUE) {
        fprintf(stderr, "SDL_Vulkan_GetInstanceExtensions failed: %s\n",
                SDL_GetError());
        goto cleanup;
    }

    enabled_extension_count =
        sdl_extension_count + (validation_layers_installed ? 1 : 0);

    enabled_extensions = malloc(enabled_extension_count * sizeof(char *));
    if (SDL_Vulkan_GetInstanceExtensions(demo->win, &sdl_extension_count,
                                         enabled_extensions) != SDL_TRUE) {
        fprintf(stderr, "SDL_Vulkan_GetInstanceExtensions failed: %s\n",
                SDL_GetError());
        goto cleanup;
    }

    if (validation_layers_installed) {
        enabled_extensions[sdl_extension_count] =
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    printf("Trying to enable the following instance extensions: ");
    for (i = 0; i < enabled_extension_count; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%s\n", enabled_extensions[i]);
    }
    printf("\n");
    for (i = 0; i < enabled_extension_count; i++) {
        int extension_missing = 1;
        uint32_t j;
        for (j = 0; j < available_instance_extension_count; j++) {
            if (strcmp(enabled_extensions[i],
                       available_instance_extensions[j].extensionName) == 0) {
                extension_missing = 0;
                break;
            }
        }
        if (extension_missing) {
            fprintf(stderr, "Extension %s is missing\n", enabled_extensions[i]);
            return ret;
        }
    }

    memset(&app_info, 0, sizeof(VkApplicationInfo));
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Demo";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    memset(&create_info, 0, sizeof(VkInstanceCreateInfo));
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = enabled_extension_count;
    create_info.ppEnabledExtensionNames = enabled_extensions;
    if (validation_layers_installed) {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = &validation_layer_name;
    }
    result = vkCreateInstance(&create_info, NULL, &demo->instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance result %d\n", result);
        return ret;
    }
    if (validation_layers_installed) {
        ret = create_debug_callback(demo);
    } else {
        ret = true;
    }
cleanup:
    if (available_instance_extensions) {
        free(available_instance_extensions);
    }
    if (enabled_extensions) {
        free(enabled_extensions);
    }

    return ret;
}

bool create_surface(struct vulkan_demo *demo) {
    SDL_bool result;
    result =
        SDL_Vulkan_CreateSurface(demo->win, demo->instance, &demo->surface);
    if (result != SDL_TRUE) {
        fprintf(stderr, "creating vulkan surface failed: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

bool find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                         struct queue_family_indices *indices) {
    VkResult result;
    uint32_t queue_family_count = 0;
    uint32_t i = 0;
    bool ret = false;
    VkQueueFamilyProperties *queue_family_properties;
    VkBool32 present_support;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count, NULL);

    queue_family_properties =
        malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_family_properties);

    for (i = 0; i < queue_family_count; i++) {
        if (queue_family_properties[i].queueCount == 0) {
            continue;
        }
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices->graphics = i;
        }

        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physical_device, i, surface, &present_support);
        if (result != VK_SUCCESS) {
            fprintf(stderr,
                    "vkGetPhysicalDeviceSurfaceSupportKHR failed with %d\n",
                    result);
            goto cleanup;
        }
        if (present_support == VK_TRUE) {
            indices->present = i;
        }
        if (indices->graphics >= 0 && indices->present >= 0) {
            break;
        }
    }
    ret = true;
cleanup:
    free(queue_family_properties);
    return ret;
}

bool query_swap_chain_support(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    struct swap_chain_support_details *swap_chain_support) {
    VkResult result;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface, &swap_chain_support->capabilities);
    if (result != VK_SUCCESS) {
        fprintf(stderr,
                "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %d\n",
                result);
        return false;
    }

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &swap_chain_support->formats_len, NULL);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %d\n",
                result);
        return false;
    }

    if (swap_chain_support->formats_len != 0) {
        swap_chain_support->formats = malloc(swap_chain_support->formats_len *
                                             sizeof(VkSurfaceFormatKHR));
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &swap_chain_support->formats_len,
            swap_chain_support->formats);

        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %d\n",
                    result);
            return false;
        }
    }

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &swap_chain_support->present_modes_len, NULL);

    if (result != VK_SUCCESS) {
        fprintf(stderr,
                "vkGetPhysicalDeviceSurfacePresentModesKHR failed: %d\n",
                result);
        return false;
    }

    if (swap_chain_support->present_modes_len != 0) {
        swap_chain_support->present_modes = malloc(
            swap_chain_support->present_modes_len * sizeof(VkPresentModeKHR));
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &swap_chain_support->present_modes_len,
            swap_chain_support->present_modes);

        if (result != VK_SUCCESS) {
            fprintf(stderr,
                    "vkGetPhysicalDeviceSurfacePresentModesKHR failed: %d\n",
                    result);
            return false;
        }
    }

    return true;
}

bool is_suitable_physical_device(VkPhysicalDevice physical_device,
                                 VkSurfaceKHR surface,
                                 struct queue_family_indices *indices) {
    VkResult result;
    uint32_t device_extension_count;
    uint32_t i;
    VkExtensionProperties *device_extensions;
    bool ret = false;
    struct swap_chain_support_details swap_chain_support;
    int found_khr_surface = 0;

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);

    printf("Probing physical device %s\n", device_properties.deviceName);

    result = vkEnumerateDeviceExtensionProperties(
        physical_device, NULL, &device_extension_count, NULL);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateDeviceExtensionProperties failed: %d\n",
                result);
        return false;
    }

    device_extensions =
        malloc(device_extension_count * sizeof(VkExtensionProperties));

    result = vkEnumerateDeviceExtensionProperties(
        physical_device, NULL, &device_extension_count, device_extensions);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateDeviceExtensionProperties failed: %d\n",
                result);
        goto cleanup;
    }

    printf("  Supported device extensions:\n");

    for (i = 0; i < device_extension_count; i++) {
        printf("    %s\n", device_extensions[i].extensionName);
        if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                   device_extensions[i].extensionName) == 0) {
            found_khr_surface = 1;
            break;
        }
    }
    if (!found_khr_surface) {
        printf("  Device doesnt support %s\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        goto cleanup;
    }
    if (!find_queue_families(physical_device, surface, indices)) {
        goto cleanup;
    }
    if (indices->graphics < 0 || indices->present < 0) {
        printf("  Device is missing graphics and/or present support. graphics: "
               "%d, present: %d\n",
               indices->graphics, indices->present);
        goto cleanup;
    }

    if (!query_swap_chain_support(physical_device, surface,
                                  &swap_chain_support)) {
        goto cleanup;
    }

    if (swap_chain_support.formats_len == 0) {
        printf(" Device doesn't support any swap chain formats\n");
        goto cleanup;
    }

    if (swap_chain_support.present_modes_len == 0) {
        printf(" Device doesn't support any swap chain present modes\n");
        goto cleanup;
    }
    ret = true;

cleanup:
    free(device_extensions);
    swap_chain_support_details_free(&swap_chain_support);

    return ret;
}

bool create_physical_device(struct vulkan_demo *demo) {
    uint32_t device_count = 0;
    VkPhysicalDevice *physical_devices;
    VkResult result;
    uint32_t i;
    bool ret = false;

    result = vkEnumeratePhysicalDevices(demo->instance, &device_count, NULL);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumeratePhysicalDevices failed: %d\n", result);
        return ret;
    }
    if (device_count == 0) {
        fprintf(stderr, "no vulkan capable GPU found!");
        return ret;
    }

    physical_devices = malloc(device_count * sizeof(VkPhysicalDevice));
    result = vkEnumeratePhysicalDevices(demo->instance, &device_count,
                                        physical_devices);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumeratePhysicalDevices failed: %d\n", result);
        goto cleanup;
    }

    for (i = 0; i < device_count; i++) {
        struct queue_family_indices indices = {-1, -1};
        if (is_suitable_physical_device(physical_devices[i], demo->surface,
                                        &indices)) {
            printf("  Selecting this device for rendering. Queue families: "
                   "graphics: %d, present: %d!\n",
                   indices.graphics, indices.present);
            demo->physical_device = physical_devices[i];
            demo->indices = indices;
            break;
        }
    }
    if (demo->physical_device == NULL) {
        fprintf(stderr, "failed to find a suitable GPU!\n");
    } else {
        ret = true;
    }
cleanup:
    free(physical_devices);
    return ret;
}

bool create_logical_device(struct vulkan_demo *demo) {
    VkResult result;
    bool ret = false;
    float queuePriority = 1.0f;
    uint32_t num_queues = 1;
    VkDeviceQueueCreateInfo *queue_create_infos;
    VkDeviceCreateInfo create_info;
    const char *swap_chain_extension_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    queue_create_infos = calloc(2, sizeof(VkDeviceQueueCreateInfo));
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = demo->indices.graphics;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queuePriority;

    if (demo->indices.present != demo->indices.graphics) {
        queue_create_infos[1].sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[1].queueFamilyIndex = demo->indices.present;
        queue_create_infos[1].queueCount = 1;
        queue_create_infos[1].pQueuePriorities = &queuePriority;
        num_queues = 2;
    }

    memset(&create_info, 0, sizeof(VkDeviceCreateInfo));
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = num_queues;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = &swap_chain_extension_name;

    result = vkCreateDevice(demo->physical_device, &create_info, NULL,
                            &demo->device);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDevice failed: %d\n", result);
        goto cleanup;
    }

    vkGetDeviceQueue(demo->device, demo->indices.graphics, 0,
                     &demo->graphics_queue);
    vkGetDeviceQueue(demo->device, demo->indices.present, 0,
                     &demo->present_queue);
    ret = true;
cleanup:
    free(queue_create_infos);
    return ret;
}

bool create_sampler(struct vulkan_demo *demo) {
    VkResult result;
    VkSamplerCreateInfo sampler_info;

    memset(&sampler_info, 0, sizeof(VkSamplerCreateInfo));
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = NULL;
    sampler_info.maxAnisotropy = 1.0;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

    result = vkCreateSampler(demo->device, &sampler_info, NULL, &demo->sampler);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSampler failed: %d\n", result);
        return false;
    }
    return true;
}

VkSurfaceFormatKHR
choose_swap_surface_format(VkSurfaceFormatKHR *available_formats,
                           uint32_t available_formats_len) {
    VkSurfaceFormatKHR undefined_format = {VK_FORMAT_B8G8R8A8_UNORM,
                                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    uint32_t i;
    if (available_formats_len == 1 &&
        available_formats[0].format == VK_FORMAT_UNDEFINED) {
        return undefined_format;
    }

    for (i = 0; i < available_formats_len; i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            available_formats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

VkPresentModeKHR
choose_swap_present_mode(VkPresentModeKHR *available_present_modes,
                         uint32_t available_present_modes_len) {
    uint32_t i;
    for (i = 0; i < available_present_modes_len; i++) {
        /*
        best mode to ensure good input latency while ensuring we are not
        producing tearing
        */
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    /* must be supported */
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(struct vulkan_demo *demo,
                              VkSurfaceCapabilitiesKHR *capabilities) {
    int width, height;
    VkExtent2D actual_extent;
    if (capabilities->currentExtent.width != 0xFFFFFFFF) {
        return capabilities->currentExtent;
    } else {
        /* not window size! */
        SDL_Vulkan_GetDrawableSize(demo->win, &width, &height);

        actual_extent.width = (uint32_t)width;
        actual_extent.height = (uint32_t)height;

        actual_extent.width = NK_MAX(
            capabilities->minImageExtent.width,
            NK_MIN(capabilities->maxImageExtent.width, actual_extent.width));
        actual_extent.height = NK_MAX(
            capabilities->minImageExtent.height,
            NK_MIN(capabilities->maxImageExtent.height, actual_extent.height));

        return actual_extent;
    }
}

bool create_swap_chain(struct vulkan_demo *demo) {
    struct swap_chain_support_details swap_chain_support;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    VkResult result;
    VkSwapchainCreateInfoKHR create_info;
    uint32_t queue_family_indices[2];
    bool ret = false;

    queue_family_indices[0] = (uint32_t)demo->indices.graphics;
    queue_family_indices[1] = (uint32_t)demo->indices.present;

    if (!query_swap_chain_support(demo->physical_device, demo->surface,
                                  &swap_chain_support)) {
        goto cleanup;
    }
    surface_format = choose_swap_surface_format(swap_chain_support.formats,
                                                swap_chain_support.formats_len);
    present_mode = choose_swap_present_mode(
        swap_chain_support.present_modes, swap_chain_support.present_modes_len);
    extent = choose_swap_extent(demo, &swap_chain_support.capabilities);

    demo->swap_chain_images_len =
        swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 &&
        demo->swap_chain_images_len >
            swap_chain_support.capabilities.maxImageCount) {
        demo->swap_chain_images_len =
            swap_chain_support.capabilities.maxImageCount;
    }

    memset(&create_info, 0, sizeof(VkSwapchainCreateInfoKHR));
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = demo->surface;

    create_info.minImageCount = demo->swap_chain_images_len;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (demo->indices.graphics != demo->indices.present) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    result = vkCreateSwapchainKHR(demo->device, &create_info, NULL,
                                  &demo->swap_chain);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSwapchainKHR failed: %d\n", result);
        goto cleanup;
    }

    result = vkGetSwapchainImagesKHR(demo->device, demo->swap_chain,
                                     &demo->swap_chain_images_len, NULL);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkGetSwapchainImagesKHR failed: %d\n", result);
        goto cleanup;
    }
    if (demo->swap_chain_images == NULL) {
        demo->swap_chain_images =
            malloc(demo->swap_chain_images_len * sizeof(VkImage));
    }
    result = vkGetSwapchainImagesKHR(demo->device, demo->swap_chain,
                                     &demo->swap_chain_images_len,
                                     demo->swap_chain_images);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkGetSwapchainImagesKHR failed: %d\n", result);
        return false;
    }

    demo->swap_chain_image_format = surface_format.format;
    demo->swap_chain_image_extent = extent;

    ret = true;
cleanup:
    swap_chain_support_details_free(&swap_chain_support);

    return ret;
}

bool create_swap_chain_image_views(struct vulkan_demo *demo) {
    uint32_t i;
    VkResult result;
    VkImageViewCreateInfo create_info;

    memset(&create_info, 0, sizeof(VkImageViewCreateInfo));
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = demo->swap_chain_image_format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (!demo->swap_chain_image_views) {
        demo->swap_chain_image_views =
            malloc(demo->swap_chain_images_len * sizeof(VkImageView));
    }

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        create_info.image = demo->swap_chain_images[i];
        result = vkCreateImageView(demo->device, &create_info, NULL,
                                   &demo->swap_chain_image_views[i]);

        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView failed: %d\n", result);
            return false;
        }
    }
    return true;
}

bool create_overlay_images(struct vulkan_demo *demo) {
    uint32_t i, j;
    VkResult result;
    VkMemoryRequirements mem_requirements;
    VkPhysicalDeviceMemoryProperties mem_properties;
    int found;
    VkImageCreateInfo image_info;
    VkMemoryAllocateInfo alloc_info;
    VkImageViewCreateInfo image_view_info;

    memset(&image_info, 0, sizeof(VkImageCreateInfo));
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = demo->swap_chain_image_extent.width;
    image_info.extent.height = demo->swap_chain_image_extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = demo->swap_chain_image_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    memset(&alloc_info, 0, sizeof(VkMemoryAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    memset(&image_view_info, 0, sizeof(VkImageViewCreateInfo));
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = demo->swap_chain_image_format;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    if (!demo->overlay_images) {
        demo->overlay_images =
            malloc(demo->swap_chain_images_len * sizeof(VkImage));
    }

    if (!demo->overlay_image_memories) {
        demo->overlay_image_memories =
            malloc(demo->swap_chain_images_len * sizeof(VkDeviceMemory));
    }
    if (!demo->overlay_image_views) {
        demo->overlay_image_views =
            malloc(demo->swap_chain_images_len * sizeof(VkImageView));
    }

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        result = vkCreateImage(demo->device, &image_info, NULL,
                               &demo->overlay_images[i]);

        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImage failed for index %lu: %d\n",
                    (unsigned long)i, result);
            return false;
        }

        vkGetImageMemoryRequirements(demo->device, demo->overlay_images[i],
                                     &mem_requirements);

        alloc_info.allocationSize = mem_requirements.size;

        vkGetPhysicalDeviceMemoryProperties(demo->physical_device,
                                            &mem_properties);
        found = 0;
        for (j = 0; j < mem_properties.memoryTypeCount; j++) {
            if ((mem_requirements.memoryTypeBits & (1 << j)) &&
                (mem_properties.memoryTypes[j].propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr,
                    "failed to find suitable memory type for index %lu!\n",
                    (unsigned long)i);
            return false;
        }
        alloc_info.memoryTypeIndex = j;
        result = vkAllocateMemory(demo->device, &alloc_info, NULL,
                                  &demo->overlay_image_memories[i]);
        if (result != VK_SUCCESS) {
            fprintf(stderr,
                    "failed to allocate vulkan memory for index %lu: %d!\n",
                    (unsigned long)i, result);
            return false;
        }
        result = vkBindImageMemory(demo->device, demo->overlay_images[i],
                                   demo->overlay_image_memories[i], 0);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "Couldn't bind image memory for index %lu: %d\n",
                    (unsigned long)i, result);
            return false;
        }

        image_view_info.image = demo->overlay_images[i];
        result = vkCreateImageView(demo->device, &image_view_info, NULL,
                                   &demo->overlay_image_views[i]);

        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView failed for index %lu: %d\n",
                    (unsigned long)i, result);
            return false;
        }
    }
    return true;
}

bool create_render_pass(struct vulkan_demo *demo) {
    VkAttachmentDescription attachment;
    VkAttachmentReference color_attachment_ref;
    VkSubpassDescription subpass;
    VkSubpassDependency dependency;
    VkRenderPassCreateInfo render_pass_info;
    VkResult result;

    memset(&attachment, 0, sizeof(VkAttachmentDescription));
    attachment.format = demo->swap_chain_image_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    memset(&color_attachment_ref, 0, sizeof(VkAttachmentReference));
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    memset(&subpass, 0, sizeof(VkSubpassDescription));
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    memset(&dependency, 0, sizeof(VkSubpassDependency));
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    memset(&render_pass_info, 0, sizeof(VkRenderPassCreateInfo));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    result = vkCreateRenderPass(demo->device, &render_pass_info, NULL,
                                &demo->render_pass);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateRenderPass failed: %d\n", result);
        return false;
    }
    return true;
}

bool create_framebuffers(struct vulkan_demo *demo) {
    uint32_t i;
    VkResult result;
    VkFramebufferCreateInfo framebuffer_info;

    if (!demo->framebuffers) {
        demo->framebuffers =
            malloc(demo->swap_chain_images_len * sizeof(VkFramebuffer));
    }

    memset(&framebuffer_info, 0, sizeof(VkFramebufferCreateInfo));
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = demo->render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.width = demo->swap_chain_image_extent.width;
    framebuffer_info.height = demo->swap_chain_image_extent.height;
    framebuffer_info.layers = 1;

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        framebuffer_info.pAttachments = &demo->swap_chain_image_views[i];

        result = vkCreateFramebuffer(demo->device, &framebuffer_info, NULL,
                                     &demo->framebuffers[i]);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateFramebuffer failed from index %lu: %d\n",
                    (unsigned long)i, result);
            return false;
        }
    }
    return true;
}

bool create_descriptor_set_layout(struct vulkan_demo *demo) {
    VkDescriptorSetLayoutBinding overlay_layout_binding;
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_nfo;
    VkResult result;

    memset(&overlay_layout_binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    overlay_layout_binding.binding = 0;
    overlay_layout_binding.descriptorCount = 1;
    overlay_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    overlay_layout_binding.pImmutableSamplers = NULL;
    overlay_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    memset(&descriptor_set_layout_create_nfo, 0,
           sizeof(VkDescriptorSetLayoutCreateInfo));
    descriptor_set_layout_create_nfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_nfo.bindingCount = 1;
    descriptor_set_layout_create_nfo.pBindings = &overlay_layout_binding;

    result = vkCreateDescriptorSetLayout(demo->device,
                                         &descriptor_set_layout_create_nfo,
                                         NULL, &demo->descriptor_set_layout);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDescriptorSetLayout failed: %d\n", result);
        return false;
    }
    return true;
}

bool create_descriptor_pool(struct vulkan_demo *demo) {
    VkDescriptorPoolSize pool_size;
    VkDescriptorPoolCreateInfo pool_info;
    VkResult result;

    memset(&pool_size, 0, sizeof(VkDescriptorPoolSize));
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = demo->swap_chain_images_len;

    memset(&pool_info, 0, sizeof(VkDescriptorPoolCreateInfo));
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = demo->swap_chain_images_len;
    result = vkCreateDescriptorPool(demo->device, &pool_info, NULL,
                                    &demo->descriptor_pool);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDescriptorPool failed: %d\n", result);
        return false;
    }
    return true;
}

void update_descriptor_sets(struct vulkan_demo *demo) {
    uint32_t i;
    VkDescriptorImageInfo descriptor_image_info;
    VkWriteDescriptorSet descriptor_write;

    memset(&descriptor_image_info, 0, sizeof(VkDescriptorImageInfo));
    descriptor_image_info.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_image_info.sampler = demo->sampler;

    memset(&descriptor_write, 0, sizeof(VkWriteDescriptorSet));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &descriptor_image_info;

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        descriptor_write.dstSet = demo->descriptor_sets[i];
        descriptor_image_info.imageView = demo->overlay_image_views[i];

        vkUpdateDescriptorSets(demo->device, 1, &descriptor_write, 0, NULL);
    }
}

bool create_descriptor_sets(struct vulkan_demo *demo) {
    bool ret = false;
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkDescriptorSetAllocateInfo alloc_info;
    uint32_t i;
    VkResult result;

    demo->descriptor_sets =
        malloc(demo->swap_chain_images_len * sizeof(VkDescriptorSet));
    descriptor_set_layouts =
        malloc(demo->swap_chain_images_len * sizeof(VkDescriptorSetLayout));

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        descriptor_set_layouts[i] = demo->descriptor_set_layout;
    }

    memset(&alloc_info, 0, sizeof(VkDescriptorSetAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = demo->descriptor_pool;
    alloc_info.descriptorSetCount = demo->swap_chain_images_len;
    alloc_info.pSetLayouts = descriptor_set_layouts;
    result = vkAllocateDescriptorSets(demo->device, &alloc_info,
                                      demo->descriptor_sets);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateDescriptorSets failed: %d\n", result);
        goto cleanup;
    }

    update_descriptor_sets(demo);

    ret = true;
cleanup:
    free(descriptor_set_layouts);

    return ret;
}

bool create_shader_module(VkDevice device, char *shader_buffer,
                          size_t shader_buffer_len,
                          VkShaderModule *shader_module) {
    VkShaderModuleCreateInfo create_info;
    VkResult result;

    memset(&create_info, 0, sizeof(VkShaderModuleCreateInfo));
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_buffer_len;
    create_info.pCode = (const uint32_t *)shader_buffer;

    result = vkCreateShaderModule(device, &create_info, NULL, shader_module);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateShaderModule failed: %d\n", result);
        return false;
    }

    return true;
}

bool create_graphics_pipeline(struct vulkan_demo *demo) {
    bool ret = false;
    char *vert_shader_code = NULL;
    char *frag_shader_code = NULL;
    VkShaderModule vert_shader_module = VK_NULL_HANDLE;
    VkShaderModule frag_shader_module = VK_NULL_HANDLE;
    FILE *fp;
    size_t file_len;
    VkPipelineShaderStageCreateInfo vert_shader_stage_info;
    VkPipelineShaderStageCreateInfo frag_shader_stage_info;
    VkPipelineShaderStageCreateInfo shader_stages[2];
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blending;
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    VkResult result;
    VkGraphicsPipelineCreateInfo pipeline_info;
    size_t read_result;

    fp = fopen("shaders/demo.vert.spv", "r");
    if (!fp) {
        fprintf(stderr, "Couldn't open shaders/demo.vert.spv\n");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    vert_shader_code = malloc(file_len);
    fseek(fp, 0, 0);
    read_result = fread(vert_shader_code, file_len, 1, fp);
    fclose(fp);
    if (read_result != 1) {
        fprintf(stderr, "Could not read vertex shader\n");
        goto cleanup;
    }

    if (!create_shader_module(demo->device, vert_shader_code, file_len,
                              &vert_shader_module)) {
        goto cleanup;
    }

    fp = fopen("shaders/demo.frag.spv", "r");
    if (!fp) {
        fprintf(stderr, "Couldn't open shaders/demo.frag.spv\n");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    frag_shader_code = malloc(file_len);
    fseek(fp, 0, 0);
    read_result = fread(frag_shader_code, file_len, 1, fp);
    fclose(fp);
    if (read_result != 1) {
        fprintf(stderr, "Could not read fragment shader\n");
        goto cleanup;
    }

    if (!create_shader_module(demo->device, frag_shader_code, file_len,
                              &frag_shader_module)) {
        goto cleanup;
    }

    memset(&vert_shader_stage_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
    vert_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    memset(&frag_shader_stage_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
    frag_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    shader_stages[0] = vert_shader_stage_info;
    shader_stages[1] = frag_shader_stage_info;

    memset(&vertex_input_info, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    memset(&input_assembly, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    memset(&viewport, 0, sizeof(VkViewport));
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)demo->swap_chain_image_extent.width;
    viewport.height = (float)demo->swap_chain_image_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    memset(&scissor, 0, sizeof(VkRect2D));
    scissor.extent.width = demo->swap_chain_image_extent.width;
    scissor.extent.height = demo->swap_chain_image_extent.height;

    memset(&viewport_state, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    memset(&rasterizer, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    memset(&multisampling, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    memset(&color_blend_attachment, 0,
           sizeof(VkPipelineColorBlendAttachmentState));
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    memset(&color_blending, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 1.0f;
    color_blending.blendConstants[1] = 1.0f;
    color_blending.blendConstants[2] = 1.0f;
    color_blending.blendConstants[3] = 1.0f;

    memset(&pipeline_layout_info, 0, sizeof(VkPipelineLayoutCreateInfo));
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &demo->descriptor_set_layout;

    result = vkCreatePipelineLayout(demo->device, &pipeline_layout_info, NULL,
                                    &demo->pipeline_layout);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreatePipelineLayout failed: %d\n", result);
        goto cleanup;
    }

    memset(&pipeline_info, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = demo->pipeline_layout;
    pipeline_info.renderPass = demo->render_pass;
    pipeline_info.basePipelineHandle = NULL;

    result = vkCreateGraphicsPipelines(demo->device, NULL, 1, &pipeline_info,
                                       NULL, &demo->pipeline);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateGraphicsPipelines failed: %d\n", result);
        goto cleanup;
    }
    ret = true;
cleanup:
    if (frag_shader_module) {
        vkDestroyShaderModule(demo->device, frag_shader_module, NULL);
    }
    if (frag_shader_code) {
        free(frag_shader_code);
    }
    if (vert_shader_module) {
        vkDestroyShaderModule(demo->device, vert_shader_module, NULL);
    }
    if (vert_shader_code) {
        free(vert_shader_code);
    }

    return ret;
}

bool create_command_pool(struct vulkan_demo *demo) {
    VkCommandPoolCreateInfo pool_info;
    VkResult result;

    memset(&pool_info, 0, sizeof(VkCommandPoolCreateInfo));
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = demo->indices.graphics;

    result = vkCreateCommandPool(demo->device, &pool_info, NULL,
                                 &demo->command_pool);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateCommandPool failed: %d\n", result);
        return false;
    }
    return true;
}

bool create_command_buffers(struct vulkan_demo *demo) {
    VkCommandBufferAllocateInfo alloc_info;
    VkResult result;

    demo->command_buffers =
        malloc(demo->swap_chain_images_len * sizeof(VkCommandBuffer));

    memset(&alloc_info, 0, sizeof(VkCommandBufferAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = demo->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = demo->swap_chain_images_len;

    result = vkAllocateCommandBuffers(demo->device, &alloc_info,
                                      demo->command_buffers);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateCommandBuffers failed: %d\n", result);
        return false;
    }

    return true;
}

bool create_semaphores(struct vulkan_demo *demo) {
    VkSemaphoreCreateInfo semaphore_info;
    VkResult result;

    memset(&semaphore_info, 0, sizeof(VkSemaphoreCreateInfo));
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    result = vkCreateSemaphore(demo->device, &semaphore_info, NULL,
                               &demo->image_available);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSemaphore failed: %d\n", result);
        return false;
    }
    result = vkCreateSemaphore(demo->device, &semaphore_info, NULL,
                               &demo->render_finished);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSemaphore failed: %d\n", result);
        return false;
    }
    return true;
}

bool create_fence(struct vulkan_demo *demo) {
    VkResult result;
    VkFenceCreateInfo fence_create_info;

    memset(&fence_create_info, 0, sizeof(VkFenceCreateInfo));
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(demo->device, &fence_create_info, NULL,
                           &demo->render_fence);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateFence failed: %d\n", result);
        return false;
    }
    return true;
}

bool create_swap_chain_related_resources(struct vulkan_demo *demo) {
    if (!create_swap_chain(demo)) {
        return false;
    }
    if (!create_swap_chain_image_views(demo)) {
        return false;
    }
    if (!create_overlay_images(demo)) {
        return false;
    }
    if (!create_render_pass(demo)) {
        return false;
    }
    if (!create_framebuffers(demo)) {
        return false;
    }
    if (!create_graphics_pipeline(demo)) {
        return false;
    }
    return true;
}

bool destroy_swap_chain_related_resources(struct vulkan_demo *demo) {
    uint32_t i;
    VkResult result;

    result = vkQueueWaitIdle(demo->graphics_queue);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkQueueWaitIdle failed: %d\n", result);
        return false;
    }

    for (i = 0; i < demo->swap_chain_images_len; i++) {
        vkDestroyFramebuffer(demo->device, demo->framebuffers[i], NULL);
        vkDestroyImageView(demo->device, demo->overlay_image_views[i], NULL);
        vkDestroyImage(demo->device, demo->overlay_images[i], NULL);
        vkFreeMemory(demo->device, demo->overlay_image_memories[i], NULL);
        vkDestroyImageView(demo->device, demo->swap_chain_image_views[i], NULL);
    }
    vkDestroySwapchainKHR(demo->device, demo->swap_chain, NULL);
    vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
    vkDestroyPipeline(demo->device, demo->pipeline, NULL);
    vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
    return true;
}

bool create_demo_texture(struct vulkan_demo *demo) {
    VkResult result;
    VkMemoryRequirements mem_requirements;
    VkPhysicalDeviceMemoryProperties mem_properties;
    int found;
    uint32_t i;
    VkImageCreateInfo image_info;
    VkMemoryAllocateInfo alloc_info;
    VkImageViewCreateInfo image_view_info;
    VkBufferCreateInfo buffer_info;
    struct {
        VkDeviceMemory memory;
        VkBuffer buffer;
    } staging_buffer;
    void *data;
    VkCommandBuffer command_buffer;
    VkCommandBufferBeginInfo begin_info;
    VkImageMemoryBarrier image_transfer_dst_memory_barrier;
    VkBufferImageCopy buffer_copy_region;
    VkImageMemoryBarrier image_shader_memory_barrier;
    VkFence fence;
    VkFenceCreateInfo fence_create;
    VkSubmitInfo submit_info;

    memset(&image_info, 0, sizeof(VkImageCreateInfo));
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = 2;
    image_info.extent.height = 2;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8_UNORM;
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    memset(&alloc_info, 0, sizeof(VkMemoryAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    memset(&image_view_info, 0, sizeof(VkImageViewCreateInfo));
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = VK_FORMAT_R8_UNORM;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    result = vkCreateImage(demo->device, &image_info, NULL,
                           &demo->demo_texture_image);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateImage failed: %d\n", result);
        return false;
    }

    vkGetImageMemoryRequirements(demo->device, demo->demo_texture_image,
                                 &mem_requirements);

    alloc_info.allocationSize = mem_requirements.size;

    vkGetPhysicalDeviceMemoryProperties(demo->physical_device, &mem_properties);
    found = 0;
    for (i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((mem_requirements.memoryTypeBits & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags &
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "failed to find suitable memory for demo texture!\n");
        return false;
    }
    alloc_info.memoryTypeIndex = i;
    result = vkAllocateMemory(demo->device, &alloc_info, NULL,
                              &demo->demo_texture_memory);
    if (result != VK_SUCCESS) {
        fprintf(stderr,
                "failed to allocate vulkan memory for demo texture: %d!\n",
                result);
        return false;
    }
    result = vkBindImageMemory(demo->device, demo->demo_texture_image,
                               demo->demo_texture_memory, 0);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Couldn't bind image memory for demo texture: %d\n",
                result);
        return false;
    }

    image_view_info.image = demo->demo_texture_image;
    result = vkCreateImageView(demo->device, &image_view_info, NULL,
                               &demo->demo_texture_image_view);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateImageView failed for demo texture: %d\n",
                result);
        return false;
    }

    memset(&buffer_info, 0, sizeof(VkBufferCreateInfo));
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = alloc_info.allocationSize;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateBuffer(demo->device, &buffer_info, NULL,
                            &staging_buffer.buffer);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateBuffer failed for demo texture: %d\n", result);
        return false;
    }
    vkGetBufferMemoryRequirements(demo->device, staging_buffer.buffer,
                                  &mem_requirements);

    alloc_info.allocationSize = mem_requirements.size;
    found = 0;
    for (i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((mem_requirements.memoryTypeBits & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "failed to find suitable staging buffer memory for "
                        "demo texture!\n");
        return false;
    }
    alloc_info.memoryTypeIndex = i;
    result = vkAllocateMemory(demo->device, &alloc_info, NULL,
                              &staging_buffer.memory);
    if (!found) {
        fprintf(stderr, "vkAllocateMemory failed for demo texture: %d\n",
                result);
        return false;
    }
    result = vkBindBufferMemory(demo->device, staging_buffer.buffer,
                                staging_buffer.memory, 0);
    if (!found) {
        fprintf(stderr, "vkBindBufferMemory failed for demo texture: %d\n",
                result);
        return false;
    }

    result = vkMapMemory(demo->device, staging_buffer.memory, 0,
                         sizeof(uint32_t), 0, &data);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkMapMemory failed for demo texture: %d\n", result);
        return false;
    }
    *((uint32_t *)data) = 0x00FFFF00;
    vkUnmapMemory(demo->device, staging_buffer.memory);

    memset(&begin_info, 0, sizeof(VkCommandBufferBeginInfo));
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    command_buffer = demo->command_buffers[0];
    result = vkBeginCommandBuffer(command_buffer, &begin_info);

    memset(&image_transfer_dst_memory_barrier, 0, sizeof(VkImageMemoryBarrier));
    image_transfer_dst_memory_barrier.sType =
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_transfer_dst_memory_barrier.image = demo->demo_texture_image;
    image_transfer_dst_memory_barrier.srcQueueFamilyIndex =
        VK_QUEUE_FAMILY_IGNORED;
    image_transfer_dst_memory_barrier.dstQueueFamilyIndex =
        VK_QUEUE_FAMILY_IGNORED;
    image_transfer_dst_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_transfer_dst_memory_barrier.newLayout =
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_transfer_dst_memory_barrier.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;
    image_transfer_dst_memory_barrier.subresourceRange.levelCount = 1;
    image_transfer_dst_memory_barrier.subresourceRange.layerCount = 1;
    image_transfer_dst_memory_barrier.dstAccessMask =
        VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &image_transfer_dst_memory_barrier);

    memset(&buffer_copy_region, 0, sizeof(VkBufferImageCopy));
    buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy_region.imageSubresource.layerCount = 1;
    buffer_copy_region.imageExtent.width = 2;
    buffer_copy_region.imageExtent.height = 2;
    buffer_copy_region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        command_buffer, staging_buffer.buffer, demo->demo_texture_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

    memset(&image_shader_memory_barrier, 0, sizeof(VkImageMemoryBarrier));
    image_shader_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_shader_memory_barrier.image = demo->demo_texture_image;
    image_shader_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_shader_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_shader_memory_barrier.oldLayout =
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_shader_memory_barrier.newLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_shader_memory_barrier.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;
    image_shader_memory_barrier.subresourceRange.levelCount = 1;
    image_shader_memory_barrier.subresourceRange.layerCount = 1;
    image_shader_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    image_shader_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                         NULL, 1, &image_shader_memory_barrier);

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEndCommandBuffer failed for demo texture: %d\n",
                result);
        return false;
    }

    memset(&fence_create, 0, sizeof(VkFenceCreateInfo));
    fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    result = vkCreateFence(demo->device, &fence_create, NULL, &fence);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateFence failed for demo texture: %d\n", result);
        return false;
    }

    memset(&submit_info, 0, sizeof(VkSubmitInfo));
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    result = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, fence);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkQueueSubmit failed for demo texture: %d\n", result);
        return false;
    }
    result = vkWaitForFences(demo->device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkWaitForFences failed for demo texture: %d\n",
                result);
        return false;
    }

    vkDestroyBuffer(demo->device, staging_buffer.buffer, NULL);
    vkFreeMemory(demo->device, staging_buffer.memory, NULL);
    vkDestroyFence(demo->device, fence, NULL);

    return true;
}

bool create_vulkan_demo(struct vulkan_demo *demo) {
    if (!create_instance(demo)) {
        return false;
    }
    if (!create_surface(demo)) {
        return false;
    }
    if (!create_physical_device(demo)) {
        return false;
    }
    if (!create_logical_device(demo)) {
        return false;
    }
    if (!create_sampler(demo)) {
        return false;
    }
    if (!create_descriptor_set_layout(demo)) {
        return false;
    }
    if (!create_swap_chain_related_resources(demo)) {
        return false;
    }
    if (!create_descriptor_pool(demo)) {
        return false;
    }
    if (!create_descriptor_sets(demo)) {
        return false;
    }
    if (!create_command_pool(demo)) {
        return false;
    }
    if (!create_command_buffers(demo)) {
        return false;
    }
    if (!create_semaphores(demo)) {
        return false;
    }
    if (!create_fence(demo)) {
        return false;
    }
    if (!create_demo_texture(demo)) {
        return false;
    }

    return true;
}

bool recreate_swap_chain(struct vulkan_demo *demo) {
    printf("recreating swapchain\n");
    if (!destroy_swap_chain_related_resources(demo)) {
        return false;
    }
    if (!create_swap_chain_related_resources(demo)) {
        return false;
    }

    update_descriptor_sets(demo);

    nk_sdl_resize(demo->swap_chain_image_extent.width,
                  demo->swap_chain_image_extent.height);

    return true;
}

bool render(struct vulkan_demo *demo, struct nk_colorf *bg,
            VkSemaphore wait_semaphore, uint32_t image_index) {
    VkCommandBufferBeginInfo command_buffer_begin_info;
    VkCommandBuffer command_buffer;
    VkRenderPassBeginInfo render_pass_info;
    VkSubmitInfo submit_info;
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkResult result;
    VkPresentInfoKHR present_info;
    VkClearValue clear_color;

    memcpy(&clear_color.color, bg, sizeof(VkClearColorValue));

    memset(&command_buffer_begin_info, 0, sizeof(VkCommandBufferBeginInfo));
    command_buffer_begin_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    command_buffer = demo->command_buffers[image_index];
    result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkBeginCommandBuffer failed: %d\n", result);
        return false;
    }

    memset(&render_pass_info, 0, sizeof(VkRenderPassBeginInfo));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = demo->render_pass;
    render_pass_info.framebuffer = demo->framebuffers[image_index];
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = demo->swap_chain_image_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      demo->pipeline);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            demo->pipeline_layout, 0, 1,
                            &demo->descriptor_sets[image_index], 0, NULL);
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkEndCommandBuffer failed: %d\n", result);
        return false;
    }

    memset(&submit_info, 0, sizeof(VkSubmitInfo));
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &demo->command_buffers[image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &demo->render_finished;

    result = vkQueueSubmit(demo->graphics_queue, 1, &submit_info,
                           demo->render_fence);

    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkQueueSubmit failed: %d\n", result);
        return false;
    }

    memset(&present_info, 0, sizeof(VkPresentInfoKHR));
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &demo->render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &demo->swap_chain;
    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(demo->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        if (!recreate_swap_chain(demo)) {
            fprintf(stderr, "failed to recreate swap chain!\n");
        }
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "vkQueuePresentKHR failed: %d\n", result);
        return false;
    }
    return true;
}

VkResult
destroy_debug_utils_messenger_ext(VkInstance instance,
                                  VkDebugUtilsMessengerEXT debugMessenger,
                                  const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
        return VK_SUCCESS;
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

bool cleanup(struct vulkan_demo *demo) {
    VkResult result;

    printf("cleaning up\n");
    result = vkDeviceWaitIdle(demo->device);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkDeviceWaitIdle failed: %d\n", result);
        return false;
    }

    destroy_swap_chain_related_resources(demo);

    vkFreeCommandBuffers(demo->device, demo->command_pool,
                         demo->swap_chain_images_len, demo->command_buffers);
    vkDestroyCommandPool(demo->device, demo->command_pool, NULL);
    vkDestroySampler(demo->device, demo->sampler, NULL);
    vkDestroySemaphore(demo->device, demo->render_finished, NULL);
    vkDestroySemaphore(demo->device, demo->image_available, NULL);
    vkDestroyFence(demo->device, demo->render_fence, NULL);

    vkDestroyImage(demo->device, demo->demo_texture_image, NULL);
    vkDestroyImageView(demo->device, demo->demo_texture_image_view, NULL);
    vkFreeMemory(demo->device, demo->demo_texture_memory, NULL);

    vkDestroyDescriptorSetLayout(demo->device, demo->descriptor_set_layout,
                                 NULL);
    vkDestroyDescriptorPool(demo->device, demo->descriptor_pool, NULL);

    vkDestroyDevice(demo->device, NULL);
    vkDestroySurfaceKHR(demo->instance, demo->surface, NULL);

    if (demo->debug_messenger) {
        result = destroy_debug_utils_messenger_ext(demo->instance,
                                                   demo->debug_messenger, NULL);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "Couldn't destroy debug messenger: %d\n", result);
            return false;
        }
    }
    vkDestroyInstance(demo->instance, NULL);
    if (demo->swap_chain_images) {
        free(demo->swap_chain_images);
    }
    if (demo->swap_chain_image_views) {
        free(demo->swap_chain_image_views);
    }

    if (demo->overlay_images) {
        free(demo->overlay_images);
    }
    if (demo->overlay_image_views) {
        free(demo->overlay_image_views);
    }
    if (demo->overlay_image_memories) {
        free(demo->overlay_image_memories);
    }

    if (demo->descriptor_sets) {
        free(demo->descriptor_sets);
    }
    if (demo->framebuffers) {
        free(demo->framebuffers);
    }
    if (demo->command_buffers) {
        free(demo->command_buffers);
    }

    SDL_DestroyWindow(demo->win);

    return true;
}

int main(void) {
    struct vulkan_demo demo;
    struct nk_context *ctx;
    struct nk_colorf bg;
    struct nk_image img;
    uint32_t image_index;
    VkResult result;
    VkSemaphore nk_semaphore;

    #ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
    #endif

    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "[SDL] failed to init!\n");
        exit(1);
    }

    memset(&demo, 0, sizeof(struct vulkan_demo));
    demo.win = SDL_CreateWindow(
        "Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!create_vulkan_demo(&demo)) {
        fprintf(stderr, "failed to create vulkan demo!\n");
        exit(1);
    }
    ctx = nk_sdl_init(demo.win, demo.device, demo.physical_device,
                      demo.indices.graphics, demo.overlay_image_views,
                      demo.swap_chain_images_len, demo.swap_chain_image_format,
                      0, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/DroidSans.ttf", 14, 0);*/
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/ProggyClean.ttf", 12, 0);*/
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas,
         * "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
        nk_sdl_font_stash_end(demo.graphics_queue);
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        /*nk_style_set_font(ctx, &droid->handle);*/
    }

    img = nk_image_ptr(demo.demo_texture_image_view);
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    while (true) {
        SDL_Event evt;

        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                goto cleanup;
            if (evt.type == SDL_WINDOWEVENT &&
                evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                recreate_swap_chain(&demo);
            nk_sdl_handle_event(&evt);
        }
        nk_sdl_handle_grab(); /* optional grabbing behavior */
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            enum { EASY, HARD };
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");

            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY))
                op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD))
                op = HARD;

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg),
                                     nk_vec2(nk_widget_width(ctx), 400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        /* Bindless Texture */
        if (nk_begin(ctx, "Texture", nk_rect(500, 300, 200, 200),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
            struct nk_rect total_space = nk_window_get_content_region(ctx);
            nk_draw_image(canvas, total_space, &img, nk_white);
        }
        nk_end(ctx);

        /* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
        calculator(ctx);
#endif
#ifdef INCLUDE_CANVAS
        canvas(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
        overview(ctx);
#endif
#ifdef INCLUDE_CONFIGURATOR
        style_configurator(ctx, color_table);
#endif
#ifdef INCLUDE_NODE_EDITOR
        node_editor(ctx);
#endif
        /* ----------------------------------------- */

        result = vkWaitForFences(demo.device, 1, &demo.render_fence, VK_TRUE,
                                 UINT64_MAX);

        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkWaitForFences failed: %d\n", result);
            return false;
        }

        result = vkResetFences(demo.device, 1, &demo.render_fence);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "vkResetFences failed: %d\n", result);
            return false;
        }

        result =
            vkAcquireNextImageKHR(demo.device, demo.swap_chain, UINT64_MAX,
                                  demo.image_available, NULL, &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swap_chain(&demo);

            /* If vkAcquireNextImageKHR does not successfully acquire an image,
             * semaphore and fence are unaffected. */
            continue;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            fprintf(stderr, "vkAcquireNextImageKHR failed: %d\n", result);
            return false;
        }

        /* Draw */
        nk_semaphore = nk_sdl_render(demo.graphics_queue, image_index,
                                     demo.image_available, NK_ANTI_ALIASING_ON);
        if (!render(&demo, &bg, nk_semaphore, image_index)) {
            fprintf(stderr, "render failed\n");
            return false;
        }
    }
cleanup:
    nk_sdl_shutdown();
    cleanup(&demo);
    SDL_Quit();
    return 0;
}
