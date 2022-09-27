#include "sparkles.h"
#include "glad/gl.h"
#include <stddef.h> // For offsetof
#include <string> // For memset

static const char* default_glsl_vertex_shader_source = R"glsl(
#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec2 vertex_uv;

layout (location = 3) in vec3 instance_position;
layout (location = 4) in float instance_scale;
layout (location = 5) in vec4 instance_color;

uniform mat4 projection;

out vec4 pixel_color;

void main() {
	vec4 world_position = vec4(instance_position + vertex_position * instance_scale, 1);
	gl_Position = projection * world_position;
	pixel_color = vertex_color * instance_color;
}  
)glsl";

static const char* default_glsl_pixel_shader_source = R"glsl(
#version 330 core

in vec4 pixel_color;
out vec4 result_color; 

void main() {
	result_color = pixel_color;
}

)glsl";

namespace Sparkles {	
	// #memory_cleanup #leak: We don't want to call new all over the place.
	
	struct Mesh {
		GLuint vbo; // Vertex buffer object
		GLuint ibo; // Index buffer object
		uint32_t index_count;
	};
	
	struct ShaderPipeline {
		GLuint program;
	};
	
	struct ParticleSystem_GL : ParticleSystem {
		GLuint instances_vbo; // Eventually we will want to use multiple buffers to avoid OpenGL synchronization delays. #opengl_sync_performance
	};
	
	// These are global variables. Maybe we should have a backend struct to hold global data?
	static ShaderPipeline* default_shader_program;
	static GLuint default_vao;
	static bool backend_initialized;

	bool backend_initialize() {	
		{
			// Create default shader program
			default_shader_program = shader_pipeline_create_glsl(default_glsl_vertex_shader_source, default_glsl_pixel_shader_source);
			if (!default_shader_program) return false;
		}
		
		{
			// Create default VAO.
			glGenVertexArrays(1, &default_vao);
			glBindVertexArray(default_vao);
			
			// Vertex position
			glEnableVertexAttribArray(0);
			glVertexAttribBinding(0, 0);
			glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
			
			// Vertex color
			glEnableVertexAttribArray(1);
			glVertexAttribBinding(1, 0);
			glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
			
			// Vertex uv
			glEnableVertexAttribArray(2);
			glVertexAttribBinding(2, 0);
			glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
			
			// Particle position
			glEnableVertexAttribArray(3);
			glVertexAttribBinding(3, 1);
			glVertexAttribFormat(3, 3, GL_FLOAT, GL_FALSE, offsetof(Particle, position));
			
			// Particle scale
			glEnableVertexAttribArray(4);
			glVertexAttribBinding(4, 1);
			glVertexAttribFormat(4, 1, GL_FLOAT, GL_FALSE, offsetof(Particle, scale));
			
			// Particle color
			glEnableVertexAttribArray(5);
			glVertexAttribBinding(5, 1);
			glVertexAttribFormat(5, 4, GL_FLOAT, GL_FALSE, offsetof(Particle, color));
			
			glVertexBindingDivisor(1, 1);
			
			glBindVertexArray(0);
		}
		
		backend_initialized = true;
		return true;
	}
	
	ParticleSystem* particle_system_create(uint32_t particle_count) {
		if (!backend_initialized) { 
			backend_initialize(); // #robustness: If this fails, we should return an error.
			backend_initialized = true;
		}
		
		uint32_t instance_buffer_size = particle_count * sizeof(Particle);
		
		auto system = new ParticleSystem_GL; // #memory_cleanup
		system->count = particle_count;
		system->particles = new Particle[particle_count]; // #memory_cleanup
		memset(system->particles, 0, instance_buffer_size);
		
		// #cleanup: This should be factored out into a function, so that it can be used for other backends.
		for (int i = 0; i < system->count; i += 1) {
			Particle* p = &system->particles[i];
			p->life = -1;
		}
		
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, instance_buffer_size, system->particles, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		system->instances_vbo = vbo;
		
		return system;
	}
	
	void particle_system_upload_and_render(ParticleSystem* system, Mesh* mesh, ShaderConstants* constants, ShaderPipeline* shader_pipeline) {
		auto system_gl = (ParticleSystem_GL*) system;
		
		{	
			//
			// Upload instance data to the GPU. 
			//
			
			// Because of sync issues, we probably want to use a smarter approach here.
			// #opengl_sync_performance
			uint32_t instance_buffer_size = system->count * sizeof(Particle);
			glBindBuffer(GL_ARRAY_BUFFER, system_gl->instances_vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, instance_buffer_size, system->particles);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		
		{
			//
			// Render 
			// 
			
			// Bind the shader program
			GLuint program = (shader_pipeline) ? shader_pipeline->program : default_shader_program->program;
			glUseProgram(program);
			
			// Set up uniform data.
			GLuint projection_uniform_loc = glGetUniformLocation(program, "projection");
			if (projection_uniform_loc >= 0) {
				glUniformMatrix4fv(projection_uniform_loc, 1, GL_TRUE, (float*) &constants->projection);
			} else {
				// #incomplete #robustness: Provide a helpful error message here.
			}
			
			// Bind the vertex format and mesh buffers
			glBindVertexArray(default_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
			glBindVertexBuffer(0, mesh->vbo, 0, sizeof(Vertex));
			glBindVertexBuffer(1, system_gl->instances_vbo, 0, sizeof(Particle));
			
			// Draw all particles in a single draw call.
			glDrawElementsInstanced(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, (void*) 0, system->count);
			
			// Reset OpenGL state.
			glBindVertexArray(0);
			glUseProgram(0);
		}
	}
	
	Mesh* mesh_create(uint32_t vertex_count, Vertex vertices[], uint32_t index_count, uint32_t indices[]) { 
		uint32_t vertex_buffer_size = vertex_count * sizeof(vertices[0]);
		uint32_t index_buffer_size = index_count * sizeof(indices[0]);
		
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		Mesh* result = new Mesh; // #memory_cleanup
		result->vbo = vbo;
		result->ibo = ibo;
		result->index_count = index_count;
		return result;
	}
	
	ShaderPipeline* shader_pipeline_create_glsl(const char* glsl_vertex_shader_source, const char* glsl_fragment_shader_source) {
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &glsl_vertex_shader_source, nullptr);
		glCompileShader(vertex_shader);
		
		int vertex_shader_compiled;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
		if (!vertex_shader_compiled) {
			char message[512];
			glGetShaderInfoLog(vertex_shader, sizeof(message), nullptr, message);
		// printf("Failed to compile vertex shader:\n%s\n", message);
			return nullptr; // #leak: Vertex shader leaks here.
		}
		
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &glsl_fragment_shader_source, nullptr);
		glCompileShader(fragment_shader);
		
		int fragment_shader_compiled = 0;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
		if (!fragment_shader_compiled) {
			char message[512];
			glGetShaderInfoLog(fragment_shader, sizeof(message), nullptr, message);
			// printf("Failed to compile fragment shader:\n%s\n", message);
			return nullptr; // #leak: Vertex and fragment shaders leak here.
		}
		
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
		
		int program_linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
		if (!program_linked) {
			char message[512];
			glGetProgramInfoLog(program, sizeof(message), nullptr, message);
			// printf("Failed to link vertex and fragment shaders:\n%s\n", message);
			return nullptr; // #leak: Vertex shader, fragment shader, and program leak here.
		}
		
		// From OpenGL docs (https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml):
		//     "If a shader object to be deleted is attached to a program object, it will be flagged for deletion, 
		//      but it will not be deleted until it is no longer attached to any program object, for any rendering context
		//      (i.e., it must be detached from wherever it was attached before it will be deleted)."
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		
		ShaderPipeline* result = new ShaderPipeline; // #memory_cleanup
		result->program = program;
		return result;
	}
}