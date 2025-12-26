#define GLM_ENABLE_EXPERIMENTAL
#include "Angel.h"
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// matrix decomposition for extracting TRS
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "TriMesh.h"
#include "MeshPainter.h"
#include "Camera.h"

// 层级建模结构体：亭子部件
struct PavilionPart {
    TriMesh* mesh = nullptr;
    std::vector<std::shared_ptr<PavilionPart>> children;
    glm::vec3 local_translation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 local_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 local_scale = glm::vec3(1.0f, 1.0f, 1.0f);
};

// 亭子全局根节点，供init和display访问
std::shared_ptr<PavilionPart> pavilion_root = nullptr;

// 递归渲染亭子
void drawPavilionPart(const std::shared_ptr<PavilionPart>& part, glm::mat4 parentModel, MeshPainter* painter, Light* light, Camera* camera) {
	if (!part) return;
	glm::mat4 model = parentModel;
	model = glm::translate(model, part->local_translation);
	model = glm::rotate(model, glm::radians(part->local_rotation.x), glm::vec3(1,0,0));
	model = glm::rotate(model, glm::radians(part->local_rotation.y), glm::vec3(0,1,0));
	model = glm::rotate(model, glm::radians(part->local_rotation.z), glm::vec3(0,0,1));
	model = glm::scale(model, part->local_scale);
	if (part->mesh) {
		// 将组合矩阵分解为平移/旋转/缩放，并设置到 TriMesh
		glm::vec3 scale;
		glm::quat orient;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		bool decomposed = glm::decompose(model, scale, orient, translation, skew, perspective);
		if (decomposed) {
			// glm::eulerAngles 返回弧度
			glm::vec3 euler = glm::degrees(glm::eulerAngles(orient));
			part->mesh->setTranslation(translation);
			part->mesh->setRotation(euler);
			part->mesh->setScale(scale);
		}
		// 找到该 mesh 对应的 openGLObject（根据 painter 中的 meshes 列表）
		auto allMeshes = painter->getMeshes();
		auto allObjs = painter->getOpenGLObj();
		int idx = -1;
		for (int i = 0; i < allMeshes.size(); i++) {
			if (allMeshes[i] == part->mesh) { idx = i; break; }
		}
		if (idx >= 0 && idx < allObjs.size()) {
			painter->drawMesh(part->mesh, allObjs[idx], light, camera);
		} else {
			// 作为回退，尝试使用第0个对象（如果存在）
			if (!allObjs.empty()) painter->drawMesh(part->mesh, allObjs[0], light, camera);
		}
	}
	for (auto& child : part->children) {
		drawPavilionPart(child, model, painter, light, camera);
	}
}

int WIDTH = 800;
int HEIGHT = 600;

Camera* camera = new Camera();
Light* light = new Light();
MeshPainter *painter = new MeshPainter();

std::vector<TriMesh *> meshList;

// 御剑飞行相关
TriMesh* sword = nullptr;
TriMesh* sword_guard = nullptr;
TriMesh* sword_handle = nullptr;

// 人物各部件
TriMesh* char_head = nullptr;
TriMesh* char_hair = nullptr;
TriMesh* char_hair_back = nullptr;
TriMesh* char_neck = nullptr;
TriMesh* char_body_upper = nullptr;
TriMesh* char_body_lower = nullptr;
TriMesh* char_belt = nullptr;
TriMesh* char_robe = nullptr;
TriMesh* char_left_arm = nullptr;
TriMesh* char_right_arm = nullptr;
TriMesh* char_left_hand = nullptr;
TriMesh* char_right_hand = nullptr;
// 五官
TriMesh* char_left_eye = nullptr;
TriMesh* char_right_eye = nullptr;
TriMesh* char_mouth = nullptr;
TriMesh* char_left_eyebrow = nullptr;
TriMesh* char_right_eyebrow = nullptr;
// 脚
TriMesh* char_left_foot = nullptr;
TriMesh* char_right_foot = nullptr;

std::vector<TriMesh*> trail_meshes;

// 第二个可控物体 - 仙鹤
TriMesh* crane_body = nullptr;
TriMesh* crane_head = nullptr;
TriMesh* crane_neck = nullptr;
TriMesh* crane_wing_l = nullptr;
TriMesh* crane_wing_r = nullptr;
TriMesh* crane_leg_l = nullptr;
TriMesh* crane_leg_r = nullptr;
float crane_x = 8.0f;
float crane_y = 6.0f;
float crane_z = 5.0f;
float crane_rotation = 45.0f;
bool control_crane = false;  // false=控制人物, true=控制仙鹤

// 位置和控制
float sword_position_x = 0.0f;
float sword_position_y = 5.0f;
float sword_position_z = 0.0f;
float sword_rotation_y = 0.0f;  // 人物/剑的朝向
float fly_speed = 0.25f;
float rotation_speed = 3.5f;
float speed_multiplier = 1.0f;

// 相机控制
float camera_angle_h = 0.0f;    // 相机水平角度偏移（鼠标控制）
float camera_angle_v = 15.0f;   // 相机垂直角度
float camera_distance = 5.0f;

// 鼠标控制
bool mouse_left_pressed = false;
double last_mouse_x = 0.0;
double last_mouse_y = 0.0;

// 动画
float time_counter = 0.0f;
float bob_amplitude = 0.05f;

void init()
{
	std::string vshader = "shaders/vshader.glsl";
	std::string fshader = "shaders/fshader.glsl";

	// 柔和仙境光照
	light->setTranslation(glm::vec3(30.0, 50.0, 30.0));
	light->setAmbient(glm::vec4(0.6, 0.6, 0.65, 1.0));
	light->setDiffuse(glm::vec4(1.0, 0.98, 0.9, 1.0));
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0));
	light->setAttenuation(1.0, 0.001, 0.00005);

	// ========== 地面（带纹理） ==========
	TriMesh* ground = new TriMesh();
	ground->generateGround(300.0);
	ground->setTranslation(glm::vec3(0.0, -3.0, 0.0));
	ground->setAmbient(glm::vec4(0.3, 0.5, 0.3, 1.0));
	ground->setDiffuse(glm::vec4(0.4, 0.65, 0.4, 1.0));
	ground->setSpecular(glm::vec4(0.05, 0.1, 0.05, 1.0));
	ground->setShininess(3.0);
	painter->addMesh(ground, "ground", "assets/test.png", vshader, fshader);  // 添加纹理
	meshList.push_back(ground);
	
	// ========== 亭子层级建模（4层嵌套） ========== 
	pavilion_root = std::make_shared<PavilionPart>();
	// 台基
	auto base = std::make_shared<PavilionPart>();
	base->mesh = new TriMesh();
	base->mesh->generateCube();
	base->mesh->setScale(glm::vec3(2.5, 0.3, 2.5));
	base->mesh->setAmbient(glm::vec4(0.7, 0.7, 0.75, 1.0));
	base->mesh->setDiffuse(glm::vec4(0.9, 0.9, 0.92, 1.0));
	base->mesh->setSpecular(glm::vec4(0.4, 0.4, 0.45, 1.0));
	base->mesh->setShininess(30.0);
	painter->addMesh(base->mesh, "pav_base", "", vshader, fshader);
	// 台阶（台基的子节点）
	auto step = std::make_shared<PavilionPart>();
	step->mesh = new TriMesh();
	step->mesh->generateCube();
	step->mesh->setScale(glm::vec3(2.0, 0.15, 2.0));
	step->local_translation = glm::vec3(0, -0.22, 0);
	step->mesh->setAmbient(glm::vec4(0.8, 0.8, 0.85, 1.0));
	step->mesh->setDiffuse(glm::vec4(0.95, 0.95, 0.98, 1.0));
	step->mesh->setSpecular(glm::vec4(0.5, 0.5, 0.55, 1.0));
	step->mesh->setShininess(25.0);
	painter->addMesh(step->mesh, "pav_step", "", vshader, fshader);
	base->children.push_back(step);
	// 柱子（台基的子节点）
	for (int i = 0; i < 4; i++) {
		auto pillar = std::make_shared<PavilionPart>();
		pillar->mesh = new TriMesh();
		pillar->mesh->generateCylinder(16, 0.12, 1.0);
		float angle = i * 3.14159f / 2.0f + 3.14159f/4.0f;
		float px = 1.0f * cos(angle);
		float pz = 1.0f * sin(angle);
		pillar->local_translation = glm::vec3(px, 0.5, pz);
		pillar->mesh->setAmbient(glm::vec4(0.7, 0.5, 0.3, 1.0));
		pillar->mesh->setDiffuse(glm::vec4(0.85, 0.7, 0.4, 1.0));
		pillar->mesh->setSpecular(glm::vec4(0.3, 0.2, 0.1, 1.0));
		pillar->mesh->setShininess(20.0);
		painter->addMesh(pillar->mesh, "pav_pillar"+std::to_string(i), "", vshader, fshader);
		base->children.push_back(pillar);
	}
	// 屋顶（柱子的子节点）
	auto roof = std::make_shared<PavilionPart>();
	roof->mesh = new TriMesh();
	roof->mesh->generateCone(32, 1.5, 0.8);
	roof->local_translation = glm::vec3(0, 1.1, 0);
	roof->mesh->setAmbient(glm::vec4(0.2, 0.3, 0.4, 1.0));
	roof->mesh->setDiffuse(glm::vec4(0.3, 0.45, 0.6, 1.0));
	roof->mesh->setSpecular(glm::vec4(0.2, 0.2, 0.25, 1.0));
	roof->mesh->setShininess(18.0);
	painter->addMesh(roof->mesh, "pav_roof", "", vshader, fshader);
	// 屋脊装饰（屋顶的子节点）
	auto ornament = std::make_shared<PavilionPart>();
	ornament->mesh = new TriMesh();
	ornament->mesh->generateCube();
	ornament->mesh->setScale(glm::vec3(0.18, 0.18, 0.18));
	ornament->local_translation = glm::vec3(0, 0.7, 0);
	ornament->mesh->setAmbient(glm::vec4(0.8, 0.7, 0.2, 1.0));
	ornament->mesh->setDiffuse(glm::vec4(0.95, 0.85, 0.3, 1.0));
	ornament->mesh->setSpecular(glm::vec4(0.7, 0.7, 0.3, 1.0));
	ornament->mesh->setShininess(40.0);
	painter->addMesh(ornament->mesh, "pav_ornament", "", vshader, fshader);
	roof->children.push_back(ornament);
	base->children.push_back(roof);
	pavilion_root->children.push_back(base);
	pavilion_root->local_translation = glm::vec3(-10.0f, -2.7f, 10.0f); // 场景中位置
	// 保存全局变量，供display递归渲染
	// g_pavilion已不需要，直接用全局pavilion_root
	TriMesh* lake = new TriMesh();
	lake->generateGround(25.0);
	lake->setTranslation(glm::vec3(0.0, -2.8, 30.0));
	lake->setAmbient(glm::vec4(0.15, 0.35, 0.55, 1.0));
	lake->setDiffuse(glm::vec4(0.25, 0.5, 0.75, 1.0));
	lake->setSpecular(glm::vec4(0.7, 0.8, 0.9, 1.0));
	lake->setShininess(90.0);
	painter->addMesh(lake, "lake", "procedural:water", vshader, fshader);
	meshList.push_back(lake);
	
	// ========== 小桥 ==========
	TriMesh* bridge = new TriMesh();
	bridge->generateCube();
	bridge->setTranslation(glm::vec3(0.0, -1.5, 18.0));
	bridge->setScale(glm::vec3(2.0, 0.3, 8.0));
	bridge->setAmbient(glm::vec4(0.5, 0.35, 0.2, 1.0));
	bridge->setDiffuse(glm::vec4(0.7, 0.5, 0.3, 1.0));
	bridge->setSpecular(glm::vec4(0.1, 0.08, 0.05, 1.0));
	bridge->setShininess(5.0);
	painter->addMesh(bridge, "bridge", "assets/table.png", vshader, fshader);  // 添加纹理
	meshList.push_back(bridge);
	
	// 桥栏杆
	for (int i = 0; i < 2; i++) {
		TriMesh* rail = new TriMesh();
		rail->generateCube();
		float side = (i == 0) ? -1.1f : 1.1f;
		rail->setTranslation(glm::vec3(side, -0.8, 18.0));
		rail->setScale(glm::vec3(0.1, 0.5, 7.5));
		rail->setAmbient(glm::vec4(0.55, 0.4, 0.25, 1.0));
		rail->setDiffuse(glm::vec4(0.75, 0.55, 0.35, 1.0));
		rail->setSpecular(glm::vec4(0.1, 0.08, 0.05, 1.0));
		rail->setShininess(5.0);
		painter->addMesh(rail, "rail_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(rail);
	}
	
	// ========== 荷叶（水面装饰） ==========
	for (int i = 0; i < 8; i++) {
		TriMesh* lotus = new TriMesh();
		lotus->generateDisk(16, 1.2 + (i%3)*0.3);
		float lx = (i%4 - 1.5f) * 5.0f + sin(i*1.3)*2.0;
		float lz = 28.0f + (i/4) * 6.0f + cos(i*0.9)*2.0;
		lotus->setTranslation(glm::vec3(lx, -2.7, lz));
		lotus->setRotation(glm::vec3(-90.0, i*30.0, 0.0));
		lotus->setAmbient(glm::vec4(0.15, 0.4, 0.15, 1.0));
		lotus->setDiffuse(glm::vec4(0.25, 0.6, 0.25, 1.0));
		lotus->setSpecular(glm::vec4(0.1, 0.2, 0.1, 1.0));
		lotus->setShininess(8.0);
		painter->addMesh(lotus, "lotus_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(lotus);
	}
	
	// ========== 石头/假山 ==========
	for (int i = 0; i < 6; i++) {
		TriMesh* rock = new TriMesh();
		rock->generateCube();
		float rx = 15.0f + (i%3)*8.0f - 12.0f;
		float rz = -15.0f + (i/3)*10.0f;
		rock->setTranslation(glm::vec3(rx, -2.0 + (i%2)*0.5, rz));
		rock->setScale(glm::vec3(1.5 + (i%3)*0.8, 1.0 + (i%2)*1.5, 1.2 + (i%3)*0.6));
		rock->setRotation(glm::vec3(0.0, i*25.0, 0.0));
		rock->setAmbient(glm::vec4(0.35, 0.33, 0.3, 1.0));
		rock->setDiffuse(glm::vec4(0.5, 0.48, 0.45, 1.0));
		rock->setSpecular(glm::vec4(0.15, 0.15, 0.15, 1.0));
		rock->setShininess(10.0);
		painter->addMesh(rock, "rock_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(rock);
	}
	
	// ========== 仙鹤（第二个可控物体） ==========
	// 身体
	crane_body = new TriMesh();
	crane_body->generateCube();
	crane_body->setScale(glm::vec3(0.4, 0.35, 0.8));
	crane_body->setAmbient(glm::vec4(0.85, 0.85, 0.88, 1.0));
	crane_body->setDiffuse(glm::vec4(0.98, 0.98, 1.0, 1.0));
	crane_body->setSpecular(glm::vec4(0.2, 0.2, 0.22, 1.0));
	crane_body->setShininess(12.0);
	painter->addMesh(crane_body, "crane_body", "", vshader, fshader);
	meshList.push_back(crane_body);
	
	// 脖子
	crane_neck = new TriMesh();
	crane_neck->generateCylinder(12, 0.06, 0.4);
	crane_neck->setAmbient(glm::vec4(0.85, 0.85, 0.88, 1.0));
	crane_neck->setDiffuse(glm::vec4(0.98, 0.98, 1.0, 1.0));
	crane_neck->setSpecular(glm::vec4(0.15, 0.15, 0.18, 1.0));
	crane_neck->setShininess(10.0);
	painter->addMesh(crane_neck, "crane_neck", "", vshader, fshader);
	meshList.push_back(crane_neck);
	
	// 头
	crane_head = new TriMesh();
	crane_head->generateCube();
	crane_head->setScale(glm::vec3(0.12, 0.1, 0.15));
	crane_head->setAmbient(glm::vec4(0.7, 0.15, 0.1, 1.0));  // 红色丹顶
	crane_head->setDiffuse(glm::vec4(0.9, 0.2, 0.15, 1.0));
	crane_head->setSpecular(glm::vec4(0.3, 0.15, 0.1, 1.0));
	crane_head->setShininess(15.0);
	painter->addMesh(crane_head, "crane_head", "", vshader, fshader);
	meshList.push_back(crane_head);
	
	// 左翅膀
	crane_wing_l = new TriMesh();
	crane_wing_l->generateCube();
	crane_wing_l->setScale(glm::vec3(0.8, 0.05, 0.35));
	crane_wing_l->setAmbient(glm::vec4(0.1, 0.1, 0.12, 1.0));  // 黑色翅尖
	crane_wing_l->setDiffuse(glm::vec4(0.2, 0.2, 0.22, 1.0));
	crane_wing_l->setSpecular(glm::vec4(0.15, 0.15, 0.18, 1.0));
	crane_wing_l->setShininess(8.0);
	painter->addMesh(crane_wing_l, "crane_wing_l", "", vshader, fshader);
	meshList.push_back(crane_wing_l);
	
	// 右翅膀
	crane_wing_r = new TriMesh();
	crane_wing_r->generateCube();
	crane_wing_r->setScale(glm::vec3(0.8, 0.05, 0.35));
	crane_wing_r->setAmbient(glm::vec4(0.1, 0.1, 0.12, 1.0));
	crane_wing_r->setDiffuse(glm::vec4(0.2, 0.2, 0.22, 1.0));
	crane_wing_r->setSpecular(glm::vec4(0.15, 0.15, 0.18, 1.0));
	crane_wing_r->setShininess(8.0);
	painter->addMesh(crane_wing_r, "crane_wing_r", "", vshader, fshader);
	meshList.push_back(crane_wing_r);
	
	// 左腿
	crane_leg_l = new TriMesh();
	crane_leg_l->generateCylinder(8, 0.025, 0.3);
	crane_leg_l->setAmbient(glm::vec4(0.12, 0.12, 0.12, 1.0));
	crane_leg_l->setDiffuse(glm::vec4(0.25, 0.25, 0.25, 1.0));
	crane_leg_l->setSpecular(glm::vec4(0.1, 0.1, 0.1, 1.0));
	crane_leg_l->setShininess(5.0);
	painter->addMesh(crane_leg_l, "crane_leg_l", "", vshader, fshader);
	meshList.push_back(crane_leg_l);
	
	// 右腿
	crane_leg_r = new TriMesh();
	crane_leg_r->generateCylinder(8, 0.025, 0.3);
	crane_leg_r->setAmbient(glm::vec4(0.12, 0.12, 0.12, 1.0));
	crane_leg_r->setDiffuse(glm::vec4(0.25, 0.25, 0.25, 1.0));
	crane_leg_r->setSpecular(glm::vec4(0.1, 0.1, 0.1, 1.0));
	crane_leg_r->setShininess(5.0);
	painter->addMesh(crane_leg_r, "crane_leg_r", "", vshader, fshader);
	meshList.push_back(crane_leg_r);

	// ========== 飞剑（人物站在剑上） ==========
	// 剑身（细长金剑，水平放置）
	sword = new TriMesh();
	sword->generateCube();
	sword->setScale(glm::vec3(0.12, 0.025, 2.0));  // 加宽一点让人站稳
	sword->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));
	sword->setDiffuse(glm::vec4(0.95, 0.82, 0.4, 1.0));
	sword->setSpecular(glm::vec4(1.0, 0.95, 0.7, 1.0));
	sword->setShininess(120.0);
	painter->addMesh(sword, "sword", "", vshader, fshader);
	meshList.push_back(sword);
	
	// 剑格
	sword_guard = new TriMesh();
	sword_guard->generateCube();
	sword_guard->setScale(glm::vec3(0.4, 0.05, 0.1));
	sword_guard->setAmbient(glm::vec4(0.6, 0.5, 0.2, 1.0));
	sword_guard->setDiffuse(glm::vec4(0.85, 0.7, 0.3, 1.0));
	sword_guard->setSpecular(glm::vec4(0.9, 0.8, 0.5, 1.0));
	sword_guard->setShininess(80.0);
	painter->addMesh(sword_guard, "sword_guard", "", vshader, fshader);
	meshList.push_back(sword_guard);
	
	// 剑柄
	sword_handle = new TriMesh();
	sword_handle->generateCube();
	sword_handle->setScale(glm::vec3(0.04, 0.03, 0.25));
	sword_handle->setAmbient(glm::vec4(0.35, 0.2, 0.1, 1.0));
	sword_handle->setDiffuse(glm::vec4(0.55, 0.35, 0.18, 1.0));
	sword_handle->setSpecular(glm::vec4(0.2, 0.15, 0.1, 1.0));
	sword_handle->setShininess(15.0);
	painter->addMesh(sword_handle, "sword_handle", "", vshader, fshader);
	meshList.push_back(sword_handle);

	// ========== 精致人物（站在剑上） ==========
	// 头部（圆润，肤色）
	char_head = new TriMesh();
	char_head->generateCube();
	char_head->setScale(glm::vec3(0.13, 0.15, 0.13));
	char_head->setAmbient(glm::vec4(0.75, 0.6, 0.5, 1.0));
	char_head->setDiffuse(glm::vec4(0.98, 0.85, 0.75, 1.0));
	char_head->setSpecular(glm::vec4(0.12, 0.1, 0.08, 1.0));
	char_head->setShininess(8.0);
	painter->addMesh(char_head, "char_head", "", vshader, fshader);
	meshList.push_back(char_head);
	
	// 发髻（黑色）
	char_hair = new TriMesh();
	char_hair->generateCube();
	char_hair->setScale(glm::vec3(0.06, 0.1, 0.06));
	char_hair->setAmbient(glm::vec4(0.08, 0.06, 0.05, 1.0));
	char_hair->setDiffuse(glm::vec4(0.15, 0.12, 0.1, 1.0));
	char_hair->setSpecular(glm::vec4(0.35, 0.32, 0.3, 1.0));
	char_hair->setShininess(30.0);
	painter->addMesh(char_hair, "char_hair", "", vshader, fshader);
	meshList.push_back(char_hair);
	
	// 披肩长发（飘逸）
	char_hair_back = new TriMesh();
	char_hair_back->generateCube();
	char_hair_back->setScale(glm::vec3(0.12, 0.28, 0.04));
	char_hair_back->setAmbient(glm::vec4(0.08, 0.06, 0.05, 1.0));
	char_hair_back->setDiffuse(glm::vec4(0.15, 0.12, 0.1, 1.0));
	char_hair_back->setSpecular(glm::vec4(0.3, 0.28, 0.25, 1.0));
	char_hair_back->setShininess(25.0);
	painter->addMesh(char_hair_back, "char_hair_back", "", vshader, fshader);
	meshList.push_back(char_hair_back);
	
	// 脖子
	char_neck = new TriMesh();
	char_neck->generateCube();
	char_neck->setScale(glm::vec3(0.06, 0.06, 0.06));
	char_neck->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));
	char_neck->setDiffuse(glm::vec4(0.95, 0.8, 0.7, 1.0));
	char_neck->setSpecular(glm::vec4(0.1, 0.08, 0.06, 1.0));
	char_neck->setShininess(8.0);
	painter->addMesh(char_neck, "char_neck", "", vshader, fshader);
	meshList.push_back(char_neck);
	
	// 上身（白色道袍）
	char_body_upper = new TriMesh();
	char_body_upper->generateCube();
	char_body_upper->setScale(glm::vec3(0.2, 0.25, 0.11));
	char_body_upper->setAmbient(glm::vec4(0.78, 0.78, 0.82, 1.0));
	char_body_upper->setDiffuse(glm::vec4(0.97, 0.97, 0.99, 1.0));
	char_body_upper->setSpecular(glm::vec4(0.22, 0.22, 0.25, 1.0));
	char_body_upper->setShininess(10.0);
	painter->addMesh(char_body_upper, "char_body_upper", "", vshader, fshader);
	meshList.push_back(char_body_upper);
	
	// 腰带（金色）
	char_belt = new TriMesh();
	char_belt->generateCube();
	char_belt->setScale(glm::vec3(0.21, 0.04, 0.12));
	char_belt->setAmbient(glm::vec4(0.65, 0.55, 0.22, 1.0));
	char_belt->setDiffuse(glm::vec4(0.88, 0.75, 0.35, 1.0));
	char_belt->setSpecular(glm::vec4(0.75, 0.65, 0.4, 1.0));
	char_belt->setShininess(45.0);
	painter->addMesh(char_belt, "char_belt", "", vshader, fshader);
	meshList.push_back(char_belt);
	
	// 下身长袍
	char_body_lower = new TriMesh();
	char_body_lower->generateCube();
	char_body_lower->setScale(glm::vec3(0.22, 0.35, 0.12));
	char_body_lower->setAmbient(glm::vec4(0.75, 0.75, 0.8, 1.0));
	char_body_lower->setDiffuse(glm::vec4(0.95, 0.95, 0.98, 1.0));
	char_body_lower->setSpecular(glm::vec4(0.18, 0.18, 0.2, 1.0));
	char_body_lower->setShininess(8.0);
	painter->addMesh(char_body_lower, "char_body_lower", "", vshader, fshader);
	meshList.push_back(char_body_lower);
	
	// 飘逸长袍下摆
	char_robe = new TriMesh();
	char_robe->generateCube();
	char_robe->setScale(glm::vec3(0.26, 0.18, 0.14));
	char_robe->setAmbient(glm::vec4(0.7, 0.7, 0.74, 1.0));
	char_robe->setDiffuse(glm::vec4(0.92, 0.92, 0.95, 1.0));
	char_robe->setSpecular(glm::vec4(0.18, 0.18, 0.2, 1.0));
	char_robe->setShininess(8.0);
	painter->addMesh(char_robe, "char_robe", "", vshader, fshader);
	meshList.push_back(char_robe);
	
	// 左臂袖子（自然下垂）
	char_left_arm = new TriMesh();
	char_left_arm->generateCube();
	char_left_arm->setScale(glm::vec3(0.06, 0.18, 0.06));
	char_left_arm->setAmbient(glm::vec4(0.58, 0.68, 0.78, 1.0));
	char_left_arm->setDiffuse(glm::vec4(0.8, 0.9, 0.98, 1.0));
	char_left_arm->setSpecular(glm::vec4(0.18, 0.2, 0.22, 1.0));
	char_left_arm->setShininess(8.0);
	painter->addMesh(char_left_arm, "char_left_arm", "", vshader, fshader);
	meshList.push_back(char_left_arm);
	
	// 右臂袖子（自然下垂）
	char_right_arm = new TriMesh();
	char_right_arm->generateCube();
	char_right_arm->setScale(glm::vec3(0.06, 0.18, 0.06));
	char_right_arm->setAmbient(glm::vec4(0.55, 0.65, 0.75, 1.0));
	char_right_arm->setDiffuse(glm::vec4(0.78, 0.88, 0.96, 1.0));
	char_right_arm->setSpecular(glm::vec4(0.2, 0.22, 0.25, 1.0));
	char_right_arm->setShininess(10.0);
	painter->addMesh(char_right_arm, "char_right_arm", "", vshader, fshader);
	meshList.push_back(char_right_arm);
	
	// 左手
	char_left_hand = new TriMesh();
	char_left_hand->generateCube();
	char_left_hand->setScale(glm::vec3(0.05, 0.06, 0.05));
	char_left_hand->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));
	char_left_hand->setDiffuse(glm::vec4(0.95, 0.8, 0.7, 1.0));
	char_left_hand->setSpecular(glm::vec4(0.1, 0.08, 0.06, 1.0));
	char_left_hand->setShininess(8.0);
	painter->addMesh(char_left_hand, "char_left_hand", "", vshader, fshader);
	meshList.push_back(char_left_hand);
	
	// 右手
	char_right_hand = new TriMesh();
	char_right_hand->generateCube();
	char_right_hand->setScale(glm::vec3(0.05, 0.06, 0.05));
	char_right_hand->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));
	char_right_hand->setDiffuse(glm::vec4(0.95, 0.8, 0.7, 1.0));
	char_right_hand->setSpecular(glm::vec4(0.1, 0.08, 0.06, 1.0));
	char_right_hand->setShininess(8.0);
	painter->addMesh(char_right_hand, "char_right_hand", "", vshader, fshader);
	meshList.push_back(char_right_hand);
	
	// ========== 五官 ==========
	// 左眼（黑色）
	char_left_eye = new TriMesh();
	char_left_eye->generateCube();
	char_left_eye->setScale(glm::vec3(0.025, 0.025, 0.015));
	char_left_eye->setAmbient(glm::vec4(0.05, 0.03, 0.02, 1.0));
	char_left_eye->setDiffuse(glm::vec4(0.1, 0.08, 0.05, 1.0));
	char_left_eye->setSpecular(glm::vec4(0.5, 0.5, 0.5, 1.0));
	char_left_eye->setShininess(60.0);
	painter->addMesh(char_left_eye, "char_left_eye", "", vshader, fshader);
	meshList.push_back(char_left_eye);
	
	// 右眼
	char_right_eye = new TriMesh();
	char_right_eye->generateCube();
	char_right_eye->setScale(glm::vec3(0.025, 0.025, 0.015));
	char_right_eye->setAmbient(glm::vec4(0.05, 0.03, 0.02, 1.0));
	char_right_eye->setDiffuse(glm::vec4(0.1, 0.08, 0.05, 1.0));
	char_right_eye->setSpecular(glm::vec4(0.5, 0.5, 0.5, 1.0));
	char_right_eye->setShininess(60.0);
	painter->addMesh(char_right_eye, "char_right_eye", "", vshader, fshader);
	meshList.push_back(char_right_eye);
	
	// 左眉毛
	char_left_eyebrow = new TriMesh();
	char_left_eyebrow->generateCube();
	char_left_eyebrow->setScale(glm::vec3(0.035, 0.01, 0.01));
	char_left_eyebrow->setAmbient(glm::vec4(0.08, 0.05, 0.03, 1.0));
	char_left_eyebrow->setDiffuse(glm::vec4(0.15, 0.1, 0.08, 1.0));
	char_left_eyebrow->setSpecular(glm::vec4(0.1, 0.1, 0.1, 1.0));
	char_left_eyebrow->setShininess(5.0);
	painter->addMesh(char_left_eyebrow, "char_left_eyebrow", "", vshader, fshader);
	meshList.push_back(char_left_eyebrow);
	
	// 右眉毛
	char_right_eyebrow = new TriMesh();
	char_right_eyebrow->generateCube();
	char_right_eyebrow->setScale(glm::vec3(0.035, 0.01, 0.01));
	char_right_eyebrow->setAmbient(glm::vec4(0.08, 0.05, 0.03, 1.0));
	char_right_eyebrow->setDiffuse(glm::vec4(0.15, 0.1, 0.08, 1.0));
	char_right_eyebrow->setSpecular(glm::vec4(0.1, 0.1, 0.1, 1.0));
	char_right_eyebrow->setShininess(5.0);
	painter->addMesh(char_right_eyebrow, "char_right_eyebrow", "", vshader, fshader);
	meshList.push_back(char_right_eyebrow);
	
	// 嘴巴（淡红色）
	char_mouth = new TriMesh();
	char_mouth->generateCube();
	char_mouth->setScale(glm::vec3(0.04, 0.015, 0.012));
	char_mouth->setAmbient(glm::vec4(0.55, 0.25, 0.25, 1.0));
	char_mouth->setDiffuse(glm::vec4(0.8, 0.45, 0.4, 1.0));
	char_mouth->setSpecular(glm::vec4(0.2, 0.15, 0.15, 1.0));
	char_mouth->setShininess(10.0);
	painter->addMesh(char_mouth, "char_mouth", "", vshader, fshader);
	meshList.push_back(char_mouth);
	
	// ========== 脚 ==========
	// 左脚
	char_left_foot = new TriMesh();
	char_left_foot->generateCube();
	char_left_foot->setScale(glm::vec3(0.05, 0.03, 0.1));
	char_left_foot->setAmbient(glm::vec4(0.15, 0.12, 0.1, 1.0));
	char_left_foot->setDiffuse(glm::vec4(0.25, 0.2, 0.18, 1.0));
	char_left_foot->setSpecular(glm::vec4(0.1, 0.08, 0.06, 1.0));
	char_left_foot->setShininess(8.0);
	painter->addMesh(char_left_foot, "char_left_foot", "", vshader, fshader);
	meshList.push_back(char_left_foot);
	
	// 右脚
	char_right_foot = new TriMesh();
	char_right_foot->generateCube();
	char_right_foot->setScale(glm::vec3(0.05, 0.03, 0.1));
	char_right_foot->setAmbient(glm::vec4(0.15, 0.12, 0.1, 1.0));
	char_right_foot->setDiffuse(glm::vec4(0.25, 0.2, 0.18, 1.0));
	char_right_foot->setSpecular(glm::vec4(0.1, 0.08, 0.06, 1.0));
	char_right_foot->setShininess(8.0);
	painter->addMesh(char_right_foot, "char_right_foot", "", vshader, fshader);
	meshList.push_back(char_right_foot);

	// ========== 剑气拖尾 ==========
	for (int i = 0; i < 5; i++) {
		TriMesh* trail = new TriMesh();
		trail->generateCube();
		float s = 1.0f - i * 0.18f;
		trail->setScale(glm::vec3(0.06*s, 0.015*s, 0.25*s));
		trail->setAmbient(glm::vec4(0.85*s, 0.75*s, 0.35*s, 1.0));
		trail->setDiffuse(glm::vec4(1.0*s, 0.9*s, 0.5*s, 1.0));
		trail->setSpecular(glm::vec4(0.5, 0.45, 0.25, 1.0));
		trail->setShininess(30.0);
		painter->addMesh(trail, "trail_" + std::to_string(i), "", vshader, fshader);
		trail_meshes.push_back(trail);
		meshList.push_back(trail);
	}

	// ========== 远景仙山 ==========
	for (int i = 0; i < 12; i++) {
		TriMesh* mountain = new TriMesh();
		mountain->generateCone(24, 8.0 + (i%4)*5.0, 30.0 + (i%5)*15.0);
		float angle = i * 2.0 * M_PI / 12.0;
		float radius = 90.0 + (i%3) * 30.0;
		mountain->setTranslation(glm::vec3(radius * cos(angle), -3.0, radius * sin(angle)));
		float g = 0.3f + (i%4)*0.06f;
		mountain->setAmbient(glm::vec4(0.1, 0.18+g*0.1, 0.12, 1.0));
		mountain->setDiffuse(glm::vec4(0.18, 0.32+g*0.08, 0.24, 1.0));
		mountain->setSpecular(glm::vec4(0.04, 0.08, 0.05, 1.0));
		mountain->setShininess(4.0);
		painter->addMesh(mountain, "mountain_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(mountain);
	}
	
	// ========== 祥云 ==========
	for (int i = 0; i < 25; i++) {
		TriMesh* cloud = new TriMesh();
		cloud->generateCube();
		float angle = i * 2.0 * M_PI / 25.0 + (i%3)*0.2;
		float radius = 40.0 + (i%6)*15.0;
		float height = 12.0 + (i%7)*5.0;
		cloud->setTranslation(glm::vec3(radius * cos(angle), height, radius * sin(angle)));
		cloud->setScale(glm::vec3(4.0+(i%4)*1.5, 0.7+(i%3)*0.3, 2.5+(i%3)*1.0));
		cloud->setAmbient(glm::vec4(0.88, 0.9, 0.95, 1.0));
		cloud->setDiffuse(glm::vec4(0.97, 0.98, 1.0, 1.0));
		cloud->setSpecular(glm::vec4(0.1, 0.1, 0.12, 1.0));
		cloud->setShininess(3.0);
		painter->addMesh(cloud, "cloud_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(cloud);
	}
	
	// ========== 仙宫建筑 ==========
	for (int i = 0; i < 4; i++) {
		float angle = i * M_PI / 2.0 + M_PI/4.0;
		float radius = 55.0;
		float bx = radius * cos(angle);
		float bz = radius * sin(angle);
		
		// 白玉台基
		TriMesh* platform = new TriMesh();
		platform->generateCube();
		platform->setTranslation(glm::vec3(bx, -1.0, bz));
		platform->setScale(glm::vec3(8.0, 2.0, 8.0));
		platform->setAmbient(glm::vec4(0.75, 0.75, 0.78, 1.0));
		platform->setDiffuse(glm::vec4(0.92, 0.92, 0.94, 1.0));
		platform->setSpecular(glm::vec4(0.5, 0.5, 0.55, 1.0));
		platform->setShininess(40.0);
		painter->addMesh(platform, "platform_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(platform);
		
		// 朱红殿身
		TriMesh* hall = new TriMesh();
		hall->generateCube();
		hall->setTranslation(glm::vec3(bx, 3.5, bz));
		hall->setScale(glm::vec3(6.0, 5.0, 6.0));
		hall->setAmbient(glm::vec4(0.5, 0.2, 0.15, 1.0));
		hall->setDiffuse(glm::vec4(0.75, 0.32, 0.22, 1.0));
		hall->setSpecular(glm::vec4(0.12, 0.08, 0.06, 1.0));
		hall->setShininess(8.0);
		painter->addMesh(hall, "hall_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(hall);
		
		// 青瓦飞檐
		TriMesh* roof = new TriMesh();
		roof->generateCone(4, 7.5, 3.5);
		roof->setTranslation(glm::vec3(bx, 8.5, bz));
		roof->setRotation(glm::vec3(0.0, 45.0, 0.0));
		roof->setAmbient(glm::vec4(0.1, 0.2, 0.25, 1.0));
		roof->setDiffuse(glm::vec4(0.15, 0.32, 0.4, 1.0));
		roof->setSpecular(glm::vec4(0.25, 0.3, 0.35, 1.0));
		roof->setShininess(20.0);
		painter->addMesh(roof, "roof_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(roof);
		
		// 金顶
		TriMesh* tip = new TriMesh();
		tip->generateCone(8, 0.3, 1.2);
		tip->setTranslation(glm::vec3(bx, 12.2, bz));
		tip->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));
		tip->setDiffuse(glm::vec4(0.95, 0.8, 0.4, 1.0));
		tip->setSpecular(glm::vec4(0.8, 0.7, 0.4, 1.0));
		tip->setShininess(70.0);
		painter->addMesh(tip, "tip_" + std::to_string(i), "", vshader, fshader);
		meshList.push_back(tip);
	}

	// 天空色
	glClearColor(0.6, 0.8, 0.95, 1.0);
	
	// 初始相机
	camera->eye = glm::vec4(0.0, 7.0, -5.0, 1.0);
	camera->at = glm::vec4(0.0, 5.0, 0.0, 1.0);
	camera->up = glm::vec4(0.0, 1.0, 0.0, 0.0);
}

// 更新御剑飞行
void updateSwordFlying()
{
	time_counter += 0.03f;
	
	float bob = sin(time_counter) * bob_amplitude;
	float sway = sin(time_counter * 0.5) * 1.5;
	float angle_rad = sword_rotation_y * M_PI / 180.0;
	
	// 剑的Y位置（人物站在上面，所以剑在脚下）
	float sword_y = sword_position_y + bob;
	
	// 飞剑（水平放置在脚下）
	if (sword != nullptr) {
		sword->setTranslation(glm::vec3(sword_position_x, sword_y, sword_position_z));
		sword->setRotation(glm::vec3(sway, sword_rotation_y, 0.0));
	}
	
	// 剑格（在剑身后部）
	if (sword_guard != nullptr) {
		float gz = -0.85f;
		float gx_off = gz * sin(angle_rad);
		float gz_off = gz * cos(angle_rad);
		sword_guard->setTranslation(glm::vec3(sword_position_x + gx_off, sword_y, sword_position_z + gz_off));
		sword_guard->setRotation(glm::vec3(sway, sword_rotation_y, 0.0));
	}
	
	// 剑柄（在剑格后面）
	if (sword_handle != nullptr) {
		float hz = -1.1f;
		float hx_off = hz * sin(angle_rad);
		float hz_off = hz * cos(angle_rad);
		sword_handle->setTranslation(glm::vec3(sword_position_x + hx_off, sword_y, sword_position_z + hz_off));
		sword_handle->setRotation(glm::vec3(sway, sword_rotation_y, 0.0));
	}
	
	// 人物基准位置（站在剑上）
	float base_y = sword_y + 0.08;  // 脚底高度
	float sway_x = sin(time_counter * 0.7) * 0.005;
	
	// 头
	if (char_head != nullptr) {
		char_head->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.95, sword_position_z));
		char_head->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 发髻
	if (char_hair != nullptr) {
		char_hair->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 1.12, sword_position_z));
		char_hair->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 长发（在头部后方）
	if (char_hair_back != nullptr) {
		float hb_z = -0.08f;
		float hb_x = hb_z * sin(angle_rad);
		float hb_zz = hb_z * cos(angle_rad);
		char_hair_back->setTranslation(glm::vec3(sword_position_x + hb_x + sway_x, base_y + 0.78, sword_position_z + hb_zz));
		char_hair_back->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 脖子
	if (char_neck != nullptr) {
		char_neck->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.82, sword_position_z));
		char_neck->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 上身
	if (char_body_upper != nullptr) {
		char_body_upper->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.62, sword_position_z));
		char_body_upper->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 腰带
	if (char_belt != nullptr) {
		char_belt->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.47, sword_position_z));
		char_belt->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 下身
	if (char_body_lower != nullptr) {
		char_body_lower->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.25, sword_position_z));
		char_body_lower->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 袍子下摆
	if (char_robe != nullptr) {
		char_robe->setTranslation(glm::vec3(sword_position_x + sway_x, base_y + 0.05, sword_position_z));
		char_robe->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 袖子自然下垂动画
	float arm_swing = sin(time_counter * 0.8) * 0.01;
	
	// 左臂（自然下垂在身侧）
	if (char_left_arm != nullptr) {
		float lx = -0.14f;
		float lx_world = lx * cos(angle_rad);
		float lz_world = -lx * sin(angle_rad);
		char_left_arm->setTranslation(glm::vec3(sword_position_x + lx_world + arm_swing, base_y + 0.48, sword_position_z + lz_world));
		char_left_arm->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 右臂（自然下垂）
	if (char_right_arm != nullptr) {
		float rx = 0.14f;
		float rx_world = rx * cos(angle_rad);
		float rz_world = -rx * sin(angle_rad);
		char_right_arm->setTranslation(glm::vec3(sword_position_x + rx_world - arm_swing, base_y + 0.48, sword_position_z + rz_world));
		char_right_arm->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 左手（在手臂末端）
	if (char_left_hand != nullptr) {
		float lhx = -0.14f;
		float lhx_world = lhx * cos(angle_rad);
		float lhz_world = -lhx * sin(angle_rad);
		char_left_hand->setTranslation(glm::vec3(sword_position_x + lhx_world + arm_swing, base_y + 0.32, sword_position_z + lhz_world));
		char_left_hand->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 右手
	if (char_right_hand != nullptr) {
		float rhx = 0.14f;
		float rhx_world = rhx * cos(angle_rad);
		float rhz_world = -rhx * sin(angle_rad);
		char_right_hand->setTranslation(glm::vec3(sword_position_x + rhx_world - arm_swing, base_y + 0.32, sword_position_z + rhz_world));
		char_right_hand->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// ========== 五官位置 ==========
	float face_z = 0.07f;  // 面朝前方的偏移
	float face_x_off = face_z * sin(angle_rad);
	float face_z_off = face_z * cos(angle_rad);
	
	// 左眼
	if (char_left_eye != nullptr) {
		float eye_x = -0.035f;
		float ex_world = eye_x * cos(angle_rad) + face_x_off;
		float ez_world = -eye_x * sin(angle_rad) + face_z_off;
		char_left_eye->setTranslation(glm::vec3(sword_position_x + ex_world + sway_x, base_y + 0.97, sword_position_z + ez_world));
		char_left_eye->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 右眼
	if (char_right_eye != nullptr) {
		float eye_x = 0.035f;
		float ex_world = eye_x * cos(angle_rad) + face_x_off;
		float ez_world = -eye_x * sin(angle_rad) + face_z_off;
		char_right_eye->setTranslation(glm::vec3(sword_position_x + ex_world + sway_x, base_y + 0.97, sword_position_z + ez_world));
		char_right_eye->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 左眉毛
	if (char_left_eyebrow != nullptr) {
		float brow_x = -0.035f;
		float bx_world = brow_x * cos(angle_rad) + face_x_off;
		float bz_world = -brow_x * sin(angle_rad) + face_z_off;
		char_left_eyebrow->setTranslation(glm::vec3(sword_position_x + bx_world + sway_x, base_y + 1.01, sword_position_z + bz_world));
		char_left_eyebrow->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 右眉毛
	if (char_right_eyebrow != nullptr) {
		float brow_x = 0.035f;
		float bx_world = brow_x * cos(angle_rad) + face_x_off;
		float bz_world = -brow_x * sin(angle_rad) + face_z_off;
		char_right_eyebrow->setTranslation(glm::vec3(sword_position_x + bx_world + sway_x, base_y + 1.01, sword_position_z + bz_world));
		char_right_eyebrow->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 嘴巴
	if (char_mouth != nullptr) {
		char_mouth->setTranslation(glm::vec3(sword_position_x + face_x_off + sway_x, base_y + 0.89, sword_position_z + face_z_off));
		char_mouth->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// ========== 脚 ==========
	// 左脚
	if (char_left_foot != nullptr) {
		float foot_x = -0.06f;
		float fx_world = foot_x * cos(angle_rad);
		float fz_world = -foot_x * sin(angle_rad);
		char_left_foot->setTranslation(glm::vec3(sword_position_x + fx_world + sway_x, base_y + 0.02, sword_position_z + fz_world));
		char_left_foot->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 右脚
	if (char_right_foot != nullptr) {
		float foot_x = 0.06f;
		float fx_world = foot_x * cos(angle_rad);
		float fz_world = -foot_x * sin(angle_rad);
		char_right_foot->setTranslation(glm::vec3(sword_position_x + fx_world + sway_x, base_y + 0.02, sword_position_z + fz_world));
		char_right_foot->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 拖尾（在剑后面）
	for (int i = 0; i < trail_meshes.size(); i++) {
		float trail_off = (i + 1) * 0.4f;
		float tx = sword_position_x - trail_off * sin(angle_rad);
		float tz = sword_position_z - trail_off * cos(angle_rad);
		trail_meshes[i]->setTranslation(glm::vec3(tx, sword_y * (1.0 - i*0.02), tz));
		trail_meshes[i]->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
	}
	
	// 相机跟随（根据控制对象切换跟随目标）
	float target_x, target_y, target_z, target_rotation;
	if (control_crane) {
		// 跟随仙鹤
		target_x = crane_x;
		target_y = crane_y;
		target_z = crane_z;
		target_rotation = crane_rotation;
	} else {
		// 跟随御剑人物
		target_x = sword_position_x;
		target_y = sword_position_y;
		target_z = sword_position_z;
		target_rotation = sword_rotation_y;
	}
	
	float total_cam_angle = target_rotation + camera_angle_h;
	float cam_angle_rad = total_cam_angle * M_PI / 180.0;
	float cam_v_rad = camera_angle_v * M_PI / 180.0;
	
	float cam_x = target_x - camera_distance * sin(cam_angle_rad) * cos(cam_v_rad);
	float cam_z = target_z - camera_distance * cos(cam_angle_rad) * cos(cam_v_rad);
	float cam_y = target_y + camera_distance * sin(cam_v_rad);
	
	camera->eye = glm::vec4(cam_x, cam_y + 0.5, cam_z, 1.0);
	camera->at = glm::vec4(target_x, target_y + 0.6, target_z, 1.0);
	camera->up = glm::vec4(0.0, 1.0, 0.0, 0.0);
	
	// ========== 仙鹤动画（第二个可控物体） ==========
	float crane_bob = sin(time_counter * 1.2) * 0.08;
	float wing_flap = sin(time_counter * 3.0) * 20.0;  // 翅膀扇动
	float crane_angle_rad = crane_rotation * M_PI / 180.0;
	
	// 身体
	if (crane_body != nullptr) {
		crane_body->setTranslation(glm::vec3(crane_x, crane_y + crane_bob, crane_z));
		crane_body->setRotation(glm::vec3(0.0, crane_rotation, 0.0));
	}
	
	// 脖子（身体前方上方）
	if (crane_neck != nullptr) {
		float nz = 0.45f;
		float nx_off = nz * sin(crane_angle_rad);
		float nz_off = nz * cos(crane_angle_rad);
		crane_neck->setTranslation(glm::vec3(crane_x + nx_off, crane_y + 0.35 + crane_bob, crane_z + nz_off));
		crane_neck->setRotation(glm::vec3(30.0, crane_rotation, 0.0));  // 脖子前倾
	}
	
	// 头（脖子顶端）
	if (crane_head != nullptr) {
		float hz = 0.7f;
		float hx_off = hz * sin(crane_angle_rad);
		float hz_off = hz * cos(crane_angle_rad);
		crane_head->setTranslation(glm::vec3(crane_x + hx_off, crane_y + 0.55 + crane_bob, crane_z + hz_off));
		crane_head->setRotation(glm::vec3(0.0, crane_rotation, 0.0));
	}
	
	// 左翅膀（扇动）
	if (crane_wing_l != nullptr) {
		crane_wing_l->setTranslation(glm::vec3(crane_x, crane_y + 0.15 + crane_bob, crane_z));
		crane_wing_l->setRotation(glm::vec3(wing_flap, crane_rotation, 0.0));
	}
	
	// 右翅膀（扇动）
	if (crane_wing_r != nullptr) {
		crane_wing_r->setTranslation(glm::vec3(crane_x, crane_y + 0.15 + crane_bob, crane_z));
		crane_wing_r->setRotation(glm::vec3(-wing_flap, crane_rotation, 0.0));
	}
	
	// 左腿
	if (crane_leg_l != nullptr) {
		float lx = -0.1f;
		float lx_off = lx * cos(crane_angle_rad);
		float lz_off = -lx * sin(crane_angle_rad);
		crane_leg_l->setTranslation(glm::vec3(crane_x + lx_off, crane_y - 0.35 + crane_bob, crane_z + lz_off));
		crane_leg_l->setRotation(glm::vec3(0.0, crane_rotation, 0.0));
	}
	
	// 右腿
	if (crane_leg_r != nullptr) {
		float rx = 0.1f;
		float rx_off = rx * cos(crane_angle_rad);
		float rz_off = -rx * sin(crane_angle_rad);
		crane_leg_r->setTranslation(glm::vec3(crane_x + rx_off, crane_y - 0.35 + crane_bob, crane_z + rz_off));
		crane_leg_r->setRotation(glm::vec3(0.0, crane_rotation, 0.0));
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	updateSwordFlying();
	
	// 获取阴影投影矩阵
	glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
	
	// 绘制阴影（先绘制阴影，再绘制正常物体）
	std::vector<openGLObject> opengl_objects = painter->getOpenGLObj();
	std::vector<TriMesh*> meshes = painter->getMeshes();
	
	// 第一遍：绘制阴影
	for (int i = 0; i < meshes.size(); i++) {
		// 跳过地面、云、湖泊的阴影（它们不需要投影阴影）
		std::string name = painter->getMeshNames()[i];
		if (name == "ground" || name == "lake" || name.find("cloud") != std::string::npos ||
			name.find("lotus") != std::string::npos) continue;
			
		glBindVertexArray(opengl_objects[i].vao);
		glUseProgram(opengl_objects[i].program);
		
		// 计算阴影模型矩阵
		glm::mat4 modelMatrix = meshes[i]->getModelMatrix();
		glm::mat4 shadowModelMatrix = shadowMatrix * modelMatrix;
		
		// 相机矩阵
		camera->viewMatrix = camera->getViewMatrix();
		camera->projMatrix = camera->getProjectionMatrix(false);
		
		glUniformMatrix4fv(opengl_objects[i].modelLocation, 1, GL_FALSE, &shadowModelMatrix[0][0]);
		glUniformMatrix4fv(opengl_objects[i].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
		glUniformMatrix4fv(opengl_objects[i].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
		
		// 设置为阴影模式
		glUniform1i(opengl_objects[i].shadowLocation, 1);
		
		glDrawArrays(GL_TRIANGLES, 0, meshes[i]->getPoints().size());
	}
	
	// 第二遍：正常绘制所有物体
	painter->drawMeshes(light, camera);

	// 递归渲染亭子层级结构
	if (pavilion_root) {
		drawPavilionPart(pavilion_root, glm::mat4(1.0f), painter, light, camera);
	}
}

void printHelp()
{
	std::cout << "================================================" << std::endl;
	std::cout << "      Sword Flying Simulation" << std::endl;
	std::cout << "================================================" << std::endl;
	std::cout << "W/S:       Forward/Backward" << std::endl;
	std::cout << "A/D:       Turn Left/Right" << std::endl;
	std::cout << "Q/E:       Ascend/Descend" << std::endl;
	std::cout << "SHIFT:     Speed Up" << std::endl;
	std::cout << "CTRL:      Slow Down" << std::endl;
	std::cout << "TAB:       Switch Control (Person/Crane)" << std::endl;
	std::cout << "SPACE:     Reset Position" << std::endl;
	std::cout << "Mouse Drag: Rotate Camera" << std::endl;
	std::cout << "Mouse Scroll: Zoom" << std::endl;
	std::cout << "ESC:       Exit" << std::endl;
	std::cout << "H:         Show Help" << std::endl;
	std::cout << "================================================" << std::endl;
}

// keyboard callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		float actual_speed = fly_speed * speed_multiplier;
		
		// 根据控制对象选择角度
		float* current_rotation = control_crane ? &crane_rotation : &sword_rotation_y;
		float* current_x = control_crane ? &crane_x : &sword_position_x;
		float* current_y = control_crane ? &crane_y : &sword_position_y;
		float* current_z = control_crane ? &crane_z : &sword_position_z;
		float angle_rad = (*current_rotation) * M_PI / 180.0;
		
		switch (key)
		{
		case GLFW_KEY_ESCAPE: 
			exit(EXIT_SUCCESS); 
			break;
		case GLFW_KEY_H: 
			printHelp(); 
			break;
		case GLFW_KEY_TAB:  // 切换控制对象
			if (action == GLFW_PRESS) {
				control_crane = !control_crane;
				if (control_crane) {
					std::cout << "Now controlling: Crane" << std::endl;
				} else {
					std::cout << "Now controlling: Sword Rider" << std::endl;
				}
			}
			break;
		case GLFW_KEY_W:
			*current_x += actual_speed * sin(angle_rad);
			*current_z += actual_speed * cos(angle_rad);
			break;
		case GLFW_KEY_S:
			*current_x -= actual_speed * sin(angle_rad);
			*current_z -= actual_speed * cos(angle_rad);
			break;
		case GLFW_KEY_A:  // A键左转
			*current_rotation += rotation_speed;
			break;
		case GLFW_KEY_D:  // D键右转
			*current_rotation -= rotation_speed;
			break;
		case GLFW_KEY_Q:
			*current_y += actual_speed * 0.6f;
			if (*current_y > 25.0f) *current_y = 25.0f;
			break;
		case GLFW_KEY_E:
			*current_y -= actual_speed * 0.6f;
			if (*current_y < 1.0f) *current_y = 1.0f;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			speed_multiplier = 2.5f;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			speed_multiplier = 0.4f;
			break;
		case GLFW_KEY_SPACE:
			sword_position_x = 0.0f;
			sword_position_y = 5.0f;
			sword_position_z = 0.0f;
			sword_rotation_y = 0.0f;
			crane_x = 8.0f;
			crane_y = 6.0f;
			crane_z = 5.0f;
			crane_rotation = 45.0f;
			camera_angle_h = 0.0f;
			camera_angle_v = 15.0f;
			speed_multiplier = 1.0f;
			std::cout << "Position Reset!" << std::endl;
			break;
		}
	}
	
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_LEFT_CONTROL) {
			speed_multiplier = 1.0f;
		}
	}
}

// 鼠标按键回调
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mouse_left_pressed = true;
			glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
		} else if (action == GLFW_RELEASE) {
			mouse_left_pressed = false;
		}
	}
}

// 鼠标移动回调
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouse_left_pressed) {
		double dx = xpos - last_mouse_x;
		double dy = ypos - last_mouse_y;
		
		camera_angle_h -= dx * 0.3f;  // 水平旋转
		camera_angle_v += dy * 0.2f;  // 垂直旋转
		
		// 限制垂直角度
		if (camera_angle_v < -30.0f) camera_angle_v = -30.0f;
		if (camera_angle_v > 60.0f) camera_angle_v = 60.0f;
		
		last_mouse_x = xpos;
		last_mouse_y = ypos;
	}
}

// 滚轮缩放
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera_distance -= yoffset * 0.5f;
	if (camera_distance < 2.0f) camera_distance = 2.0f;
	if (camera_distance > 15.0f) camera_distance = 15.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void cleanData() {
	delete camera;
	camera = NULL;
	delete light;
	light = NULL;
	painter->cleanMeshes();
	delete painter;
	painter = NULL;
	for (int i = 0; i < meshList.size(); i++) {
		delete meshList[i];
	}
	meshList.clear();
}

int main(int argc, char **argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(800, 600, u8"2023155010-郑佳鑫-期末大作业-御剑飞行", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	init();
	printHelp();
	glEnable(GL_DEPTH_TEST);
	
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();
	glfwTerminate();
	return 0;
}
