/*
 * Nuklear - 1.32.0 - public domain
 * no warranty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_SDL_VULKAN_H_
#define NK_SDL_VULKAN_H_

// NUKLEAR_SHADERS_START
// will be replaced with the real shader code
// so we can have some ide support while editing the .in file
#include "nuklear.h"

unsigned char nuklearshaders_nuklear_vert_spv[] = {};
unsigned int nuklearshaders_nuklear_vert_spv_len = 0;
unsigned char nuklearshaders_nuklear_frag_spv[] = {};
unsigned int nuklearshaders_nuklear_frag_spv_len = 0;
// NUKLEAR_SHADERS_END

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <SDL2/SDL.h>

enum nk_sdl_init_state { NK_SDL_DEFAULT = 0 };

NK_API struct nk_context *
nk_sdl_init(SDL_Window *win, VkDevice logical_device,
            VkPhysicalDevice physical_device,
            uint32_t graphics_queue_family_index, VkImageView *image_views,
            uint32_t image_views_len, VkFormat color_format,
            enum nk_sdl_init_state init_state, VkDeviceSize max_vertex_buffer,
            VkDeviceSize max_element_buffer);
NK_API void nk_sdl_shutdown(void);
NK_API void nk_sdl_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void nk_sdl_font_stash_end(VkQueue graphics_queue);
NK_API int nk_sdl_handle_event(SDL_Event *evt);
NK_API VkSemaphore nk_sdl_render(VkQueue graphics_queue, uint32_t buffer_index,
                                 VkSemaphore wait_semaphore,
                                 enum nk_anti_aliasing AA);
NK_API void nk_sdl_resize(uint32_t framebuffer_width,
                          uint32_t framebuffer_height);
NK_API void nk_sdl_device_destroy(void);
NK_API void
nk_sdl_device_create(VkDevice logical_device, VkPhysicalDevice physical_device,
                     uint32_t graphics_queue_family_index,
                     VkImageView *image_views, uint32_t image_views_len,
                     VkFormat color_format, VkDeviceSize max_vertex_buffer,
                     VkDeviceSize max_element_buffer,
                     uint32_t framebuffer_width, uint32_t framebuffer_height);
NK_API void nk_sdl_handle_grab(void);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDL_VULKAN_IMPLEMENTATION
#undef NK_SDL_VULKAN_IMPLEMENTATION
#include <stdlib.h>

#ifndef NK_SDL_TEXT_MAX
#define NK_SDL_TEXT_MAX 256
#endif
#ifndef NK_SDL_MAX_TEXTURES
#define NK_SDL_MAX_TEXTURES 256
#endif
#ifndef NK_SDL_MAX_KEYS
#define NK_SDL_MAX_KEYS 32
#endif

#define VK_COLOR_COMPONENT_MASK_RGBA                                           \
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |                      \
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT

struct nk_sdl_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

struct nk_vulkan_texture_descriptor_set {
    VkImageView image_view;
    VkDescriptorSet descriptor_set;
};

struct nk_sdl_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    int max_vertex_buffer;
    int max_element_buffer;
    VkDevice logical_device;
    VkPhysicalDevice physical_device;
    VkImageView *image_views;
    uint32_t image_views_len;
    VkFormat color_format;
    VkFramebuffer *framebuffers;
    uint32_t framebuffers_len;
    VkCommandBuffer *command_buffers;
    uint32_t command_buffers_len;
    VkSampler sampler;
    VkCommandPool command_pool;
    VkSemaphore render_completed;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    void *mapped_vertex;
    VkBuffer index_buffer;
    VkDeviceMemory index_memory;
    void *mapped_index;
    VkBuffer uniform_buffer;
    VkDeviceMemory uniform_memory;
    void *mapped_uniform;
    VkRenderPass render_pass;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout uniform_descriptor_set_layout;
    VkDescriptorSet uniform_descriptor_set;
    VkDescriptorSetLayout texture_descriptor_set_layout;
    struct nk_vulkan_texture_descriptor_set *texture_descriptor_sets;
    uint32_t texture_descriptor_sets_len;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkImage font_image;
    VkImageView font_image_view;
    VkDeviceMemory font_memory;
};

static struct nk_sdl {
    SDL_Window *win;
    int width, height;
    int display_width, display_height;
    struct nk_sdl_device vulkan;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    unsigned int text[NK_SDL_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    uint64_t delta_time_milliseconds_last;
} sdl;

struct Mat4f {
    float m[16];
};

NK_INTERN uint32_t nk_sdl_find_memory_index(VkPhysicalDevice physical_device,
                                            uint32_t type_filter,
                                            VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    uint32_t i;

    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
    for (i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    assert(0);
    return 0;
}

NK_INTERN void nk_sdl_create_sampler(struct nk_sdl_device *dev) {
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

    result = vkCreateSampler(dev->logical_device, &sampler_info, NULL,
                             &dev->sampler);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void
nk_sdl_create_command_pool(struct nk_sdl_device *dev,
                           uint32_t graphics_queue_family_index) {
    VkResult result;
    VkCommandPoolCreateInfo pool_info;
    memset(&pool_info, 0, sizeof(VkCommandPoolCreateInfo));

    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(dev->logical_device, &pool_info, NULL,
                                 &dev->command_pool);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_create_command_buffers(struct nk_sdl_device *dev) {
    VkResult result;
    VkCommandBufferAllocateInfo allocate_info;
    memset(&allocate_info, 0, sizeof(VkCommandBufferAllocateInfo));

    dev->command_buffers = (VkCommandBuffer *)malloc(dev->image_views_len *
                                                     sizeof(VkCommandBuffer));
    dev->command_buffers_len = dev->image_views_len;

    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = dev->command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = dev->command_buffers_len;

    result = vkAllocateCommandBuffers(dev->logical_device, &allocate_info,
                                      dev->command_buffers);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_create_semaphore(struct nk_sdl_device *dev) {
    VkResult result;
    VkSemaphoreCreateInfo semaphore_info;
    memset(&semaphore_info, 0, sizeof(VkSemaphoreCreateInfo));

    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    result = (vkCreateSemaphore(dev->logical_device, &semaphore_info, NULL,
                                &dev->render_completed));
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_create_buffer_and_memory(struct nk_sdl_device *dev,
                                               VkBuffer *buffer,
                                               VkBufferUsageFlags usage,
                                               VkDeviceMemory *memory,
                                               VkDeviceSize size) {
    VkMemoryRequirements mem_reqs;
    VkResult result;
    VkBufferCreateInfo buffer_info;
    VkMemoryAllocateInfo alloc_info;

    memset(&buffer_info, 0, sizeof(VkBufferCreateInfo));
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateBuffer(dev->logical_device, &buffer_info, NULL, buffer);
    NK_ASSERT(result == VK_SUCCESS);

    vkGetBufferMemoryRequirements(dev->logical_device, *buffer, &mem_reqs);

    memset(&alloc_info, 0, sizeof(VkMemoryAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex =
        nk_sdl_find_memory_index(dev->physical_device, mem_reqs.memoryTypeBits,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(dev->logical_device, &alloc_info, NULL, memory);
    NK_ASSERT(result == VK_SUCCESS);
    result = vkBindBufferMemory(dev->logical_device, *buffer, *memory, 0);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_create_render_pass(struct nk_sdl_device *dev) {
    VkAttachmentDescription attachment;
    VkAttachmentReference color_reference;
    VkSubpassDependency subpass_dependency;
    VkSubpassDescription subpass_description;
    VkRenderPassCreateInfo render_pass_info;
    VkResult result;

    memset(&attachment, 0, sizeof(VkAttachmentDescription));
    attachment.format = dev->color_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    memset(&color_reference, 0, sizeof(VkAttachmentReference));
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    memset(&subpass_dependency, 0, sizeof(VkSubpassDependency));
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    memset(&subpass_description, 0, sizeof(VkSubpassDescription));
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_reference;

    memset(&render_pass_info, 0, sizeof(VkRenderPassCreateInfo));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_description;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &subpass_dependency;

    result = vkCreateRenderPass(dev->logical_device, &render_pass_info, NULL,
                                &dev->render_pass);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_create_framebuffers(struct nk_sdl_device *dev,
                                          uint32_t framebuffer_width,
                                          uint32_t framebuffer_height) {

    VkFramebufferCreateInfo framebuffer_create_info;
    uint32_t i;
    VkResult result;

    dev->framebuffers =
        (VkFramebuffer *)malloc(dev->image_views_len * sizeof(VkFramebuffer));

    memset(&framebuffer_create_info, 0, sizeof(VkFramebufferCreateInfo));
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = dev->render_pass;
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.width = framebuffer_width;
    framebuffer_create_info.height = framebuffer_height;
    framebuffer_create_info.layers = 1;
    for (i = 0; i < dev->image_views_len; i++) {
        framebuffer_create_info.pAttachments = &dev->image_views[i];
        result =
            vkCreateFramebuffer(dev->logical_device, &framebuffer_create_info,
                                NULL, &dev->framebuffers[i]);
        NK_ASSERT(result == VK_SUCCESS);
    }
    dev->framebuffers_len = dev->image_views_len;
}

NK_INTERN void nk_sdl_create_descriptor_pool(struct nk_sdl_device *dev) {
    VkDescriptorPoolSize pool_sizes[2];
    VkDescriptorPoolCreateInfo pool_info;
    VkResult result;

    memset(&pool_sizes, 0, sizeof(VkDescriptorPoolSize) * 2);
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = NK_SDL_MAX_TEXTURES;

    memset(&pool_info, 0, sizeof(VkDescriptorPoolCreateInfo));
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = 1 + NK_SDL_MAX_TEXTURES;

    result = vkCreateDescriptorPool(dev->logical_device, &pool_info, NULL,
                                    &dev->descriptor_pool);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void
nk_sdl_create_uniform_descriptor_set_layout(struct nk_sdl_device *dev) {
    VkDescriptorSetLayoutBinding binding;
    VkDescriptorSetLayoutCreateInfo descriptor_set_info;
    VkResult result;

    memset(&binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    memset(&descriptor_set_info, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    descriptor_set_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_info.bindingCount = 1;
    descriptor_set_info.pBindings = &binding;

    result =
        vkCreateDescriptorSetLayout(dev->logical_device, &descriptor_set_info,
                                    NULL, &dev->uniform_descriptor_set_layout);

    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void
nk_sdl_create_and_update_uniform_descriptor_set(struct nk_sdl_device *dev) {
    VkDescriptorSetAllocateInfo allocate_info;
    VkDescriptorBufferInfo buffer_info;
    VkWriteDescriptorSet descriptor_write;
    VkResult result;

    memset(&allocate_info, 0, sizeof(VkDescriptorSetAllocateInfo));
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = dev->descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &dev->uniform_descriptor_set_layout;

    result = vkAllocateDescriptorSets(dev->logical_device, &allocate_info,
                                      &dev->uniform_descriptor_set);
    NK_ASSERT(result == VK_SUCCESS);

    memset(&buffer_info, 0, sizeof(VkDescriptorBufferInfo));
    buffer_info.buffer = dev->uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct Mat4f);

    memset(&descriptor_write, 0, sizeof(VkWriteDescriptorSet));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = dev->uniform_descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(dev->logical_device, 1, &descriptor_write, 0, NULL);
}

NK_INTERN void
nk_sdl_create_texture_descriptor_set_layout(struct nk_sdl_device *dev) {
    VkDescriptorSetLayoutBinding binding;
    VkDescriptorSetLayoutCreateInfo descriptor_set_info;
    VkResult result;

    memset(&binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    memset(&descriptor_set_info, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    descriptor_set_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_info.bindingCount = 1;
    descriptor_set_info.pBindings = &binding;

    result =
        vkCreateDescriptorSetLayout(dev->logical_device, &descriptor_set_info,
                                    NULL, &dev->texture_descriptor_set_layout);

    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void
nk_sdl_create_texture_descriptor_sets(struct nk_sdl_device *dev) {
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkDescriptorSet *descriptor_sets;
    VkDescriptorSetAllocateInfo allocate_info;
    VkResult result;
    int i;

    descriptor_set_layouts = (VkDescriptorSetLayout *)malloc(
        NK_SDL_MAX_TEXTURES * sizeof(VkDescriptorSetLayout));
    descriptor_sets = (VkDescriptorSet *)malloc(NK_SDL_MAX_TEXTURES *
                                                sizeof(VkDescriptorSet));

    dev->texture_descriptor_sets =
        (struct nk_vulkan_texture_descriptor_set *)malloc(
            NK_SDL_MAX_TEXTURES *
            sizeof(struct nk_vulkan_texture_descriptor_set));
    dev->texture_descriptor_sets_len = 0;

    for (i = 0; i < NK_SDL_MAX_TEXTURES; i++) {
        descriptor_set_layouts[i] = dev->texture_descriptor_set_layout;
        descriptor_sets[i] = dev->texture_descriptor_sets[i].descriptor_set;
    }

    memset(&allocate_info, 0, sizeof(VkDescriptorSetAllocateInfo));
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = dev->descriptor_pool;
    allocate_info.descriptorSetCount = NK_SDL_MAX_TEXTURES;
    allocate_info.pSetLayouts = descriptor_set_layouts;

    result = vkAllocateDescriptorSets(dev->logical_device, &allocate_info,
                                      descriptor_sets);
    NK_ASSERT(result == VK_SUCCESS);

    for (i = 0; i < NK_SDL_MAX_TEXTURES; i++) {
        dev->texture_descriptor_sets[i].descriptor_set = descriptor_sets[i];
    }
    free(descriptor_set_layouts);
    free(descriptor_sets);
}

NK_INTERN void nk_sdl_create_pipeline_layout(struct nk_sdl_device *dev) {
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    VkDescriptorSetLayout descriptor_set_layouts[2];
    VkResult result;

    descriptor_set_layouts[0] = dev->uniform_descriptor_set_layout;
    descriptor_set_layouts[1] = dev->texture_descriptor_set_layout;

    memset(&pipeline_layout_info, 0, sizeof(VkPipelineLayoutCreateInfo));
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 2;
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts;

    result = (vkCreatePipelineLayout(dev->logical_device, &pipeline_layout_info,
                                     NULL, &dev->pipeline_layout));
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN VkPipelineShaderStageCreateInfo
nk_sdl_create_shader(struct nk_sdl_device *dev, unsigned char *spv_shader,
                     uint32_t size, VkShaderStageFlagBits stage_bit) {
    VkShaderModuleCreateInfo create_info;
    VkPipelineShaderStageCreateInfo shader_info;
    VkShaderModule module = NULL;
    VkResult result;

    memset(&create_info, 0, sizeof(VkShaderModuleCreateInfo));
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = size;
    create_info.pCode = (const uint32_t *)spv_shader;
    result =
        vkCreateShaderModule(dev->logical_device, &create_info, NULL, &module);
    NK_ASSERT(result == VK_SUCCESS);

    memset(&shader_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
    shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info.stage = stage_bit;
    shader_info.module = module;
    shader_info.pName = "main";
    return shader_info;
}

NK_INTERN void nk_sdl_create_pipeline(struct nk_sdl_device *dev) {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
    VkPipelineRasterizationStateCreateInfo rasterization_state;
    VkPipelineColorBlendAttachmentState attachment_state = {
        VK_TRUE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_MASK_RGBA,
    };
    VkPipelineColorBlendStateCreateInfo color_blend_state;
    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineMultisampleStateCreateInfo multisample_state;
    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state;
    VkPipelineShaderStageCreateInfo shader_stages[2];
    VkVertexInputBindingDescription vertex_input_info;
    VkVertexInputAttributeDescription vertex_attribute_description[3];
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkResult result;

    memset(&input_assembly_state, 0,
           sizeof(VkPipelineInputAssemblyStateCreateInfo));
    input_assembly_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = VK_FALSE;

    memset(&rasterization_state, 0,
           sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterization_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode = VK_CULL_MODE_NONE;
    rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state.lineWidth = 1.0f;

    memset(&color_blend_state, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    color_blend_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment_state;

    memset(&viewport_state, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    memset(&multisample_state, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisample_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    memset(&dynamic_state, 0, sizeof(VkPipelineDynamicStateCreateInfo));
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pDynamicStates = dynamic_states;
    dynamic_state.dynamicStateCount = 2;

    shader_stages[0] = nk_sdl_create_shader(
        dev, nuklearshaders_nuklear_vert_spv,
        nuklearshaders_nuklear_vert_spv_len, VK_SHADER_STAGE_VERTEX_BIT);
    shader_stages[1] = nk_sdl_create_shader(
        dev, nuklearshaders_nuklear_frag_spv,
        nuklearshaders_nuklear_frag_spv_len, VK_SHADER_STAGE_FRAGMENT_BIT);

    memset(&vertex_input_info, 0, sizeof(VkVertexInputBindingDescription));
    vertex_input_info.binding = 0;
    vertex_input_info.stride = sizeof(struct nk_sdl_vertex);
    vertex_input_info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    memset(&vertex_attribute_description, 0,
           sizeof(VkVertexInputAttributeDescription) * 3);
    vertex_attribute_description[0].location = 0;
    vertex_attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attribute_description[0].offset =
        NK_OFFSETOF(struct nk_sdl_vertex, position);
    vertex_attribute_description[1].location = 1;
    vertex_attribute_description[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attribute_description[1].offset =
        NK_OFFSETOF(struct nk_sdl_vertex, uv);
    vertex_attribute_description[2].location = 2;
    vertex_attribute_description[2].format = VK_FORMAT_R8G8B8A8_UINT;
    vertex_attribute_description[2].offset =
        NK_OFFSETOF(struct nk_sdl_vertex, col);

    memset(&vertex_input, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertex_input.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &vertex_input_info;
    vertex_input.vertexAttributeDescriptionCount = 3;
    vertex_input.pVertexAttributeDescriptions = vertex_attribute_description;

    memset(&pipeline_info, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.flags = 0;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly_state;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterization_state;
    pipeline_info.pMultisampleState = &multisample_state;
    pipeline_info.pColorBlendState = &color_blend_state;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = dev->pipeline_layout;
    pipeline_info.renderPass = dev->render_pass;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = NULL;

    result = vkCreateGraphicsPipelines(dev->logical_device, NULL, 1,
                                       &pipeline_info, NULL, &dev->pipeline);
    NK_ASSERT(result == VK_SUCCESS);

    vkDestroyShaderModule(dev->logical_device, shader_stages[0].module, NULL);
    vkDestroyShaderModule(dev->logical_device, shader_stages[1].module, NULL);
}

NK_INTERN void nk_sdl_create_render_resources(struct nk_sdl_device *dev,
                                              uint32_t framebuffer_width,
                                              uint32_t framebuffer_height) {
    nk_sdl_create_render_pass(dev);
    nk_sdl_create_framebuffers(dev, framebuffer_width, framebuffer_height);
    nk_sdl_create_descriptor_pool(dev);
    nk_sdl_create_uniform_descriptor_set_layout(dev);
    nk_sdl_create_and_update_uniform_descriptor_set(dev);
    nk_sdl_create_texture_descriptor_set_layout(dev);
    nk_sdl_create_texture_descriptor_sets(dev);
    nk_sdl_create_pipeline_layout(dev);
    nk_sdl_create_pipeline(dev);
}

NK_API void
nk_sdl_device_create(VkDevice logical_device, VkPhysicalDevice physical_device,
                     uint32_t graphics_queue_family_index,
                     VkImageView *image_views, uint32_t image_views_len,
                     VkFormat color_format, VkDeviceSize max_vertex_buffer,
                     VkDeviceSize max_element_buffer,
                     uint32_t framebuffer_width, uint32_t framebuffer_height) {
    struct nk_sdl_device *dev = &sdl.vulkan;
    dev->max_vertex_buffer = max_vertex_buffer;
    dev->max_element_buffer = max_element_buffer;
    nk_buffer_init_default(&dev->cmds);
    dev->logical_device = logical_device;
    dev->physical_device = physical_device;
    dev->image_views = image_views;
    dev->image_views_len = image_views_len;
    dev->color_format = color_format;
    dev->framebuffers = NULL;
    dev->framebuffers_len = 0;

    nk_sdl_create_sampler(dev);
    nk_sdl_create_command_pool(dev, graphics_queue_family_index);
    nk_sdl_create_command_buffers(dev);
    nk_sdl_create_semaphore(dev);

    nk_sdl_create_buffer_and_memory(dev, &dev->vertex_buffer,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    &dev->vertex_memory, max_vertex_buffer);
    nk_sdl_create_buffer_and_memory(dev, &dev->index_buffer,
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                    &dev->index_memory, max_element_buffer);
    nk_sdl_create_buffer_and_memory(dev, &dev->uniform_buffer,
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                    &dev->uniform_memory, sizeof(struct Mat4f));

    vkMapMemory(dev->logical_device, dev->vertex_memory, 0, max_vertex_buffer,
                0, &dev->mapped_vertex);
    vkMapMemory(dev->logical_device, dev->index_memory, 0, max_element_buffer,
                0, &dev->mapped_index);
    vkMapMemory(dev->logical_device, dev->uniform_memory, 0,
                sizeof(struct Mat4f), 0, &dev->mapped_uniform);

    nk_sdl_create_render_resources(dev, framebuffer_width, framebuffer_height);
}

NK_INTERN void nk_sdl_device_upload_atlas(VkQueue graphics_queue,
                                          const void *image, int width,
                                          int height) {
    struct nk_sdl_device *dev = &sdl.vulkan;

    VkImageCreateInfo image_info;
    VkResult result;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo alloc_info;
    VkBufferCreateInfo buffer_info;
    uint8_t *data = 0;
    VkCommandBufferBeginInfo begin_info;
    VkCommandBuffer command_buffer;
    VkImageMemoryBarrier image_memory_barrier;
    VkBufferImageCopy buffer_copy_region;
    VkImageMemoryBarrier image_shader_memory_barrier;
    VkFence fence;
    VkFenceCreateInfo fence_create;
    VkSubmitInfo submit_info;
    VkImageViewCreateInfo image_view_info;
    struct {
        VkDeviceMemory memory;
        VkBuffer buffer;
    } staging_buffer;

    memset(&image_info, 0, sizeof(VkImageCreateInfo));
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent.width = (uint32_t)width;
    image_info.extent.height = (uint32_t)height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    result =
        vkCreateImage(dev->logical_device, &image_info, NULL, &dev->font_image);
    NK_ASSERT(result == VK_SUCCESS);

    vkGetImageMemoryRequirements(dev->logical_device, dev->font_image,
                                 &mem_reqs);

    memset(&alloc_info, 0, sizeof(VkMemoryAllocateInfo));
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex =
        nk_sdl_find_memory_index(dev->physical_device, mem_reqs.memoryTypeBits,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    result = vkAllocateMemory(dev->logical_device, &alloc_info, NULL,
                              &dev->font_memory);
    NK_ASSERT(result == VK_SUCCESS);
    result = vkBindImageMemory(dev->logical_device, dev->font_image,
                               dev->font_memory, 0);
    NK_ASSERT(result == VK_SUCCESS);

    memset(&buffer_info, 0, sizeof(VkBufferCreateInfo));
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = alloc_info.allocationSize;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateBuffer(dev->logical_device, &buffer_info, NULL,
                            &staging_buffer.buffer);
    NK_ASSERT(result == VK_SUCCESS);
    vkGetBufferMemoryRequirements(dev->logical_device, staging_buffer.buffer,
                                  &mem_reqs);

    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex =
        nk_sdl_find_memory_index(dev->physical_device, mem_reqs.memoryTypeBits,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(dev->logical_device, &alloc_info, NULL,
                              &staging_buffer.memory);
    NK_ASSERT(result == VK_SUCCESS);
    result = vkBindBufferMemory(dev->logical_device, staging_buffer.buffer,
                                staging_buffer.memory, 0);
    NK_ASSERT(result == VK_SUCCESS);

    result = vkMapMemory(dev->logical_device, staging_buffer.memory, 0,
                         alloc_info.allocationSize, 0, (void **)&data);
    NK_ASSERT(result == VK_SUCCESS);
    memcpy(data, image, width * height * 4);
    vkUnmapMemory(dev->logical_device, staging_buffer.memory);

    memset(&begin_info, 0, sizeof(VkCommandBufferBeginInfo));
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    NK_ASSERT(dev->command_buffers_len > 0);
    /*
    use the same command buffer as for render as we are regenerating the
    buffer during render anyway
    */
    command_buffer = dev->command_buffers[0];
    result = vkBeginCommandBuffer(command_buffer, &begin_info);
    NK_ASSERT(result == VK_SUCCESS);

    memset(&image_memory_barrier, 0, sizeof(VkImageMemoryBarrier));
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.image = dev->font_image;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.levelCount = 1;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &image_memory_barrier);

    memset(&buffer_copy_region, 0, sizeof(VkBufferImageCopy));
    buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy_region.imageSubresource.layerCount = 1;
    buffer_copy_region.imageExtent.width = (uint32_t)width;
    buffer_copy_region.imageExtent.height = (uint32_t)height;
    buffer_copy_region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        command_buffer, staging_buffer.buffer, dev->font_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

    memset(&image_shader_memory_barrier, 0, sizeof(VkImageMemoryBarrier));
    image_shader_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_shader_memory_barrier.image = dev->font_image;
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
    NK_ASSERT(result == VK_SUCCESS);

    memset(&fence_create, 0, sizeof(VkFenceCreateInfo));
    fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    result = vkCreateFence(dev->logical_device, &fence_create, NULL, &fence);
    NK_ASSERT(result == VK_SUCCESS);

    memset(&submit_info, 0, sizeof(VkSubmitInfo));
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    result = vkQueueSubmit(graphics_queue, 1, &submit_info, fence);
    NK_ASSERT(result == VK_SUCCESS);
    result =
        vkWaitForFences(dev->logical_device, 1, &fence, VK_TRUE, UINT64_MAX);
    NK_ASSERT(result == VK_SUCCESS);

    vkDestroyFence(dev->logical_device, fence, NULL);

    vkFreeMemory(dev->logical_device, staging_buffer.memory, NULL);
    vkDestroyBuffer(dev->logical_device, staging_buffer.buffer, NULL);

    memset(&image_view_info, 0, sizeof(VkImageViewCreateInfo));
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = dev->font_image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = image_info.format;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.layerCount = 1;
    image_view_info.subresourceRange.levelCount = 1;

    result = vkCreateImageView(dev->logical_device, &image_view_info, NULL,
                               &dev->font_image_view);
    NK_ASSERT(result == VK_SUCCESS);
}

NK_INTERN void nk_sdl_destroy_render_resources(struct nk_sdl_device *dev) {
    uint32_t i;

    vkDestroyPipeline(dev->logical_device, dev->pipeline, NULL);
    vkDestroyPipelineLayout(dev->logical_device, dev->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(dev->logical_device,
                                 dev->texture_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(dev->logical_device,
                                 dev->uniform_descriptor_set_layout, NULL);
    vkDestroyDescriptorPool(dev->logical_device, dev->descriptor_pool, NULL);
    for (i = 0; i < dev->framebuffers_len; i++) {
        vkDestroyFramebuffer(dev->logical_device, dev->framebuffers[i], NULL);
    }
    free(dev->framebuffers);
    dev->framebuffers_len = 0;
    free(dev->texture_descriptor_sets);
    dev->texture_descriptor_sets_len = 0;
    vkDestroyRenderPass(dev->logical_device, dev->render_pass, NULL);
}

NK_API void nk_sdl_resize(uint32_t framebuffer_width,
                          uint32_t framebuffer_height) {
    struct nk_sdl_device *dev = &sdl.vulkan;

    SDL_GetWindowSize(sdl.win, &sdl.width, &sdl.height);
    sdl.display_width = framebuffer_width;
    sdl.display_height = framebuffer_height;

    nk_sdl_destroy_render_resources(dev);
    nk_sdl_create_render_resources(dev, sdl.display_width, sdl.display_height);
}

NK_API void nk_sdl_device_destroy(void) {
    struct nk_sdl_device *dev = &sdl.vulkan;

    vkDeviceWaitIdle(dev->logical_device);

    nk_sdl_destroy_render_resources(dev);

    vkFreeCommandBuffers(dev->logical_device, dev->command_pool,
                         dev->command_buffers_len, dev->command_buffers);
    vkDestroyCommandPool(dev->logical_device, dev->command_pool, NULL);
    vkDestroySemaphore(dev->logical_device, dev->render_completed, NULL);

    vkUnmapMemory(dev->logical_device, dev->vertex_memory);
    vkUnmapMemory(dev->logical_device, dev->index_memory);
    vkUnmapMemory(dev->logical_device, dev->uniform_memory);

    vkFreeMemory(dev->logical_device, dev->vertex_memory, NULL);
    vkFreeMemory(dev->logical_device, dev->index_memory, NULL);
    vkFreeMemory(dev->logical_device, dev->uniform_memory, NULL);

    vkDestroyBuffer(dev->logical_device, dev->vertex_buffer, NULL);
    vkDestroyBuffer(dev->logical_device, dev->index_buffer, NULL);
    vkDestroyBuffer(dev->logical_device, dev->uniform_buffer, NULL);

    vkDestroySampler(dev->logical_device, dev->sampler, NULL);

    vkFreeMemory(dev->logical_device, dev->font_memory, NULL);
    vkDestroyImage(dev->logical_device, dev->font_image, NULL);
    vkDestroyImageView(dev->logical_device, dev->font_image_view, NULL);

    free(dev->command_buffers);
    nk_buffer_free(&dev->cmds);
}

NK_API
void nk_sdl_shutdown(void) {
    nk_font_atlas_clear(&sdl.atlas);
    nk_free(&sdl.ctx);
    nk_sdl_device_destroy();
    memset(&sdl, 0, sizeof(sdl));
}

NK_API void nk_sdl_font_stash_begin(struct nk_font_atlas **atlas) {
    nk_font_atlas_init_default(&sdl.atlas);
    nk_font_atlas_begin(&sdl.atlas);
    *atlas = &sdl.atlas;
}

NK_API void nk_sdl_font_stash_end(VkQueue graphics_queue) {
    struct nk_sdl_device *dev = &sdl.vulkan;

    const void *image;
    int w, h;
    image = nk_font_atlas_bake(&sdl.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_sdl_device_upload_atlas(graphics_queue, image, w, h);
    nk_font_atlas_end(&sdl.atlas, nk_handle_ptr(dev->font_image_view),
                      &dev->tex_null);
    if (sdl.atlas.default_font) {
        nk_style_set_font(&sdl.ctx, &sdl.atlas.default_font->handle);
    }
}

NK_API void nk_sdl_handle_grab(void) {
    struct nk_context *ctx = &sdl.ctx;
    if (ctx->input.mouse.grab) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else if (ctx->input.mouse.ungrab) {
        /* better support for older SDL by setting mode first; causes an extra
         * mouse motion event */
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_WarpMouseInWindow(sdl.win, (int)ctx->input.mouse.prev.x,
                              (int)ctx->input.mouse.prev.y);
    } else if (ctx->input.mouse.grabbed) {
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }
}

NK_API int nk_sdl_handle_event(SDL_Event *evt) {
    struct nk_context *ctx = &sdl.ctx;

    switch (evt->type) {
    case SDL_KEYUP: /* KEYUP & KEYDOWN share same routine */
    case SDL_KEYDOWN: {
        int down = evt->type == SDL_KEYDOWN;
        const Uint8 *state = SDL_GetKeyboardState(0);
        switch (evt->key.keysym.sym) {
        case SDLK_RSHIFT: /* RSHIFT & LSHIFT share same routine */
        case SDLK_LSHIFT:
            nk_input_key(ctx, NK_KEY_SHIFT, down);
            break;
        case SDLK_DELETE:
            nk_input_key(ctx, NK_KEY_DEL, down);
            break;
        case SDLK_RETURN:
            nk_input_key(ctx, NK_KEY_ENTER, down);
            break;
        case SDLK_TAB:
            nk_input_key(ctx, NK_KEY_TAB, down);
            break;
        case SDLK_BACKSPACE:
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
            break;
        case SDLK_HOME:
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
            nk_input_key(ctx, NK_KEY_SCROLL_START, down);
            break;
        case SDLK_END:
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
            nk_input_key(ctx, NK_KEY_SCROLL_END, down);
            break;
        case SDLK_PAGEDOWN:
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
            break;
        case SDLK_PAGEUP:
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
            break;
        case SDLK_z:
            nk_input_key(ctx, NK_KEY_TEXT_UNDO,
                         down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_r:
            nk_input_key(ctx, NK_KEY_TEXT_REDO,
                         down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_c:
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_v:
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_x:
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_b:
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START,
                         down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_e:
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END,
                         down && state[SDL_SCANCODE_LCTRL]);
            break;
        case SDLK_UP:
            nk_input_key(ctx, NK_KEY_UP, down);
            break;
        case SDLK_DOWN:
            nk_input_key(ctx, NK_KEY_DOWN, down);
            break;
        case SDLK_LEFT:
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else
                nk_input_key(ctx, NK_KEY_LEFT, down);
            break;
        case SDLK_RIGHT:
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else
                nk_input_key(ctx, NK_KEY_RIGHT, down);
            break;
        }
        return 1;
    }

    case SDL_MOUSEBUTTONUP: /* MOUSEBUTTONUP & MOUSEBUTTONDOWN share same
                               routine */
    case SDL_MOUSEBUTTONDOWN: {
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        switch (evt->button.button) {
        case SDL_BUTTON_LEFT:
            if (evt->button.clicks > 1)
                nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
            break;
        case SDL_BUTTON_MIDDLE:
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
            break;
        case SDL_BUTTON_RIGHT:
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
            break;
        }
        return 1;
    }

    case SDL_MOUSEMOTION: {
        if (ctx->input.mouse.grabbed) {
            int x = (int)ctx->input.mouse.prev.x,
                y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        } else
            nk_input_motion(ctx, evt->motion.x, evt->motion.y);
        return 1;
    }

    case SDL_TEXTINPUT: {
        nk_glyph glyph;
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
        return 1;
    }

    case SDL_MOUSEWHEEL:
        nk_input_scroll(ctx,nk_vec2(evt->wheel.preciseX, evt->wheel.preciseY));
        return 1;
    }
    return 0;
}

NK_INTERN void update_texture_descriptor_set(
    struct nk_sdl_device *dev,
    struct nk_vulkan_texture_descriptor_set *texture_descriptor_set,
    VkImageView image_view) {
    VkDescriptorImageInfo descriptor_image_info;
    VkWriteDescriptorSet descriptor_write;

    texture_descriptor_set->image_view = image_view;

    memset(&descriptor_image_info, 0, sizeof(VkDescriptorImageInfo));
    descriptor_image_info.sampler = dev->sampler;
    descriptor_image_info.imageView = texture_descriptor_set->image_view;
    descriptor_image_info.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    memset(&descriptor_write, 0, sizeof(VkWriteDescriptorSet));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = texture_descriptor_set->descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &descriptor_image_info;

    vkUpdateDescriptorSets(dev->logical_device, 1, &descriptor_write, 0, NULL);
}

NK_API
VkSemaphore nk_sdl_render(VkQueue graphics_queue, uint32_t buffer_index,
                          VkSemaphore wait_semaphore,
                          enum nk_anti_aliasing AA) {
    struct nk_sdl_device *dev = &sdl.vulkan;
    struct nk_buffer vbuf, ebuf;

    struct Mat4f projection = {
        {2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         0.0f, -1.0f, 1.0f, 0.0f, 1.0f},
    };

    VkCommandBufferBeginInfo begin_info;
    VkClearValue clear_value = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    VkRenderPassBeginInfo render_pass_begin_nfo;
    VkCommandBuffer command_buffer;
    VkResult result;
    VkViewport viewport;

    VkDeviceSize doffset = 0;
    VkImageView current_texture = NULL;
    uint32_t index_offset = 0;
    VkRect2D scissor;
    uint32_t wait_semaphore_count;
    VkSemaphore *wait_semaphores;
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info;
    uint64_t time_now;

    time_now = SDL_GetTicks64();
    sdl.ctx.delta_time_seconds =
        (float)(time_now - sdl.delta_time_milliseconds_last) / 1000.0f;
    sdl.delta_time_milliseconds_last = time_now;

    projection.m[0] /= sdl.display_width;
    projection.m[5] /= sdl.display_height;

    memcpy(dev->mapped_uniform, &projection, sizeof(projection));

    memset(&begin_info, 0, sizeof(VkCommandBufferBeginInfo));
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    memset(&render_pass_begin_nfo, 0, sizeof(VkRenderPassBeginInfo));
    render_pass_begin_nfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_nfo.renderPass = dev->render_pass;
    render_pass_begin_nfo.renderArea.extent.width = (uint32_t)sdl.display_width;
    render_pass_begin_nfo.renderArea.extent.height =
        (uint32_t)sdl.display_height;
    render_pass_begin_nfo.clearValueCount = 1;
    render_pass_begin_nfo.pClearValues = &clear_value;
    render_pass_begin_nfo.framebuffer = dev->framebuffers[buffer_index];

    command_buffer = dev->command_buffers[buffer_index];

    result = vkBeginCommandBuffer(command_buffer, &begin_info);
    NK_ASSERT(result == VK_SUCCESS);
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_nfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    memset(&viewport, 0, sizeof(VkViewport));
    viewport.width = (float)sdl.width;
    viewport.height = (float)sdl.height;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      dev->pipeline);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            dev->pipeline_layout, 0, 1,
                            &dev->uniform_descriptor_set, 0, NULL);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        /* load draw vertices & elements directly into vertex + element buffer
         */
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] =
                {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT,
                  NK_OFFSETOF(struct nk_sdl_vertex, position)},
                 {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,
                  NK_OFFSETOF(struct nk_sdl_vertex, uv)},
                 {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8,
                  NK_OFFSETOF(struct nk_sdl_vertex, col)},
                 {NK_VERTEX_LAYOUT_END}};
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_sdl_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
            config.tex_null = dev->tex_null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, dev->mapped_vertex,
                                 (size_t)dev->max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, dev->mapped_index,
                                 (size_t)dev->max_element_buffer);
            nk_convert(&sdl.ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }

        /* iterate over and execute each draw command */

        vkCmdBindVertexBuffers(command_buffer, 0, 1, &dev->vertex_buffer,
                               &doffset);
        vkCmdBindIndexBuffer(command_buffer, dev->index_buffer, 0,
                             VK_INDEX_TYPE_UINT16);

        nk_draw_foreach(cmd, &sdl.ctx, &dev->cmds) {
            if (!cmd->texture.ptr) {
                continue;
            }
            if (cmd->texture.ptr && cmd->texture.ptr != current_texture) {
                int found = 0;
                uint32_t i;
                for (i = 0; i < dev->texture_descriptor_sets_len; i++) {
                    if (dev->texture_descriptor_sets[i].image_view ==
                        cmd->texture.ptr) {
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    update_texture_descriptor_set(
                        dev, &dev->texture_descriptor_sets[i],
                        (VkImageView)cmd->texture.ptr);
                    dev->texture_descriptor_sets_len++;
                }
                vkCmdBindDescriptorSets(
                    command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    dev->pipeline_layout, 1, 1,
                    &dev->texture_descriptor_sets[i].descriptor_set, 0, NULL);
            }

            if (!cmd->elem_count)
                continue;

            scissor.offset.x = (int32_t)(NK_MAX(cmd->clip_rect.x, 0.f));
            scissor.offset.y = (int32_t)(NK_MAX(cmd->clip_rect.y, 0.f));
            scissor.extent.width = (uint32_t)(cmd->clip_rect.w);
            scissor.extent.height = (uint32_t)(cmd->clip_rect.h);
            vkCmdSetScissor(command_buffer, 0, 1, &scissor);
            vkCmdDrawIndexed(command_buffer, cmd->elem_count, 1, index_offset,
                             0, 0);
            index_offset += cmd->elem_count;
        }
        nk_clear(&sdl.ctx);
    }

    vkCmdEndRenderPass(command_buffer);
    result = vkEndCommandBuffer(command_buffer);
    NK_ASSERT(result == VK_SUCCESS);

    if (wait_semaphore) {
        wait_semaphore_count = 1;
        wait_semaphores = &wait_semaphore;
    } else {
        wait_semaphore_count = 0;
        wait_semaphores = NULL;
    }

    memset(&submit_info, 0, sizeof(VkSubmitInfo));
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.waitSemaphoreCount = wait_semaphore_count;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &dev->render_completed;

    result = vkQueueSubmit(graphics_queue, 1, &submit_info, NULL);
    NK_ASSERT(result == VK_SUCCESS);

    return dev->render_completed;
}

NK_INTERN void nk_sdl_clipboard_paste(nk_handle usr,
                                      struct nk_text_edit *edit) {
    const char *text = SDL_GetClipboardText();
    if (text) {
        nk_textedit_paste(edit, text, nk_strlen(text));
        SDL_free((void *)text);
    }
    (void)usr;
}

NK_INTERN void nk_sdl_clipboard_copy(nk_handle usr, const char *text, int len) {
    char *str = 0;
    (void)usr;
    if (!len)
        return;
    str = (char *)malloc((size_t)len + 1);
    if (!str)
        return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    SDL_SetClipboardText(str);
    free(str);
}

NK_API struct nk_context *
nk_sdl_init(SDL_Window *win, VkDevice logical_device,
            VkPhysicalDevice physical_device,
            uint32_t graphics_queue_family_index, VkImageView *image_views,
            uint32_t image_views_len, VkFormat color_format,
            enum nk_sdl_init_state init_state, VkDeviceSize max_vertex_buffer,
            VkDeviceSize max_element_buffer) {
    (void)init_state;

    memset(&sdl, 0, sizeof(struct nk_sdl));
    sdl.win = win;

    nk_init_default(&sdl.ctx, 0);
    sdl.ctx.clip.copy = nk_sdl_clipboard_copy;
    sdl.ctx.clip.paste = nk_sdl_clipboard_paste;
    sdl.ctx.clip.userdata = nk_handle_ptr(0);

    SDL_GetWindowSize(win, &sdl.width, &sdl.height);
    SDL_Vulkan_GetDrawableSize(win, &sdl.display_width, &sdl.display_height);
    nk_sdl_device_create(logical_device, physical_device,
                         graphics_queue_family_index, image_views,
                         image_views_len, color_format, max_vertex_buffer,
                         max_element_buffer, (uint32_t)sdl.display_width,
                         (uint32_t)sdl.display_height);

    return &sdl.ctx;
}

#endif
