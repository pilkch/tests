#version 330 core

//-----------------------------------------------------------------------------
// parallax_normal_mapping_dir.vert
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
// This vertex shader is a standard tangent space normal mapping vertex shader
// where the light and view vectors are transformed into tangent space.
//
//-----------------------------------------------------------------------------

// Precision qualifiers are added for code portability with OpenGL ES, not for
// functionality. According to the GLSL 1.30 and later specs: the same object
// declared in different shaders that are linked together must have the same
// precision qualification. This applies to inputs, outputs, uniforms, and
// globals.

precision highp float;

struct cDirectionalLight
{
  vec3 direction;
  vec4 ambientColour;
  vec4 diffuseColour;
  vec4 specularColour;
};
uniform cDirectionalLight directionalLight;


// GLSL 1.30 (OpenGL 3.0) deprecates most of the built-in uniform state
// variables. In GLSL 1.40 (OpenGL 3.1) they have been removed. Typically we
// need to supply projection, view, and model matrices in order to transform
// vertices from model space to clip space.

uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matModelView;
uniform mat3 matNormal;


#define POSITION 0
#define NORMAL 1
#define TEXCOORD0 2
#define TANGENT 3
layout(location = POSITION) in vec3 position;
layout(location = NORMAL) in vec3 normal;
layout(location = TEXCOORD0) in vec2 texCoord0;
layout(location = TANGENT) in vec4 tangent;

// GLSL 1.30 (OpenGL 3.0) also deprecates varying shader variables. This has
// been replaced with user-defined generic shader output variables. The output
// of the vertex shader and the input of the fragment shader form an interface.
// For this interface, vertex shader output variables and fragment shader input
// variables of the same name must match in type and qualification (other than
// out matching to in).

out vec2 vertOutTexCoord0;

out vec3 LightDir;
out vec3 ViewDir;

void main()
{
  // Create a matrix to transform vectors from eye space to tangent space.

  vec3 n = normalize(matNormal * normal);
  vec3 t = normalize(matNormal * tangent.xyz);
  vec3 b = cross(n, t) * tangent.w;

  mat3 tbnMatrix = mat3(
    t.x, b.x, n.x,
    t.y, b.y, n.y,
    t.z, b.z, n.z
  );

  // All vertex shaders should write the transformed homogeneous clip space
  // vertex position into the gl_Position variables.

  mat4 mvpMatrix = matProjection * matModelView;
  vec4 pos = vec4(position.xyz, 1.0);

  gl_Position = mvpMatrix * pos;

  // Calculate the view direction in eye space. In eye space space the camera
  // position is the origin. The eye space view direction is simply the
  // inverse of the vector to any vertex that has been transformed into eye
  // space. Once we have the eye space view direction vector we transform it
  // to tangent space.

  pos = matModelView * pos;
  ViewDir = -(pos.xyz / pos.w);
  ViewDir = tbnMatrix * ViewDir;

  // Transform the light direction into tangent space space. Directional
  // lights are specified in world space. For example, a directional light
  // aimed along the world negative z axis has the vector (0, 0, -1).
  // Since the tbnMatrix will only transform eye space vectors to tangent
  // space we first transform the light direction vector to eye space and
  // then transform it to tangent space.

  LightDir = vec3(matView * vec4(-directionalLight.direction, 0.0f));
  LightDir = tbnMatrix * LightDir;

  vertOutTexCoord0 = texCoord0;
}
