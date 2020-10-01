#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	glm::vec2 xVector(1.0f, x);
	glm::mat2 matrice(1.0f, -1.0f, 0.0f, 1.0f);
	glm::mat3x2 point(p0.x, p1.x, p0.y,
					  p1.y, p0.z, p1.z);

	return xVector * matrice * point;
}

glm::vec3 interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x) {

	glm::vec4 xVector(1.0f, x, x*x, x*x*x);

	glm::mat4 matrice = glm::transpose(glm::mat4(0.0f,   1.0f,   0.0f,  0.0f,
												 -t,     0.0f,   t,     0.0f,
												 2*t,    t-3.0f, 3.0f-2*t,-t,
												 -t,     2.0-t,  t-2.0, t));
	glm::mat3x4 point(p0.x, p1.x, p2.x, p3.x,
								  p0.y, p1.y, p2.y, p3.y,
								  p0.z, p1.z, p2.z, p3.z);
	
	return xVector * matrice * point;
}
