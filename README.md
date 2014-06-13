# Cinder-Pipeline
`Cinder-Pipeline` is an imaging pipeline to construct and evaluate chained shader-based effectors.

A `Pipeline` is the context around which nodes are created and evaluated. `Pipeline` evaluates a `Node` by analyzing the node's input dependencies and constructs a collection of ordered `Branch` to represent the render order.
A `Node` has multiple input `NodePort` which are both keyed and typed. Several data types are supported:
```C++
enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2f};
```
Although multiple data types are supported, being an imaging pipeline, dependencies are only calculated on `NodePortType::FBOImage` ports. Supporting that, all `Node` instances have a fixed single output port `"image"` of type `NodePortType::FBOImage`.

Currently there are two derived node classes, `SourceNode` and `EffectorNode`. `SourceNode` has a port `"texture"` of type `NodePortType::Texture` (internally `gl::TextureRef`) that is simply intended to bring textures into the pipeline. `EffectorNode` is shader-backed and is intended to act as a base class for specific operators.

### USAGE
```C++
// TODO - construct
// TODO - connect
// TODO - evaluate
```

```C++
// TODO - custom effector node
```

### LIMITATIONS
At present, there is little if any error checking on missing or cyclic input connections; beware!

### GREETZ
- Heavily inspired by Rich Eakin's [Cinder-Audio2](https://forum.libcinder.org/topic/rfc-cinder-audio2-available-for-alpha-testing)
- Neil Mendoza for [ofxPostProcessing](https://github.com/neilmendoza/ofxPostProcessing)
- Patricio Gonzalezvivo for [ofxFX](https://github.com/patriciogonzalezvivo/ofxFX)
- [Shake](http://en.wikipedia.org/wiki/Shake_(software)) and [Quartz Composer](http://en.wikipedia.org/wiki/Quartz_Composer)
