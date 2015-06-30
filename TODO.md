
### CONTEXT
- Allow node removal
- Some mechanism to allow scaling - e.g. scale before blurring, rescale and blend like suggested [Deus Ex: Human Revolution - Graphics Study](http://www.adriancourreges.com/blog/2015/03/10/deus-ex-human-revolution-graphics-study/)
- Expose a mechanism to allow samplers
- Support Multi-pass shaders
- Investigate [cereal](https://github.com/USCiLab/cereal) for serialization and deserialization
- Migrate from JsonTree to JsonCpp for serialization
- Load a context from JSON
- Tile when texture size exceeds GL_MAX_TEXTURE_SIZE
- Use [GL_NV_texture_barrier](http://www.opengl.org/registry/specs/NV/texture_barrier.txt) for [texture-internal ping-pong'ing](https://www.opengl.org/discussion_boards/showthread.php/173265-GL_NV_texture_barrier-on-ATI?p=1214070&viewfull=1#post1214070)
- Investigate the [Boost Graph Library](http://www.boost.org/doc/libs/1_55_0/libs/graph/doc/index.html)
- Investigate graph algorithms in [bss-util](https://github.com/Black-Sphere-Studios/bss-util) to improve the dependency solver

### NODE
- Pull shader stuff out of Effector node to allow for shader-based generators
- Validate value type in Node::setValueForInputPortKey
- If not using ci::gl::GlslProg, consider using [GL_ARB_separate_shader_objects](http://www.opengl.org/registry/specs/ARB/separate_shader_objects.txt) on 4.1+. Also see [One Year of Porting](http://www.slideshare.net/slideshow/embed_code/34431339?rel=0#) and [A bit of perspective on OpenGL 4.1 separate programs](http://www.g-truc.net/post-0348.html)

### NODES
- Rename mixAmount to intensity
- Create a simple shader-based solid color generator node
- Compile vibrance node with preprocessed curve instead of using a uniform
- Colorspace conversion nodes! [YCoCg](https://en.wikipedia.org/wiki/YCgCo)/[CIELAB](https://en.wikipedia.org/wiki/Lab_color_space#CIELAB)/[HUSL](http://www.husl-colors.org)/RGB/sRGB
- Source node classes: FBOImage, RTT?
- FXAA node like [triton](https://github.com/johang88/triton/blob/master/Data/core_data/shaders/post/fxaa.glsl)

### ISF NODE
- Support the event input type
- Support multiple render passes, "PASSES"
- Support persistent render buffers, "PERSISTENT_BUFFERS"
- Support static image resources "IMPORTED"

### GENERAL
- Use `CI_ASSERT` places that make sense
- Make a pass as using `const` and `mutable` where appropriate
- Include a sample application
- Profile with [Cinder-Profiler](https://github.com/num3ric/Cinder-Profiler) and [Remotery](https://github.com/Celtoys/Remotery)
- Sprinkle in some [`glGetError`](https://www.opengl.org/wiki/GLAPI/glGetError) use?
- Think about optimizations by [reducing binds](http://calebjohnston.com/storage/glsl/#optimization)
