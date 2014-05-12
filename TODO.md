
### PIPELINE
- add debug mechanism to save image to disk of each step of the render tree
- investigate caching branches across executes
- validate node inputs on execute
- use [GL_NV_texture_barrier](http://www.opengl.org/registry/specs/NV/texture_barrier.txt) for [texture-internal ping-pong'ing](https://www.opengl.org/discussion_boards/showthread.php/173265-GL_NV_texture_barrier-on-ATI?p=1214070&viewfull=1#post1214070)
- tile when texture size exceeds GL_MAX_TEXTURE_SIZE
- mechanism to allow samplers?

### NODE
- expose port descriptors, input ports
- validate node inputs and outputs, prevent cycles
- don't recreate shaders, a single static one might be fine
- RTT source via lambda?
- multi-pass shaders
- if not using ci::gl::GlslProg, consider using [GL_ARB_separate_shader_objects](http://www.opengl.org/registry/specs/ARB/separate_shader_objects.txt) once Cinder supports OpenGL 4.1+. Also see [One Year of Porting](http://www.slideshare.net/slideshow/embed_code/34431339?rel=0#) and [A bit of perspective on OpenGL 4.1 separate programs](http://www.g-truc.net/post-0348.html).
- wrap fsevent for shader reloading?

### GENERAL
- include a sample application
- check that a node setting could be animated via Timeline
- add some profiling, either [sansumbrella's Pockets](https://github.com/sansumbrella/Pockets/blob/dev/src/pockets/Profiling.h) or a simple cinder::Timer.
