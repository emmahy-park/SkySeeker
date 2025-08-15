#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "../tinyECS/components.hpp"
#include "../tinyECS/tiny_ecs.hpp"

#include <map>
#include "util/screen_manager.hpp"
#include "upgrade_system.hpp"

// imgui
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_glfw.h"
#include "../ext/imgui/imgui_impl_opengl3.h"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game

class ScreenManager;
class UpgradeSystem;

class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
		// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("invaders/blue_1.png"),
		textures_path("towers/tower01.png"),
		textures_path("projectiles/gold_bubble.png"),
		textures_path("startscreen/startscreen.png"),
		textures_path("effects/light_map.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("vignette"),
		shader_path("font"),
		shader_path("particle"),
		shader_path("shadowMap"),
		shader_path("shadow")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

	struct Character {
		unsigned int TextureID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		unsigned int Advance;
		char character;
	};

	GLuint textured_quad_vao;
	GLuint textured_quad_vbo;
	GLuint quad_texture;
	GLuint textured_quad_shader;

	ImFont* bigFont;
	ImFont* medFont;
	ImFont* smallFont;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	void initializeShadows();

	void loadGlTextures(GLuint* handles, const std::string* texture_paths, size_t num_textures);
	
	void unloadMapTilesets(GLuint* handles, int num_handles);

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	GLuint getTextureHandle(TEXTURE_ASSET_ID id) { return texture_gl_handles[(int)id]; };

	void initializeGlGeometryBuffers();

	bool fontInit(GLFWwindow* window);
	bool initImGui(GLFWwindow* window);
	void shutdown(GLFWwindow* window);

	bool particleInit();
	bool initTexturedQuad();

	void drawStartScreen(ScreenManager& screenManager);
	void drawPausedScreen(ScreenManager& screenManager);
	void drawCreditsScreen(ScreenManager& screenManager);
	void drawUpgradeScreen(ScreenManager& screenManager, UpgradeSystem& upgrade_system);
	void drawMenu(ScreenManager& screenManager);
	void drawDeathScreen(ScreenManager& screenManager);

	void renderText(std::string text, glm::vec2 position, glm::vec3 color, float scale, float maxWidth, std::string fontName);

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix(vec2 position);

	Entity get_screen_state_entity() { return screen_state_entity; }

	float getCenteredX(const std::string& text, float scale, std::string fontName);

	void drawItemPopupScreen(Entity& item_entity);
	void drawItemStat(Entity& item_entity);

private:
	// Internal drawing functions for each entity type
	void drawParticles(const mat3& projection, GLuint texture_id, int num_particles);
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void drawInventoryScreen();
	void createShadowMap(std::vector<Entity> entities, const mat3& projection);
	void updateShadowMap();
	void drawLight(Entity light_entity, const mat3& projection);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint shadowMapFBO;
	GLuint lightShadowFBO;
	GLuint lightShadowMap;
	GLuint shadowMap;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	// font elements
	std::map<std::string, std::map<char, Character>> m_fonts;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

	bool isUpgradeMenuOpen = false;
	int selectedIcon;
	
	struct NodeData {
		bool not_enough_souls = false;
		bool just_purchased = false;
	};

	std::unordered_map<std::string, NodeData> node_data;

	std::vector<std::string> iconFiles = {
		textures_path("player/player_icon.png"),
		textures_path("items/placeholder_sword.png")
	};

	std::vector<std::string> iconNames = {
		"Player",
		"Parry"
	};
	GLuint particle_VAO;
	GLuint particle_transform_VBO;

	std::string item_name_text = "";
	std::string item_description_text = "";
	std::string item_bonus_text = "";

};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

void getParticleTransforms(std::vector<mat3>& transforms, GLuint texture_id);
