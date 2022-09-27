# Sparkles!
A particle system library written in C++ and OpenGL.

## API

The API is documented in ```include/sparkles.h```.

``` C++
#include "sparkles.h"
using namespace Sparkles;

// Initialization code
int particle_count = 100;
ParticleSystem* particle_system = particle_system_create(particle_count);

// Simulation code
for (int i = 0; i < particle_system->count; i += 1) {
  Particle* particle = particle_system->particles[i];
  
  // Spawn and simulate particles as you wish.
}

// Render code
ShaderConstants my_shader_constants = /*...*/;
Mesh* my_mesh = /*...*/;
particle_system_upload_and_render(particle_system, my_mesh, &my_shader_constants);
```

## Showcase

Open the ```example``` folder and run ```build_win32.bat``` in a developer console with Microsoft Build Tools to build our particle showcase.

You will to run the script from a a developer console with Microsoft Build Tools. 
Check __[this link](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line)__ for reference.

(Right now, we only support Windows, but we will eventually support other platforms as well).

In the near future, I will add screenshots, and more informative descriptions here. 

## Roadmap

This demo is currently in development. 

I plan to make it into a multi-platform library that supports other graphics APIs, such as D3D12 and Vulkan.  

## License

This project is released under the MIT license. See LICENSE for more information.
