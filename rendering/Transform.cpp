#include "Transform.h"

Transform::Transform() : Transform(glm::mat4(1)) {}

Transform::Transform(glm::mat4 M) : M(M) {}

Transform::Transform(glm::mat4 translation, glm::mat4 rotation, glm::mat4 scale) {
	this->translation = translation;
	this->rotation = rotation;
	this->scale = scale;
	this->M = translation * rotation * scale;
}

char * Transform::serialize() {
	char buf[1024] = { 0 };
	char * currLoc = buf;
	//copying over nodeid
	memcpy(currLoc, &node_id, sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	//copying over the Transfromation Matrix
	memcpy(currLoc, &(M[0][0]), sizeof(glm::mat4));
	currLoc += sizeof(glm::mat4);
	//copying over the model ids size + models
	unsigned int model_id_size = model_ids.size();
	memcpy(currLoc, &(model_id_size), sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	for (auto model_id : model_ids) {
		memcpy(currLoc, &model_id, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
	}
	//copying over the child ids size + models
	unsigned int children_size = children.size();
	memcpy(currLoc, &(children_size), sizeof(unsigned int));
	currLoc += sizeof(unsigned int);
	for (auto child_id : children) {
		memcpy(currLoc, &child_id, sizeof(unsigned int));
		currLoc += sizeof(unsigned int);
	}
	return buf;
}

void Transform::addChild(const unsigned int id, Transform* child) {
	children.insert({ id, child });
}

void Transform::removeChild(unsigned int id) {
	Transform * child = children[id];
	delete child;
	children.erase(id);
}

void Transform::draw(Shader * shader, const std::vector<ModelData> &models, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx) {
	if (!enabled)
		return;

	glm::mat4 childMtx = parentMtx * M;

	for (auto child : children)
		child.second->draw(shader, models, childMtx, viewProjMtx);

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