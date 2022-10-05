#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

static ParticleSystem* system;

//
// Particle system parameters 
//
constexpr int max_particle_count = 1000;

struct Params {
	CubicBezier fuse_curve = {
		{-5, -3}, // p0
		{+5, +3}, // p1
		{-3, -1}, // c0
		{+3, +1}, // c1
	};
	
	float fuse_burning_speed = 1;
};

static Params params;

//
// Simulation variables
//
float consumed_fuse_t;

//
// Graphics variables
//
Mesh* fuse_mesh;

void bomb_init() {
	fuse_mesh = mesh_generate_bezier(params.fuse_curve, 30, 0.1);
}

void bomb_frame(float dt) {
	
}