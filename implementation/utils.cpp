#include <math.h>

// #random_number_cleanup
#include <stdlib.h> // srand
#include <time.h> // time

// #include "sparkles.h"
#include "sparkles_utils.h"

namespace Sparkles {
	
	float random_get_uniform() {
		
		static bool first = true;
		if (first) {
			// Initialize random number generator seed to the current time, so that every time we run the program we get different results.
			// Eventually we will want to be very careful about random numbers, but for the mean time, we just use c standard lib.
			// #random_number_cleanup
			srand(time(NULL));
			first = false;
		}
		
		float result = rand() / (float) RAND_MAX;
		return result;
	}
	
	float random_get(RandomScalar spec) {
		switch (spec.distribution) {
		  case Distribution::UNIFORM: return spec.min + random_get_uniform() * (spec.max - spec.min);
		  default: assert(false);
		}
		
		return 0;
	}
	
	vec2 random_get(RandomVec2 spec) {
		switch (spec.coordinate_system) {
		  case CoordinateSystem::RECTANGULAR: {
				float x = random_get(spec.x);
				float y = random_get(spec.y);
				return {x, y};
			} break;
			
		  case CoordinateSystem::POLAR: {
				float angle = random_get(spec.angle);
				float radius = random_get(spec.radius);
				return {radius * (float) cos(angle), radius * (float) sin(angle)};
			} break;
			
		  default: assert(false);
		}
		
		return {};
	}
	
	vec4 random_get(RandomColor spec) {
		switch (spec.color_system) {
		  case ColorSystem::RGB: {
				float red   = random_get(spec.red);
				float green = random_get(spec.green);
				float blue  = random_get(spec.blue);
				float alpha = random_get(spec.alpha);
				
				return {red, green, blue, alpha};
			} break;
			
		  case ColorSystem::HSB:
			assert(false); // #unimplemented
			return {};
			
		  default: assert(false);
		}
		
		return {};
	}
	
	
	void particle_simulate(Particle* p, PhysicsParams* physics, float dt) {
		p->velocity += physics->gravity * dt;
		p->position += p->velocity * dt;
		p->velocity *= physics->friction; // #temporary: This should not be frame rate dependant!
		p->life -= dt;
	}
	
	void particle_spawn(Particle* p, SpawnParams* spawn) {
		p->position.xy = random_get(spawn->position);
		p->position.z = 0;
		p->scale = random_get(spawn->scale);
		p->color = random_get(spawn->color);
		p->velocity.xy = random_get(spawn->velocity) * 0.2;
		p->life = random_get(spawn->life);
	}
	
	
}