#version 450

out vec4 FragColor; //The color of this fragment

in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
}fs_in;

in vec4 LightSpacePos;

uniform sampler2D _ShadowMap;
uniform float _ShadowBiasMin;
uniform float _ShadowBiasMax;
uniform sampler2D _MainTex; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos)
{
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    sampleCoord = sampleCoord * 0.5 + 0.5;
	float bias = max(_ShadowBiasMax * (1.0 - dot(normalize(fs_in.WorldNormal),-_LightDirection)),_ShadowBiasMin);

	float myDepth = sampleCoord.z - bias;

	float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;

	return step(shadowMapDepth,myDepth);
}


void main()
{
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(fs_in.WorldNormal);
	//Light pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);
	//Calculate specularly reflected light
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	//Combination of specular and diffuse reflection
	float shadow = calcShadow(_ShadowMap, LightSpacePos); 
	vec3 light = (_Material.Ka * 0.15) + ((_Material.Kd + _Material.Ks) * _LightColor) * (1.0 - shadow);
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * light,1.0);
}


