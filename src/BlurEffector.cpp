//
//  BlurEffector.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "BlurEffector.h"

using namespace Cinder::Pipeline;

// taken from https://github.com/paulhoux/Cinder-Samples/blob/master/BloomingNeon/assets/blur_frag.glsl
const std::string FragmentShaderBlur = R"(
    #version 120
    uniform sampler2D image;
    uniform vec2 sampleOffset;

    void main() {
        vec2 position = gl_TexCoord[0].st;
        vec4 sum = vec4(0.0);
        sum += texture2D(image, position + -10.0 * sampleOffset) * 0.009167927656011385;
        sum += texture2D(image, position +  -9.0 * sampleOffset) * 0.014053461291849008;
        sum += texture2D(image, position +  -8.0 * sampleOffset) * 0.020595286319257878;
        sum += texture2D(image, position +  -7.0 * sampleOffset) * 0.028855245532226279;
        sum += texture2D(image, position +  -6.0 * sampleOffset) * 0.038650411513543079;
        sum += texture2D(image, position +  -5.0 * sampleOffset) * 0.049494378859311142;
        sum += texture2D(image, position +  -4.0 * sampleOffset) * 0.060594058578763078;
        sum += texture2D(image, position +  -3.0 * sampleOffset) * 0.070921288047096992;
        sum += texture2D(image, position +  -2.0 * sampleOffset) * 0.079358891804948081;
        sum += texture2D(image, position +  -1.0 * sampleOffset) * 0.084895951965930902;
        sum += texture2D(image, position +   0.0 * sampleOffset) * 0.086826196862124602;
        sum += texture2D(image, position +  +1.0 * sampleOffset) * 0.084895951965930902;
        sum += texture2D(image, position +  +2.0 * sampleOffset) * 0.079358891804948081;
        sum += texture2D(image, position +  +3.0 * sampleOffset) * 0.070921288047096992;
        sum += texture2D(image, position +  +4.0 * sampleOffset) * 0.060594058578763078;
        sum += texture2D(image, position +  +5.0 * sampleOffset) * 0.049494378859311142;
        sum += texture2D(image, position +  +6.0 * sampleOffset) * 0.038650411513543079;
        sum += texture2D(image, position +  +7.0 * sampleOffset) * 0.028855245532226279;
        sum += texture2D(image, position +  +8.0 * sampleOffset) * 0.020595286319257878;
        sum += texture2D(image, position +  +9.0 * sampleOffset) * 0.014053461291849008;
        sum += texture2D(image, position + +10.0 * sampleOffset) * 0.009167927656011385;
        gl_FragColor = sum;
    }
)";

BlurEffectorRef BlurEffector::create() {
    return BlurEffectorRef(new BlurEffector())->shared_from_this();
}

BlurEffector::BlurEffector() {
    setupShader(sVertexShaderPassThrough, FragmentShaderBlur);
}

BlurEffector::~BlurEffector() {
}

void BlurEffector::execute(gl::Fbo& inputFBO, const int inputFBOAttachment, gl::Fbo& outputFBO, const int outputFBOAttachment) {
	outputFBO.bindFramebuffer(); {
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + outputFBOAttachment);
        inputFBO.bindTexture(0, inputFBOAttachment); {
            mShader->bind(); {
                mShader->uniform("image", 0);
                mShader->uniform("sampleOffset", mSampleOffset);
                gl::drawSolidRect(outputFBO.getBounds());
            } mShader->unbind();
        } inputFBO.unbindTexture();
    } outputFBO.unbindFramebuffer();
}
