//
//  BlendNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 24 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "BlendNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

// http://en.wikipedia.org/wiki/Alpha_compositing
// http://cairographics.org/operators/
// http://www.svgopen.org/2005/papers/abstractsvgopen/
const std::string FragmentShaderBlend = R"(
    #version 150
    uniform sampler2D image;
    uniform sampler2D blendImage;
    uniform int blendMode;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    #ifndef DEFAULT_OVERRIDE
        #define OVER
    #endif

    void main() {
        vec4 baseColor = texture(image, vTexCoord0);
        vec4 blendColor = texture(blendImage, vTexCoord0);

        #ifdef SUBTRACT
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor = max(baseColor - blendColor, vec4(0.0));
        #endif
        #ifdef OVER
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor.rgb = blendColor.rgb + baseColor.rgb * (1.0 - blendColor.a);
            oFragColor.a = blendColor.a + baseColor.a * (1.0 - blendColor.a);
        #endif
        #ifdef MULTIPLY
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor.rgb = baseColor.rgb * blendColor.rgb;
            oFragColor.a = 1.0;
        #endif
    }
)";

BlendNode::BlendNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(BlendNodeInputPortKeyBlendImage, NodePortType::FBOImage),
        NodePort::create(BlendNodeInputPortKeyBlendMode, NodePortType::Index, "Mode",
          static_cast<int>(BlendMode::Over), {static_cast<int>(BlendMode::Subtract), static_cast<int>(BlendMode::Over), static_cast<int>(BlendMode::Multiply)}, {"Subtract", "Over", "Multiply"}),
    };
    setInputPorts(inputPorts);

    // setup shader when radius changes
    connectValueForInputPortKeyChangedHandler(BlendNodeInputPortKeyBlendMode, [&](const std::string& key, const boost::any& newValue, const boost::any& oldValue) {
        int index = boost::any_cast<int>(newValue);
        BlendMode mode = static_cast<BlendMode>(getInputPortForKey(BlendNodeInputPortKeyBlendMode)->getValues().at(index));
        setupShaderForBlendMode(mode);
    });

    setupShader(sVertexShaderPassThrough, FragmentShaderBlend);
}

BlendNode::~BlendNode() {
}

void BlendNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    FBOImageRef inputFBOImage2 = getValueForInputPortKey<FBOImageRef>(BlendNodeInputPortKeyBlendImage);

    gl::ScopedTextureBind texture(inputFBOImage->getTexture(), 0);
    gl::ScopedTextureBind texture2(inputFBOImage2->getTexture(), 1);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(BlendNodeInputPortKeyBlendImage, 1);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}

void BlendNode::setupShaderForBlendMode(BlendMode mode) {
    auto format = gl::GlslProg::Format().vertex(sVertexShaderPassThrough).fragment(FragmentShaderBlend);
    format.define("DEFAULT_OVERRIDE");
    switch (mode) {
        case BlendMode::Subtract:
            format.define("SUBTRACT");
            break;
        case BlendMode::Over:
            format.define("OVER");
            break;
        case BlendMode::Multiply:
            format.define("MULTIPLY");
            break;
    }

    try {
        mShader = gl::GlslProg::create(format);
    } catch (gl::GlslProgExc e) {
        throw e;
    }
}
