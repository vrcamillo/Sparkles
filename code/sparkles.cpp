#include "common.h"

// #todo: It would be nice to have a nice visualization for distributions and coordinate systems.

enum class Distribution {
	UNIFORM, // Every value in the sample has the same chance of being chosen.
};

enum class CoordinateSystem {
	RECTANGULAR,
	POLAR,
};

enum class ColorSystem {
	RGB,
	HSV,
};

struct RandomScalar {
	Distribution type = Distribution::UNIFORM;
	float min = 0;
	float max = 0;
};

struct RandomVec2 {
	CoordinateSystem coordinate_system = CoordinateSystem::RECTANGULAR;
	
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
	ColorSystem color_system = ColorSystem::RGB;
	
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

struct ParticleSpawnParams {
	RandomVec2 position;
	RandomVec2 velocity;
	RandomVec2 life;
	RandomColor color;
};

struct ParticlePhysicsParams {
	vec2f gravity;
	vec2f friction;
};

void initialize() {
	
}

void do_frame() {
	
}