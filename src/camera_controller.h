#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H
#include "camera.h"
#include "player.h"

class Camera_controller {
	private:
		int tile_size = 20;
	public:
		int** visao(Camera& camera, int n, int m);
		void deteccao(int** Mapa, Camera& camera, Player& jogador, int n, int m);
};

#endif
