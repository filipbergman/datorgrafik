#version 410
uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform vec3 camera_position;
uniform samplerCube my_skybox;
uniform sampler2D my_waves;
in VS_OUT {
	vec3 vertex;
	vec3 normal;
	float x_derivate;
	float y_derivate;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	mat3 TBNSurface;
} fs_in;
out vec4 frag_color;

void main() {
	// Animated normal mapping
	vec3 n0 = texture(my_waves, fs_in.normalCoord0).rgb * 2 - 1;
	vec3 n1 = texture(my_waves, fs_in.normalCoord1).rgb * 2 - 1;
	vec3 n2 = texture(my_waves, fs_in.normalCoord2).rgb * 2 - 1;
	vec3 n_bump = normalize(n0 + n1 + n2);

	vec3 T = normalize(vec3(1.0, fs_in.x_derivate, 0.0));
	vec3 B = normalize(vec3(0.0, fs_in.y_derivate, 1.0));
	vec3 N = normalize(vec3(-fs_in.x_derivate, 1.0f, -fs_in.y_derivate));
	mat3 TBN = mat3(T, B, N);

	// TODO: varför funkar det utan TBNSurface?
	vec3 toWorld = vec3(normal_model_to_world * vec4(/*fs_in.TBNSurface **/ TBN * n_bump, 0.0));

	vec3 n = normalize(toWorld);

	// Color
	vec4 deep_color = vec4(0.0f, 0.0f, 0.1f, 1.0f);
	vec4 shallow_color = vec4(0.0f, 0.4f, 0.5f, 1.0f);
	
	vec3 viewDir = normalize(camera_position - fs_in.vertex);
	float facing = 1 - max(dot(viewDir, n), 0.0f);

	// Reflection
	vec3 reflectDir = normalize(reflect(-viewDir, n));
	vec4 reflection = texture(my_skybox, reflectDir);

	// Fresnel effect
	float R0 = 0.02037;
	float fastFresnel = R0 + (1-R0)*pow(1-dot(viewDir, n), 5);

	// Refraction
	vec3 refractionDir = refract(-viewDir, n, 1/1.33);
	vec4 refraction = texture(my_skybox, refractionDir);


	frag_color = mix(deep_color, shallow_color, facing)
				+ (reflection * fastFresnel)
				+ (0.7 * refraction * (1 - fastFresnel));
}
