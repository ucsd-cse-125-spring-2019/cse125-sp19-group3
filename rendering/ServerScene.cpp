#include "ServerScene.h"

void ServerScene::initialize_objects()
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();

	// Load the shader program. Make sure you have the correct filepath up top
	shader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

	root = new Transform(glm::mat4(1.0f));
	
	player_t = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)), 
		glm::rotate(glm::mat4(1.0f), -90 / 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	player_t->model_ids.insert(PLAYER);
	root->addChild(1, player_t);

	player_m = new Model(std::string("../BaseMesh_Anim.fbx"));

	player = new Player(player_t, player_m);

	cube = new Cube();
	cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 10)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); 
				// * glm::scale(glm::mat4(1.0f), glm::vec3(100, 0.01, 100)) * cube->toWorld;

	models.push_back(ModelData{player_m, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), shader, COLOR, 0});
}

void ServerScene::update()
{
	// Call the update function the cube
	//cube->update();
	time += 1.0 / 60;
	camera->Update();
	player->update(time);
}

void ServerScene::render()
{
	// Render the cube
	//cube->draw(shader, glm::mat4(1.0f), camera->GetViewProjectMtx());

	// Now send these values to the shader program
	root->draw(shader, models, glm::mat4(1.0f), camera->GetViewProjectMtx());
}

void ServerScene::handlePlayerMovement(glm::vec3 destination)
{
	player->setDestination(destination);
	float dotResult = glm::dot(glm::normalize(destination - player->currentPos), player->currentOri);

	if (abs(dotResult) < 1.0) {
		float angle = glm::acos(dotResult);
		printf("rotate angle = %f", angle);
		glm::vec3 axis = glm::cross(player->currentOri, glm::normalize(destination - player->currentPos));
		if (glm::length(axis) != 0) {
			player->rotate(angle, axis);
		}
	}
}

unsigned int ServerScene::serializeSceneGraph(char* data) {
	return serializeSceneGraph(root, data);
}

unsigned int ServerScene::serializeSceneGraph(Transform* t, char* data) {
	memcpy(data, &t->M[0][0], sizeof(glm::mat4));
	data += sizeof(glm::mat4);
	unsigned int size = sizeof(glm::mat4);

	for (auto child : t->children) {
		*data++ = 'T';
		size += sizeof(char);
		memcpy(data, &child.first, sizeof(unsigned int));
		data += sizeof(unsigned int);
		size += sizeof(unsigned int);
		size += serializeSceneGraph(child.second, data);
	}

	for (auto model_id : t->model_ids) {
		*data++ = 'M';
		size += sizeof(char);
		memcpy(data, &model_id, sizeof(unsigned int));
		data += sizeof(unsigned int);
		size += sizeof(unsigned int);
	}

	*data++ = '\0';
	size += sizeof(char);
	return size;
}
