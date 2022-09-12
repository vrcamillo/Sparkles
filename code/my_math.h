struct vec2f {
	float x = 0;
	float y = 0;
};

vec2f operator+(vec2f a, vec2f b);
vec2f operator-(vec2f a, vec2f b);
vec2f operator*(vec2f a, vec2f b);
vec2f operator/(vec2f a, vec2f b);

vec2f operator*(vec2f v, float f);
vec2f operator*(float f, vec2f v);

vec2f operator/(vec2f v, float f);

struct vec3f {
	float x = 0;
	float y = 0;
	float z = 0;
};

vec3f operator+(vec3f a, vec3f b);
vec3f operator-(vec3f a, vec3f b);
vec3f operator*(vec3f a, vec3f b);
vec3f operator/(vec3f a, vec3f b);

vec3f operator*(vec3f v, float f);
vec3f operator*(float f, vec3f v);

vec3f operator/(vec3f v, float f);

struct vec4f {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 0;
};

vec4f operator+(vec4f a, vec4f b);
vec4f operator-(vec4f a, vec4f b);
vec4f operator*(vec4f a, vec4f b);
vec4f operator/(vec4f a, vec4f b);

vec4f operator*(vec4f v, float f);
vec4f operator*(float f, vec4f v);

vec4f operator/(vec4f v, float f);
