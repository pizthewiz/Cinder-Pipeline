
### PIPELINE
- FBO setup in pipeline
- resolve chain for effector evaluation
- select node for evaluation
- bind FBO once for the whole chain to [avoid expense](http://www.gamedev.net/topic/568054-which-is-faster-binding-another-shader-program-or-another-fbo-diff-res/?view=findpost&p=4634924)
- take advantage of GL_NV_texture_barrier when supported to make a texture read/write

### NODE
- publish inputs and outputs for connection
- don't recreate shaders, a single static one might be fine
- if not using ci::gl::GlslProg, consider using GL_ARB_separate_shader_objects when supported

### GENERAL
- include a sample application
- ascii debug visualization of pipeline
- check that a node setting could be animated via Timeline
