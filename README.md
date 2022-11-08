# Sparkles!
A particle system library written in C++ and OpenGL.

## API

The API is documented in ```include/sparkles.h```.

Here's a sample code:
``` C++
#include "sparkles.h"
using namespace Sparkles;

// Initialization code
Sparkles::initialize();

// Create a particle system.
int particle_count = 100;
ParticleSystem* particle_system = particle_system_create(particle_count);

// Simulation code
for (int i = 0; i < particle_system->count; i += 1) {
  Particle* particle = particle_system->particles[i];
  
  // Spawn and simulate particles as you wish.
}

// Render code
RenderState my_render_state = /*...*/;
Mesh* my_mesh = /*...*/;
particle_system_upload_and_render(particle_system, my_mesh, &my_render_state);
```

## Showcase

Open the ```example``` folder and run ```build_win32.bat``` to build our particle showcase.

You will to run the script from a developer console with Microsoft Build Tools installed. 
Check __[this link](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line)__ for reference.

(Right now, we only support Windows, but we will eventually support other platforms as well).

In order to see the simulation code, take a look at ```examples\code\sandbox.cpp```. That's where most of the magic happens. 

<img src="/example/images/editor.gif"/>

## Examples

The following samples are located under ```examples\samples```.

<div style="display: grid;">
<img src="/example/images/firework1.gif" height="300"/>
<img src="/example/images/firework2.gif" height="300"/>
<img src="/example/images/fire.gif" height="300"/>
<img src="/example/images/cold_fire.gif" height="300"/>
<img src="/example/images/pulse1.gif" height="300"/>
<img src="/example/images/pulse2.gif" height="300"/>
</div>

## Roadmap

This demo is currently in development. 

I plan to make it into a multi-platform library that supports other graphics APIs, such as D3D12 and Vulkan.

## License

This project is released under the MIT license. See LICENSE for more information.
