# OGG

## Todos

### Engine Items

- [ ] Implement UI system. Remove MicroUI and Nuklear.
- [ ] Naming scheme for file structure should likely change
- [ ] Update all Entities to GLTF Models
- [ ] Reimplement the entire animation system
- [ ] Remove model editor
- [ ] Update Asset loading to use Arenas over malloc/free
- [ ] Remove the rest of the mallocs from project
- [ ] The great Migration to String8 excusively (This requires the removal of shmap from stb_ds.h)

### Game Items

- [ ] Game Console (g_log.h/.c)


## File Naming Convention

- b_ : base layer

- r_ : Render api
    - rs_  : Software rasterizer backend
    - rgl_ : Opengl backend

- e_ : Editor system

- g_ : Gameplay

- s_ : Serialization

- p_ : Platform

- tp_ : ThirdParty
