
### CONTEXT
- allow node port disconnection
- allow node removal
- JSON load
- prevent cycles and verify port type match on connection
- use [GL_NV_texture_barrier](http://www.opengl.org/registry/specs/NV/texture_barrier.txt) for [texture-internal ping-pong'ing](https://www.opengl.org/discussion_boards/showthread.php/173265-GL_NV_texture_barrier-on-ATI?p=1214070&viewfull=1#post1214070)
- tile when texture size exceeds GL_MAX_TEXTURE_SIZE
- mechanism to allow samplers?
- multi-pass shaders?
- add debug mechanism to save image to disk of each step of the render tree

### NODE
- don't recreate shaders, a single static one might be fine
- add an enumeration and string port type
- port min, max, label, enum values
- validate value type on assignment?
- if not using ci::gl::GlslProg, consider using [GL_ARB_separate_shader_objects](http://www.opengl.org/registry/specs/ARB/separate_shader_objects.txt) once Cinder supports OpenGL 4.1+. Also see [One Year of Porting](http://www.slideshare.net/slideshow/embed_code/34431339?rel=0#) and [A bit of perspective on OpenGL 4.1 separate programs](http://www.g-truc.net/post-0348.html).

### NODES
- source node classes: file, texture, FBOImage, RTT?
- [ISF](http://vdmx.vidvox.net/blog/isf) node

### GENERAL
- include a sample application
- investigate the [Boost Graph Library](http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/index.html)
- add some profiling, either [sansumbrella's Pockets](https://github.com/sansumbrella/Pockets/blob/dev/src/pockets/Profiling.h) or a simple cinder::Timer.
- replace vector with set when uniqueness is needed
