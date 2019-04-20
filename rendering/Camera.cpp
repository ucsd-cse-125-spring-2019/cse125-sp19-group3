////////////////////////////////////////
// Camera.cpp
////////////////////////////////////////

#include "Camera.h"

////////////////////////////////////////////////////////////////////////////////

Camera::Camera() {
	Reset();
}

////////////////////////////////////////////////////////////////////////////////

void Camera::Update() {
	// Compute camera world matrix
	glm::mat4 world(1);
	world[3][2]=Distance;
	world=glm::eulerAngleY(glm::radians(-Azimuth)) * glm::eulerAngleX(glm::radians(-Incline)) * world;

	// Compute view matrix (inverse of world matrix)
	//glm::mat4 view = glm::inverse(world);
	glm::mat4 view = glm::lookAt(cam_pos, cam_look_at, cam_up);

	// Compute perspective projection matrix
	glm::mat4 project = glm::perspective(glm::radians(FOV),Aspect,NearClip,FarClip);

	// Compute final view-projection matrix
	ViewProjectMtx=project * view;
}

////////////////////////////////////////////////////////////////////////////////

void Camera::Reset() {
	FOV=45.0f;
	Aspect=1.33f;
	NearClip=0.1f;
	FarClip=1000.0f;

	Distance=10.0f;
	Azimuth=0.0f;
	Incline=20.0f;

	cam_pos = glm::vec3(10.0f, 30.0f, 30.0f);
	cam_look_at = glm::vec3(0.0f, 0.0f, 0.0f);
	cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////
