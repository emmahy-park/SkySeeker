
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "../ext/glm/glm/ext/matrix_clip_space.hpp"
#include "../ext/glm/glm/gtc/type_ptr.hpp"

// imgui
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_glfw.h"
#include "../ext/imgui/imgui_impl_opengl3.h"

#include "item_system.hpp"
#include "../util/util.hpp"
#include "world_system.hpp"

// Creative Component: Particle System
void RenderSystem::drawParticles(const mat3 &projection, GLuint texture_id, int num_particles) {
	const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::PARTICLE];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// texture-mapped entities - use data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	GLint in_transform_loc = glGetAttribLocation(program, "in_transform");

	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	glBindBuffer(GL_ARRAY_BUFFER, particle_transform_VBO);
	std::vector<mat3> transform;
	getParticleTransforms(transform, texture_id);
	glBufferData(GL_ARRAY_BUFFER, transform.size() * sizeof(glm::mat3), transform.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(in_transform_loc);
	glVertexAttribPointer(in_transform_loc, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)0);
	glEnableVertexAttribArray(in_transform_loc + 1);
	glVertexAttribPointer(in_transform_loc + 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(1 * sizeof(vec3)));
	glEnableVertexAttribArray(in_transform_loc + 2);
	glVertexAttribPointer(in_transform_loc + 2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(vec3)));
	gl_has_errors();

	glVertexAttribDivisor(in_position_loc, 0);
	glVertexAttribDivisor(in_texcoord_loc, 0);
	glVertexAttribDivisor(in_transform_loc, 1);
	glVertexAttribDivisor(in_transform_loc + 1, 1);
	glVertexAttribDivisor(in_transform_loc + 2, 1);
	gl_has_errors();

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();


	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, num_particles);

	gl_has_errors();
}

void getParticleTransforms(std::vector<mat3>& transforms, GLuint texture_id) {
	for (Entity e : registry.particles.entities) {
		TextureInfo texture_info = registry.textureinfos.get(e);
		if (texture_info.texture_id == texture_id) {
			Position& position = registry.positions.get(e);
			Transform transform;
			transform.translate(position.position);
			transform.scale(position.scale);
			transform.rotate(radians(position.angle));
			transforms.push_back(transform.mat);
		}
	}
}



void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{

	Position &position = registry.positions.get(entity);
	TextureInfo& info = registry.textureinfos.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(position.position);
	transform.scale(position.scale);
	transform.rotate(radians(position.angle));

	const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::TEXTURED];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	GLint tiletexcoord_loc = glGetUniformLocation(program, "tiletexcoord");
	GLint u_time_loc = glGetUniformLocation(program, "u_time"); 
	GLint u_damaged_loc = glGetUniformLocation(program, "u_damaged");
	GLint u_health_loc = glGetUniformLocation(program, "u_health");
	GLint u_ishealthbar_loc = glGetUniformLocation(program, "u_ishealthbar");
	GLint u_dashed_loc = glGetUniformLocation(program, "u_dashed");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							sizeof(TexturedVertex), (void *)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void *)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position
	
	// [1] Tile coordinates to get area of texture we want to use
	vec4 tiletexcoord = {info.top_left, info.bottom_right};
	glUniform4f(tiletexcoord_loc, tiletexcoord.x, tiletexcoord.y, tiletexcoord.z, tiletexcoord.w);
	gl_has_errors();

	// M1 Creative Element: Simple rendering effects (flashing effect on attacked)
	glUniform1f(u_time_loc, glfwGetTime()); // pass a time value
	bool u_damaged = GL_FALSE;
	bool u_dashed = GL_FALSE;
	if (registry.livings.has(entity)) {
		Living& living = registry.livings.get(entity);
		u_damaged = living.invincible_time > 0 ? GL_TRUE : GL_FALSE;
	}
	if (registry.players.has(entity)){
		Player& player = registry.players.get(entity);
		u_dashed = player.current_state == PLAYER_DASHING ? GL_TRUE : GL_FALSE;		
	}
	glUniform1f(u_damaged_loc, u_damaged);

	// Healthbar
	bool u_ishealthbar = false;
	float u_health = 1;

	if (registry.healthbar.has(entity)) {
		HealthBar& health = registry.healthbar.get(entity);
		Player& player = registry.players.components[0];
		u_health = health.health / player.base_max_health;
		u_ishealthbar = true;
	}
	glUniform1f(u_health_loc, u_health);
	glUniform1f(u_ishealthbar_loc, u_ishealthbar);
	glUniform1f(u_dashed_loc, u_dashed);
	
	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	// [1] Texture ID = Texture Handle in OpenGL
	glBindTexture(GL_TEXTURE_2D, info.texture_id);
	gl_has_errors();


	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen()
{

	// Setting shaders
	// get the vignette texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// add the "vignette" effect
	const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];

	// set clock
	GLuint time_uloc       = glGetUniformLocation(vignette_program, "time");
	GLuint paused_uloc = glGetUniformLocation(vignette_program, "darken_screen_factor");
	GLuint shadow_texture_uloc = glGetUniformLocation(vignette_program, "shadow_texture");

	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	glUniform1i(shadow_texture_uloc, 1);
	
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	// std::cout << "screen.darken_screen_factor: " << screen.darken_screen_factor << " entity id: " << screen_state_entity << std::endl;
	float darken_factor = screen.is_paused || screen.is_death ? 0.1f : screen.darken_screen_factor;
	glUniform1f(paused_uloc, darken_factor);
	// if (screen.is_paused) {
	// 	glUniform1f(paused_uloc, 0.1f);
	// } else {
	// 	glUniform1f(paused_uloc, screen.darken_screen_factor);
	// }
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	glActiveTexture(GL_TEXTURE1);

	glBindTexture(GL_TEXTURE_2D, shadowMap);
	gl_has_errors();


	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();

	// Render text after drawing sprites
	for (Entity entity : registry.fonts.entities) {
		Font& font = registry.fonts.get(entity);
		renderText(font.text, font.position, font.color, font.scale, font.maxWidth, font.fontName);
	}
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{

	mat3 projection_2D = createProjectionMatrix(registry.screenStates.components[0].camera_position);

	// draw all entities with a render request to the frame buffer
	std::vector<Entity> entities = registry.positions.entities;
	std::sort(entities.begin(), entities.end(),
		[](Entity e, Entity other) {
			return registry.positions.get(e).layer < registry.positions.get(other).layer;
		});


	// First get the shadow map/s
	//getShadowMaps(entities, projection_2D);
	int count = 0;
	// Shadows every 3 frames
	if (count % 3 == 0) {
		createShadowMap(entities, projection_2D);
	}

	registry.fonts.clear();
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	// white background
	glClearColor(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();

	for (Entity entity : entities)
	{
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		if (registry.textureinfos.has(entity) && !registry.particles.has(entity)) {
			drawTexturedMesh(entity, projection_2D);
		}
		
	}

	// Creative Component: Particle System
	std::vector<Entity> particle_entities = registry.particles.entities;
	std::sort(particle_entities.begin(), particle_entities.end(),
		[](Entity e, Entity other) {
			return registry.textureinfos.get(e).texture_id < registry.textureinfos.get(other).texture_id;
		});

	if (registry.particles.size() > 0) {
		int num_particles = 0;
		GLuint id_to_compare = registry.textureinfos.get(particle_entities[0]).texture_id;
		for (Entity e : particle_entities) {
			if (registry.textureinfos.get(e).texture_id == id_to_compare) {
				num_particles += 1;
			}
			else {
				drawParticles(projection_2D, id_to_compare, num_particles);
				id_to_compare = registry.textureinfos.get(e).texture_id;
				num_particles = 1;
			}
		}
		drawParticles(projection_2D, id_to_compare, num_particles);
	}


	// [5] Improved Gameplay: Gameplay Tutorial
	// Only in Tutorial
	ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
	if (screen_state.tutorialActive) {
		std::string instruction;
		switch (screen_state.tutorialStep)
		{
		case 0:
			instruction = "Press W to move up \n\nExit through the door to skip tutorial";
			break;
		case 1:
			instruction = "Press A to move left \n\nExit through the door to skip tutorial";
			break;
		case 2:
			instruction = "Press D to move right \n\nExit through the door to skip tutorial";
			break;
		case 3:
			instruction = "Press S to move down \n\nExit through the door to skip tutorial";
			break;
		case 4:
			instruction = "Press W/A/D/S then SPACE BAR to dash \n\nExit through the door to skip tutorial";
			break;
		case 5:
			instruction = "Click the left mouse button to perform a melee attack \n\nExit through the door to skip tutorial";
			break;
		case 6:
			instruction = "Try to defeat the enemy using a melee attack";
			break;
		case 7:
			instruction = "Press Q or E to parry \n\nExit through the door to skip tutorial";
			break;
		case 8:
			instruction = "Click the right mouse button to perform a ranged attack \n\nExit through the door to skip tutorial";
			break;
		case 9:
			instruction = "Hold the right mouse button to charge the ranged attack \n\nExit through the door to skip tutorial";
			break;
		case 10:
			instruction = "Try to defeat the enemy using a ranged attack";
			break;
		case 11:
			instruction = "Walk over to an item and press F to pick it up! \nYou can only pick up one item per room \n\nExit through the door to skip tutorial";
			break;
		case 12:
			instruction = "Press I to open the inventory UI and see the item you picked up! \n\nExit through the door to skip tutorial";
			break;
		case 13:
			instruction = "Press P to pause the game \nYou can resume / save and quit in the paused screen \n\nExit through the door to skip tutorial";
			break;
		case 14:
			// see drawUpgradeScreen
			break;
		case 15:
			instruction = "Walk to the exit to go back to the menu!";
			break;
		}

		Entity font_title = Entity();
		Font& tutorialTitle = registry.fonts.emplace(font_title);
		tutorialTitle.text = "Tutorial";
		tutorialTitle.position = vec2(getCenteredX(tutorialTitle.text, 1.5f, "KnightWarrior"), 600);
		tutorialTitle.color = vec3(1.f);
		tutorialTitle.scale = 1.5f;
		tutorialTitle.maxWidth = 600.f;
		tutorialTitle.fontName = "KnightWarrior";

		Entity tutorialText = Entity();
		Font& tutorialInstructions = registry.fonts.emplace(tutorialText);
		tutorialInstructions.text = instruction;
		tutorialInstructions.position = vec2(40, 500);
		tutorialInstructions.color = vec3(1.f);
		tutorialInstructions.scale = 0.5f;
		tutorialInstructions.maxWidth = 800.f;
	}

	// draw framebuffer to screen
	// adding "vignette" effect when applied
	drawToScreen();

	// M4 Creative Component: External Integration
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Inventory UI in play screen
	if (!screen_state.isInventoryClosed && !screen_state.tutorialActive) {
		drawInventoryScreen();
	}

	// Inventory UI in tutorial screen only available for step 13
	if (!screen_state.isInventoryClosed && screen_state.tutorialActive && screen_state.tutorialStep == 13) {
		drawInventoryScreen();
	}

	// For New Feature: Item Stat Block
	if (WorldSystem::is_itempopup_visible) {
		// Set window size
		ImGui::SetNextWindowSize(ImVec2(200, 200));
		ImGui::SetNextWindowPos(ImVec2(550, 300));

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

		ImGui::Begin("Item Details", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		ImGui::PushFont(smallFont);
		ImGui::Text("Item: %s", item_name_text.c_str());
		ImGui::Spacing();
		ImGui::TextWrapped("%s", item_description_text.c_str());
		ImGui::Spacing();
		ImGui::Text("Bonuses:\n%s", item_bonus_text.c_str());
		ImGui::PopFont();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// For New Feature: Item Stat Block
void RenderSystem::drawItemStat(Entity& item_entity) {
	Item& items = registry.items.get(item_entity);
	std::string bonuses;
	ItemInfo& iteminfo = items.info;
	Util::get_bonus_string_from_items(bonuses, iteminfo);
	item_name_text = items.info.item_name;
	item_description_text = items.info.description;
	item_bonus_text = bonuses;
}

mat3 RenderSystem::createProjectionMatrix(vec2 position)
{
	// fake projection matrix, scaled to window coordinates
	float left   = position.x - WINDOW_WIDTH_PX / 2;
	float top    = position.y - WINDOW_HEIGHT_PX/ 2;
	float right  = (float) position.x + WINDOW_WIDTH_PX / 2;
	float bottom = (float) position.y + WINDOW_HEIGHT_PX / 2;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}

// Start Screen: render by simply clearing the screen and drawing text directly to the screen
void RenderSystem::drawStartScreen(ScreenManager& screenManager) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // default framebuffer: rendering goes to the screen

	glClearColor(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
								// and alpha blending, one would have to sort
								// sprites back to front
	gl_has_errors();

	GLuint textured_program = effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
	glUseProgram(textured_program);

	glUniform1f(glGetUniformLocation(textured_program, "u_time"), 0.0f);
	glUniform1f(glGetUniformLocation(textured_program, "u_damaged"), 0.0f);
	glUniform1f(glGetUniformLocation(textured_program, "u_health"), 0.0f);
	glUniform1f(glGetUniformLocation(textured_program, "u_ishealthbar"), 0.0f);
	glUniform1f(glGetUniformLocation(textured_program, "u_dashed"), 0.0f);

	glUniform4f(glGetUniformLocation(textured_program, "tiletexcoord"), 0.0f, 0.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(textured_program, "fcolor"), 1.0f, 1.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, quad_texture);

	Transform transform;
	transform.translate({WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});
	transform.scale({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});

	mat3 mat = transform.mat;
	mat3 projection = createProjectionMatrix({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});

	glUniformMatrix3fv(glGetUniformLocation(textured_program, "transform"), 1, GL_FALSE, (float*)&mat);
	glUniformMatrix3fv(glGetUniformLocation(textured_program, "projection"), 1, GL_FALSE, (float*)&projection);

	glBindVertexArray(textured_quad_vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
	glBindVertexArray(0);

	float centeredX = getCenteredX("SKY SEEKER", 1.5f, "KnightWarrior");
	renderText("SKY SEEKER", glm::vec2(centeredX, 600), glm::vec3(1.0f), 1.5f, 600.f, "KnightWarrior");
	drawMenu(screenManager);

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Game Pause Screen: render by creating font entities and using drawToScreen
void RenderSystem::drawPausedScreen(ScreenManager& screenManager) {
	registry.fonts.clear();

	Entity pauseText = Entity();
	Font& pausedScreenTitle = registry.fonts.emplace(pauseText);
	pausedScreenTitle.text = "Game Paused";
	pausedScreenTitle.position = vec2(getCenteredX(pausedScreenTitle.text, 1.5f, "KnightWarrior"), 600);
	pausedScreenTitle.color = vec3(1.f);
	pausedScreenTitle.scale = 1.5f;
	pausedScreenTitle.maxWidth = 800.f;
	pausedScreenTitle.fontName = "KnightWarrior";

	drawMenu(screenManager);
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Credits Screen: render by simply clearing the screen and drawing text directly to the screen
void RenderSystem::drawCreditsScreen(ScreenManager& screenManager) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // default framebuffer: rendering goes to the screen

	glClearColor(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
								// and alpha blending, one would have to sort
								// sprites back to front
	gl_has_errors();

	float centeredX = getCenteredX("CREDITS", 1.5f, "KnightWarrior");
	renderText("CREDITS", glm::vec2(centeredX, 600), glm::vec3(1.0f), 1.5f, 600.f, "KnightWarrior");
	centeredX = getCenteredX("Cecillia", 1.0f, "KnightWarrior");
	renderText("Benson", glm::vec2(centeredX- 200, 420), glm::vec3(1.0f), 1.0f, 600.f, "KnightWarrior");
	renderText("Cecillia", glm::vec2(centeredX, 420), glm::vec3(1.0f), 1.0f, 600.f, "KnightWarrior");
	renderText("Elton", glm::vec2(centeredX + 230, 420), glm::vec3(1.0f), 1.0f, 600.f, "KnightWarrior");
	centeredX = getCenteredX("Stephane", 1.0f, "KnightWarrior");
	renderText("Emma", glm::vec2(centeredX - 180, 270), glm::vec3(1.0f), 1.0f, 600.f, "KnightWarrior");
	renderText("Stephane", glm::vec2(centeredX, 270), glm::vec3(1.0f), 1.0f, 600.f, "KnightWarrior");
	renderText("Back", glm::vec2(centeredX + 400, 100), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 600.f, "KnightWarrior");

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// M4 Creative Component: External Integration
void RenderSystem::drawInventoryScreen() {

	Player& player = registry.players.components[0];

	// Set window size
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, 100));
	ImGui::SetNextWindowPos(ImVec2(0, (WINDOW_HEIGHT_PX-100)));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

	Entity gamestate_entity = registry.gameStates.entities[0];
	GameState& gamestate = registry.gameStates.get(gamestate_entity);
	int souls = static_cast<int>(gamestate.souls);

	ImGui::Begin("Inventory", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	// Display soul count
	ImGui::Text("Souls: %d", souls);
	ImGui::Separator();

	// Top row with inventory items
	ImGui::Columns(8, "top_row", false);
	for (int i = 0; i < player.inventory.size(); i++) {
		if (player.inventory[i].item_name != "souls") {
			ImGui::PushID(i);
			GLuint item_texture = item_handles[player.inventory[i].texture_name];
			ImGui::Image(item_texture, ImVec2(30, 30));
			ImGui::PushFont(smallFont);
			ImGui::TextWrapped("%s x%d", player.inventory[i].item_name.c_str(), player.inventory[i].quantity);
			ImGui::PopID();
			ImGui::PopFont();
			ImGui::NextColumn();
		}
		
	}
	ImGui::Columns(1); // Reset columns

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::End();
}

void RenderSystem::drawUpgradeScreen(ScreenManager& screenManager, UpgradeSystem& upgrade_system) {
	GLuint handle;
	std::vector<GLuint> iconTextures(iconFiles.size());

	for (int i = 0; i < iconFiles.size(); ++i) {
		loadGlTextures(&handle, &iconFiles[i], 1);
		iconTextures[i] = handle;
	}

	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Set window size
	ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(BACKGROUND_COLOUR.r, BACKGROUND_COLOUR.g, BACKGROUND_COLOUR.b, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

	Entity gamestate_entity = registry.gameStates.entities[0];
	GameState& gamestate = registry.gameStates.get(gamestate_entity);
	int souls = static_cast<int>(gamestate.souls);

	ImGui::PushFont(bigFont);
	ImGui::Begin("UPGRADE SYSTEM", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::PopFont();

	// Display soul count
	ImGui::Text("Souls: %d", souls);
	ImGui::Separator();

	// Top row with 2 clickable icons
	ImGui::Columns(iconNames.size(), "top_row", false);
	for (int i = 0; i < iconNames.size(); i++) {
		ImGui::PushID(i);

		ImTextureID iconTexture = iconTextures[i];

		ImVec4 buttonColor = ImVec4(0.35f, 0.40f, 0.43f, 1.0f);
		ImVec4 buttonHovered = ImVec4(0.40f, 0.45f, 0.48f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHovered);

		ImGui::ImageButton("icons", iconTexture, ImVec2(60, 60));

		if (ImGui::IsItemClicked()) {
			// std::cout << "Icon " << i << " clicked\n";
			if (selectedIcon == i && isUpgradeMenuOpen) isUpgradeMenuOpen = false;

			else {
				isUpgradeMenuOpen = true;
				selectedIcon = i;
			}

			node_data.clear();

		} else {
			ImGui::Text("%s", iconNames[i].c_str());
		}

		ImGui::PopID();
		ImGui::NextColumn();
		ImGui::PopStyleColor(2);
	}
	ImGui::Columns(1); // Reset columns
	ImGui::Separator();

	if (isUpgradeMenuOpen && selectedIcon >= 0 && selectedIcon < iconNames.size()) {
		ImGui::Text("Upgrade Menu");

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 startPos = ImVec2(450.f, 250.f);

		ImVec4 buttonColor = ImVec4(0.16f, 0.22f, 0.27f, 1.0f);
		ImVec4 buttonHovered = ImVec4(0.2f, 0.28f, 0.34f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHovered);

		if (ImGui::TreeNode("Item Upgrades")) {
			UpgradeTree tree = upgrade_system.get_upgrade_tree(upgrade_system.get_upgrade_name(selectedIcon));
			std::vector<UpgradeInfo> current_upgrades;
			upgrade_system.get_current_upgrades(current_upgrades);

			for (auto& iter : tree.upgrades) {
				UpgradeInfo upgrade = iter.second;
				std::string upgrade_name = upgrade.upgrade_name;

				NodeData& data = node_data[upgrade_name];

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.12f, 0.15f, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));

				ImGui::BeginChild(upgrade.upgrade_name.c_str(), ImVec2(0, 180), true);

				ImVec4 titleColor = ImVec4(1.0f, 0.85f, 0.3f, 1.0f);
				ImGui::TextColored(titleColor, "%s", upgrade.upgrade_name.c_str());

				ImGui::TextWrapped("%s", upgrade.description.c_str());

				std::string bonuses;
				Util::get_bonus_string_from_upgrade(bonuses, upgrade);
				// ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Bonuses:\n%s", bonuses.c_str());
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.84f, 1.0f), "Bonuses:\n%s", bonuses.c_str());

				ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "Soulds Required: %d", upgrade.cost);

				if (!upgrade_system.check_prerequisites(upgrade)) {
					ImGui::Text("Requires:");
					for (std::string prereq: upgrade.prerequisite_upgrades) {
						ImGui::BulletText("%s", prereq.c_str());
					}
				} else if (std::find_if(current_upgrades.begin(), current_upgrades.end(), 
				[&upgrade_name](const UpgradeInfo ui) { return upgrade_name == ui.upgrade_name; }) != current_upgrades.end()) {
					if (data.just_purchased) {
						ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Purchased!");
					}
					else {
						ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Already Purchased!");
					}
					
				}
				else if (ImGui::Button("Upgrade", ImVec2(100, 30))) {
					if (screenManager.getCurrentScreen() != ScreenType::TutorialUpgradeScreen) {
						if (!upgrade_system.try_buy_upgrade(upgrade)) {
							data.not_enough_souls = true;
						}
						else {
							data.just_purchased = true;
						}
					}
				}

				if (data.not_enough_souls) {
					ImGui::TextColored(ImVec4(1.0f, 0.43f, 0.38f, 1.0f), "Not enough souls!");
				}

				ImGui::EndChild();
				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor();
			}

			ImGui::TreePop();
		}

		ImGui::PopStyleColor(2);
	}

	// Backbutton
	ImGui::Separator();

	ImVec4 buttonColor = ImVec4(0.35f, 0.40f, 0.43f, 1.0f);
	ImVec4 buttonHovered = ImVec4(0.40f, 0.45f, 0.48f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHovered);

	if (screenManager.getCurrentScreen() != ScreenType::TutorialUpgradeScreen) {
		ImGui::Button("Back to Main Menu", ImVec2(200, 50));
		if (ImGui::IsItemClicked()) {
			screenManager.setScreen(ScreenType::StartScreen);
			isUpgradeMenuOpen = false;
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(5);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (screenManager.getCurrentScreen() == ScreenType::TutorialUpgradeScreen) {
		std::string instruction = "Upgrade System can be accessed from the Start Screen";
		renderText(instruction, vec2(getCenteredX(instruction, 0.5f, "KenneyPixel"), 360), vec3(1.f), 0.5, 800.f, "KenneyPixel");
		instruction = "You can purchase upgrades using collected souls";
		renderText(instruction, vec2(getCenteredX(instruction, 0.5f, "KenneyPixel"), 330), vec3(1.f), 0.5, 800.f, "KenneyPixel");
		instruction = "Upgrade buttons are disabled in the tutorial";
		renderText(instruction, vec2(getCenteredX(instruction, 0.5f, "KenneyPixel"), 300), vec3(1.f), 0.5, 800.f, "KenneyPixel");

		instruction = "Press Enter to resume the tutorial";
		renderText(instruction, vec2(getCenteredX(instruction, 1.0f, "KnightWarrior"), 200), vec3(1.0f, 0.0f, 0.0f), 1.0, 900.f, "KnightWarrior");
	}

	glfwSwapBuffers(window);
}

// Menu Screen:
// For Start menu: directly render text to the screen
// For Paused menu: create font entity and render text using drawToScreen
void RenderSystem::drawMenu(ScreenManager& screenManager) {
	std::vector<std::string> menuItems = screenManager.getMenuItems();
	
	glm::vec2 startPos = {100, 300}; // Start position for first item
	float spacing = -50.0f; // Space between menu items
	float menuItemHeight = 48.0f;
	float menuItemWidth = 200.0f;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	mouseY = WINDOW_HEIGHT_PX - mouseY;
	
	for (int i = 0; i < menuItems.size(); i++) {

		glm::vec2 menuItemPosition = {startPos.x, startPos.y + i * spacing};
		
		bool isHovered = (mouseX >= menuItemPosition.x && mouseX <= menuItemPosition.x + menuItemWidth &&
						  mouseY >= menuItemPosition.y && mouseY <= menuItemPosition.y + menuItemHeight);

		//glm::vec3 color = (i == screenManager.getSelectedIndex()) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f); // Red if selected
		//glm::vec3 color = isHovered ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f);
		glm::vec3 color = (i == screenManager.getSelectedIndex() || isHovered) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f);

		if (screenManager.getCurrentScreen() == ScreenType::StartScreen) {
			renderText(menuItems[i], {startPos.x, startPos.y + i * spacing}, color, 1.0f, 600.f, "KnightWarrior");
		}
		 else {
			Entity pauseMenu = Entity();
			Font& pausedScreenMenu = registry.fonts.emplace(pauseMenu);
			pausedScreenMenu.text = menuItems[i];
			pausedScreenMenu.position = {startPos.x, startPos.y + i * spacing};
			pausedScreenMenu.color = color;
			pausedScreenMenu.scale = 1.0f;
			pausedScreenMenu.maxWidth = 900.f;
			pausedScreenMenu.fontName = "KnightWarrior";
		}
	}
}

// Game Death Screen: render by creating font entities and using drawToScreen
void RenderSystem::drawDeathScreen(ScreenManager& screenManager) {
	registry.fonts.clear();

	Entity deathText = Entity();
	Font& deathScreenTitle = registry.fonts.emplace(deathText);
	deathScreenTitle.text = "You Died";
	deathScreenTitle.position = vec2(getCenteredX(deathScreenTitle.text, 1.5f, "KnightWarrior"), 600);
	deathScreenTitle.color = vec3(1.f);
	deathScreenTitle.scale = 1.5f;
	deathScreenTitle.maxWidth = 800.f;
	deathScreenTitle.fontName = "KnightWarrior";

	drawMenu(screenManager);
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Render text function from FreeType
void RenderSystem::renderText(std::string text, glm::vec2 position, glm::vec3 color, float scale, float maxWidth, std::string fontName) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FONT]);
	glUniform3f(glGetUniformLocation(effects[(GLuint)EFFECT_ASSET_ID::FONT], "textColor"), color.x, color.y, color.z);

	glm::mat4 projection = glm::ortho(0.0f, 980.0f, 0.0f, 720.0f);
	glUniformMatrix4fv(glGetUniformLocation(effects[(GLuint)EFFECT_ASSET_ID::FONT], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(m_font_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);

	float startX = position.x;
	float lineHeight = 48.0f * scale;

	auto it = m_fonts.find(fontName);
	if (it == m_fonts.end()) {
		std::cerr << "ERROR: Font '" << fontName << "' not found in loaded fonts." << std::endl;
		return;
	}

	const std::map<char, Character>& characterMap = it->second;

	// iterate through all characters
    for (char c : text)
    {
		if (c == '\n') {
			position.x = startX;
			position.y -= lineHeight;
			continue;
		}

		if (characterMap.find(c) == characterMap.end()) {
			std::cerr << "Warning: Character '" << c << "' not found in font atlas." << std::endl;
			continue;
		}

        Character ch = characterMap.find(c)->second;

		if (ch.TextureID == 0) { 
			std::cerr << "Error: TextureID for character '" << c << "' is invalid." << std::endl;
			continue;
		}

		float xpos = position.x + ch.Bearing.x * scale;
		float ypos = position.y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		if (position.x + w > startX + maxWidth) {
			position.x = startX;
			position.y -= lineHeight;
			xpos = position.x + ch.Bearing.x * scale;
			ypos = position.y - (ch.Size.y - ch.Bearing.y) * scale;
		}
		
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{xpos, ypos, 0.0f, 1.0f}, // bottom-left
			{xpos, ypos + h, 0.0f, 0.0f}, // top-left
			{xpos + w, ypos + h, 1.0f, 0.0f}, // top-right

			{xpos, ypos, 0.0f, 1.0f}, // bottom-left
			{xpos + w, ypos + h, 1.0f, 0.0f}, //top-right
			{xpos + w, ypos, 1.0f, 1.0f}, //bottom-right
		};
	   
		glActiveTexture(GL_TEXTURE0);
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2*sizeof(GLfloat)));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		position.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

		if (xpos + w > 980 || ypos + h > 720) {
			std::cerr << "WARNING: Character '" << c << "' is outside screen bounds." << std::endl;
		}
	}
}

// Find center of the text
float RenderSystem::getCenteredX(const std::string& text, float scale, std::string fontName) {
	float width = 0.0f;

	const std::map<char, Character>& characterMap = m_fonts.find(fontName)->second;

	for (char c : text) {

		if (characterMap.find(c) != characterMap.end()) {
			width += (characterMap.find(c)->second.Advance >> 6) * scale;
		}
	}

	return (WINDOW_WIDTH_PX - width) / 2.0f;
}

void RenderSystem::shutdown(GLFWwindow* window) {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


// M4 Creative Component: Dynamic shadows
void RenderSystem::createShadowMap(std::vector<Entity> entities, const mat3& projection) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClearColor(0.f, 0.f, 0.f, 0.0f);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (Entity lightSource : registry.lightSources.entities) {

		glBindFramebuffer(GL_FRAMEBUFFER, lightShadowFBO);
		glClearColor(0.f, 0.f, 0.f, 0.0f);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawLight(lightSource, projection);

		LightSource& ls = registry.lightSources.get(lightSource);
		vec2 lightPosition = vec2(registry.positions.get(lightSource).position);

		const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::SHADOW];

		// Setup program
		glUseProgram(program);
		gl_has_errors();

		GLuint vbo;
		GLuint ibo;

		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ibo);


		// Setting vertex and index buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		gl_has_errors();

		// texture-mapped entities - use data location as in the vertex buffer
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(vec3), (void*)0);
		gl_has_errors();


		GLint currProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(currProgram, "transform");

		GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
		gl_has_errors();

		GLuint in_light_position_loc = glGetUniformLocation(currProgram, "in_light_position");
		glUniform2f(in_light_position_loc, lightPosition.x, lightPosition.y);
		gl_has_errors();


		// Reuse program for all entities
		for (Entity entity : entities) {
			if (!((registry.mapTiles.has(entity) && registry.collidables.has(entity)) 
				|| registry.projectiles.has(entity)) 
				|| entity == lightSource) {
				continue;
			}


			Position p = registry.positions.get(entity);

			if (glm::distance(p.position, lightPosition) > ls.radius + max(p.scale.x, p.scale.y)) {
				continue;
			}

			vec2 scale = p.scale;
			if (registry.collidables.has(entity)) {
				scale = registry.collidables.get(entity).scale;
				if (scale == vec2(0, 0)) scale = p.scale;
			}

			// Transformation code, see Rendering and Transformation in the template
			// specification for more info Incrementally updates transformation matrix,
			// thus ORDER IS IMPORTANT
			Transform transform;
			transform.translate(p.position);
			transform.scale(scale);
			transform.rotate(radians(p.angle));

			glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
			gl_has_errors();

			// Do 4 different draws:

			std::vector<vec3> vertices;
			std::vector<uint16_t> indices;


			// If the light is on the right side...
			if (lightPosition.x > (p.position.x - scale.x/2.f) || registry.mapTiles.has(entity)) {
				// LEFT
				vertices = {
					{-0.5, -0.5, 1.0},
					{-0.5, -0.5, 0.0},
					{-0.5, 0.5, 1.0},
					{-0.5, 0.5, 0.0},
				};

				indices = {
					0, 1, 2, 2, 1, 3
				};

				glBufferData(GL_ARRAY_BUFFER,
					sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				// Drawing of num_indices/3 triangles specified in the index buffer
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
				gl_has_errors();
			}



			// If the light is on the top side...
			if (lightPosition.y < (p.position.y + scale.y / 2.f) || registry.mapTiles.has(entity)) {
				// BOTTOM
				vertices = {
					{-0.5, 0.5, 1.0},
					{-0.5, 0.5, 0.0},
					{0.5, 0.5, 1.0},
					{0.5, 0.5, 0.0},
				};

				indices = {
					0, 1, 2, 2, 1, 3
				};

				glBufferData(GL_ARRAY_BUFFER,
					sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				// Drawing of num_indices/3 triangles specified in the index buffer
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
				gl_has_errors();
			}


			// If the light is on the left side...
			if (lightPosition.x < (p.position.x + scale.x / 2.f) || registry.mapTiles.has(entity)) {
				// RIGHT
				vertices = {
					{0.5, -0.5, 1.0},
					{0.5, -0.5, 0.0},
					{0.5, 0.5, 1.0},
					{0.5, 0.5, 0.0},
				};

				indices = {
					0, 1, 2, 2, 1, 3
				};

				glBufferData(GL_ARRAY_BUFFER,
					sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				// Drawing of num_indices/3 triangles specified in the index buffer
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
				gl_has_errors();
			}


			// If the light is on the bottom side...
			if (lightPosition.y > (p.position.y - scale.y / 2.f) || registry.mapTiles.has(entity)) {
				// TOP
				vertices = {
					{-0.5, -0.5, 1.0},
					{-0.5, -0.5, 0.0},
					{0.5, -0.5, 1.0},
					{0.5, -0.5, 0.0},
				};

				indices = {
					0, 1, 2, 2, 1, 3
				};

				glBufferData(GL_ARRAY_BUFFER,
					sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
				gl_has_errors();

				// Drawing of num_indices/3 triangles specified in the index buffer
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
				gl_has_errors();
			}

		}

		updateShadowMap();


	}
}

void RenderSystem::updateShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::SHADOW_MAP];

	// Setup program
	glUseProgram(program);
	gl_has_errors();

	const GLuint vbo = vertex_buffers[(int)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE];
	const GLuint ibo = index_buffers[(int)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE];


	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();


	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	GLuint light_texture_uloc = glGetUniformLocation(program, "light_texture");

	glUniform1i(light_texture_uloc, 1);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, shadowMap);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE1);

	glBindTexture(GL_TEXTURE_2D, lightShadowMap);
	gl_has_errors();


	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
	// no offset from the bound index buffer
	gl_has_errors();
}

void RenderSystem::drawLight(Entity light_entity, const mat3& projection) {
	// just draw a projectile based on the light parameters
	Position& position = registry.positions.get(light_entity);
	LightSource& ls = registry.lightSources.get(light_entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(position.position);
	transform.scale(vec2(ls.radius));
	transform.rotate(radians(position.angle));

	const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::TEXTURED];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	GLint tiletexcoord_loc = glGetUniformLocation(program, "tiletexcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	// [1] Tile coordinates to get area of texture we want to use
	vec4 tiletexcoord = { 0, 0, 1, 1 };
	glUniform4f(tiletexcoord_loc, tiletexcoord.x, tiletexcoord.y, tiletexcoord.z, tiletexcoord.w);
	gl_has_errors();



	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	// PLACEHOLDER
	glBindTexture(GL_TEXTURE_2D, getTextureHandle(TEXTURE_ASSET_ID::LIGHT_MAP));
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}