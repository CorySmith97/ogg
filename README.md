# OGG

## Notes

For the moment, this project is entirely software rasterized. There is a high probability that at some point in the future I will want to add another backend. If that happens it will likely be a vulkan or sokol backend. However for the mean time I want to push the software rasterizer to the limits.

Dependencies currently are:
SDL2
stb_ds.h (This will be removed first once arenas are implemented)
stb_image.h (Only used for texture loading. Idk if it will be removed)
microui.h (Planned to be in for the forseeable future for editor things)
