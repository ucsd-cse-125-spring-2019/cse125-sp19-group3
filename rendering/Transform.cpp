#include "Transform.h"


Transform::Transform() : Transform(0, glm::mat4(1)) {}

Transform::Transform(unsigned int nodeId, glm::mat4 M) : node_id(nodeId), M(M) {}

Transform::Transform(unsigned int nodeId, glm::mat4 translation, glm::mat4 rotation, glm::mat4 scale) {
	this->node_id = nodeId;
	this->translation = translation;
	this->rotation = rotation;
	this->scale = scale;
	this->M = translation * rotation * scale;
}

unsigned int Transform::serialize(char * data) {
	unsigned int size = 0;
	char * currLoc = data;
	//copying over nodeid
	memcpy(currLoc, &node_id, sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	size += sizeof(unsigned int);

	// copy over enabled
	memcpy(currLoc, &enabled, sizeof(bool));
	currLoc += sizeof(bool);
	size += sizeof(bool);

	// copy over evading status
	memcpy(currLoc, &isEvading, sizeof(bool));
	currLoc += sizeof(bool);
	size += sizeof(bool);

	// copy over the invincible state
	memcpy(currLoc, &isInvincible, sizeof(bool));
	currLoc += sizeof(bool);
	size += sizeof(bool);

	memcpy(currLoc, &isInvisible, sizeof(bool));
	currLoc += sizeof(bool);
	size += sizeof(bool);

	//copying over the Transfromation Matrix
	memcpy(currLoc, &(M[0][0]), sizeof(glm::mat4));
	currLoc += sizeof(glm::mat4);
	size += sizeof(glm::mat4);

	//copying over the model ids size + models
	unsigned int model_id_size = model_ids.size();
	memcpy(currLoc, &model_id_size, sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	size += sizeof(unsigned int);

	for (auto model_id : model_ids) {
		memcpy(currLoc, &model_id, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
		size += sizeof(unsigned int);
	}

	//copying over the child ids size + models
	unsigned int children_size = children_ids.size();
	memcpy(currLoc, &(children_size), sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	size += sizeof(unsigned int);

	for (auto child_id : children_ids) {
		memcpy(currLoc, &child_id, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
		size += sizeof(unsigned int);
	}
	return size;
}

unsigned int Transform::deserializeAndUpdate(char * data, Shader* particleShader, GLuint particleTexture) {
	unsigned int numModels, numChilds;
	unsigned int size = 0;
	glm::mat4 newMat(1.0f);
	char * currLoc = data;
	// clear all existing model_ids and children
	model_ids.clear();
	children_ids.clear();


	//memCopy of node id + enabled + evading status + transform mat
	memcpy(&node_id, currLoc, sizeof(unsigned int));
	size += sizeof(unsigned int);
	currLoc += sizeof(unsigned int);

	memcpy(&enabled, currLoc, sizeof(bool));
	size += sizeof(bool);
	currLoc += sizeof(bool);

	memcpy(&isEvading, currLoc, sizeof(bool));
	size += sizeof(bool);
	currLoc += sizeof(bool);

	memcpy(&isInvincible, currLoc, sizeof(bool));
	size += sizeof(bool);
	currLoc += sizeof(bool);

	memcpy(&isInvisible, currLoc, sizeof(bool));
	size += sizeof(bool);
	currLoc += sizeof(bool);

	memcpy(&(newMat[0][0]), currLoc, sizeof(glm::mat4));
	size += sizeof(glm::mat4);
	currLoc += sizeof(glm::mat4);

	//memcopy model ids size
	memcpy(&numModels, currLoc, sizeof(unsigned int));
	size += sizeof(unsigned int);
	currLoc += sizeof(unsigned int);

	for (int i = 0; i < numModels; i++) {
		unsigned int modelId;
		memcpy(&modelId, currLoc, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
		size += sizeof(unsigned int);
		model_ids.insert(modelId);
	}

	// memcopy children ids size
	memcpy(&numChilds, currLoc, sizeof(unsigned int));
	size += sizeof(unsigned int);
	currLoc += sizeof(unsigned int);
	for (int i = 0; i < numChilds; i++) {
		unsigned int childId;
		memcpy(&childId, currLoc, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
		size += sizeof(unsigned int);
		children_ids.insert(childId);
	}

	setDestination(newMat);

	if (!particle_effect)
		particle_effect = new Particles( particleTexture, particleShader, { M[3][0], M[3][1], M[3][2] });
	return size;
}


void Transform::addChild(const unsigned int id) {
	children_ids.insert(id);
}

void Transform::removeChild(unsigned int id) {
	children_ids.erase(id);
}

void Transform::draw( std::unordered_map<unsigned int, ModelData> &models, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx, unordered_map<unsigned int, Transform *> &sceneGraphMap) {
	if (!enabled)
		return;

	glm::mat4 childMtx = parentMtx * M;

	for (unsigned int child_id : children_ids) {
		Transform * child = sceneGraphMap[child_id];
		child->draw( models, childMtx, viewProjMtx, sceneGraphMap);
	}

	for (unsigned int model_id : model_ids) {
		if (models[model_id].renderMode == COLOR) {
			models[model_id].shader->use();
			models[model_id].shader->setVec4("color", models[model_id].color);
			models[model_id].model->draw(models[model_id].shader, childMtx, viewProjMtx);
		}
		else if (models[model_id].renderMode == TEXTURE) {
			models[model_id].shader->use();
			models[model_id].shader->setInt("isEvading", isEvading ? 1 : 0);
			models[model_id].shader->setInt("isInvincible", isInvincible ? 1 : 0);
			models[model_id].shader->setInt("isCharging", isCharging ? 1 : 0);
			models[model_id].shader->setInt("isInvisible", isInvisible ? 1 : 0);
			models[model_id].shader->setInt("UseTex", 1);
			if (isInvisible) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
			}
			glBindTexture(GL_TEXTURE_2D, models[model_id].texID);
			models[model_id].model->draw(models[model_id].shader, childMtx, viewProjMtx);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_BLEND);
		}
		//TODO: CHANGE THIS LATER
		if (model_id == 200) {
			particle_effect->draw();
		}
	}

}

bool collisionSphere2Sphere(glm::vec3 myNextPos, float myRadius, glm::vec3 otherPos, float otherRadius) {
	const glm::vec3 checkObjPos = otherPos;
	//printf(" other location calculated: %f \n", checkObjPos.x);
	float distX = myNextPos.x - checkObjPos.x;
	float distY = myNextPos.y - checkObjPos.y;
	float distZ = myNextPos.z - checkObjPos.z;

	float distToObject = sqrt(distX*distX + distY * distY + distZ * distZ);

	float threshhold = myRadius + otherRadius;

	//printf("distToObject is %f, threshold is %f \n", distToObject,threshhold);
	return distToObject < threshhold;
}

float clamp(float value, float min, float max) {
	return std::max(min, std::min(max, value));
}

bool collisionSphere2Box(glm::vec3 myNextPos, float myRadius, glm::vec3 otherPos, glm::vec3 otherSize) {
	glm::vec3 center(myNextPos);
	// Calculate AABB info (center, half-extents)
	glm::vec3 aabb_half_extents(otherSize.x / 2, otherSize.y / 2, otherSize.z/2);
	glm::vec3 aabb_center(
		otherPos.x,
		otherPos.y,
		otherPos.z
	);
	// Get difference vector between both centers
	glm::vec3 difference = center - aabb_center;
	glm::vec3 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// Add clamped value to AABB_center and we get the value of box closest to circle
	glm::vec3 closest = aabb_center + clamped;
	// Retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - center;
	return glm::length(difference) < myRadius;
}


bool Transform::isCollided(glm::vec3 forwardVector, unordered_map<unsigned int,float>& modelRadius, unordered_map<unsigned int, Transform *> &sceneGraphMap, Transform * otherNode, unordered_map<unsigned int, glm::vec3> &modelBoundingBoxes, bool toEnv) {
	bool result = false;
	for (unsigned int child_id : otherNode->children_ids) {
		Transform * child = sceneGraphMap[child_id];
		result |= isCollided(forwardVector, modelRadius,sceneGraphMap, child, modelBoundingBoxes,toEnv);
		if (result)
			return result;
	}
	glm::vec3 nextPos = glm::vec3({ translation[3][0], translation[3][1], translation[3][2] })+forwardVector;
	for (unsigned int model_id : model_ids) {
		float currModelRadius = modelRadius[model_id];
		for (unsigned int other_id : otherNode->model_ids) {
			if (!toEnv) {
				float otherModelRadius = modelRadius[other_id];
				result |= collisionSphere2Sphere(nextPos, currModelRadius,
					{ otherNode->translation[3][0], otherNode->translation[3][1], otherNode->translation[3][2] }, otherModelRadius);
			}
			else {
				glm::vec3 otherSize = glm::vec3(glm::vec4(modelBoundingBoxes[other_id],1.0f)*otherNode->scale);
				if ((int)otherNode->initialRotation % 180 != 0)
					otherSize = { otherSize.z,otherSize.y,otherSize.x };

				result |= collisionSphere2Box(nextPos, currModelRadius,
					{ otherNode->translation[3][0], otherNode->translation[3][1], otherNode->translation[3][2] }, otherSize);
				if(result)
				{
					
					printf("Collision Detected: Collided with model id %d, %f, %f, %f \n", other_id, otherSize.x, otherSize.y, otherSize.z);
					//printf("Player's next Pos will be: %f, %f, %f \n", nextPos.x, nextPos.y, nextPos.z);
					//printf("Model at %f,%f,%f \n", otherNode->translation[3][0], otherNode->translation[3][1], otherNode->translation[3][2]);
				}
			}
			if (result) {
				return result;
			}
		}
	}
	return result;
}


void Transform::update() {
	M = translation * rotation * scale;
}

void Transform::clientUpdate() {
	glm::vec3 currTranslation = { M[3][0], M[3][1], M[3][2] };
	if (glm::length(destination - currTranslation) > 0.2f) {
		speed = glm::distance(destination, currTranslation) *0.89f;
		currTranslation = currTranslation + speed * direction;
	}
	M = glm::translate(glm::mat4(1),currTranslation) * rotation * scale;
	if(particle_effect)
		particle_effect->update(currTranslation);
}

void Transform::setDestination(glm::mat4 & updatedM) {
	glm::vec3 dest = { updatedM[3][0], updatedM[3][1], updatedM[3][2] };
	glm::vec3 currTranslation = { M[3][0], M[3][1], M[3][2] };
	glm::vec3 scaleVec = { glm::length(glm::vec3(updatedM[0])), glm::length(glm::vec3(updatedM[1])), glm::length(glm::vec3(updatedM[2])) };
	rotation = { updatedM[0] / scaleVec[0], updatedM[1] / scaleVec[1], updatedM[2] / scaleVec[2], {0,0,0,1} };
	scale = glm::scale(glm::mat4(1.0f), scaleVec);
	if (glm::length(dest - currTranslation) > 0.2f) {
		destination = dest;
		float dist = glm::distance(destination, currTranslation);
		speed = dist;
		direction = glm::normalize(destination - currTranslation);
	}
	M = glm::translate(glm::mat4(1), currTranslation) * rotation * scale;
}