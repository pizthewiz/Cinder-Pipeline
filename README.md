# Cinder-Pipeline
`Cinder-Pipeline` is an imaging pipeline to construct and evaluate chained shader-based effectors.

A `Context` is the container around which nodes are created, connections are made and nodes are evaluated. `Context` evaluates a `Node` by walking up the tree through the node's input connections and constructs a collection of ordered `Branch`.
A `Node` has multiple input `NodePort` which are both keyed and typed. Several data types are supported:
```C++
enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2f};
```
Although multiple data types are supported, being an imaging pipeline, dependencies are only calculated on `NodePortType::FBOImage` ports. Supporting that, all `Node` instances have a fixed single output port `"image"` of type `NodePortType::FBOImage`.

Currently there are two derived node classes, `SourceNode` and `EffectorNode`. `SourceNode` is intended to be used directly and has a port `"texture"` of type `NodePortType::Texture` (internally `gl::TextureRef`) that will bring textures into the pipeline. `EffectorNode` is intended to act as a base class for shader-based operators and the input ports can vary based on the specific need.

### USAGE
```C++
mContext = Context::create();

// setup context internals (FBO) at source texture size
gl::TextureRef sourceTexture = gl::Texture::create(loadImage(loadAsset(RES_LENNA_IMAGE)));
mContext->setup(sourceTexture->getSize());

// create source
SourceNodeRef sourceNode = mContext->makeNode(new SourceNode);
sourceNode->setValueForInputPortKey(sourceTexture, "texture");

// create horizontal blur, set size and connect to source
BlurNodeRef blurNodeHorizontal = mContext->makeNode(new BlurNode);
blurNodeHorizontal->setValueForInputPortKey(Vec2f(1.0f/sourceTexture->getWidth(), 0.0f), "pixelSize");
mContext->connectNodes(sourceTexture, blurNodeHorizontal);

// create vertical blur, set size and connect to horizontal
BlurNodeRef blurNodeVertical = mContext->makeNode(new BlurNode);
blurNodeVertical->setValueForInputPortKey(Vec2f(0.0f, 1.0f/sourceTexture->getHeight()), "pixelSize");
mContext->connectNodes(blurNodeHorizontal, blurNodeVertical);

// evaluate
mTexture = mContext->evaluate(blurNodeVertical);
```

```C++
// TODO - custom effector node
```

### LIMITATIONS
At present, there is little if any error checking on missing or cyclic input connections; beware!

### GREETZ
- Heavily inspired by Rich Eakin's [Cinder-Audio2](https://forum.libcinder.org/topic/rfc-cinder-audio2-available-for-alpha-testing)
- [Shake](http://en.wikipedia.org/wiki/Shake_(software)) and [Quartz Composer](http://en.wikipedia.org/wiki/Quartz_Composer)
- Neil Mendoza for [ofxPostProcessing](https://github.com/neilmendoza/ofxPostProcessing)
- Patricio Gonzalezvivo for [ofxFX](https://github.com/patriciogonzalezvivo/ofxFX)
