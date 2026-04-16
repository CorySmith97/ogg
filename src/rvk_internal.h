#ifndef RVK_INTERNAL_H
#define RVK_INTERNAL_H

typedef struct {
    SDL_Window*window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat image_format;
    VkExtent2D extent;
    VkRenderPass pass;
    VkPipeline pipeline;
    array_VkImage_t swapchain_image;
    array_VkImageView_t swapchain_image_views;
    VkPipelineLayout pipeline_layout;
    array_VkFramebuffer_t framebuffers;
    VkCommandPool command_pool;
    array_VkCommandBuffer_t command_buffers;
    array_VkSemaphore_t image_available_sema;
    array_VkSemaphore_t render_finished_sema;
    array_VkFence_t in_flight_frames;
} Vk_Context;


#endif // RVK_INTERNAL_H
