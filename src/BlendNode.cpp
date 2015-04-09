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

    #define MODE_SUBTRACT 0
    #define MODE_OVER 1
    #define MODE_MULTIPLY 2

    void main() {
        vec4 baseColor = texture(image, vTexCoord0);
        vec4 blendColor = texture(blendImage, vTexCoord0);

        if (blendMode == MODE_SUBTRACT) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor = max(baseColor - blendColor, vec4(0.0));
        } else if (blendMode == MODE_OVER) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor.rgb = blendColor.rgb + baseColor.rgb * (1.0 - blendColor.a);
            oFragColor.a = blendColor.a + baseColor.a * (1.0 - blendColor.a);
        } else if (blendMode == MODE_MULTIPLY) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            oFragColor.rgb = baseColor.rgb * blendColor.rgb;
            oFragColor.a = 1.0;
        }
    }
)";

BlendNode::BlendNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(BlendNodeInputPortKeyBlendImage, NodePortType::FBOImage),
        NodePort::create(BlendNodeInputPortKeyBlendMode, NodePortType::Index, "Mode",
          static_cast<int>(BlendNode::BlendMode::Over), {static_cast<int>(BlendNode::BlendMode::Subtract), static_cast<int>(BlendNode::BlendMode::Over), static_cast<int>(BlendNode::BlendMode::Multiply)}, {"Subtract", "Over", "Multiply"}),
    };
    setInputPorts(inputPorts);

    setupShader(sVertexShaderPassThrough, FragmentShaderBlend);
}

BlendNode::~BlendNode() {
}

void BlendNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    FBOImageRef inputFBOImage2 = getValueForInputPortKey<FBOImageRef>(BlendNodeInputPortKeyBlendImage);
    int index = getValueForInputPortKey<int>(BlendNodeInputPortKeyBlendMode);
    int BlendMode = getInputPortForKey(BlendNodeInputPortKeyBlendMode)->getValues().at(index);

    gl::ScopedTextureBind texture(inputFBOImage->getTexture(), 0);
    gl::ScopedTextureBind texture2(inputFBOImage2->getTexture(), 1);
    gl::ScopedGlslProg shader(mShader);

    mShader->uniform(NodeInputPortKeyImage, 0);
    mShader->uniform(BlendNodeInputPortKeyBlendImage, 1);
    mShader->uniform(BlendNodeInputPortKeyBlendMode, BlendMode);
    gl::drawSolidRect(outputFBOImage->getFBO()->getBounds());
}
