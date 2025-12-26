#include "MeshPainter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

MeshPainter::MeshPainter(){};
MeshPainter::~MeshPainter(){};

std::vector<std::string> MeshPainter::getMeshNames(){ return mesh_names;};
std::vector<TriMesh *> MeshPainter::getMeshes(){ return meshes;};
std::vector<openGLObject> MeshPainter::getOpenGLObj(){ return opengl_objects;};

void MeshPainter::bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &texture_image, const std::string &vshader, const std::string &fshader){
    // 初始化各种对象
    std::vector<glm::vec3> points = mesh->getPoints();
    std::vector<glm::vec3> normals = mesh->getNormals();
    std::vector<glm::vec3> colors = mesh->getColors();
    std::vector<glm::vec2> textures = mesh->getTextures();

	// 创建顶点数组对象
	glGenVertexArrays(1, &object.vao);  	// 分配1个顶点数组对象
	glBindVertexArray(object.vao);  	// 绑定顶点数组对象

	// 创建并初始化顶点缓存对象
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 points.size() * sizeof(glm::vec3) +
                     colors.size() * sizeof(glm::vec3) +
                     normals.size() * sizeof(glm::vec3) +
                     textures.size() * sizeof(glm::vec2),
                 NULL, GL_STATIC_DRAW);

    // 绑定顶点数据
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec3), points.data());
    // 绑定颜色数据
    glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec3), colors.data());
    // 绑定法向量数据
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + colors.size()) * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), normals.data());
    // 绑定纹理数据
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + normals.size() + colors.size()) * sizeof(glm::vec3), textures.size() * sizeof(glm::vec2), textures.data());


	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

    // 将顶点传入着色器
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // 将颜色传入着色器
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(glm::vec3)));

    // 将法向量传入着色器
	object.nLocation = glGetAttribLocation(object.program, "vNormal");
	glEnableVertexAttribArray(object.nLocation);
	glVertexAttribPointer(object.nLocation, 3, 
		GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET( (points.size() + colors.size())  * sizeof(glm::vec3)));

    // 将纹理坐标传入着色器
	object.tLocation = glGetAttribLocation(object.program, "vTexture");
	glEnableVertexAttribArray(object.tLocation);
	glVertexAttribPointer(object.tLocation, 2, 
		GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET( (points.size() + colors.size() + normals.size())  * sizeof(glm::vec3)));


	// 获得矩阵位置
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");

	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");

    // 读取纹理图片数
    object.texture_image = texture_image;
    // 创建纹理的缓存对象
    glGenTextures(1, &object.texture);
    // 调用stb_image生成纹理
    load_texture_STBImage(object.texture_image, object.texture);
    
    // Clean up
    glUseProgram(0);
	glBindVertexArray(0);
};


void MeshPainter::bindLightAndMaterial( TriMesh* mesh, openGLObject &object, Light* light, Camera* camera ) {
    // 传递材质、光源等数据给着色器
    
    // 传递相机的位置
    glUniform3fv(glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0]);

    // 传递物体的材质
    glm::vec4 meshAmbient = mesh->getAmbient();
    glm::vec4 meshDiffuse = mesh->getDiffuse();
    glm::vec4 meshSpecular = mesh->getSpecular();
    float meshShininess = mesh->getShininess();

    glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
    glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
    glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
    glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);


    // 传递光源信息
    glm::vec4 lightAmbient = light->getAmbient();
    glm::vec4 lightDiffuse = light->getDiffuse();
    glm::vec4 lightSpecular = light->getSpecular();
    glm::vec3 lightPosition = light->getTranslation();
    glUniform4fv(glGetUniformLocation(object.program, "light.ambient"), 1, &lightAmbient[0]);
    glUniform4fv(glGetUniformLocation(object.program, "light.diffuse"), 1, &lightDiffuse[0]);
    glUniform4fv(glGetUniformLocation(object.program, "light.specular"), 1, &lightSpecular[0]);
    glUniform3fv(glGetUniformLocation(object.program, "light.position"), 1, &lightPosition[0]);

	glUniform1f(glGetUniformLocation(object.program, "light.constant"), light->getConstant() );
	glUniform1f(glGetUniformLocation(object.program, "light.linear"), light->getLinear() );
	glUniform1f(glGetUniformLocation(object.program, "light.quadratic"), light->getQuadratic() );

}


void MeshPainter::addMesh( TriMesh* mesh, const std::string &name, const std::string &texture_image, const std::string &vshader, const std::string &fshader ){
	mesh_names.push_back(name);
    meshes.push_back(mesh);

    openGLObject object;
    // 绑定openGL对象，并传递顶点属性的数据
    bindObjectAndData(mesh, object, texture_image, vshader, fshader);

    opengl_objects.push_back(object);
};



void MeshPainter::drawMesh(TriMesh* mesh, openGLObject &object, Light *light, Camera* camera){
    
    // 相机矩阵计算 - 不要调用updateCamera，直接使用已设置的eye/at/up
	camera->viewMatrix = camera->getViewMatrix();
	camera->projMatrix = camera->getProjectionMatrix(false);


	glBindVertexArray(object.vao);
	glUseProgram(object.program);

	// 物体的变换矩阵
	glm::mat4 modelMatrix = mesh->getModelMatrix();

	// 传递矩阵
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
	// 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
	glUniform1i(object.shadowLocation, 0);
	
	// 判断是否使用纹理
	int useTexture = (object.texture_image.empty()) ? 0 : 1;
	glUniform1i(glGetUniformLocation(object.program, "useTexture"), useTexture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, object.texture);// 该语句必须，否则将只使用同一个纹理进行绘制
    // 传递纹理数据 将生成的纹理传给shader
    glUniform1i(glGetUniformLocation(object.program, "texture1"), 0);
    
	// 将材质和光源数据传递给着色器
	bindLightAndMaterial(mesh, object, light, camera);
	// 绘制
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

	glBindVertexArray(0);

	glUseProgram(0);

};


void MeshPainter::drawMeshes(Light *light, Camera* camera){
    for (int i = 0; i < meshes.size(); i++)
    {
        drawMesh(meshes[i], opengl_objects[i], light, camera);
    }
};

void MeshPainter::cleanMeshes(){
    // 将数据都清空释放
    mesh_names.clear();

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->cleanData();

        delete meshes[i];
        meshes[i] = NULL;


        glDeleteVertexArrays(1, &opengl_objects[i].vao);

        glDeleteBuffers(1, &opengl_objects[i].vbo);
        glDeleteProgram(opengl_objects[i].program);
    }

    meshes.clear();
    opengl_objects.clear();
};


void MeshPainter::load_texture_STBImage(const std::string &file_name, GLuint& texture){
    // 读取纹理图片，并将其传递给着色器
    if (file_name.rfind("procedural:", 0) == 0) {
        // procedural:water -> 生成一张程序化水面纹理
        int width = 512;
        int height = 512;
        std::vector<unsigned char> data(width * height * 3);
        // 生成基于正弦的波纹和颜色变化
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float u = (float)x / width;
                float v = (float)y / height;
                float t = u * 10.0f + v * 8.0f;
                float wave = 0.5f + 0.5f * sinf(t * 6.28318f);
                float disturbance = 0.5f + 0.5f * sinf((u*20.0f + v*10.0f) * 6.28318f);
                float intensity = 0.6f * wave + 0.4f * disturbance;
                // 基础颜色蓝绿调
                float r = 0.05f + 0.05f * intensity;
                float g = 0.25f + 0.35f * intensity;
                float b = 0.45f + 0.45f * intensity;
                int idx = (y * width + x) * 3;
                data[idx + 0] = (unsigned char)glm::clamp(r * 255.0f, 0.0f, 255.0f);
                data[idx + 1] = (unsigned char)glm::clamp(g * 255.0f, 0.0f, 255.0f);
                data[idx + 2] = (unsigned char)glm::clamp(b * 255.0f, 0.0f, 255.0f);
            }
        }
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        return;
    }

    int width, height, channels = 0;
    unsigned char *pixels = NULL;
    // 读取图片的时候先翻转一下图片，如果不设置的话显示出来是反过来的图片
    stbi_set_flip_vertically_on_load(true);
    // 读取图片数据
    pixels = stbi_load(file_name.c_str(), &width, &height, &channels, 0);

    if (!pixels) {
        // 读取失败，创建一张默认占位纹理（紫色方块）
        int defW = 4, defH = 4;
        std::vector<unsigned char> def(defW * defH * 3);
        for (int i = 0; i < defW * defH; ++i) {
            def[i*3+0] = 180; def[i*3+1] = 0; def[i*3+2] = 180;
        }
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, defW, defH, 0, GL_RGB, GL_UNSIGNED_BYTE, def.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        return;
    }

    // 调整行对齐格式
    if (width * channels % 4 != 0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLenum format = GL_RGB;
    // 设置通道格式
    switch (channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        format = GL_RGB;
        break;
    }

    // 绑定纹理对象
    glBindTexture(GL_TEXTURE_2D, texture);

    // 将图片的rgb数据上传给opengl
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        pixels
    );

    // 指定插值方法
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // 恢复初始对齐格式
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    // 释放图形内存
    stbi_image_free(pixels);
};
