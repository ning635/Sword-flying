#include "TriMesh.h"

// 一些基础颜色
const glm::vec3 basic_colors[8] = {
	glm::vec3(1.0, 1.0, 1.0), // White
	glm::vec3(1.0, 1.0, 0.0), // Yellow
	glm::vec3(0.0, 1.0, 0.0), // Green
	glm::vec3(0.0, 1.0, 1.0), // Cyan
	glm::vec3(1.0, 0.0, 1.0), // Magenta
	glm::vec3(1.0, 0.0, 0.0), // Red
	glm::vec3(0.0, 0.0, 0.0), // Black
	glm::vec3(0.0, 0.0, 1.0)	 // Blue
};


// 立方体的各个点
const glm::vec3 cube_vertices[8] = {
	glm::vec3(-0.5, -0.5, -0.5),
	glm::vec3(0.5, -0.5, -0.5),
	glm::vec3(-0.5, 0.5, -0.5),
	glm::vec3(0.5, 0.5, -0.5),
	glm::vec3(-0.5, -0.5, 0.5),
	glm::vec3(0.5, -0.5, 0.5),
	glm::vec3(-0.5, 0.5, 0.5),
	glm::vec3(0.5, 0.5, 0.5)};

// 三角形的点
const glm::vec3 triangle_vertices[3] = {
	glm::vec3(-0.5, -0.5, 0.0),
	glm::vec3(0.5, -0.5, 0.0),
	glm::vec3(0.0, 0.5, 0.0)};

// 正方形平面
const glm::vec3 square_vertices[4] = {
	glm::vec3(-0.5, -0.5, 0.0),
	glm::vec3(0.5, -0.5, 0.0),
	glm::vec3(0.5, 0.5, 0.0),
	glm::vec3(-0.5, 0.5, 0.0),
};

TriMesh::TriMesh()
{
	scale = glm::vec3(1.0);
	rotation = glm::vec3(0.0);
	translation = glm::vec3(0.0);
}


TriMesh::~TriMesh()
{
}

std::vector<glm::vec3> TriMesh::getVertexPositions()
{
	return vertex_positions;
}

std::vector<glm::vec3> TriMesh::getVertexColors()
{
	return vertex_colors;
}

std::vector<glm::vec3> TriMesh::getVertexNormals()
{
	return vertex_normals;
}

std::vector<glm::vec2> TriMesh::getVertexTextures()
{
	return vertex_textures;
}

std::vector<vec3i> TriMesh::getFaces()
{
	return faces;
}

std::vector<glm::vec3> TriMesh::getPoints()
{
	return points;
}

std::vector<glm::vec3> TriMesh::getColors()
{
	return colors;
}

std::vector<glm::vec3> TriMesh::getNormals()
{
	return normals;
}

std::vector<glm::vec2> TriMesh::getTextures()
{
	return textures;
}

void TriMesh::computeTriangleNormals()
{
	face_normals.resize(faces.size());
	for (size_t i = 0; i < faces.size(); i++)
	{
		auto &face = faces[i];
		glm::vec3 v01 = vertex_positions[face.y] - vertex_positions[face.x];
		glm::vec3 v02 = vertex_positions[face.z] - vertex_positions[face.x];
		face_normals[i] = normalize(cross(v01, v02));
	}
}

void TriMesh::computeVertexNormals()
{
	// 计算面片的法向量
	if (face_normals.size() == 0 && faces.size() > 0)
	{
		computeTriangleNormals();
	}
	// 初始化法向量为0
	vertex_normals.resize(vertex_positions.size(), glm::vec3(0, 0, 0));
	for (size_t i = 0; i < faces.size(); i++)
	{
		auto &face = faces[i];
		vertex_normals[face.x] += face_normals[i];
		vertex_normals[face.y] += face_normals[i];
		vertex_normals[face.z] += face_normals[i];
	}
	for (size_t i = 0; i < vertex_normals.size(); i++)
	{
		vertex_normals[i] = normalize(vertex_normals[i]);
	}
	// 球心在原点的球法向量为坐标
	// for (int i = 0; i < vertex_positions.size(); i++)
	// 	vertex_normals.push_back(vertex_positions[i] - vec3(0.0, 0.0, 0.0));
}

glm::vec3 TriMesh::getTranslation()
{
	return translation;
}

glm::vec3 TriMesh::getRotation()
{
	return rotation;
}

glm::vec3 TriMesh::getScale()
{
	return scale;
}

glm::mat4 TriMesh::getModelMatrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	glm::vec3 trans = getTranslation();
	model = glm::translate(model, getTranslation());
	model = glm::rotate(model, glm::radians(getRotation()[2]), glm::vec3(0.0, 0.0, 1.0));
	model = glm::rotate(model, glm::radians(getRotation()[1]), glm::vec3(0.0, 1.0, 0.0));
	model = glm::rotate(model, glm::radians(getRotation()[0]), glm::vec3(1.0, 0.0, 0.0));
	model = glm::scale(model, getScale());
	return model;
}

void TriMesh::setTranslation(glm::vec3 translation)
{
	this->translation = translation;
}

void TriMesh::setRotation(glm::vec3 rotation)
{
	this->rotation = rotation;
}

void TriMesh::setScale(glm::vec3 scale)
{
	this->scale = scale;
}

glm::vec4 TriMesh::getAmbient() { return ambient; };
glm::vec4 TriMesh::getDiffuse() { return diffuse; };
glm::vec4 TriMesh::getSpecular() { return specular; };
float TriMesh::getShininess() { return shininess; };

void TriMesh::setAmbient(glm::vec4 _ambient) { ambient = _ambient; };
void TriMesh::setDiffuse(glm::vec4 _diffuse) { diffuse = _diffuse; };
void TriMesh::setSpecular(glm::vec4 _specular) { specular = _specular; };
void TriMesh::setShininess(float _shininess) { shininess = _shininess; };

void TriMesh::cleanData()
{
	vertex_positions.clear();
	vertex_colors.clear();
	vertex_normals.clear();
	vertex_textures.clear();

	faces.clear();
	normal_index.clear();
	color_index.clear();
	texture_index.clear();

	face_normals.clear();


	points.clear();
	colors.clear();
	normals.clear();
	textures.clear();
}

void TriMesh::storeFacesPoints()
{
	// 计算法向量
	if (vertex_normals.size() == 0)
		computeVertexNormals();

	// 根据每个三角面片的顶点下标存储要传入GPU的数据
	for (int i = 0; i < faces.size(); i++)
	{
		// 坐标
		points.push_back(vertex_positions[faces[i].x]);
		points.push_back(vertex_positions[faces[i].y]);
		points.push_back(vertex_positions[faces[i].z]);
		// 颜色
 		colors.push_back(vertex_colors[color_index[i].x]);
		colors.push_back(vertex_colors[color_index[i].y]);
		colors.push_back(vertex_colors[color_index[i].z]);
		// 法向量
		if (vertex_normals.size() != 0)
		{
			normals.push_back(vertex_normals[normal_index[i].x]);
			normals.push_back(vertex_normals[normal_index[i].y]);
			normals.push_back(vertex_normals[normal_index[i].z]);
		}
		// 纹理
		if (vertex_textures.size() != 0)
		{
			textures.push_back(vertex_textures[texture_index[i].x]);
			textures.push_back(vertex_textures[texture_index[i].y]);
			textures.push_back(vertex_textures[texture_index[i].z]);
		}
	}
}

// 立方体生成12个三角形的顶点索引
void TriMesh::generateCube()
{
	// 创建顶点前要先把那些vector清空
	cleanData();

	for (int i = 0; i < 8; i++)
	{
		vertex_positions.push_back(cube_vertices[i]);
		vertex_colors.push_back(basic_colors[i]);
	}

	// 每个三角面片的顶点下标
	faces.push_back(vec3i(0, 3, 1));
	faces.push_back(vec3i(0, 2, 3));
	faces.push_back(vec3i(1, 5, 4));
	faces.push_back(vec3i(1, 4, 0));
	faces.push_back(vec3i(4, 2, 0));
	faces.push_back(vec3i(4, 6, 2));
	faces.push_back(vec3i(5, 6, 4));
	faces.push_back(vec3i(5, 7, 6));
	faces.push_back(vec3i(2, 6, 7));
	faces.push_back(vec3i(2, 7, 3));
	faces.push_back(vec3i(1, 7, 5));
	faces.push_back(vec3i(1, 3, 7));
	
	// faces.push_back(vec3i(0, 1, 3));
	// faces.push_back(vec3i(0, 3, 2));

	// faces.push_back(vec3i(1, 4, 5));
	// faces.push_back(vec3i(1, 0, 4));

	// faces.push_back(vec3i(4, 0, 2));
	// faces.push_back(vec3i(4, 2, 6));

	// faces.push_back(vec3i(5, 6, 4));
	// faces.push_back(vec3i(5, 7, 6));

	// faces.push_back(vec3i(2, 6, 7));
	// faces.push_back(vec3i(2, 7, 3));
	
	// faces.push_back(vec3i(1, 7, 5));
	// faces.push_back(vec3i(1, 3, 7));

	// 顶点纹理坐标，这里是每个面都用一个正方形图片的情况
	vertex_textures.push_back(glm::vec2(0, 0));
	vertex_textures.push_back(glm::vec2(1, 0));
	vertex_textures.push_back(glm::vec2(0, 1));
	vertex_textures.push_back(glm::vec2(1, 1));
	vertex_textures.push_back(glm::vec2(0, 0));
	vertex_textures.push_back(glm::vec2(1, 0));
	vertex_textures.push_back(glm::vec2(0, 1));
	vertex_textures.push_back(glm::vec2(1, 1));

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();

	normals.clear();
	// 正方形的法向量不能靠之前顶点法向量的方法直接计算，因为每个四边形平面是正交的，不是连续曲面
	for (int i = 0; i < faces.size(); i++)
	{
		normals.push_back( face_normals[i] );
		normals.push_back( face_normals[i] );
		normals.push_back( face_normals[i] );
	}
}

void TriMesh::generateTriangle(glm::vec3 color)
{
	// 创建顶点前要先把那些vector清空
	cleanData();

	for (int i = 0; i < 3; i++)
	{
		vertex_positions.push_back(triangle_vertices[i]);
		vertex_colors.push_back(color);
	}

	// 每个三角面片的顶点下标
	faces.push_back(vec3i(0, 1, 2));

	// 顶点纹理坐标
	vertex_textures.push_back(glm::vec2(0, 0));
	vertex_textures.push_back(glm::vec2(1, 0));
	vertex_textures.push_back(glm::vec2(0.5, 1));

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();
}

void TriMesh::generateSquare(glm::vec3 color)
{
	// 创建顶点前要先把那些vector清空
	cleanData();

	for (int i = 0; i < 4; i++)
	{
		vertex_positions.push_back(square_vertices[i]);
		vertex_colors.push_back(color);
	}

	// 每个三角面片的顶点下标
	faces.push_back(vec3i(0, 1, 2));
	faces.push_back(vec3i(0, 2, 3));

	// 顶点纹理坐标
	vertex_textures.push_back(glm::vec2(0, 0));
	vertex_textures.push_back(glm::vec2(1, 0));
	vertex_textures.push_back(glm::vec2(1, 1));
	vertex_textures.push_back(glm::vec2(0, 1));

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();
}

void TriMesh::generateCylinder(int num_division, float radius, float height)
{

	cleanData();

	int num_samples = num_division;
	float step = 2 * M_PI / num_samples; // 每个切片的弧度

	// 按cos和sin生成x，y坐标，z为负，即得到下表面顶点坐标
	// 顶点， 纹理
	float z = -height;
	for (int i = 0; i < num_samples; i++)
	{
		float r_r_r = i * step;
		float x = radius * cos(r_r_r);
		float y = radius * sin(r_r_r);
		// 添加顶点坐标
		vertex_positions.push_back(glm::vec3(x, y, z));
		vertex_normals.push_back( normalize(glm::vec3(x, y, 0)));
		// 这里颜色和法向量一样
		vertex_colors.push_back( normalize(glm::vec3(x, y, 0)));
	}

	// 按cos和sin生成x，y坐标，z为正，即得到上表面顶点坐标
	z = height;
	for (int i = 0; i < num_samples; i++)
	{
		float r_r_r = i * step;
		float x = radius * cos(r_r_r);
		float y = radius * sin(r_r_r);
		vertex_positions.push_back(glm::vec3(x, y, z));
		vertex_normals.push_back( normalize(glm::vec3(x, y, 0)));
		vertex_colors.push_back( normalize(glm::vec3(x, y, 0)));
	}

	// 面片生成三角面片，每个矩形由两个三角形面片构成
	for (int i = 0; i < num_samples; i++)
	{
		// 面片1
		faces.push_back(vec3i(i, (i + 1) % num_samples, (i) + num_samples));
		// 面片2
		faces.push_back(vec3i((i) + num_samples, (i + 1) % num_samples, (i + num_samples + 1) % (num_samples) + num_samples));

		// 面片1对应的顶点的纹理坐标
		vertex_textures.push_back(glm::vec2(1.0 * i / num_samples, 0.0));
		vertex_textures.push_back(glm::vec2(1.0 * (i+1) / num_samples, 0.0));
		vertex_textures.push_back(glm::vec2(1.0 * i / num_samples, 1.0));
		// 对应的三角面片的纹理坐标的下标
		texture_index.push_back( vec3i( 6*i, 6*i+1, 6*i+2 ) );

		// 面片2对应的顶点的纹理坐标
		vertex_textures.push_back(glm::vec2(1.0 * i / num_samples, 1.0));
		vertex_textures.push_back(glm::vec2(1.0 * (i+1) / num_samples, 0.0));
		vertex_textures.push_back(glm::vec2(1.0 * (i+1) / num_samples, 1.0));
		// 对应的三角面片的纹理坐标的下标
		texture_index.push_back( vec3i( 6*i+3, 6*i+4, 6*i+5 ) );
	}

	// 三角面片的每个顶点的法向量的下标，这里和顶点坐标的下标 faces是一致的，所以我们用faces就行
	normal_index = faces;
	// 三角面片的每个顶点的颜色的下标
	color_index = faces;

	storeFacesPoints();
}

void TriMesh::generateDisk(int num_division, float radius)
{
	cleanData();
	int num_samples = num_division;
	float step = 2 * M_PI / num_samples;

	// 圆心
	vertex_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	vertex_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	vertex_colors.push_back(glm::vec3(0.5f, 0.5f, 0.5f));

	// 圆周上的点
	for (int i = 0; i < num_samples; i++) {
		float angle = i * step;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		vertex_positions.push_back(glm::vec3(x, y, 0.0f));
		vertex_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
		vertex_colors.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
	}

	// 生成三角形面片
	for (int i = 0; i < num_samples; i++) {
		int next = (i + 1) % num_samples;
		faces.push_back(vec3i(0, i + 1, next + 1));
	}

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();
}

void TriMesh::generateCone(int num_division, float radius, float height)
{
	cleanData();
	int num_samples = num_division;
	float step = 2 * M_PI / num_samples;

	// 锥顶点
	vertex_positions.push_back(glm::vec3(0.0f, height, 0.0f));
	vertex_normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	vertex_colors.push_back(glm::vec3(0.6f, 0.4f, 0.2f));

	// 底面圆周上的点
	for (int i = 0; i < num_samples; i++) {
		float angle = i * step;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		vertex_positions.push_back(glm::vec3(x, 0.0f, z));
		// 计算侧面法向量
		glm::vec3 normal = glm::normalize(glm::vec3(x, radius / height, z));
		vertex_normals.push_back(normal);
		vertex_colors.push_back(glm::vec3(0.5f, 0.35f, 0.15f));
	}

	// 底面中心点
	int bottom_center = num_samples + 1;
	vertex_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	vertex_normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
	vertex_colors.push_back(glm::vec3(0.4f, 0.3f, 0.1f));

	// 生成侧面三角形（从锥顶到底边）
	for (int i = 0; i < num_samples; i++) {
		int next = (i + 1) % num_samples + 1;
		faces.push_back(vec3i(0, next, i + 1)); // 锥顶 -> 下一个点 -> 当前点
	}

	// 生成底面三角形
	for (int i = 0; i < num_samples; i++) {
		int next = (i + 1) % num_samples + 1;
		faces.push_back(vec3i(bottom_center, i + 1, next));
	}

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();
}

void TriMesh::readOff(const std::string &filename)
{
	// fin打开文件读取文件信息
	if (filename.empty())
	{
		return;
	}
	std::ifstream fin;
	fin.open(filename);
	if (!fin)
	{
		printf("File on error\n");
		return;
	}
	else
	{
		printf("File open success\n");

		cleanData();

		int nVertices, nFaces, nEdges;

		// 读取OFF字符串
		std::string str;
		fin >> str;
		// 读取文件中顶点数、面片数、边数
		fin >> nVertices >> nFaces >> nEdges;
		// 根据顶点数，循环读取每个顶点坐标
		for (int i = 0; i < nVertices; i++)
		{
			glm::vec3 tmp_node;
			fin >> tmp_node.x >> tmp_node.y >> tmp_node.z;
			vertex_positions.push_back(tmp_node);
			vertex_colors.push_back(tmp_node);
		}
		// 根据面片数，循环读取每个面片信息，并用构建的vec3i结构体保存
		for (int i = 0; i < nFaces; i++)
		{
			int num, a, b, c;
			// num记录此面片由几个顶点构成，a、b、c为构成该面片顶点序号
			fin >> num >> a >> b >> c;
			faces.push_back(vec3i(a, b, c));
		}
	}
	fin.close();

	normal_index = faces;
	color_index = faces;
	texture_index = faces;

	storeFacesPoints();
};

// Light
glm::mat4 Light::getShadowProjectionMatrix()
{
	// 这里只实现了Y=0平面上的阴影投影矩阵，其他情况自己补充
	float lx, ly, lz;

	glm::mat4 modelMatrix = this->getModelMatrix();
	glm::vec4 light_position = modelMatrix * glm::vec4(this->translation, 1.0);

	lx = light_position[0];
	ly = light_position[1];
	lz = light_position[2];

	return glm::mat4(
		-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
}

// 设置衰减系数的参数
void Light::setAttenuation(float _constant, float _linear, float _quadratic)
{
	constant = _constant;
	linear = _linear;
	quadratic = _quadratic;
}

float Light::getConstant() { return constant; };
float Light::getLinear() { return linear; };
float Light::getQuadratic() { return quadratic; };

// 生成剑模型（剑柄+剑身+剑尖）
void TriMesh::generateSword()
{
	cleanData();
	
	// 精致仙剑 - 水平放置，优雅造型
	glm::vec3 blade_gold(0.95f, 0.85f, 0.5f);     // 剑身金色
	glm::vec3 edge_white(0.98f, 0.98f, 1.0f);     // 剑刃亮白
	glm::vec3 guard_jade(0.6f, 0.85f, 0.75f);     // 护手玉色
	glm::vec3 handle_red(0.6f, 0.15f, 0.1f);      // 剑柄红色
	
	// 剑身 - 修长优雅
	float blade_len = 1.8f;
	float blade_w = 0.18f;
	float blade_h = 0.025f;
	
	// 剑身主体（8顶点立方体）
	int b = vertex_positions.size();
	vertex_positions.push_back(glm::vec3(-blade_w/2, -blade_h, -blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, -blade_h, -blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, blade_h, -blade_len/2));
	vertex_positions.push_back(glm::vec3(-blade_w/2, blade_h, -blade_len/2));
	vertex_positions.push_back(glm::vec3(-blade_w/2, -blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, -blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(-blade_w/2, blade_h, blade_len/2));
	for(int i=0;i<8;i++) vertex_colors.push_back(blade_gold);
	
	// 剑身6面
	faces.push_back(vec3i(b+0,b+1,b+2)); faces.push_back(vec3i(b+0,b+2,b+3));
	faces.push_back(vec3i(b+4,b+6,b+5)); faces.push_back(vec3i(b+4,b+7,b+6));
	faces.push_back(vec3i(b+0,b+4,b+5)); faces.push_back(vec3i(b+0,b+5,b+1));
	faces.push_back(vec3i(b+2,b+6,b+7)); faces.push_back(vec3i(b+2,b+7,b+3));
	faces.push_back(vec3i(b+0,b+3,b+7)); faces.push_back(vec3i(b+0,b+7,b+4));
	faces.push_back(vec3i(b+1,b+5,b+6)); faces.push_back(vec3i(b+1,b+6,b+2));
	
	// 剑尖（三角形）
	int t = vertex_positions.size();
	vertex_positions.push_back(glm::vec3(0, 0, blade_len/2 + 0.25f)); // 尖端
	vertex_positions.push_back(glm::vec3(-blade_w/2, -blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, -blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(blade_w/2, blade_h, blade_len/2));
	vertex_positions.push_back(glm::vec3(-blade_w/2, blade_h, blade_len/2));
	for(int i=0;i<5;i++) vertex_colors.push_back(edge_white);
	faces.push_back(vec3i(t,t+1,t+2));
	faces.push_back(vec3i(t,t+2,t+3));
	faces.push_back(vec3i(t,t+3,t+4));
	faces.push_back(vec3i(t,t+4,t+1));
	
	// 剑格（玉色护手，椭圆形装饰）
	int g = vertex_positions.size();
	float gw = 0.35f, gh = 0.06f, gd = 0.08f;
	float gz = -blade_len/2 + 0.15f;
	vertex_positions.push_back(glm::vec3(-gw/2, -gh, gz-gd/2));
	vertex_positions.push_back(glm::vec3(gw/2, -gh, gz-gd/2));
	vertex_positions.push_back(glm::vec3(gw/2, gh, gz-gd/2));
	vertex_positions.push_back(glm::vec3(-gw/2, gh, gz-gd/2));
	vertex_positions.push_back(glm::vec3(-gw/2, -gh, gz+gd/2));
	vertex_positions.push_back(glm::vec3(gw/2, -gh, gz+gd/2));
	vertex_positions.push_back(glm::vec3(gw/2, gh, gz+gd/2));
	vertex_positions.push_back(glm::vec3(-gw/2, gh, gz+gd/2));
	for(int i=0;i<8;i++) vertex_colors.push_back(guard_jade);
	faces.push_back(vec3i(g+0,g+1,g+2)); faces.push_back(vec3i(g+0,g+2,g+3));
	faces.push_back(vec3i(g+4,g+6,g+5)); faces.push_back(vec3i(g+4,g+7,g+6));
	faces.push_back(vec3i(g+0,g+4,g+5)); faces.push_back(vec3i(g+0,g+5,g+1));
	faces.push_back(vec3i(g+2,g+6,g+7)); faces.push_back(vec3i(g+2,g+7,g+3));
	faces.push_back(vec3i(g+0,g+3,g+7)); faces.push_back(vec3i(g+0,g+7,g+4));
	faces.push_back(vec3i(g+1,g+5,g+6)); faces.push_back(vec3i(g+1,g+6,g+2));
	
	// 剑柄（红色）
	int h = vertex_positions.size();
	float hr = 0.025f, hl = 0.25f;
	float hz = -blade_len/2;
	vertex_positions.push_back(glm::vec3(-hr, -hr, hz));
	vertex_positions.push_back(glm::vec3(hr, -hr, hz));
	vertex_positions.push_back(glm::vec3(hr, hr, hz));
	vertex_positions.push_back(glm::vec3(-hr, hr, hz));
	vertex_positions.push_back(glm::vec3(-hr, -hr, hz-hl));
	vertex_positions.push_back(glm::vec3(hr, -hr, hz-hl));
	vertex_positions.push_back(glm::vec3(hr, hr, hz-hl));
	vertex_positions.push_back(glm::vec3(-hr, hr, hz-hl));
	for(int i=0;i<8;i++) vertex_colors.push_back(handle_red);
	faces.push_back(vec3i(h+0,h+1,h+2)); faces.push_back(vec3i(h+0,h+2,h+3));
	faces.push_back(vec3i(h+4,h+6,h+5)); faces.push_back(vec3i(h+4,h+7,h+6));
	faces.push_back(vec3i(h+0,h+4,h+5)); faces.push_back(vec3i(h+0,h+5,h+1));
	faces.push_back(vec3i(h+2,h+6,h+7)); faces.push_back(vec3i(h+2,h+7,h+3));
	faces.push_back(vec3i(h+0,h+3,h+7)); faces.push_back(vec3i(h+0,h+7,h+4));
	faces.push_back(vec3i(h+1,h+5,h+6)); faces.push_back(vec3i(h+1,h+6,h+2));
	
	normal_index = faces;
	color_index = faces;
	texture_index = faces;
	storeFacesPoints();
}

// 精致仙侠人物
void TriMesh::generateSimpleCharacter()
{
	cleanData();
	
	glm::vec3 skin(0.95f, 0.82f, 0.72f);
	glm::vec3 hair(0.12f, 0.1f, 0.08f);
	glm::vec3 robe_w(0.96f, 0.96f, 0.98f);
	glm::vec3 robe_b(0.7f, 0.82f, 0.92f);
	glm::vec3 belt(0.85f, 0.7f, 0.4f);
	
	auto addCube = [&](glm::vec3 pos, glm::vec3 size, glm::vec3 color) {
		int base = vertex_positions.size();
		for(int i=0;i<8;i++) {
			vertex_positions.push_back(cube_vertices[i] * size + pos);
			vertex_colors.push_back(color);
		}
		faces.push_back(vec3i(base+0,base+1,base+2)); faces.push_back(vec3i(base+0,base+2,base+3));
		faces.push_back(vec3i(base+4,base+6,base+5)); faces.push_back(vec3i(base+4,base+7,base+6));
		faces.push_back(vec3i(base+0,base+4,base+5)); faces.push_back(vec3i(base+0,base+5,base+1));
		faces.push_back(vec3i(base+2,base+6,base+7)); faces.push_back(vec3i(base+2,base+7,base+3));
		faces.push_back(vec3i(base+1,base+5,base+6)); faces.push_back(vec3i(base+1,base+6,base+2));
		faces.push_back(vec3i(base+0,base+3,base+7)); faces.push_back(vec3i(base+0,base+7,base+4));
	};
	
	// 头部
	addCube(glm::vec3(0, 0.72f, 0), glm::vec3(0.09f, 0.1f, 0.08f), skin);
	// 发髻（头顶）
	addCube(glm::vec3(0, 0.85f, 0), glm::vec3(0.06f, 0.06f, 0.06f), hair);
	// 头发后部
	addCube(glm::vec3(0, 0.75f, -0.05f), glm::vec3(0.08f, 0.08f, 0.03f), hair);
	
	// 颈部
	addCube(glm::vec3(0, 0.6f, 0), glm::vec3(0.035f, 0.04f, 0.03f), skin);
	
	// 上身（白色道袍）
	addCube(glm::vec3(0, 0.42f, 0), glm::vec3(0.12f, 0.15f, 0.07f), robe_w);
	// 衣领
	addCube(glm::vec3(0, 0.52f, 0.04f), glm::vec3(0.08f, 0.06f, 0.02f), robe_b);
	
	// 腰带
	addCube(glm::vec3(0, 0.28f, 0), glm::vec3(0.13f, 0.03f, 0.075f), belt);
	
	// 下摆（宽大飘逸）
	addCube(glm::vec3(0, 0.08f, 0), glm::vec3(0.15f, 0.18f, 0.09f), robe_w);
	// 下摆内衬
	addCube(glm::vec3(0, 0.0f, 0.05f), glm::vec3(0.12f, 0.15f, 0.02f), robe_b);
	
	// 左袖（飘逸宽袖）
	addCube(glm::vec3(-0.18f, 0.45f, 0), glm::vec3(0.08f, 0.05f, 0.06f), robe_w);
	addCube(glm::vec3(-0.28f, 0.42f, 0), glm::vec3(0.06f, 0.08f, 0.08f), robe_b);
	// 左手
	addCube(glm::vec3(-0.32f, 0.38f, 0), glm::vec3(0.025f, 0.04f, 0.025f), skin);
	
	// 右袖
	addCube(glm::vec3(0.18f, 0.45f, 0), glm::vec3(0.08f, 0.05f, 0.06f), robe_w);
	addCube(glm::vec3(0.28f, 0.42f, 0), glm::vec3(0.06f, 0.08f, 0.08f), robe_b);
	// 右手
	addCube(glm::vec3(0.32f, 0.38f, 0), glm::vec3(0.025f, 0.04f, 0.025f), skin);
	
	// 飘带（装饰）
	addCube(glm::vec3(-0.08f, 0.15f, 0.08f), glm::vec3(0.015f, 0.2f, 0.01f), robe_b);
	addCube(glm::vec3(0.08f, 0.15f, 0.08f), glm::vec3(0.015f, 0.2f, 0.01f), robe_b);
	
	normal_index = faces;
	color_index = faces;
	texture_index = faces;
	storeFacesPoints();
}

// 生成地面
void TriMesh::generateGround(float size)
{
	cleanData();
	
	float half_size = size / 2.0f;
	
	// 定义绿色
	glm::vec3 green_color(0.3f, 0.6f, 0.3f);
	
	// 地面四个顶点
	vertex_positions.push_back(glm::vec3(-half_size, 0, -half_size));
	vertex_positions.push_back(glm::vec3(half_size, 0, -half_size));
	vertex_positions.push_back(glm::vec3(half_size, 0, half_size));
	vertex_positions.push_back(glm::vec3(-half_size, 0, half_size));
	
	// 颜色
	vertex_colors.push_back(green_color);
	vertex_colors.push_back(green_color);
	vertex_colors.push_back(green_color);
	vertex_colors.push_back(green_color);
	
	// 纹理坐标
	vertex_textures.push_back(glm::vec2(0, 0));
	vertex_textures.push_back(glm::vec2(10, 0));
	vertex_textures.push_back(glm::vec2(10, 10));
	vertex_textures.push_back(glm::vec2(0, 10));
	
	// 两个三角形组成地面
	faces.push_back(vec3i(0, 1, 2));
	faces.push_back(vec3i(0, 2, 3));
	
	normal_index = faces;
	color_index = faces;
	texture_index = faces;
	
	storeFacesPoints();
}
