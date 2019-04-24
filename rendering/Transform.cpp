#include "Transform.h"

Transform::Transform() : Transform(glm::mat4(1)) {}

Transform::Transform(glm::mat4 M) : M(M) {}

Transform::Transform(glm::mat4 translation, glm::mat4 rotation, glm::mat4 scale) {
	this->translation = translation;
	this->rotation = rotation;
	this->scale = scale;
	this->M = translation * rotation * scale;
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