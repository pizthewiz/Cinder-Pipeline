
### PIPELINE
- FBO setup in pipeline
- resolve chain for effector evaluation
- bind FBO once for the whole chain to [avoid expense](http://www.gamedev.net/topic/568054-which-is-faster-binding-another-shader-program-or-another-fbo-diff-res/?view=findpost&p=4634924)
- take advantage of GL_NV_texture_barrier when supported to make a texture read/write

### NODE
- don't recreate shaders, a single static one might be fine
- publish inputs and outputs for chaining
- some API to connect inputs and outputs
- if not using ci::gl::GlslProg, consider using GL_ARB_separate_shader_objects when supported

### GENERAL
- include a sample application
- ??? debug visualization of pipeline
