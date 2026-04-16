
static Vk_Context vkctx = {0};


/* Function Prototypes */
int main_loop(gfx_context *r);
int init_vk(gfx_context *r);
int init_vk_instance(gfx_context *r);
int init_validation_layers(gfx_context *r);
int init_physical_devices(gfx_context *r);
bool is_device_suitible(gfx_context *r, VkPhysicalDevice device);
int init_queue_families(gfx_context *r);
int init_logical_devices(gfx_context *r);
int init_window_surface(gfx_context *r);
int init_swapchain(gfx_context *r);
int init_image_views(gfx_context *r);
int init_graphic_pipeline(gfx_context *r);
int init_sync_prims(gfx_context *r);
int init_render_pass(gfx_context *r);
int init_frame_buffer(gfx_context *r);
int init_command_pool(gfx_context *r);
int init_command_buffer(gfx_context *r);
int record_command_buffer(gfx_context *r, u32 image_idx);
int recreate_swapchain(gfx_context *r);
int deinit_swapchain(gfx_context *r);
static void framebuffer_resize(SDL_Window *window, i32 width, i32 height);
array_byte_t load_shader_bytes(gfx_context *r, const char *file_name);
void renderer_cleanup(gfx_context *r);
void panic(const char *msg);
static void setup_debug(gfx_context *r);
queue_family_t find_queue_families(gfx_context *r, VkPhysicalDevice device);
bool check_device_extension_support(VkPhysicalDevice device);
swapchain_support_details_t query_swapchain_details(gfx_context *r, VkPhysicalDevice device);
VkSurfaceFormatKHR choose_surface_format(const VkSurfaceFormatKHR *formats, uint32_t format_count);
VkPresentModeKHR choose_present_mode(const VkPresentModeKHR *present_modes, uint32_t present_modes_count);
VkExtent2D choose_extent(gfx_context *r, VkSurfaceCapabilitiesKHR capabilites);
VkShaderModule init_shader_module(gfx_context *r, array_byte_t *b);
int draw(gfx_context *r);

/*
     ___ __  __ ____  _     _____ __  __ _____ _   _ _____  _  _____ ___ ___  _   _
    |_ _|  \/  |  _ \| |   | ____|  \/  | ____| \ | |_   _|/ \|_   _|_ _/ _ \| \ | |
     | || |\/| | |_) | |   |  _| | |\/| |  _| |  \| | | | / _ \ | |  | | | | |  \| |
     | || |  | |  __/| |___| |___| |  | | |___| |\  | | |/ ___ \| |  | | |_| | |\  |
    |___|_|  |_|_|   |_____|_____|_|  |_|_____|_| \_| |_/_/   \_\_| |___\___/|_| \_|

    */

#ifdef ENGINE_IMPLEMENTATION

/*
     * Internal Types
     */

/*
     * Internal Function Prototypes
     */

#define MAX(_X, _Y) (_X > _Y ? _X : _Y)
#define MIN(_X, _Y) (_X < _Y ? _X : _Y)
#define CLAMP(_X, _MN, _MX) (MAX(_MN, MIN(_X, _MX)))
#define FIXLIST(_T, _N) \
struct              \
{                   \
    _T data[_N];    \
    uint32_t _N;    \
}

typedef struct arena_t {
    u32 capacity, len, alignment;
    void *data;
} arena_t;

void *arena_push(arena_t *a, u32 size, u32 align, b32 zero);
u32   arena_pos(arena_t *a);
void  arena_pop_to(arena_t *a, u32 pos);

static const char *validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
};

static const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

u32 current_frame = 0;

#define MAX_FRAMES_IN_FLIGHT 2

#ifdef NDEBUG
const bool enabled_validation_layers = true;
#else
const bool enabled_validation_layers = true;
#endif

int init_vk(gfx_context *r)
{
    renderer_results_e res;

    init_vk_instance(r);
    init_window_surface(r);
    setup_debug(r);
    init_physical_devices(r);
    init_logical_devices(r);
    init_swapchain(r);
    init_image_views(r);
    init_render_pass(r);
    init_graphic_pipeline(r);
    init_frame_buffer(r);
    init_command_pool(r);
    init_command_buffer(r);
    init_sync_prims(r);

    return R_SUCCESS;
}

int main_loop(gfx_context *r) {
    printf("Hello we init 1?\n");
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_FALSE)
    {
        return -1;
    }

    printf("Hello we init?\n");

    SDL_Vulkan_LoadLibrary(NULL);
    printf("Hello we vulkna init?\n");
    r->window = SDL_CreateWindow("Hello",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 WIDTH, HEIGHT,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    printf("Hello we window init?\n");
    init_vk(r);
    printf("vk window init?\n");

    bool running = true;
    while(running) {
        SDL_Event windowEvent;
        while(SDL_PollEvent(&windowEvent))
            if(windowEvent.type == SDL_QUIT) {
                running = false;
                break;
            }
        draw(r);
    }

    vkDeviceWaitIdle(r->device);
    renderer_cleanup(r);

    return 0;
}

int recreate_swapchain(gfx_context *r) {
    vkDeviceWaitIdle(r->device);

    deinit_swapchain(r);

    init_swapchain(r);
    init_image_views(r);
    init_frame_buffer(r);
}

int draw(gfx_context *r) {
    vkWaitForFences(r->device, 1, &r->in_flight_frames.data[current_frame], VK_TRUE, UINT64_MAX);
    vkResetFences(r->device, 1, &r->in_flight_frames.data[current_frame]);

    u32 image_idx;
    VkResult res = vkAcquireNextImageKHR(r->device, r->swapchain, UINT64_MAX, r->image_available_sema.data[current_frame], VK_NULL_HANDLE, &image_idx);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain(r);
        return 1;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        return R_FAILURE;
    }


    vkResetCommandBuffer(r->command_buffers.data[current_frame], 0);
    record_command_buffer(r, image_idx);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {r->image_available_sema.data[current_frame]};
    VkSemaphore signal_semaphores[] = {r->render_finished_sema.data[current_frame]};
    VkPipelineStageFlags wait_flags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pWaitDstStageMask = wait_flags;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &r->command_buffers.data[current_frame];

    res = vkQueueSubmit(r->graphics_queue, 1, &submit_info, r->in_flight_frames.data[current_frame]);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {r->swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_idx;

    vkQueuePresentKHR(r->present_queue, &present_info);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;


    return R_SUCCESS;
}

void panic(const char *msg)
{
    fprintf(stderr, msg);
    exit(1);
}

queue_family_t find_queue_families(gfx_context *r, VkPhysicalDevice device)
{
    queue_family_t indices = {UINT32_MAX};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families);

    for (int i = 0; i < queue_family_count; i++)
    {
        VkQueueFamilyProperties qf = families[i];
        if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, r->surface, &present_support);
        if (present_support && i != indices.graphics_family)
        {
            indices.present_family = i;
        }
    }
    return indices;
}

bool check_device_extension_support(VkPhysicalDevice device)
{
    // @todo add extensions checking
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain
    return true;
}

// Caller is responsible for releasing the memory of formats/present_modes
swapchain_support_details_t query_swapchain_details(gfx_context *r, VkPhysicalDevice device)
{
    swapchain_support_details_t details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, r->surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, r->surface, &details.formats_count, NULL);

    if (details.formats_count != 0)
    {
        details.formats = malloc(details.formats_count * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, r->surface, &details.formats_count, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, r->surface, &details.present_modes_count, NULL);

    if (details.present_modes_count != 0)
    {
        details.present_modes = malloc(details.present_modes_count * sizeof(VkSurfacePresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, r->surface, &details.present_modes_count, details.present_modes);
    }

    return details;
}

static bool check_validation_support(void)
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties supported_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, &supported_layers);

    for (int i = 0; i < sizeof(validation_layers) / sizeof(validation_layers[0]); i++)
    {
        bool layer_found = false;
        char *validation_layer_name = validation_layers[i];

        for (int j = 0; j < layer_count; j++)
        {
            char *layer_name = supported_layers[i].layerName;
            if (strcmp(validation_layer_name, layer_name))
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
            return false;
    }

    return true;
}

VkSurfaceFormatKHR choose_surface_format(const VkSurfaceFormatKHR *formats, uint32_t format_count)
{
    for (int i = 0; i < format_count; i++)
    {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }
    return formats[0];
}

VkExtent2D choose_extent(gfx_context *r, VkSurfaceCapabilitiesKHR capabilites)
{
    if (capabilites.currentExtent.width != UINT32_MAX)
    {
        return capabilites.currentExtent;
    }
    i32 width, height;

    SDL_Vulkan_GetDrawableSize(r->window, &width, &height);
    VkExtent2D actual_extent = {
        .width = width,
        .height = height,
    };

    actual_extent.width = CLAMP(actual_extent.width,
                                capabilites.minImageExtent.width,
                                capabilites.maxImageExtent.width);

    actual_extent.height = CLAMP(actual_extent.height,
                                 capabilites.minImageExtent.height,
                                 capabilites.maxImageExtent.height);

    return actual_extent;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback,
    void *user_data)
{
    printf("[Validation]: %s\n\n", p_callback->pMessage);

    return VK_FALSE;
}

VkResult create_debug_utils_messenger(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *create_info,
    const VkAllocationCallbacks *alloc_callback,
    VkDebugUtilsMessengerEXT *debug_messenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        return func(instance, create_info, alloc_callback, debug_messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_utils_messenger(
    VkInstance instance,
    const VkAllocationCallbacks *alloc_callback,
    VkDebugUtilsMessengerEXT *debug_messenger)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, debug_messenger, alloc_callback);
    }
}

static void init_debug_info(VkDebugUtilsMessengerCreateInfoEXT *create_info)
{
    *create_info = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = NULL,
    };
}

static void setup_debug(gfx_context *r)
{
    if (!validation_layers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = NULL,
    };

    if (create_debug_utils_messenger(r->instance, &create_info, NULL, &r->debug_messenger) != VK_SUCCESS)
    {
        exit(1);
    }
}

static fixed_size_array_t get_required_extensions(gfx_context *r)
{
    uint32_t sdl_ext_count = 0;

    if (!SDL_Vulkan_GetInstanceExtensions(r->window, &sdl_ext_count, NULL)) {
        SDL_Log("SDL_Vulkan_GetInstanceExtensions(count) failed: %s", SDL_GetError());
        exit(1);
    }

    const char **sdl_extensions = malloc(sizeof(const char *) * sdl_ext_count);
    if (!sdl_extensions) {
        panic("malloc failed for SDL Vulkan extensions");
    }

    if (!SDL_Vulkan_GetInstanceExtensions(r->window, &sdl_ext_count, sdl_extensions)) {
        SDL_Log("SDL_Vulkan_GetInstanceExtensions(list) failed: %s", SDL_GetError());
        exit(1);
    }

    uint32_t extra = 1; // portability enumeration
    if (enabled_validation_layers) {
        extra += 1; // debug utils
    }

    const char **extensions = malloc(sizeof(const char *) * (sdl_ext_count + extra));
    if (!extensions) {
        panic("malloc failed for Vulkan extension list");
    }

    memcpy(extensions, sdl_extensions, sizeof(const char *) * sdl_ext_count);
    free(sdl_extensions);

    uint32_t n = sdl_ext_count;
    extensions[n++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    if (enabled_validation_layers) {
        extensions[n++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    return (fixed_size_array_t){
        .len = n,
        .data = extensions,
    };
}

int init_vk_instance(gfx_context *r)
{
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;
    app_info.pApplicationName = "NEW Engine";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.pEngineName = "Oldschool";

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    fixed_size_array_t required_extensions = get_required_extensions(r);

    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    create_info.enabledExtensionCount = required_extensions.len;
    create_info.ppEnabledExtensionNames = required_extensions.data;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    if (enabled_validation_layers)
    {
        create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
        create_info.ppEnabledLayerNames = validation_layers;
        init_debug_info(&debug_create_info);
        create_info.pNext = &debug_create_info;
    }
    else
{
        create_info.enabledLayerCount = 0;
    }

    if (enabled_validation_layers && !check_validation_support())
    {
        fprintf(stderr, "Failure to load validation layer extensions\n");
        exit(1);
    }

    VkResult result = vkCreateInstance(&create_info, NULL, &r->instance);
    assert(result == VK_SUCCESS);

    return 1;
}

int init_physical_devices(gfx_context *r)
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(r->instance, &device_count, NULL);
    if (device_count == 0)
    {
        panic("Failed to find a vulkan physical device");
    }
    VkPhysicalDevice physical_devices[device_count];
    vkEnumeratePhysicalDevices(r->instance, &device_count, physical_devices);

    for (int i = 0; i < device_count; i++)
    {
        VkPhysicalDevice device = physical_devices[i];
        if (is_device_suitible(r, device))
        {
            r->physical_device = device;
            break;
        }
    }

    if (r->physical_device == VK_NULL_HANDLE)
    {
        panic("Failed to find a vulkan physical device");
    }
    return R_SUCCESS;
}

bool is_device_suitible(gfx_context *r, VkPhysicalDevice device)
{
    // @todo come back and add a real device picking method.
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    queue_family_t qf = find_queue_families(r, device);

    bool extensions_supported = check_device_extension_support(device);

    bool swapchain_adequate = false;
    if (extensions_supported)
    {
        swapchain_support_details_t sc_details = query_swapchain_details(r, device);
        swapchain_adequate = (sc_details.formats_count != 0) && (sc_details.present_modes_count != 0);
    }

    return (qf.graphics_family != UINT32_MAX ? true : false) && extensions_supported && swapchain_adequate;
}

int init_queue_families(gfx_context *r)
{
    return R_TODO;
}

int init_logical_devices(gfx_context *r)
{
    queue_family_t qf = find_queue_families(r, r->physical_device);
    float queue_prio = 1.0;

    VkDeviceQueueCreateInfo queue_create_info[2] = {0};
    queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].queueFamilyIndex = qf.graphics_family;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_prio;

    queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[1].queueFamilyIndex = qf.present_family;
    queue_create_info[1].queueCount = 1;
    queue_create_info[1].pQueuePriorities = &queue_prio;

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_info;
    create_info.queueCreateInfoCount = 2;
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = sizeof(device_extensions) / sizeof(device_extensions[0]);
    create_info.ppEnabledExtensionNames = device_extensions;

    VkResult res = vkCreateDevice(r->physical_device, &create_info, NULL, &r->device);
    assert(res == VK_SUCCESS);

    vkGetDeviceQueue(r->device, qf.graphics_family, 0, &r->graphics_queue);
    vkGetDeviceQueue(r->device, qf.present_family, 0, &r->present_queue);

    return R_SUCCESS;
}

/* SWAPCHAIN SETUP */

int init_window_surface(gfx_context *r)
{
    // @todo Add windows surface support via win32
    if (!SDL_Vulkan_CreateSurface(r->window, r->instance, &r->surface)) {
        SDL_Log("Failed to create Vulkan surface: %s", SDL_GetError());
        assert(false);
    }
    return R_SUCCESS;
}

int deinit_swapchain(gfx_context *r) {
    for (size_t i = 0; i < r->framebuffers.len; i++) {
        vkDestroyFramebuffer(r->device, r->framebuffers.data[i], NULL);
    }
    for (size_t i = 0; i < r->swapchain_image_views.len; i++) {
        vkDestroyImageView(r->device, r->swapchain_image_views.data[i], NULL);
    }
    vkDestroySwapchainKHR(r->device, r->swapchain, NULL);
}

int init_swapchain(gfx_context *r)
{
    swapchain_support_details_t sc_details = query_swapchain_details(r, r->physical_device);

    VkSurfaceFormatKHR format = choose_surface_format(sc_details.formats, sc_details.formats_count);
    VkPresentModeKHR present_mode = choose_present_mode(sc_details.present_modes, sc_details.present_modes_count);
    VkExtent2D extent = choose_extent(r, sc_details.capabilities);

    u32 image_count = sc_details.capabilities.minImageCount + 1;
    if (sc_details.capabilities.maxImageCount > 0 &&
        image_count > sc_details.capabilities.maxImageCount)
    {
        image_count = sc_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.minImageCount = image_count;
    create_info.surface = r->surface;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = extent;
    create_info.presentMode = present_mode;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    queue_family_t indices = find_queue_families(r, r->physical_device);
    u32 families[] = {indices.graphics_family, indices.present_family};
    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = families;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = sc_details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(r->device, &create_info, NULL, &r->swapchain);
    if (res != VK_SUCCESS)
        panic("failed to create swapchain");

    vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchain_image.len, NULL);
    r->swapchain_image.capacity = r->swapchain_image.len;
    r->swapchain_image.data = (VkImage *)malloc(sizeof(VkImage) * r->swapchain_image.len);
    vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchain_image.len, r->swapchain_image.data);
    r->extent = extent;
    r->image_format = format.format;

    return R_TODO;
}

VkPresentModeKHR choose_present_mode(
    const VkPresentModeKHR *present_modes,
    uint32_t present_modes_count)
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

/* SWAPCHAIN END */

int init_image_views(gfx_context *r)
{
    if (r->swapchain_image_views.capacity < r->swapchain_image.len) {
        r->swapchain_image_views.capacity = r->swapchain_image.capacity;
        r->swapchain_image_views.len = r->swapchain_image.len;
        r->swapchain_image_views.data = (VkImageView *)malloc(sizeof(VkImageView) * r->swapchain_image_views.capacity);
    }

    for (size_t i = 0; i < r->swapchain_image_views.len; i++)
    {
        VkImageViewCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = r->swapchain_image.data[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = r->image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult res = vkCreateImageView(r->device, &create_info, NULL, &r->swapchain_image_views.data[i]);
        if (res != VK_SUCCESS)
            panic("Failed to create image view");
    }
    return R_SUCCESS;
}

/* Graphics Pipelines are not dynamic. There must be define and built,
 * and then run as is. Due to this nature, a good system to create these
 * as custom implemenations is a must.
 **/

int init_graphic_pipeline(gfx_context *r) {
    array_byte_t vs = load_shader_bytes(r, "src/shaders/basic.vs.spv");
    array_byte_t fs = load_shader_bytes(r, "src/shaders/basic.fs.spv");

    VkShaderModule vs_shader = init_shader_module(r, &vs);
    VkShaderModule fs_shader = init_shader_module(r, &fs);

    VkPipelineShaderStageCreateInfo vert_create_info = {0};
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = vs_shader;
    vert_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_create_info = {0};
    frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.module = fs_shader;
    frag_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_create_info, frag_create_info };

    VkDynamicState dynamic_state[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dstate_create_info = {0};
    dstate_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dstate_create_info.dynamicStateCount = 2;
    dstate_create_info.pDynamicStates = dynamic_state;

    VkPipelineVertexInputStateCreateInfo vi_create_info = {0};
    vi_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi_create_info.vertexAttributeDescriptionCount = 0;
    vi_create_info.pVertexAttributeDescriptions = NULL;
    vi_create_info.vertexBindingDescriptionCount = 0;
    vi_create_info.pVertexBindingDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo va_create_info = {0};
    va_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    va_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    va_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)r->extent.width;
    viewport.height = (f32)r->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0,0};
    scissor.extent = r->extent;

    VkPipelineViewportStateCreateInfo view_create_info = {0};
    view_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    view_create_info.viewportCount = 1;
    view_create_info.pViewports = &viewport;
    view_create_info.scissorCount = 1;
    view_create_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo raster_create_info = {0};
    raster_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_create_info.depthClampEnable = VK_FALSE;
    raster_create_info.rasterizerDiscardEnable = VK_FALSE;
    raster_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_create_info.lineWidth = 1.0f;
    raster_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster_create_info.depthBiasEnable = VK_FALSE;
    raster_create_info.depthBiasConstantFactor = 0.0f; 
    raster_create_info.depthBiasClamp = 0.0f;
    raster_create_info.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo ms_create_info = {0};
    ms_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms_create_info.sampleShadingEnable = VK_FALSE;
    ms_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_create_info.minSampleShading = 1.0f; // Optional
    ms_create_info.pSampleMask = NULL; // Optional
    ms_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    ms_create_info.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState cb_attach = {0};
    cb_attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cb_attach.blendEnable = VK_FALSE;
    cb_attach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    cb_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    cb_attach.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    cb_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    cb_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    cb_attach.alphaBlendOp = VK_BLEND_OP_ADD;


    VkPipelineColorBlendStateCreateInfo color_blending = {0};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &cb_attach;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    
    VkPipelineLayoutCreateInfo pl_create_info = {0};
    pl_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pl_create_info.setLayoutCount = 0; // Optional
    pl_create_info.pSetLayouts = nullptr; // Optional
    pl_create_info.pushConstantRangeCount = 0; // Optional
    pl_create_info.pPushConstantRanges = nullptr; // Optional

    VkResult res = vkCreatePipelineLayout(r->device, &pl_create_info, NULL, &r->pipeline_layout);
    if (res != VK_SUCCESS) {
        panic("failed to create the pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {0};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vi_create_info;
    pipeline_create_info.pInputAssemblyState = &va_create_info;
    pipeline_create_info.pViewportState = &view_create_info;
    pipeline_create_info.pRasterizationState = &raster_create_info;
    pipeline_create_info.pMultisampleState = &ms_create_info;
    pipeline_create_info.pDepthStencilState = NULL;
    pipeline_create_info.pColorBlendState = &color_blending;
    pipeline_create_info.pDynamicState = &dstate_create_info;

    pipeline_create_info.layout = r->pipeline_layout;
    pipeline_create_info.renderPass = r->pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    res = vkCreateGraphicsPipelines(r->device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &r->pipeline);
    if (res != VK_SUCCESS)
        panic("Failed to create graphics pipeline");


    vkDestroyShaderModule(r->device, vs_shader, NULL);
    vkDestroyShaderModule(r->device, fs_shader, NULL);
    return R_TODO;
}

VkShaderModule init_shader_module(gfx_context *r, array_byte_t *b) {
    VkShaderModule module = {0};
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = b->len;
    create_info.pCode = (uint32_t*)b->data;

    VkResult res = vkCreateShaderModule(r->device, &create_info, NULL, &module);
    if (res != VK_SUCCESS)
        panic("Failed to create shader module");

    return module;
}

int init_sync_prims(gfx_context *r) {
    r->image_available_sema.capacity = MAX_FRAMES_IN_FLIGHT;
    r->image_available_sema.len = MAX_FRAMES_IN_FLIGHT;
    r->image_available_sema.data = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);

    r->render_finished_sema.capacity = MAX_FRAMES_IN_FLIGHT;
    r->render_finished_sema.len = MAX_FRAMES_IN_FLIGHT;
    r->render_finished_sema.data = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);

    r->in_flight_frames.capacity = MAX_FRAMES_IN_FLIGHT;
    r->in_flight_frames.len = MAX_FRAMES_IN_FLIGHT;
    r->in_flight_frames.data = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo sema_create_info = {0};
    sema_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult res = vkCreateSemaphore(r->device, &sema_create_info, NULL, &r->image_available_sema.data[i]);
        if (res != VK_SUCCESS)
            panic("Failed to create semaphore image_available_sema");

        res = vkCreateSemaphore(r->device, &sema_create_info, NULL, &r->render_finished_sema.data[i]);
        if (res != VK_SUCCESS)
            panic("Failed to create semaphore render_finished_sema");

        res = vkCreateFence(r->device, &fence_create_info, NULL, &r->in_flight_frames.data[i]);
        if (res != VK_SUCCESS)
            panic("Failed to create fence in_flight_frames");
    }

    return R_SUCCESS;
}

int init_render_pass(gfx_context *r) {
    VkAttachmentDescription color_attachment = {0};
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_attachment.format = r->image_format;

    VkAttachmentReference color_ref = {0};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassCreateInfo pass_create_info = {0};
    pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    pass_create_info.attachmentCount = 1;
    pass_create_info.pAttachments = &color_attachment;
    pass_create_info.subpassCount = 1;
    pass_create_info.pSubpasses = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    pass_create_info.dependencyCount = 1;
    pass_create_info.pDependencies = &dependency;

    VkResult res = vkCreateRenderPass(r->device, &pass_create_info, NULL, &r->pass);
    if (res != VK_SUCCESS)
        panic("Failed to create render pass");

    return R_SUCCESS;
}

static void framebuffer_resize(SDL_Window *window, i32 width, i32 height) {
}

int init_frame_buffer(gfx_context *r) {
    if (r->framebuffers.capacity < r->swapchain_image.len) {
        r->framebuffers = (array_VkFramebuffer_t){
            .len = r->swapchain_image.len,
            .capacity = r->swapchain_image.capacity,
            .data = (VkFramebuffer*) malloc(sizeof(VkFramebuffer) * r->swapchain_image.capacity),
        };
    }

    for (size_t i = 0; i < r->framebuffers.len; i++) {
        VkImageView attachments[] = {
            r->swapchain_image_views.data[i],
        };

        VkFramebufferCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = r->pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attachments;
        create_info.width = r->extent.width;
        create_info.height = r->extent.height;
        create_info.layers = 1;

        VkResult res = vkCreateFramebuffer(r->device, &create_info, NULL, &r->framebuffers.data[i]);
        if (res != VK_SUCCESS)
            panic("Failed to create framebuffer");
    }
    return R_SUCCESS;
}

int init_command_pool(gfx_context *r) {
    queue_family_t indices = find_queue_families(r, r->physical_device);

    VkCommandPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = indices.graphics_family;

    VkResult res = vkCreateCommandPool(r->device, &create_info, NULL, &r->command_pool);
    if (res != VK_SUCCESS)
        panic("Failed to create framebuffer");

    return R_SUCCESS;
}

int init_command_buffer(gfx_context *r) {
    //array_resize(&r->command_buffers, VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    r->command_buffers.capacity = MAX_FRAMES_IN_FLIGHT;
    r->command_buffers.len = MAX_FRAMES_IN_FLIGHT;
    r->command_buffers.data = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = r->command_pool;
    alloc_info.commandBufferCount = r->command_buffers.len;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkResult res = vkAllocateCommandBuffers(r->device, &alloc_info, r->command_buffers.data);
    if (res != VK_SUCCESS)
        panic("Failed to create framebuffer");
}

int record_command_buffer(gfx_context *r, u32 image_idx) {
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    VkResult res = vkBeginCommandBuffer(r->command_buffers.data[current_frame], &begin_info);
    if (res != VK_SUCCESS)
        panic("Failed to create framebuffer");

    VkRenderPassBeginInfo render_being_info = {0};
    render_being_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_being_info.renderPass = r->pass;
    render_being_info.framebuffer = r->framebuffers.data[image_idx];
    render_being_info.renderArea.extent = r->extent;
    render_being_info.renderArea.offset = (VkOffset2D){0,0};

    VkClearValue clear_value = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    render_being_info.clearValueCount = 1;
    render_being_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(r->command_buffers.data[current_frame], &render_being_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(r->command_buffers.data[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline);

    vkCmdDraw(r->command_buffers.data[current_frame], 3, 1, 0, 0);

    vkCmdEndRenderPass(r->command_buffers.data[current_frame]);

    vkEndCommandBuffer(r->command_buffers.data[current_frame]);
    return R_SUCCESS;
}

array_byte_t load_shader_bytes(gfx_context *r, const char *file_name) {
    array_byte_t bytes = {0};

    FILE *f = fopen(file_name, "rb");
    if (f == NULL) {
        panic("Failed to open Shader");
    }
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    rewind(f);

    bytes.len = size;
    bytes.capacity = size;
    printf("file size : %d\n", size);
    bytes.data = malloc(sizeof(u8) * size);

    fread(bytes.data, size, sizeof(u8), f);

    return bytes;
}

void renderer_cleanup(gfx_context *r) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(r->device, r->image_available_sema.data[i], NULL);
        vkDestroySemaphore(r->device, r->render_finished_sema.data[i], NULL);
        vkDestroyFence(r->device, r->in_flight_frames.data[i], NULL);
    }

    vkDestroyCommandPool(r->device, r->command_pool, NULL);
    for (size_t i = 0; i < r->framebuffers.len; i++)
    {
        vkDestroyFramebuffer(r->device, r->framebuffers.data[i], NULL);
    }

    vkDestroyPipeline(r->device, r->pipeline, NULL);
    vkDestroyPipelineLayout(r->device, r->pipeline_layout, NULL);
    vkDestroyRenderPass(r->device, r->pass, NULL);

    for (size_t i = 0; i < r->swapchain_image_views.len; i++)
    {
        vkDestroyImageView(r->device, r->swapchain_image_views.data[i], NULL);
    }

    vkDestroySwapchainKHR(r->device, r->swapchain, NULL);
    vkDestroyDevice(r->device, NULL);

    if (enabled_validation_layers)
        destroy_debug_utils_messenger(r->instance, NULL, r->debug_messenger);

    vkDestroySurfaceKHR(r->instance, r->surface, NULL);
    vkDestroyInstance(r->instance, NULL);

    SDL_DestroyWindow(r->window);

}

