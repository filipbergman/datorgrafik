#version 410
uniform sampler2D diff_map;
uniform sampler2D bump_map;
uniform sampler2D rough_map;
uniform vec3 light_position;
uniform float shininess;
uniform vec3 diffuse;
uniform vec3 ambient;
uniform vec3 specular;
uniform bool use_normal_mapping;
uniform vec3 camera_position;
uniform bool has_textures;
in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 texCoords;
	mat3 TBN;
} fs_in;
out vec4 frag_color;

void main() { 
	vec3 n = normalize(fs_in.normal);
	vec3 LightDir = normalize(light_position - fs_in.vertex);

	vec3 s = specular;
	vec3 d = diffuse;
	if(has_textures) {
		vec3 d = texture(diff_map, fs_in.texCoords).rgb;
	}

	// .rgb takes the first three "coordinates"(ignores the opacity)
	vec3 bump_texture = (texture(bump_map, fs_in.texCoords).rgb)*2-1;
	vec3 rough_texture = texture(rough_map, fs_in.texCoords).rgb;

	vec3 bump = normalize(fs_in.TBN * bump_texture);

	if(use_normal_mapping && has_textures) {
		n = bump;
	}

	// Diffuse
	float diff = max(dot(n, LightDir), 0.0);

	// Specular
	vec3 viewDir = normalize(camera_position-fs_in.vertex);
	vec3 reflectDir = reflect(-LightDir,n);

	float specularStrength = 0.3;
	float spec = pow(max(dot(n,reflectDir),0.0),shininess) * specularStrength;

	s = rough_texture * spec;
	d = d*diff;

	frag_color = vec4(ambient, 1.0f) +
				 vec4(d, 1.0f) +
				 vec4(s, 1.0f);
}
