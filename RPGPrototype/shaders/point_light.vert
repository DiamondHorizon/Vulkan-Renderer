#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct PointLight {
	vec3 position;
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 inverseView;
	vec4 ambientLightColor; // Simulates Indirect Illumination
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (push_constant) uniform Push {
	vec3 position;
	vec4 color;
	float radius;
} push;

void main() {
	fragOffset = OFFSETS[gl_VertexIndex];
	
	// Transform the light position to camera space
	vec4 lightInCameraSpace = ubo.view * vec4(push.position, 1.0);

	// Apply offset in camera space
	vec4 positionInCameraSpace = lightInCameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
	gl_Position = ubo.projection * positionInCameraSpace;

	// vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	// vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};
	
	// vec3 worldPosition = ubo.lightPosition.xyz + LIGHT_RADIUS * fragOffset.x * cameraRightWorld + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

	// gl_Position = ubo.projection * ubo.view * vec4(worldPosition, 1.0);
}