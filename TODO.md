
### CONTEXT
- overload >> to allow connections, just to the default "image" port. Maybe node ("image")->("maskImage") otherNode
- allow node removal
- investigate [cereal](https://github.com/USCiLab/cereal) for serialization and deserialization
- load from JSON
- prevent cycles on connection
- use [GL_NV_texture_barrier](http://www.opengl.org/registry/specs/NV/texture_barrier.txt) for [texture-internal ping-pong'ing](https://www.opengl.org/discussion_boards/showthread.php/173265-GL_NV_texture_barrier-on-ATI?p=1214070&viewfull=1#post1214070)
- tile when texture size exceeds GL_MAX_TEXTURE_SIZE
- mechanism to allow samplers?
- multi-pass shaders?
- add debug mechanism to save image to disk of each step of the render tree

### NODE
- pull shader stuff out of Effector node to allow for shader-based generators
- validate value type on assignment
- don't recreate shaders, a single static one might be fine
- if not using ci::gl::GlslProg, consider using [GL_ARB_separate_shader_objects](http://www.opengl.org/registry/specs/ARB/separate_shader_objects.txt) once Cinder supports OpenGL 4.1+. Also see [One Year of Porting](http://www.slideshare.net/slideshow/embed_code/34431339?rel=0#) and [A bit of perspective on OpenGL 4.1 separate programs](http://www.g-truc.net/post-0348.html).
- Colorspace conversion nodes! YCoCg > RGB

### NODES
- create a simple shader-based solid color generator node
- source node classes: FBOImage, RTT?
- add a Photoshop-like red overlay to show Vibrance selection strength

### ISF NODE
- support the event input type
- support multiple render passes, "PASSES"
- support persistent render buffers, "PERSISTENT_BUFFERS"
- support static image resources "IMPORTED"

### GENERAL
- include a sample application
- sprinkle in some [`glGetError`](https://www.opengl.org/wiki/GLAPI/glGetError) use?
- investigate the [Boost Graph Library](http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/index.html)
- add some profiling, either [sansumbrella's Pockets](https://github.com/sansumbrella/Pockets/blob/dev/src/pockets/Profiling.h) or a simple cinder::Timer.
- replace vector with set when uniqueness is needed
