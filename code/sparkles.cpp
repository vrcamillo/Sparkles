#include "common.h"

// #todo: It would be nice to have a nice visualization for distributions and coordinate systems.

enum class Distribution {
	UNIFORM, // Every value in the sample has the same chance of being chosen.
};

const char* distribution_names[] = {
	"Uniform",
};

enum class CoordinateSystem {
	RECTANGULAR,
	POLAR,
};

const char* coordinate_system_names[] = {
	"Rectangular",
	"Polar",
};

enum class ColorSystem {
	RGB,
	HSV,
};

const char* color_system_names[] = {
	"RGB",
	"HSV",
};

struct RandomScalar {
	Distribution distribution;
	float min;
	float max;
};

struct RandomVec2 {
	CoordinateSystem coordinate_system;
	
	union {
		struct {
			RandomScalar x;
			RandomScalar y;
		};
		struct {
			RandomScalar angle;
			RandomScalar radius;
		};
	};
};

struct RandomColor {
	ColorSystem color_system;
	
	union {
		struct {
			RandomScalar red;
			RandomScalar green;
			RandomScalar blue;
		};
		struct {
			RandomScalar hue;
			RandomScalar saturation;
			RandomScalar brightness;
		};
	};
	
	RandomScalar alpha;
};

struct ParticleParams {
	struct {
		vec2f gravity;
		float friction;	
	} physics;
	
	struct {
		RandomVec2 position;
		RandomVec2 velocity;
		RandomScalar life;
		RandomColor color;
	} spawn;
};


struct Particle {
	// GPU data
	vec2f position;
	float scale;
	vec4f color;
	
	// CPU data
	vec2f velocity;
	float life;
};

struct ParticleSystem {
	ParticleParams params;
	uint32_t count;
	Particle* particles;	
};

ParticleSystem the_system;

void particle_system_initialize(ParticleSystem* system, uint32_t particle_count, ParticleParams* params) {
	system->params = *params;	
	system->count = particle_count;
	system->particles = new Particle[particle_count];
}

void initialize() {
	ParticleParams params = {};
	
	params.physics.gravity = {0, +1};
	params.physics.friction = 0.9f;
	
	params.spawn.position = {
		.coordinate_system = CoordinateSystem::RECTANGULAR,
		.x = {Distribution::UNIFORM, -1, 1},
		.y = {Distribution::UNIFORM, -0.25, +0.25},
	};
	params.spawn.velocity = {
		.coordinate_system = CoordinateSystem::RECTANGULAR,
		.x = {Distribution::UNIFORM, 0, 0},
		.y = {Distribution::UNIFORM, 1, 2},
	};
	params.spawn.life = {Distribution::UNIFORM, 1, 3};
	params.spawn.color = {
		.color_system = ColorSystem::RGB,
		.red   = {Distribution::UNIFORM, 0.8, 1},
		.green = {Distribution::UNIFORM, 0.8, 1},
		.blue  = {Distribution::UNIFORM, 0.8, 1},
		.alpha = {Distribution::UNIFORM, 0.8, 1},
	};
	
	particle_system_initialize(&the_system, 1000, &params);
}

void do_random_scalar(const char* label, RandomScalar* scalar, float min, float max) {  
	ImGui::Text(label);
	// if (ImGui::TreeNode(label)) {
	ImGui::Indent();
	ImGui::Combo("Distribution", (int*) &scalar->distribution, distribution_names, IM_ARRAYSIZE(distribution_names));
	
	ImGui::InputFloat("Min value", &scalar->min, min, max);
	ImGui::InputFloat("Max value", &scalar->max, min, max);
	ImGui::Unindent();
	ImGui::Separator();
	
		//ImGui::TreePop();
	//}
}

void do_side_panel() {
	int screen_w, screen_h;
	glfwGetFramebufferSize(global_window, &screen_w, &screen_h);
	
	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize({screen_w * 0.25f, (float) screen_h});
	ImGui::Begin("Particle Parameters", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	
	do_random_scalar("Life", &the_system.params.spawn.life, 0, 10);
	
	ImGui::End();
}

void do_frame() {
	
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	do_side_panel();
	
	
}