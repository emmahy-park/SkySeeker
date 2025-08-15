#pragma once

#include "common.hpp"
#include "../tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"


// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// legacy
// the player
Entity createChicken(RenderSystem* renderer, vec2 position);

Entity createItem(ItemInfo info, bool is_pickup);
Entity createHealthBar(RenderSystem* renderer);

Entity createCollidableItem(ItemInfo info, vec2 position);
Entity createProjectile(Entity& owner, vec2 projectile_owner_position, vec2 mouse_position, float range, float speed, vec2 size, float damage_multiplier, GLuint texture_handle);

Entity createLightSource(vec2 position);