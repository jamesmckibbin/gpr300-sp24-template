#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <jameslib/framebuffer.h>


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI(jameslib::Framebuffer shadowFBO, jameslib::Framebuffer gBuffer);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Transform monkeyTransform;
ew::Transform planeTransform;

ew::Camera camera;
ew::Camera directionalLight;
ew::CameraController cameraController;

int boxBlurEnabled = 0;
float blurStrength = 1.0f;

float shadowBiasMin = 0.001f;
float shadowBiasMax = 0.010f;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	jameslib::Framebuffer framebuffer = jameslib::createFramebuffer(screenWidth, screenHeight, GL_RGB16F, 1);
	jameslib::Framebuffer shadowFBO = jameslib::createFramebuffer(1024, 1024, GL_RGB16F, 1);
	jameslib::Framebuffer gBuffer = jameslib::createGBuffer(screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader ppShader = ew::Shader("assets/postprocess.vert", "assets/postprocess.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader geomPassShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh = ew::Mesh(ew::createPlane(10, 10, 5));
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	directionalLight.target = glm::vec3(0, -3, 0);
	directionalLight.orthographic = true;
	directionalLight.position = glm::vec3(10, 10, 10);
	directionalLight.orthoHeight = 10;
	directionalLight.aspectRatio = 1;

	planeTransform.position += glm::vec3(0, -3, 0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		//RENDER SCENE TO G-BUFFER

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.fbo);
		glViewport(0, 0, gBuffer.width, gBuffer.height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		geomPassShader.use();
		geomPassShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

		geomPassShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();
		geomPassShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		//RENDER

		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO.fbo);
		glViewport(0, 0, 1024, 1024);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		shadowShader.use();
		shadowShader.setMat4("_ViewProjection", directionalLight.projectionMatrix() * directionalLight.viewMatrix());

		shadowShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();
		shadowShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
		glViewport(0, 0, framebuffer.width, framebuffer.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glBindTextureUnit(0, brickTexture);
		glBindTextureUnit(1, shadowFBO.depthBuffer);
		shader.use();
		shader.setInt("_MainTex", 0);
		shader.setInt("_ShadowMap", 1);
		shader.setMat4("_Model", glm::mat4(1.0f));
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setMat4("_LightViewProj", directionalLight.projectionMatrix() * directionalLight.viewMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		shader.setFloat("_ShadowBiasMin", shadowBiasMin);
		shader.setFloat("_ShadowBiasMin", shadowBiasMax);

		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw();
		shader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		ppShader.use();
		ppShader.setInt("_BlurEnabled", boxBlurEnabled);
		ppShader.setFloat("_BlurStrength", blurStrength);

		glBindVertexArray(dummyVAO);
		glBindTextureUnit(0, framebuffer.colorBuffers[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		drawUI(shadowFBO, gBuffer);

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &framebuffer.fbo);
	glDeleteFramebuffers(1, &shadowFBO.fbo);

	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}


void drawUI(jameslib::Framebuffer shadowFBO, jameslib::Framebuffer gBuffer) {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	if (ImGui::CollapsingHeader("Post Processing")) {
		ImGui::SliderInt("Box Blur", &boxBlurEnabled, 0, 1);
		ImGui::SliderFloat("Blur Strength", &blurStrength, 0.0f, 1.0f);
	}
	if (ImGui::CollapsingHeader("Directional Light")) {
		ImGui::SliderFloat3("Position", &directionalLight.position.x, -10.0f, 10.0f);
		ImGui::SliderFloat("Shadow Bias Min", &shadowBiasMin, 0.001, 0.010);
		ImGui::SliderFloat("Shadow Bias Max", &shadowBiasMax, 0.005, 0.030);

	}
	ImGui::End();

	ImGui::Begin("Shadow Map");
	ImGui::BeginChild("Shadow Map");
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)shadowFBO.depthBuffer, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Begin("GBuffers"); {
		ImVec2 texSize = ImVec2(gBuffer.width / 4, gBuffer.height / 4);
		for (size_t i = 0; i < 3; i++)
		{
			ImGui::Image((ImTextureID)gBuffer.colorBuffers[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

