#include <iostream>
#include <array>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <cgl/utils.h>

constexpr int CLIENT_WIDTH = 500;
constexpr int CLIENT_HEIGHT = 500;
constexpr const char* CLIENT_TITLE = "Conway's Game of Life";

constexpr int TICK_RATE = 500;
constexpr const char* INPUT_PNG_PATH = "input.png";

constexpr const char* COMPUTE_SHADER_PATH = "shader/cgl.comp";
constexpr const char* VERTEX_SHADER_PATH = "shader/cgl.vert";
constexpr const char* FRAGMENT_SHADER_PATH = "shader/cgl.frag";

constexpr std::array<GLfloat, 12> vertices =
{
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
};

constexpr std::array<GLfloat, 8> uvs =
{
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
};

constexpr std::array<GLuint, 6> indices =
{
	0, 1, 2,
	0, 2, 3
};

int main()
{
	CGL_ASSERT(glfwInit());

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	auto window = glfwCreateWindow(CLIENT_WIDTH, CLIENT_HEIGHT, CLIENT_TITLE, nullptr, nullptr);
	CGL_ASSERT(window);

	glfwMakeContextCurrent(window);
	CGL_ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

	glfwSwapInterval(0);

	// PRE-WINDOW INITIALISATION

	pixel_grid grid(INPUT_PNG_PATH);

	shader_program program_svg_compute;
	program_svg_compute.load_shader(GL_COMPUTE_SHADER, COMPUTE_SHADER_PATH);
	program_svg_compute.link();

	shader_program program_svg_render;
	program_svg_render.load_shader(GL_VERTEX_SHADER, VERTEX_SHADER_PATH);
	program_svg_render.load_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_PATH);
	program_svg_render.link();

	texture texture_screen{ GL_TEXTURE_2D };
	texture_screen.params(
		{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		}
	);

	glActiveTexture(GL_TEXTURE0);
	texture_screen.storage(1, GL_RGBA32F, grid.width, grid.height, 0);
	texture_screen.sub_image(0, 0, 0, grid.width, grid.height, GL_RGBA, GL_FLOAT, grid.data.data());
	texture_screen.bind_image_texture(0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// texture screen output

	texture texture_screen_out{ GL_TEXTURE_2D };
	texture_screen_out.params(
		{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
		}
	);

	glActiveTexture(GL_TEXTURE1);
	texture_screen_out.storage(1, GL_RGBA32F, grid.width, grid.height, 0);
	texture_screen_out.bind_image_texture(1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	buffer_object vao;
	vao.bind();

	buffer vrt_buf(vertices, 0, 3);
	buffer uvs_buf(uvs, 1, 2);
	buffer ind_buf(indices);

	vao.unbind();

	int ticks = 0;

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		// compute

		if (ticks % TICK_RATE == 0)
		{
			program_svg_compute.bind();
			glDispatchCompute(ceil(grid.width / 8), ceil(grid.height / 4), 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glCopyImageSubData(texture_screen_out.handle, GL_TEXTURE_2D, 0, 0, 0, 0, texture_screen.handle, GL_TEXTURE_2D, 0, 0, 0, 0, grid.width, grid.height, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			program_svg_compute.unbind();
		}

		// render

		program_svg_render.bind();

		vao.bind();

		glActiveTexture(GL_TEXTURE0);
		texture_screen_out.bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		vao.unbind();

		program_svg_render.unbind();

		// prep next frame

		ticks++;

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	program_svg_compute.destroy();
	program_svg_render.destroy();
}