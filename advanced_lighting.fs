// 컴퓨터공학과 1976433 황주이
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ObjColor;

// HW6
struct Light {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	float constant;
	float linear;
	float quadratic;
};
uniform Light light;
uniform bool radial;

void main()
{           

    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
	
    // ambient
    vec3 ambient = 0.1 *  color * light.ambient;
	
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color * light.diffuse;
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    const float specularStrength = 0.5;
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32); //32
	vec3 specular = specularStrength * lightColor * spec * light.specular;
	
	if (!radial)
	{
		float theta = dot(lightDir, normalize(-light.direction));
		float epsilon = (light.cutOff - light.outerCutOff);
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		diffuse *= intensity;
		specular *= intensity;
	}
	
    float distance    = length(light.position - fs_in.FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
	ambient  *= attenuation; 
	diffuse   *= attenuation;
	specular *= attenuation;
	
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}