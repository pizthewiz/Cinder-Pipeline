
### PIPELINE
- sort out 3 vs 4 attachments
- add debug mechanism to save image to disk of each step of the render tree
- validate node inputs on execute
- investigate caching branches across executes
- use [GL_NV_texture_barrier](http://www.opengl.org/registry/specs/NV/texture_barrier.txt) for [texture-internal ping-pong'ing](https://www.opengl.org/discussion_boards/showthread.php/173265-GL_NV_texture_barrier-on-ATI?p=1214070&viewfull=1#post1214070)
- tile when texture size exceeds GL_MAX_TEXTURE_SIZE
- mechanism to allow samplers to set values?

### NODE
- validation of node inputs and outputs, prevent cycles
- named inputs instead of ordered inputs
- don't recreate shaders, a single static one might be fine
- if not using ci::gl::GlslProg, consider using GL_ARB_separate_shader_objects when supported

### GENERAL
- include a sample application
- check that a node setting could be animated via Timeline
