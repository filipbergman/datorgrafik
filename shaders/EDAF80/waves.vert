#version 410
layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBinormal;
uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform float time;

out VS_OUT {
	vec3 vertex;
	vec3 normal;
	float x_derivate;
	float y_derivate;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	mat3 TBNSurface;
} vs_out;

float wave(vec2 position, vec2 direction, float amplitude, float frequency,
		   float phase, float sharpness, float time) {
	return amplitude * pow(sin((position.x * direction.x + position.y * direction.y)
		   * frequency + time * phase)
		   * 0.5 + 0.5, sharpness);
}

float derivateX(vec2 position, vec2 direction, float amplitude, float frequency,
		   float phase, float sharpness, float time) {
	return 0.5 * sharpness * frequency * amplitude *
	pow(sin((direction.x * position.x + direction.y * position.y) * frequency + time * phase) * 0.5 + 0.5, sharpness - 1)
	* cos((direction.x * position.x + direction.y * position.y) * frequency + time * phase) * direction.x;
}

float derivateY(vec2 position, vec2 direction, float amplitude, float frequency,
		   float phase, float sharpness, float time) {
	return 0.5 * sharpness * frequency * amplitude *
	pow(sin((direction.x * position.x + direction.y * position.y) * frequency + time * phase) * 0.5 + 0.5, sharpness - 1)
	* cos((direction.x * position.x + direction.y * position.y) * frequency + time * phase) * direction.y;
}

void main() {
	vec2 texScale = vec2(8, 4);
	float normalTime = mod(time, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0);
	vs_out.normalCoord0 = vTexCoords.xy*texScale + normalTime*normalSpeed;
	vs_out.normalCoord1 = vTexCoords.xy*texScale*2 + normalTime*normalSpeed*4;
	vs_out.normalCoord2 = vTexCoords.xy*texScale*4 + normalTime*normalSpeed*8;

	vec3 T = normalize(vec3(vTangent));
	vec3 B = normalize(vec3(vBinormal));
	vec3 N = normalize(vec3(vNormal));
	vs_out.TBNSurface = mat3(T, B, N);

	vec3 displaced_vertex = vVertex;
	float super_wave = wave(vVertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, time)
	+ wave(vVertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, time);
	displaced_vertex += super_wave * N; // displaced_vertex.y användes tidigare, nu flyttas vertices i normalens riktning.
	vs_out.x_derivate = derivateX(vVertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, time);
	vs_out.y_derivate = derivateY(vVertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, time);


	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.normal = vec3(normal_model_to_world * vec4(vNormal, 0.0));

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);
} 
