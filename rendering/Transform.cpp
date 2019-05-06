#include "Transform.h"

Transform::Transform(char * data) {
	deserializeAndUpdate(data);
}

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

unsigned int Transform::deserializeAndUpdate(char * data) {
	unsigned int numModels, numChilds;
	unsigned int size = 0;
	char * currLoc = data;
	// clear all existing model_ids and children
	model_ids.clear();
	children_ids.clear();


	//memCopy of node id + transform mat
	memcpy(&node_id, currLoc, sizeof(unsigned int));
	size += sizeof(unsigned int);
	currLoc += sizeof(unsigned int);
	memcpy(&(M[0][0]), currLoc, sizeof(glm::mat4));
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
	return size;
}


void Transform::addChild(const unsigned int id) {
	children_ids.insert(id);
}

void Transform::removeChild(unsigned int id) {
	children_ids.erase(id);
}

void Transform::draw(Shader * shader, const std::vector<ModelData> &models, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx, unordered_map<unsigned int, Transform *> &sceneGraphMap) {
	if (!enabled)
		return;

	glm::mat4 childMtx = parentMtx * M;

	for (unsigned int child_id : children_ids) {
		Transform * child = sceneGraphMap[child_id];
		child->draw(shader, models, childMtx, viewProjMtx, sceneGraphMap);
	}

	for (auto model_id : model_ids) {
		if (models[model_id].renderMode == COLOR) {
			models[model_id].shader->use();
			models[model_id].shader->setVec4("color", models[model_id].color);
			models[model_id].model->draw(models[model_id].shader, childMtx, viewProjMtx);
		}
		else if (models[model_id].renderMode == TEXTURE) {
			models[model_id].shader->use();
			glBindTexture(GL_TEXTURE_2D, models[model_id].texID);
			models[model_id].model->draw(models[model_id].shader, childMtx, viewProjMtx);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void Transform::update() {
	M = translation * rotation * scale;
}