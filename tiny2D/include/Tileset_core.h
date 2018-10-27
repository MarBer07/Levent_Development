#pragma once
#pragma warning(disable:4996)
#include <assert.h>
#include "Tileset_data.h"
#include "SDL2-2.0.8\include\SDL.h"
#include "SDL2-2.0.8\include\SDL_image.h"

namespace Tileset
{

	void init(Tileset *t, int array_size)
	{
		t->n_tilesets = 0;
		t->array_size = array_size;
		t->tex = (SDL_Texture**)malloc(sizeof(SDL_Texture*)*t->array_size);
		t->n_cols = (int*)malloc(sizeof(int)*t->array_size);
		t->n_rows = (int*)malloc(sizeof(int)*t->array_size);
		t->tile_w = (int*)malloc(sizeof(int)*t->array_size);
		t->tile_h = (int*)malloc(sizeof(int)*t->array_size);
	}

	void resize(Tileset *t)
	{
		t->array_size += t->array_size >> 1;

		t->tex = (SDL_Texture**)realloc(t->tex, sizeof(SDL_Texture*)*t->array_size);
		t->n_cols = (int*)realloc(t->n_cols, sizeof(int)*t->array_size);
		t->n_rows = (int*)realloc(t->n_rows, sizeof(int)*t->array_size);
		t->tile_w = (int*)realloc(t->tile_w, sizeof(int)*t->array_size);
		t->tile_h = (int*)realloc(t->tile_h, sizeof(int)*t->array_size);

	}

	int make(Tileset *t)
	{
		++t->n_tilesets;
		return t->n_tilesets - 1;
	}

	/*
	tile_w, tile_h = size of single tile in pixels
	n_cols, n_rows = number of rows and columns of tiles in the tileset file
	*/
	void modify(int index,Tileset *t, const char *image_filename, int tile_w, int tile_h, int n_cols, int n_rows, SDL_Renderer *renderer)
	{
		SDL_Surface *sprite_surface = IMG_Load(image_filename);
		assert(sprite_surface);
		SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, sprite_surface);

		SDL_FreeSurface(sprite_surface);

		t->tile_w[index] = tile_w;
		t->tile_h[index] = tile_h;
		t->tex[index] = texture;
		t->n_cols[index] = n_cols;
		t->n_rows[index] = n_rows;
	}

	void draw_Single(int which_tileset, int tile_row, int tile_col, const Tileset *t, int dest_x, int dest_y, int dest_w, int dest_h, const SDL_Renderer *renderer)
	{
		SDL_Rect src;
		src.x = tile_col * t->tile_w[which_tileset];
		src.y = tile_row * t->tile_h[which_tileset];
		src.w = t->tile_w[which_tileset];
		src.h = t->tile_h[which_tileset];

		SDL_Rect dest;
		dest.x = dest_x;
		dest.y = dest_y;
		dest.w = dest_w;
		dest.h = dest_h;

		SDL_RenderCopyEx((SDL_Renderer*)renderer, t->tex[which_tileset], &src, &dest, 0, NULL, SDL_FLIP_NONE);
	}

	void draw_Grid(int which_tileset, const Tileset *tileset, const Grid_Camera::Grid_Camera *cam, const Grid::Grid *g, SDL_Renderer *renderer)
	{
		int x0 = cam->world_coord.x;
		int y0 = cam->world_coord.y;
		int x1 = cam->world_coord.x + cam->world_coord.w;
		int y1 = cam->world_coord.y + cam->world_coord.h;

		int ty = cam->calibration.tile_y;
		for (int i = y0; i <= y1; i++)
		{
			int tx = cam->calibration.tile_x;

			int *tmp_level_data = &g->data[i*g->n_cols];
			for (int j = x0; j <= x1; j++)
			{
				int grid_data = tmp_level_data[j];
				int tileset_row = grid_data / tileset->n_cols[which_tileset];
				int tileset_col = grid_data % tileset->n_cols[which_tileset];
				draw_Single(which_tileset, tileset_row, tileset_col, tileset, tx, ty, cam->calibration.tile_w, cam->calibration.tile_h, renderer);

				tx += cam->calibration.tile_w;
			}
			ty += cam->calibration.tile_h;
		}

	}

	namespace File
	{
		int add(Tileset *tileset, const char *filename, SDL_Renderer *renderer)
		{
			char img_filename[128];
			FILE *f = fopen(filename, "r");
			int tile_w = 0;
			int tile_h = 0;
			int n_rows = 0;
			int n_cols = 0;
			int r = fscanf(f, "img=%s tile_w=%d tile_h=%d n_cols=%d n_rows=%d\n", img_filename, &tile_w, &tile_h, &n_cols, &n_rows);
			if (r != 5) return tileset->n_tilesets;
			modify(make(tileset), tileset, img_filename, tile_w, tile_h, n_cols, n_rows, renderer);
			return tileset->n_tilesets;
		}
	}
}