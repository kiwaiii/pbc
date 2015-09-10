//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <glf/camera.hpp>
#include <glf/utils.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>
#include <limits>

namespace glf
{
	//-------------------------------------------------------------------------
	// Camera methods
	//-------------------------------------------------------------------------
	Camera::Camera() :
		nearPlane(0.1f),
		farPlane(100.f),
		vFov(std::numeric_limits<float>::infinity()),
		ratio(1.f)
	{

	}
	//-------------------------------------------------------------------------
	Camera::~Camera()
	{

	}
	//-------------------------------------------------------------------------	
	const glm::mat4& Camera::View() const
	{
		viewMatrix = glm::lookAt(Eye(), Center(), Up());
		return viewMatrix;
	}
	//-------------------------------------------------------------------------	
	const glm::mat4& Camera::Projection() const
	{
		return projectionMatrix;
	}
	//-------------------------------------------------------------------------
	void Camera::Projection(const glm::mat4& _proj, float _near, float _far)
	{
		nearPlane = _near;
		farPlane = _far;
		ratio = 1.f;
		vFov = std::numeric_limits<float>::infinity();
		projectionMatrix = _proj;
	}
	//-------------------------------------------------------------------------
	void Camera::Orthogonal(float _halfSize, float _near, float _far)
	{
		nearPlane = _near;
		farPlane = _far;
		//projectionMatrix	= glm::ortho3D(-_halfSize, _halfSize, -_halfSize, _halfSize, _near, _far);
		projectionMatrix = glm::ortho(-_halfSize, _halfSize, -_halfSize, _halfSize, _near, _far);
		resolution.x = (int)(_halfSize*2.0f);
		resolution.y = (int)(_halfSize*2.0f);
	}
	//-------------------------------------------------------------------------
	void Camera::Perspective(float _verticalFov,
		int   _width,
		int   _height,
		float _near,
		float _far)
	{
		nearPlane = _near;
		farPlane = _far;
		ratio = _width / float(_height);
		resolution.x = _width;
		resolution.y = _height;
		vFov = _verticalFov;
		projectionMatrix = glm::perspective(_verticalFov,
			_width / float(_height),
			_near, _far);
	}

	//-------------------------------------------------------------------------
	// Hybrid Camera methods
	//-------------------------------------------------------------------------
	OrbitCamera::OrbitCamera(float _speed) :
		Camera(),
		Theta(M_PI*0.5f),
		Phi(M_PI*1.5f),
		Distance(5.f),
		lastPos(-1, -1),
		action(NONE),
		center(0, 0, 0),
		phiAxis(0, 0, 1),
		speed(_speed)
	{

	}
	//-------------------------------------------------------------------------
	OrbitCamera::~OrbitCamera()
	{

	}
	//-------------------------------------------------------------------------
	void OrbitCamera::Speed(float _speed)
	{
		speed = _speed;
	}
	//-------------------------------------------------------------------------
	glm::vec3 OrbitCamera::Eye() const
	{
		float sinTheta = sin(Theta);
		return glm::vec3(Distance*sinTheta*cos(Phi), Distance*sinTheta*sin(Phi), Distance*cos(Theta)) + center;
	}
	//-------------------------------------------------------------------------
	glm::vec3 OrbitCamera::Center() const
	{
		return center;
	}
	//-------------------------------------------------------------------------
	glm::vec3 OrbitCamera::Up() const
	{
		float sinTheta = sin(Theta);
		glm::vec3 dir = glm::vec3(Distance*sinTheta*cos(Phi), Distance*sinTheta*sin(Phi), Distance*cos(Theta));
		glm::vec3 n = glm::vec3(0, 0, 1);
		//glm::vec3 dir	= glm::normalize(Eye());
		glm::vec3 right = glm::normalize(glm::cross(n, dir));
		return glm::normalize(glm::cross(dir, right)); // up
	}
	//-------------------------------------------------------------------------
	void OrbitCamera::MouseEvent(int _x, int _y, Mouse::Button _b, Mouse::State _s)
	{
		switch (_b)
		{
		case Mouse::RIGHT:
			if (_s == Mouse::PRESS)
				action = action | PANE;
			else
				action = action & ~PANE;
			break;
		case Mouse::LEFT:
			if (_s == Mouse::PRESS)
				action = action | MOVE;
			else
				action = action & ~MOVE;
			break;
		case Mouse::SCROLL_UP:
			if (_s == Mouse::PRESS)
			{
				//Distance -= glm::clamp((float)exp(Distance-5.f),0.001f,1.f);
				Distance -= Distance*speed;
				Distance = std::max(Distance, 0.001f);
			}
			break;
		case Mouse::SCROLL_DOWN:
			if (_s == Mouse::PRESS)
			{
				//Distance += glm::clamp(Distance*2.f,0.001f,1.f);
				Distance += Distance*speed;
				Distance = std::min(Distance, 1000000000.f);
			}
			break;
		default:
			break;
		}

		if (action == NONE)
			lastPos.x = lastPos.y = -1;
	}
	//-------------------------------------------------------------------------
	void OrbitCamera::MoveEvent(float _x, float _y)
	{
		// Compute centered coordinate
		glm::ivec2 pos(_x, _y);

		// If is the fisrt move after click, update coordinate
		if (lastPos.x == -1 || lastPos.y == -1)
			lastPos = pos;

		if (action & PANE)
		{
			glm::vec3 dir = center - Eye();
			glm::vec3 right = glm::normalize(glm::cross(dir, phiAxis));
			glm::vec3 up = glm::normalize(glm::cross(right, dir));


			float horizontallyOff = Distance * (pos.x - lastPos.x) / 500.f;
			float verticallyOff = Distance * (pos.y - lastPos.y) / 1000.f;
			center += -horizontallyOff * right + verticallyOff * up;
		}
		else if (action & MOVE)
		{
			Phi -= (pos.x - lastPos.x) / 100.f;
			Theta -= (pos.y - lastPos.y) / 200.f;

			Theta = glm::clamp(Theta, 0.01f, 3.14f);
			//			Theta = std::max(Theta, 0.01f);
			//			Theta = std::min(Theta, 3.14f);

						// Check bound and recenter value 
						// Add 2PI to have positive value (fmod need positive value)
			if (Phi < 0)
				Phi += 2.f*M_PI;
			if (Theta < 0)
				Theta += 2.f*M_PI;
			Phi = fmod(Phi, float(2.f*M_PI));
			//Theta = fmod(Theta,glm::Constant::HALF_PI);
			Theta = fmod(Theta, float(M_PI));
		}

		// Update last coordinate
		lastPos = pos;
	}

	//-------------------------------------------------------------------------
	// Flying Camera methods
	//-------------------------------------------------------------------------
	FlyingCamera::FlyingCamera(float _speed) :
		Camera(),
		lastPos(-1, -1),
		action(NONE),
		center(0, 0, 0),
		direction(0, 1, 0),
		up(0, 0, 1),
		speed(_speed)
	{

	}
	//-------------------------------------------------------------------------
	FlyingCamera::~FlyingCamera()
	{

	}
	//-------------------------------------------------------------------------
	void FlyingCamera::Speed(float _speed)
	{
		speed = _speed;
	}
	//-------------------------------------------------------------------------
	glm::vec3 FlyingCamera::Eye() const
	{
		return center;
	}
	//-------------------------------------------------------------------------
	glm::vec3 FlyingCamera::Center() const
	{
		return center + direction;
	}
	//-------------------------------------------------------------------------
	glm::vec3 FlyingCamera::Up() const
	{
		return up;
	}
	//-------------------------------------------------------------------------
	void FlyingCamera::MouseEvent(int _x, int _y, Mouse::Button _b, Mouse::State _s)
	{
		switch (_b)
		{
		case Mouse::LEFT:
			if (_s == Mouse::PRESS)
				action = action | MOVE;
			else
				action = action & ~MOVE;
			break;
		case Mouse::SCROLL_UP:
			if (_s == Mouse::PRESS)
			{
				speed = std::max(32.0f, speed * 2.0f);
			}
			break;
		case Mouse::SCROLL_DOWN:
			if (_s == Mouse::PRESS)
			{
				speed = std::min(0.0625f, speed / 2.0f);
			}
			break;
		default:
			break;
		}

		if (action == NONE)
			lastPos.x = lastPos.y = -1;
	}
	//-------------------------------------------------------------------------
	void FlyingCamera::MoveEvent(float _x, float _y)
	{
		// Compute centered coordinate
		glm::ivec2 pos(_x, _y);

		// If is the first move after click, update coordinate
		if (lastPos.x == -1 || lastPos.y == -1)
			lastPos = pos;

		if (action & MOVE)
		{
			// Horizontal
			float phi;
			{
				float hFov = glm::radians(VFov()) * Ratio();
				float hAdjacent = 1.0f / tan(hFov * 0.5f);
				float hNormCoord0 = ((float)lastPos.x / Resolution().x) * 2.0f - 1.0f;
				float hNormCoord1 = ((float)pos.x / Resolution().x) * 2.0f - 1.0f;
				phi = atan2f(hNormCoord1, hAdjacent) - atan2f(hNormCoord0, hAdjacent);
			}
			glm::mat4x4 rotPhi = glm::rotate(-glm::degrees(phi), up);

			// Vertical
			float theta;
			{
				float vFov = glm::radians(VFov());
				float vAdjacent = 1.0f / tan(vFov * 0.5f);
				float vNormCoord0 = ((float)lastPos.y / Resolution().y) * 2.0f - 1.0f;
				float vNormCoord1 = ((float)pos.y / Resolution().y) * 2.0f - 1.0f;
				theta = atan2f(vNormCoord1, vAdjacent) - atan2f(vNormCoord0, vAdjacent);
			}

			// Rotate the direction vector
			{
				glm::vec3 right = glm::cross(direction, up);
				glm::mat4x4 rotTheta = glm::rotate(-glm::degrees(theta), right);
				glm::mat4x4 rotation = rotTheta * rotPhi;

				direction = glm::vec3(rotation * glm::vec4(direction, 0.f));
			}

			// Enforce up vector to be in the vertical plane containing the direction vector
			if (glm::dot(direction, glm::vec3(0, 0, 1)) < 0.99)
			{
				glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 0, 1)));
				up = glm::normalize(glm::cross(right, direction));
			}
			else
			{
				glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
				up = glm::normalize(glm::cross(right, direction));
			}
		}

		// Update last coordinate
		lastPos = pos;
	}
	//-------------------------------------------------------------------------
	void FlyingCamera::KeyboardEvent(Keyboard::Key _k)
	{
		switch (_k)
		{
		case Keyboard::UP:
		case Keyboard::W:
		{
			center += direction;
		}
		break;

		case Keyboard::DOWN:
		case Keyboard::S:
		{
			center -= direction;
		}
		break;

		case Keyboard::LEFT:
		case Keyboard::A:
		{
			glm::vec3 right = glm::cross(direction, up);
			center -= right;
		}
		break;

		case Keyboard::RIGHT:
		case Keyboard::D:
		{
			glm::vec3 right = glm::cross(direction, up);
			center += right;
		}
		break;
		}
	}
	//-------------------------------------------------------------------------
	void FlyingCamera::KeyboardEvent(float deltaTime, Keyboard::State(*evaluator)(Keyboard::Key))
	{
		const glm::vec3* orientation = nullptr;
		if (evaluator(Keyboard::UP) == Keyboard::PRESS ||
			evaluator(Keyboard::W) == Keyboard::PRESS)
		{
			center += direction * speed * deltaTime;
		}
		else if (evaluator(Keyboard::DOWN) == Keyboard::PRESS ||
				 evaluator(Keyboard::S) == Keyboard::PRESS)
		{
			center -= direction * speed * deltaTime;
		}
		else if (evaluator(Keyboard::LEFT) == Keyboard::PRESS ||
				 evaluator(Keyboard::A) == Keyboard::PRESS)
		{
			glm::vec3 right = glm::cross(direction, up);
			center -= right * speed * deltaTime;
		}
		else if (evaluator(Keyboard::RIGHT) == Keyboard::PRESS ||
				 evaluator(Keyboard::D) == Keyboard::PRESS)
		{
			glm::vec3 right = glm::cross(direction, up);
			center += right * speed * deltaTime;
		}
	}
}
