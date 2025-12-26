#version 330 core

// 给光源数据一个结构体
struct Light{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	vec3 position;
    // 光源衰减系数的三个参数
    float constant; // 常数项
    float linear;	// 一次项
    float quadratic;// 二次项

};

// 给物体材质数据一个结构体
struct Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float shininess;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;


// 相机坐标
uniform vec3 eye_position;
// 光源
uniform Light light;
// 物体材质
uniform Material material;

uniform int isShadow;
// 纹理数据
uniform sampler2D texture1;
// 是否使用纹理
uniform int useTexture;


out vec4 fColor;

void main()
{
	if (isShadow == 1) {
		fColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else {
		// 计算光照
		vec3 N = normalize(normal);
		vec3 L = normalize(light.position - position);
		vec3 V = normalize(eye_position - position);
		vec3 R = reflect(-L, N);
		
		// 环境光
		vec4 ambient = light.ambient * material.ambient;
		
		// 漫反射
		float diff = max(dot(N, L), 0.0);
		vec4 diffuse = diff * light.diffuse * material.diffuse;
		
		// 镜面反射
		float spec = pow(max(dot(V, R), 0.0), material.shininess);
		vec4 specular = spec * light.specular * material.specular;
		
		// 光源衰减
		float distance = length(light.position - position);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
		
		// 最终颜色
		vec4 lightColor = ambient + (diffuse + specular) * attenuation;
		
		// 如果有纹理则混合纹理
		if (useTexture == 1) {
			vec4 texColor = texture(texture1, texCoord);
			fColor = lightColor * texColor;
		} else {
			fColor = lightColor;
		}
		
		// 确保alpha为1
		fColor.a = 1.0;
	}
}
