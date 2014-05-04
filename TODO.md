
### PIPELINE
- take advantage of GL_NV_texture_barrier when supported to make a texture read/write

### NODE
- publish inputs and outputs for connection
- don't recreate shaders, a single static one might be fine
- if not using ci::gl::GlslProg, consider using GL_ARB_separate_shader_objects when supported

### GENERAL
- include a sample application
- check that a node setting could be animated via Timeline
