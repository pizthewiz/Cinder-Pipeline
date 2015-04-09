# Cinder-Pipeline
`Cinder-Pipeline` is an imaging pipeline to construct and evaluate chained shader-based effectors.

**NOTE**: `Cinder-Pipeline` is to be used with the heir apparent [glNext](https://github.com/cinder/Cinder/tree/glNext) Cinder branch.

A `Context` is the container around which nodes are created, connections are made and nodes are evaluated. `Context` evaluates a `Node` by walking up the tree through the node's input connections and constructs a collection of ordered `Branch`.
A `Node` may have multiple input `NodePort` which are both keyed and typed. Several data types are supported:
```C++
enum class NodePortType {FBOImage, Texture, Bool, Float, Int, Vec2, Color, Index, FilePath};
```
Although multiple data types are supported, being an imaging pipeline, dependencies are only calculated on `NodePortType::FBOImage` ports. Supporting that, all `Node` instances have a fixed single output port `NodeOutputPortKeyImage` of type `NodePortType::FBOImage`.

Currently there are two abstract derived node classes, `SourceNode` and `EffectorNode`. `SourceNode` has two derived classes `TextureSourceNode` and `FileSourceNode` that will both bring textures into the pipeline. `EffectorNode` is intended to act as a base for shader-based operators and the input ports can vary based on the specific need.

### USAGE
```C++
// setup context internals (FBO) at source texture size
auto context = Context::create();
context->setup(texture->getSize(), 2);

// create source
auto sourceNode = context->makeNode(new TextureSourceNode);
auto texture = gl::Texture::create(loadImage(loadAsset(RES_LENNA_IMAGE)));
sourceNode->setValueForInputPortKey(texture, TextureSourceNodeInputPortKeyTexture);

// create red color tint
auto tintNode = context->makeNode(new TintNode);
ColorAf color = ColorAf(1.0f, 0.0f, 0.0f, 1.0f);
tintNode->setValueForInputPortKey(tintColor, TintNodeInputPortKeyColor);
context->connectNodes(sourceNode, tintNode);

// create horizontal blur
auto blurNode = context->makeNode(new GaussianBlurNode);
int direction = static_cast<int>(GaussianBlurNode::BlurDirection::Horizontal);
blurNode->setValueForInputPortKey(direction, GaussianBlurNodeInputPortKeyDirection);
context->connectNodes(tintNode, blurNode);

// evaluate (source -> tint -> blur)
auto resultTexture = context->evaluate(blurNode);
```

### CUSTOM EFFECTOR NODE
```C++
// TODO
```

### LIMITATIONS
At present, there is little if any error checking - beware!

### GREETZ
- Heavily inspired by Rich Eakin's [Cinder-Audio2](https://forum.libcinder.org/topic/rfc-cinder-audio2-available-for-alpha-testing)
- [Shake](http://en.wikipedia.org/wiki/Shake_(software)) and [Quartz Composer](http://en.wikipedia.org/wiki/Quartz_Composer)
- Neil Mendoza for [ofxPostProcessing](https://github.com/neilmendoza/ofxPostProcessing)
- Patricio Gonzalezvivo for [ofxFX](https://github.com/patriciogonzalezvivo/ofxFX)
