#version 330 core

// From:
// http://sunandblackcat.com/tipFullView.php?l=eng&topicid=31&topic=Advanced-GLSL-Lighting-Models

// Per vertex hemisphere lighting
uniform vec3 skyColour = vec3(0.35f, 0.96f, 1.0f);
uniform vec3 groundColour = vec3(1.0f, 0.95f, 0.26f);

const vec3 topHemisphereDirection = vec3(0.0f, 1.0f, 0.0f);

uniform mat4 matModelViewProjection;
uniform mat3 matNormal;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

// Send parameters to the fragment shader
out vec3 vertOutColour;

void main(void)
{
   // vertex to screen space
  gl_Position = matModelViewProjection * vec4(position, 1.0);
  
   // normal to world space
   vec3 N	= normalize(matNormal * normal);
   // cosine between normal and direction to upper hemisphere 
   // 1 - normal is oriented to upper hemisphere
   // -1 - normal is oriented to lower hemisphere
   float NdotL	= dot(N, topHemisphereDirection);
   // from [-1, 1] to [0, 1] range
   float lightInfluence	= NdotL * 0.5 + 0.5;
   
   // interpolate colors from upper and lower hemispheres
   vertOutColour = mix(groundColour, skyColour, lightInfluence);
}
