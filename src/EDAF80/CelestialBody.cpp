#include "CelestialBody.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"

CelestialBody::CelestialBody(bonobo::mesh_data const& shape,
	GLuint const* program, GLuint diffuse_texture_id)
{
	_body.set_geometry(shape);
	_body.set_program(program);
	_body.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
}

glm::mat4 CelestialBody::render(std::chrono::microseconds ellapsed_time,
	glm::mat4 const& view_projection,
	glm::mat4 const& parent_transform)
{
	// BODY
	glm::mat4 scaler = glm::scale(glm::mat4(1.0f), _body_scale);
	std::chrono::duration<float> const ellapsed_time_s = ellapsed_time;
	_spin_angle += ellapsed_time_s.count() * _spin_speed;

	glm::mat4 spinMatrix = glm::rotate(glm::mat4(1.0f), _spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 tiltMatrix = glm::rotate(glm::mat4(1.0f), _spin_incl, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 config = {tiltMatrix * spinMatrix * scaler};

	// ORBIT
	_orbit_angle += ellapsed_time_s.count() * _orbit_speed;
	glm::mat4 orbitSpinMatrix = glm::rotate(glm::mat4(1.0f), _orbit_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_orbit_radius, 0.0f, 0.0f));

	glm::mat4 orbitRotationMatrix = glm::rotate(glm::mat4(1.0f), _orbit_incl, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 finalConfig = parent_transform * orbitRotationMatrix * orbitSpinMatrix * translationMatrix * config;

	// RING
	glm::mat4 ringScaler = glm::scale(glm::mat4(1.0f), glm::vec3(_ring_scale,0));
	glm::mat4 ringAngleMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 ringConfig = parent_transform * orbitRotationMatrix * orbitSpinMatrix * ringAngleMatrix * translationMatrix * ringScaler;
	_ring.render(view_projection, ringConfig);

	_body.render(view_projection, finalConfig);
	return parent_transform * orbitRotationMatrix * orbitSpinMatrix * translationMatrix;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
	_body_scale = scale;
}


void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
	_spin_incl = configuration.inclination;
	_spin_speed = configuration.speed;

}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
	_orbit_incl = configuration.inclination;
	_orbit_radius = configuration.radius;
	_orbit_speed = configuration.speed;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape,
	GLuint const* program,
	GLuint diffuse_texture_id,
	glm::vec2 const& scale)
{
	_ring.set_geometry(shape);
	_ring.set_program(program);
	_ring.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_ring_scale = scale;
}

void CelestialBody::add_child(CelestialBody* child) {
	children.push_back(child);
}

std::vector<CelestialBody*> const& CelestialBody::get_children() const {
	return children;
}

