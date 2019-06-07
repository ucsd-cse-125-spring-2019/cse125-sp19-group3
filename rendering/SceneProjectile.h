#pragma once
#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Model.h"
#include "Transform.h"
#include "../networking/KillStreak/CoreTypes.hpp"

class SceneProjectile {
	const static unsigned int KUNAI_ID = 200;
public:
		SceneProjectile(unsigned int nodeIdCounter, unsigned int ownerId, Point initPoint, Point finalPoint, Transform * skillRoot, float speed, float range) {
			this->ownerId = ownerId;
			this->currentPos = initPoint;
			this->direction = glm::normalize(finalPoint - initPoint);
			this->initPos = initPoint;
			this->currentOri = Point(0.0f, 1.0f, 0.0f);
			this->speed = speed;
			this->range = range;
			skillRoot->addChild(nodeIdCounter);
			
			float dotResult = glm::dot(glm::normalize(finalPoint - initPoint), currentOri);
			float angle = glm::acos(dotResult);
			Point axis = glm::cross(currentOri, glm::normalize(finalPoint - initPoint));
			if (glm::length(axis) > 0) {
				node = new Transform(nodeIdCounter, glm::translate(glm::mat4(1.0f), currentPos),
					glm::rotate(glm::mat4(1.0f), 180 / 180.f * glm::pi<float>(), glm::vec3(1, 0, 0)),
					glm::scale(glm::mat4(1.0f), Point(0.1f))
				);
				rotate(angle, axis);
			}
			else {
				node = new Transform(nodeIdCounter, glm::translate(glm::mat4(1.0f), currentPos),
					glm::rotate(glm::mat4(1.0f), 0 / 180.f * glm::pi<float>(), glm::vec3(1, 0, 0)),
					glm::scale(glm::mat4(1.0f), Point(0.1f))
				);
			}
			node->model_ids.insert(KUNAI_ID);
		};
		~SceneProjectile() {};
		void move();
		void rotate(float angle, Point axis);
		void translate(Point forward);
		void update();
		bool outOfRange();

		unsigned int ownerId;
		Point initPos;
		Point currentPos;
		Point currentOri;
		Point direction;
		Transform* node;
		float speed = 0.6f;
		float range = 1.0f;
};



#endif

