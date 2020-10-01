#version 410
layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBinormal;
uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform mat4 MVP;

out VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
} vs_out;

void main() {
	vec3 T = normalize(vec3(normal_model_to_world * vec4(vTangent,   0.0)));
	vec3 B = normalize(vec3(normal_model_to_world * vec4(vBinormal, 0.0)));
	vec3 N = normalize(vec3(normal_model_to_world * vec4(vNormal,    0.0)));
	//vec3 T = normalize(vTangent);
	//vec3 B = normalize(vBinormal);
	//vec3 N = normalize(vNormal);
	vs_out.TBN = mat3(T, B, N);
	vs_out.texCoords = vTexCoords.xy;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vVertex, 1.0));
	vs_out.normal = vec3(normal_model_to_world * vec4(vNormal, 0.0));
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vVertex, 1.0);
} 
