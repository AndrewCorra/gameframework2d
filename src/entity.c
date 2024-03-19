#include "simple_logger.h"
#include "entity.h"

typedef struct {
	Entity* entity_list;
	Uint32 entity_max;
}EntityManager;
void entity_system_close();

static EntityManager _entity_manager = { 0 }; /**<initalize a LOCAL global entity manager*/

void entity_system_initialize(Uint32 max) {
	if (_entity_manager.entity_list) {
		slog("cannot have two instances of an entity manager, one is already active");
		return;
	}
	if (!max) {
		slog("cannot allocate 0 entities!");
		return;
	}
	_entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max);
	if (!_entity_manager.entity_list)
	{
		slog("failed to allocate global entity list");
		return;
	}
	_entity_manager.entity_max = max;
	slog("entity system initialized");
	atexit(entity_system_close);
}

void entity_system_close() {
	entity_clear_all(NULL);
	if (_entity_manager.entity_list) {
		free(_entity_manager.entity_list);
	}
	memset(&_entity_manager, 0, sizeof(EntityManager));
}

void entity_clear_all(Entity *ignore)
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++) {
		if (&_entity_manager.entity_list[i] == ignore)continue;
		if (!_entity_manager.entity_list[i]._inuse)continue;//skip this iteration of the loop
		entity_free(&_entity_manager.entity_list[i]);
	}
}

Entity *entity_new() {
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++) {
		if (_entity_manager.entity_list[i]._inuse)continue; //skip active entities
		memset(&_entity_manager.entity_list[i], 0, sizeof(Entity));
		_entity_manager.entity_list[i]._inuse = 1;
		return &_entity_manager.entity_list[i];
	}
	slog("no more avaliable entities");
	return NULL;
}

void entity_free(Entity *self) {
	if (!self)return; //can't do work with nothing
	gf2d_sprite_free(self->sprite);
	if (self->free)self->free(self->data);
} 

void entity_think(Entity* self) {
	if (!self)return;
	if (self->think)self->think(self);
}

void entity_system_think()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++) {
		if (!_entity_manager.entity_list[i]._inuse)continue; //skip inactive entities
		entity_think(&_entity_manager.entity_list[i]);
	}
}


void entity_update(Entity* self) {
	if (!self)return;
	if (self->update)self->update(self);
}

void entity_system_update()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++) {
		if (!_entity_manager.entity_list[i]._inuse)continue; //skip inactive entities
		entity_update(&_entity_manager.entity_list[i]);
	}
}

void entity_draw(Entity *self) {
	if (!self)return;
	if (self->sprite) {
		gf2d_sprite_render(
			self->sprite,
			self->position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
	}
}

void entity_system_draw() {
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++) {
		if (!_entity_manager.entity_list[i]._inuse)continue; //skip inactive entities
		entity_draw(&_entity_manager.entity_list[i]);
	}
}