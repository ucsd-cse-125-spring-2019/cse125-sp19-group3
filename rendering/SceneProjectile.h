#pragma once
#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Model.h"
#include "Transform.h"
#include "../networking/KillStreak/CoreTypes.hpp"

class SceneProjectile {
public:
		SceneProjectile(unsigned int nodeIdCounter, unsigned int ownerId, Point initPoint, Point finalPoint, Transform * skillRoot, float speed, float range, unsigned int projectileModelType) {
			this->ownerId = ownerId;
			this->currentPos = initPoint;
			this->direction = glm::normalize(finalPoint - initPoint);
			this->initPos = initPoint;
			this->currentOri = Point(0.0f, 1.0f, 0.0f);
			this->speed = speed;
			this->range = range;
			node = new Transform(nodeIdCounter, glm::translate(glm::mat4(1.0f), currentPos),
				glm::rotate(glm::mat4(1.0f), 0 / 180.f * glm::pi<float>(), glm::vec3(1, 0, 0)),
				glm::scale(glm::mat4(1.0f), Point(0.02f, 0.02f, 0.02f))
			);
			node->model_ids.insert(projectileModelType);
			skillRoot->addChild(nodeIdCounter);
		
			float dotResult = glm::dot(glm::normalize(finalPoint - initPoint), currentOri);
			float angle = glm::acos(dotResult);
			Point axis = glm::cross(currentOri, glm::normalize(finalPoint - initPoint));
			rotate(angle, axis);
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
		bool isAOE = false;
		bool isSilence = false;
};



#endif

