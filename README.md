# Cinder-Pipeline
`Cinder-Pipeline` is an imaging pipeline to construct and evaluate chained shader-based effectors.

A `Context` is the container around which nodes are created, connections are made and nodes are evaluated. `Context` evaluates a `Node` by walking up the tree through the node's input connections and constructs a collection of ordered `Branch`.
A `Node` has multiple input `NodePort` which are both keyed and typed. Several data types are supported:
```C++
enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2f, Vec4f};
```
Although multiple data types are supported, being an imaging pipeline, dependencies are only calculated on `NodePortType::FBOImage` ports. Supporting that, all `Node` instances have a fixed single output port `NodeOutputPortKeyImage` of type `NodePortType::FBOImage`.

Currently there are two derived node classes, `SourceNode` and `EffectorNode`. `SourceNode` is intended to be used directly and has a port `SourceNodeInputPortKeyTexture` of type `NodePortType::Texture` (internally `gl::TextureRef`) that will bring textures into the pipeline. `EffectorNode` is intended to act as a base class for shader-based operators and the input ports can vary based on the specific need.

### USAGE
```C++

gl::TextureRef texture = gl::Texture::create(loadImage(loadAsset(RES_LENNA_IMAGE)));

// setup context internals (FBO) at source texture size
mContext = Context::create();
mContext->setup(texture->getSize(), 2);

// create source
SourceNodeRef sourceNode = mContext->makeNode(new SourceNode);
sourceNode->setValueForInputPortKey(texture, SourceNodeInputPortKeyTexture);

// create a color tint node and set to red
TintNodeRef tintNode = mContext->makeNode(new TintNode);
Vec4f color = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
tintNode->setValueForInputPortKey(tintColor, TintNodeInputPortKeyColor);
mContext->connectNodes(sourceNode, tintNode);

// create horizontal blur, set size and connect to source
BlurNodeRef blurNodeHorizontal = mContext->makeNode(new BlurNode);
Vec2f size = Vec2f(1.0f/texture->getWidth(), 0.0f);
blurNodeHorizontal->setValueForInputPortKey(size, BlurNodeInputPortKeyPixelSize);
mContext->connectNodes(tintNode, blurNodeHorizontal);

// create vertical blur, set size and connect to horizontal
BlurNodeRef blurNodeVertical = mContext->makeNode(new BlurNode);
size = Vec2f(0.0f, 1.0f/texture->getHeight());
blurNodeVertical->setValueForInputPortKey(size, BlurNodeInputPortKeyPixelSize);
mContext->connectNodes(blurNodeHorizontal, blurNodeVertical);

// evaluate
mTexture = mContext->evaluate(blurNodeVertical);
```

```C++
// TODO - custom effector node
```

### LIMITATIONS
At present, there is little if any error checking - beware!

### GREETZ
- Heavily inspired by Rich Eakin's [Cinder-Audio2](https://forum.libcinder.org/topic/rfc-cinder-audio2-available-for-alpha-testing)
- [Shake](http://en.wikipedia.org/wiki/Shake_(software)) and [Quartz Composer](http://en.wikipedia.org/wiki/Quartz_Composer)
- Neil Mendoza for [ofxPostProcessing](https://github.com/neilmendoza/ofxPostProcessing)
- Patricio Gonzalezvivo for [ofxFX](https://github.com/patriciogonzalezvivo/ofxFX)
