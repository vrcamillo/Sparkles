#include "sparkles.h"
#include "glad/gl.h"

#include <stddef.h> // For offsetof
#include <string> // For memset

static const char* glsl_default_instancing_vertex_shader_source = R"glsl(
#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec2 vertex_uv;

layout (location = 3) in vec3 instance_position;
layout (location = 4) in float instance_scale;
layout (location = 5) in vec4 instance_color;

uniform mat4 projection;

out vec2 pixel_uv;
out vec4 pixel_color;

void main() {
	vec4 world_position = vec4(instance_position + vertex_position * instance_scale, 1);
	gl_Position = projection * world_position;
	pixel_color = vertex_color * instance_color;
	pixel_uv = vertex_uv;
}  
)glsl";

static const char* glsl_default_vertex_shader_source = R"glsl(
#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec2 vertex_uv;

uniform mat4 projection;

out vec2 pixel_uv;
out vec4 pixel_color;

void main() {
	vec4 world_position = vec4(vertex_position, 1);
	gl_Position = projection * world_position;
	pixel_color = vertex_color;
	pixel_uv = vertex_uv;
}  
)glsl";

static const char* glsl_default_pixel_shader_source = R"glsl(
#version 410

in vec4 pixel_color;
in vec2 pixel_uv;
out vec4 result_color; 

uniform bool use_texture;
uniform sampler2D sampler;

void main() {
	if (use_texture) {
		vec4 texture_color = texture(sampler, pixel_uv);
		result_color = texture_color * pixel_color;
	} else {
		result_color = pixel_color;
	}
}

)glsl";

// #memory_cleanup #leak: We don't want to call 'new' all over the place.

namespace Sparkles {	
	
	// Here we define our custom graphics data structures, which are forward declared  in "sparkles.h"
	struct Mesh {
		GLuint vbo = 0; // Vertex buffer object
		GLuint ibo = 0; // Index buffer object
		uint32_t vertex_count = 0;
		uint32_t index_count = 0;
	};
	
	struct Shader {
		GLuint handle = 0; // OpenGL shader handle.
	};
	
	struct Texture {
		GLuint handle = 0; // OpenGL texture handle.
	};
	
	struct RenderTarget {
		GLuint fbo = 0; // Framebuffer buffer object.
		Texture* color_attachment;
	};
	
	struct ParticleSystem_GL : ParticleSystem {
		GLuint instances_vbo = 0; // Eventually we will want to use multiple buffers to avoid OpenGL synchronization delays. #opengl_sync_performance
	};
	
	struct ShaderLinkage {
		GLuint program = 0;
		Shader* vertex_shader = nullptr;
		Shader* pixel_shader = nullptr;
	};
	
	// These are global variables. Maybe we should have a backend struct to hold global data?
	static Shader* default_instancing_vertex_shader;
	static Shader* default_vertex_shader;
	static Shader* default_pixel_shader;
	static GLuint default_vao;
	static GLuint default_instancing_vao;
	static bool backend_initialized;

	// #temporary: Eventually we will want an actual table here.
	constexpr int shader_linkage_table_capacity = 128;
	int shader_linkage_table_length;
	ShaderLinkage shader_linkage_table[shader_linkage_table_capacity]; 
	
	bool initialize() {		
		{
			// Create default shader program
			default_instancing_vertex_shader = shader_create(ShaderLanguage::GLSL, ShaderType::VERTEX, glsl_default_instancing_vertex_shader_source);
			SPARKLES_ASSERT(default_instancing_vertex_shader);
			
			default_vertex_shader = shader_create(ShaderLanguage::GLSL, ShaderType::VERTEX, glsl_default_vertex_shader_source);
			SPARKLES_ASSERT(default_vertex_shader);
			
			default_pixel_shader = shader_create(ShaderLanguage::GLSL, ShaderType::PIXEL, glsl_default_pixel_shader_source);
			SPARKLES_ASSERT(default_pixel_shader);
		}
		
		{
			// Create default VAOs.
			
			{
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
				
				glBindVertexArray(0);
			}
			
			{
				glGenVertexArrays(1, &default_instancing_vao);
				glBindVertexArray(default_instancing_vao);
				
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
		}
		
		{
			// Set default blend state. #temporary
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		
		backend_initialized = true;
		return true;
	}
	
	static ShaderLinkage* opengl_get_or_create_shader_program(Shader* vertex_shader, Shader* pixel_shader, bool instancing) {
		if (!vertex_shader) vertex_shader = instancing ? default_instancing_vertex_shader : default_vertex_shader;
		if (!pixel_shader)  pixel_shader  = default_pixel_shader;
		
		for (int i = 0; i < shader_linkage_table_length; i += 1) {
			ShaderLinkage* entry = &shader_linkage_table[i];
			if (vertex_shader == entry->vertex_shader && pixel_shader == entry->pixel_shader) {
				return entry;
			}
		}
		
		SPARKLES_ASSERT(shader_linkage_table_length < shader_linkage_table_capacity); // Not being robust here because we plan to turn shader_linkage_table into an actual table.
		
		// If we get here, it means we did not find both shaders linked together in a program, so we generate a new one.
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader->handle);
		glAttachShader(program, pixel_shader->handle);
		glLinkProgram(program);
		
		int program_linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
		if (!program_linked) {
			char message[512];
			glGetProgramInfoLog(program, sizeof(message), nullptr, message);
			SPARKLES_LOG("Failed to link vertex and fragment shaders:\n%s\n", message);
			glDeleteProgram(program);
			return 0;
		}
		
		auto entry = &shader_linkage_table[shader_linkage_table_length++];
		entry->program = program;
		entry->vertex_shader = vertex_shader;
		entry->pixel_shader = pixel_shader;
		return entry;
	}
	
	static void opengl_apply_render_state(RenderState* render_state, bool instancing) {
		ShaderLinkage* linkage = opengl_get_or_create_shader_program(render_state->vertex_shader, render_state->pixel_shader, instancing);
		glUseProgram(linkage->program);
		
		glBindFramebuffer(GL_FRAMEBUFFER, render_state->render_target ? render_state->render_target->fbo : 0);
		
		Rect viewport = render_state->viewport;
		glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
		
		// Apply uniforms
		GLuint projection_uniform_loc = glGetUniformLocation(linkage->program, "projection");
		if (projection_uniform_loc >= 0) {
			glUniformMatrix4fv(projection_uniform_loc, 1, GL_TRUE, (float*) &render_state->projection);
		} else {
			// #incomplete #robustness: Provide a helpful error message here.
		}	
		
		GLuint use_texture_uniform_loc = glGetUniformLocation(linkage->program, "use_texture");
		if (use_texture_uniform_loc >= 0) {
			glUniform1i(use_texture_uniform_loc, render_state->texture0 != nullptr);
		} else {
			// #incomplete #robustness: Provide a helpful error message here.
		}	
		
		
		if (render_state->texture0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, render_state->texture0->handle);
		}
		
		glBindVertexArray(instancing ? default_instancing_vao : default_vao);
	}
	
	static void opengl_reset_render_state() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		glBindVertexArray(0);
	}
	
	ParticleSystem* particle_system_create(uint32_t particle_count) {		
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
	
	void particle_system_upload_and_render(ParticleSystem* system, Mesh* mesh, RenderState* render_state) {
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
			
			opengl_apply_render_state(render_state, true);
			
			// Bind the vertex format and mesh buffers
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
			glBindVertexBuffer(0, mesh->vbo, 0, sizeof(Vertex));
			glBindVertexBuffer(1, system_gl->instances_vbo, 0, sizeof(Particle));
			
			// Draw all particles in a single draw call.
			glDrawElementsInstanced(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, (void*) 0, system->count);
			
			opengl_reset_render_state();
		}
	}
	
	Shader* shader_create(ShaderLanguage language, ShaderType type, const char* shader_source_code) {
		
		GLenum gl_shader_type = -1;
		switch (type) {
		  case ShaderType::VERTEX: gl_shader_type = GL_VERTEX_SHADER;   break;
		  case ShaderType::PIXEL:  gl_shader_type = GL_FRAGMENT_SHADER; break;
		  default: SPARKLES_ASSERT(false);
		}
		
		GLuint handle = glCreateShader(gl_shader_type);
		glShaderSource(handle, 1, &shader_source_code, nullptr);
		glCompileShader(handle);
		
		int shader_compiled;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &shader_compiled);
		if (!shader_compiled) {
			char message[512];
			glGetShaderInfoLog(handle, sizeof(message), nullptr, message);
			SPARKLES_LOG("Failed to compile vertex shader:\n%s\n", message);
			glDeleteShader(handle);
			return nullptr;
		}
	
		Shader* result = new Shader; // #memory_cleanup
		result->handle = handle;
		return result;
	}
	
	Mesh* mesh_create(uint32_t vertex_count, uint32_t index_count, Vertex vertices[], uint32_t indices[]) { 
		uint32_t vertex_buffer_size = vertex_count * sizeof(vertices[0]);
		uint32_t index_buffer_size = index_count * sizeof(indices[0]);
		
		// The rationale here is that, if we provide vertices at mesh_create, this mesh is probably going to be static throughout the program; otherwise, we assume it will be updated regularly.
		// This doesn't have to be true, and OpenGL guaranteees (https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml) that these are just hints that are only used for performance optimizations within the driver.
		GLenum usage = (vertices == nullptr) ? GL_STREAM_DRAW : GL_STATIC_DRAW;
		
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices, usage);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		Mesh* result = new Mesh; // #memory_cleanup
		result->vbo = vbo;
		result->ibo = ibo;
		result->vertex_count = vertex_count;
		result->index_count = index_count;
		return result;
	}
	
	void mesh_upload(Mesh* mesh, uint32_t vertex_count, Vertex vertices[], uint32_t index_count, uint32_t indices[]) {
		SPARKLES_ASSERT(vertex_count <= mesh->vertex_count);
		SPARKLES_ASSERT(index_count <= mesh->index_count);
		
		uint32_t vertex_buffer_size = vertex_count * sizeof(vertices[0]);
		uint32_t index_buffer_size = index_count * sizeof(indices[0]);
		
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void mesh_render(Mesh* mesh, RenderState* render_state) {
		opengl_apply_render_state(render_state, false);
		
		// Bind the vertex format and mesh buffers
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
		glBindVertexBuffer(0, mesh->vbo, 0, sizeof(Vertex));
		
		// Draw all particles in a single draw call.
		glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, (void*) 0);
		
		opengl_reset_render_state();
		
	}
	
	struct OpenGLTextureFormatInfo {
		GLenum gl_format;
		GLenum gl_internal_format;
		GLenum gl_type;
		GLint gl_swizzle[4];
	};
	
	static OpenGLTextureFormatInfo opengl_get_texture_format_info(TextureFormat format) {
		OpenGLTextureFormatInfo info = {};
		
		info.gl_swizzle[0] = GL_RED;
		info.gl_swizzle[1] = GL_GREEN;
		info.gl_swizzle[2] = GL_BLUE;
		info.gl_swizzle[3] = GL_ALPHA;
		
		
		switch (format) {
		  case TextureFormat::RGBA_UINT8: {
				info.gl_internal_format = GL_RGBA8; 
				info.gl_type = GL_UNSIGNED_BYTE;
				info.gl_format = GL_RGBA;
			} break;
			
		  case TextureFormat::RGBA_FLOAT16: {
				info.gl_internal_format = GL_RGBA16F; 
				info.gl_type = GL_HALF_FLOAT;
				info.gl_format = GL_RGBA;
			} break;
			
		  case TextureFormat::ALPHA_FLOAT32: {
				info.gl_internal_format = GL_R32F; 
				info.gl_type = GL_FLOAT;
				info.gl_format = GL_RED;
				
				info.gl_swizzle[0] = GL_ONE;
				info.gl_swizzle[1] = GL_ONE;
				info.gl_swizzle[2] = GL_ONE;
				info.gl_swizzle[3] = GL_RED;
			} break;
			
		  default: 
			SPARKLES_ASSERT(false);
		}	
		
		return info;
	}
	
	Texture* texture_create(TextureFormat format, uint32_t width, uint32_t height, void* image_data) {
		auto format_info = opengl_get_texture_format_info(format);
		
		GLuint handle;
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexImage2D(GL_TEXTURE_2D, 0, format_info.gl_internal_format, width, height, 0, format_info.gl_format, format_info.gl_type, image_data);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, format_info.gl_swizzle);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		Texture* texture = new Texture; // #memory_cleanup
		texture->handle = handle;
		return texture;
	}

	// Render target
	RenderTarget* render_target_create(TextureFormat format, uint32_t width, uint32_t height) {		
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
		
		auto color_attachment = texture_create(format, width, height, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment->handle, 0);
		
		GLenum completion_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		SPARKLES_ASSERT(completion_status == GL_FRAMEBUFFER_COMPLETE);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		
		RenderTarget* result = new RenderTarget;
		result->fbo = fbo;
		result->color_attachment = color_attachment;
		return result;
	}

	Texture* render_target_flush(RenderTarget* render_target) {
		// In OpenGL, we don't need to synchronize here, but that's not necessarily the case for other Graphics APIs.
		return render_target->color_attachment;
	}
	
	void render_target_clear(RenderTarget* render_target, vec4 color) {
		if (render_target) {
			glBindFramebuffer(GL_FRAMEBUFFER, render_target->fbo);
		} else {	
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
}