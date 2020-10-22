#version 330 core

//-----------------------------------------------------------------------------
// parallax_normal_mapping_dir.frag
//
// Copyright (c) 2010 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
//
// Tangent space parallax normal mapping shader for a single directional light
// source.
//
// http://www.dhpoware.com/demos/gl3ParallaxNormalMapping.html
//
// Parallax mapping is a method for approximating the correct appearance of
// uneven surfaces by modifying the texture coordinate for each fragment of the
// surface. This provides the illusion of depth to such surfaces.
//
// The theory behind parallax mapping with offset limiting is explained here:
// http://web.archive.org/web/20060207121301/http://www.infiscape.com/doc/parallax_mapping.pdf
//
// This fragment shader is almost the same as a standard tangent space normal
// mapping fragment shader with the exception of the parallax mapping term.
// Rather than using the interpolated texture coordinate from the vertex shader
// to index the normal and color map textures a new texture coordinate is
// calculated and used instead.
//
//-----------------------------------------------------------------------------

// Precision qualifiers are added for code portability with OpenGL ES, not for
// functionality. According to the GLSL 1.30 and later specs: the same object
// declared in different shaders that are linked together must have the same
// precision qualification. This applies to inputs, outputs, uniforms, and
// globals.

precision highp float;

// Custom user defined structures used to model the directional light source
// and the characteristics of the material that the light will interact with.
// These are setup in the host application. This must match the declaration in
// the vertex shader.

struct cDirectionalLight
{
  vec3 direction;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};
uniform cDirectionalLight directionalLight;

struct cMaterial
{
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
  float fShininess;
};
uniform cMaterial material;

uniform sampler2D texUnit0; // Diffuse texture
uniform sampler2D texUnit1; // Specular map
uniform sampler2D texUnit2; // Normal map
uniform sampler2D texUnit3; // Height map

uniform vec2 ParallaxScaleBias;

// Fragment shader input variable declarations must exactly match the vertex
// shader's output variable declarations. The output of the vertex shader and
// the input of the fragment shader form an interface. For this interface,
// vertex shader output variables and fragment shader input variables of the
// same name must match in type and qualification (other than out matching to
// in).

in vec2 vertOutTexCoord0;

in vec3 LightDir;
in vec3 ViewDir;

// GLSL 1.3 deprecates gl_FragColor. Its replacement is user defined output
// variables. Fragment shader output variables can be explicitly bound to a
// render target. When a program is linked any output variables without an
// explicit binding specified through glBindFragDataLocation() will
// automatically be bound to fragment colors by the GL.

out vec4 FragColor;

void main()
{
  vec3 L = normalize(LightDir);
  vec3 V = normalize(ViewDir);

  float height = texture(texUnit3, vertOutTexCoord0).r;
  height = height * ParallaxScaleBias.x + ParallaxScaleBias.y;

  vec2 texCoord = vertOutTexCoord0 + (height * V.xy);

  vec3 N = normalize(texture(texUnit2, texCoord.st).xyz * 2.0 - 1.0);
  vec3 R = normalize(-reflect(L, N));

  float nDotL = max(0.0, dot(N, L));
  float rDotV = max(0.0, dot(R, V));

  vec4 ambient = directionalLight.ambientColour * material.ambientColour;
  vec4 diffuse = directionalLight.diffuseColour * material.diffuseColour * nDotL;
  vec4 specular = directionalLight.specularColour * material.specularColour * pow(rDotV, material.fShininess);
  vec4 texel = texture(texUnit0, texCoord);
  float gloss = texture(texUnit1, texCoord).r;

  FragColor = (ambient + diffuse + (gloss * specular)) * texel;
}
