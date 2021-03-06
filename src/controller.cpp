#include "controller.h"
#include <typeinfo>

Controller::Controller() : viewer(new Viewer()), 
	collisioncontroller(new collisionController()),
	portacontroller(new Porta_controller()),
	slcontroller(new SLcontroller()),
	trcontroller(new TRcontroller()),
	cameracontroller(new Camera_controller()),
	event(new Eventos())
{
	// create the 4 maps
	mapVec.push_back(std::shared_ptr<Map> (new Map(0)));
	mapVec.push_back(std::shared_ptr<Map> (new Map(1)));
	mapVec.push_back(std::shared_ptr<Map> (new Map(2)));
	mapVec.push_back(std::shared_ptr<Map> (new Map(3)));

	// pass tilesize and bounding box size
	tileSize = viewer->tileRect.w;
	boxSize = tileSize - tileSize/4;
	mapVec[0]->setTileSize(tileSize);
	mapVec[1]->setTileSize(tileSize);
	mapVec[2]->setTileSize(tileSize);
	mapVec[3]->setTileSize(tileSize);

	// character bounding box height
	int boundBoxH = (int) ((float) boxSize*((float) 41/24));
	collisioncontroller->set_boundBoxSize(boxSize, boundBoxH);
	collisioncontroller->set_tileSize(tileSize);
	portacontroller->setTileSize(tileSize);
	portacontroller->setBoundBoxSize(boxSize);
	cameracontroller->setTileSize(tileSize);
	cameracontroller->setBoundBoxSize(boxSize);
	event->setTileSize(tileSize);

	// configure keyboard state
	state = SDL_GetKeyboardState(nullptr); 
	collisioncontroller->set_state(state);

	// load Map
	mapVec[0]->loadMap("../assets/maps/m0.map");
	mapVec[1]->loadMap("../assets/maps/m1.map");
	mapVec[2]->loadMap("../assets/maps/m2.map");
	mapVec[3]->loadMap("../assets/maps/m3.map");


	// allocate new elements
	for (int i=0; i < mapVec[0]->getHeight(); i++){
		for (int j=0; j < mapVec[0]->getWidth(); j++){
			// for each map
			for (int k=0; k<4; k++){
				// sets map start tile 
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="start")
					event->setStart(j*tileSize,i*tileSize, k);

				// sets game end tile (destination)
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="end")
					event->setEnd(j*tileSize,i*tileSize, k);

				// sets next room tile (destination)
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="to0")
					event->setNext(j*tileSize,i*tileSize, k, 0);
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="to1")
					event->setNext(j*tileSize,i*tileSize, k, 1);
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="to2")
					event->setNext(j*tileSize,i*tileSize, k, 2);
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="to3")
					event->setNext(j*tileSize,i*tileSize, k, 3);

				// allocattes a new porta
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="porta_fechada")
					portaVec.push_back(std::shared_ptr<Porta> (new Porta(j*tileSize,i*tileSize, 0, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="porta_aberta")
					portaVec.push_back(std::shared_ptr<Porta> (new Porta(j*tileSize,i*tileSize, 1, k)));

				// allocattes a new camera
				int alcance = tileSize*2;
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_cima")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(0, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_baixo")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(1, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_direita")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(2, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_esquerda")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(3, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_cima_direita")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(6, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_cima_esquerda")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(7, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_baixo_direita")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(4, alcance, j*tileSize,i*tileSize, k)));
				if (mapVec[k]->get_textMap()[std::make_pair(j,i)]=="camera_baixo_esquerda")
					cameraVec.push_back(std::shared_ptr<Camera> (new Camera(5, alcance, j*tileSize,i*tileSize, k)));
			}
		}
	}
	// mapa inicial
	collisioncontroller->set_map(mapVec[0]);
	viewer->updateMap(mapVec[0]->get_textMap());	
}

/* monitor loop*/
void Controller::monitorLoop(std::string ip_remoto){
	std::cout << "Monitor mode" << std::endl;
	//int transmission = 0;
	std::string str;

	// Configurar o Cliente
	trcontroller->configClient(ip_remoto);

	int boundBoxH = (int) ((float) boxSize*((float) 41/24));
	while(rodando){
		// Load Received State
		json stateJSON = trcontroller->receiveJSON();

		str = "";
		// check for new players/ delete removed players
		str.push_back('y');
		for (int i=1; i<21; i++){
			str+=std::to_string(i);
			if(!(stateJSON.contains(str))){
				//if((players.count(i)==1) && (i!=trcontroller->player_number)){
				if((players.count(i)==1)){
				// ... but not on receive State
					players.erase(i);
				}	
			}
			// player on received State...
			if(stateJSON.contains(str)){
				// ... but not on list
				if(players.count(i)==0){
					players.insert(std::make_pair(i,std::shared_ptr<Player> (new Player(0, 0, boxSize, boundBoxH))));
				}	
			}
		str = "y";
		}

		// reset str
		str = "";

		// update all players
		std::map<int, std::shared_ptr<Player>>::iterator pl;
		for (pl = players.begin(); pl != players.end(); ++pl){
			str.push_back('y'); 
			str+=std::to_string((pl->first));
			slcontroller->load(stateJSON, *(pl->second), str);
			str.pop_back(); str.pop_back();
			// update all portas
			for (int i = 0; i < portaVec.size(); i++){
				str.push_back('p'); 
				str+=std::to_string(i);
				slcontroller->load(stateJSON, *(portaVec[i]), str);
				str.pop_back(); str.pop_back();
				portaVec[i]->atualiza_porta(collisioncontroller->getCollisionMap(), tileSize, *mapVec[pl->second->getCurrentMap()]);
			}
			// update all cameras
			for (int i = 0; i < cameraVec.size(); i++){
				str.push_back('c');
				str+=std::to_string(i);
				slcontroller->load(stateJSON, *(cameraVec[i]), str);
				str.pop_back(); str.pop_back();
			}
		}

		// Rendering
		viewer->updateMap(mapVec[players[trcontroller->player_number]->getCurrentMap()]->get_textMap());	
		viewer->render(players, players[trcontroller->player_number]->getCurrentMap());

		// Send Player Input 
		if(state[SDL_SCANCODE_UP]) {
			trcontroller->sendString("U", trcontroller->remote_endpoint);
			if (state[SDL_SCANCODE_UP]){
				buttonEventCounter++;
				buttonReady = 0;
			}
		}

		else if(state[SDL_SCANCODE_LEFT]) {
			trcontroller->sendString("L", trcontroller->remote_endpoint);
			if (state[SDL_SCANCODE_LEFT]){
				buttonEventCounter++;
				buttonReady = 0;
			}
		}
				
		else if(state[SDL_SCANCODE_RIGHT]) {
			trcontroller->sendString("R", trcontroller->remote_endpoint);
			if (state[SDL_SCANCODE_RIGHT]){
				buttonEventCounter++;
				buttonReady = 0;
			}
		}

		else if(state[SDL_SCANCODE_DOWN]) {
			trcontroller->sendString("D", trcontroller->remote_endpoint);
			if (state[SDL_SCANCODE_DOWN]){
				buttonEventCounter++;
				buttonReady = 0;
			}
		}
		
		else if(buttonReady && state[SDL_SCANCODE_E]) {
			trcontroller->sendString("E", trcontroller->remote_endpoint);
			if (state[SDL_SCANCODE_E]){
				buttonEventCounter++;
				buttonReady = 0;
			}
		}

		// increment reset timer
		buttonEventCounter = (buttonEventCounter+1)%30;
		if (buttonEventCounter==0){buttonReady = 1;}

		while (SDL_PollEvent(&evento)) {
			if (evento.type == SDL_QUIT) {
				rodando = false;
				trcontroller->sendString("Client: Adeus", trcontroller->remote_endpoint);
			}
		}
    		SDL_Delay(5);
	}
}

/* main game loop*/
void Controller::gameLoop(){
	std::cout << "Game mode" << std::endl;
	// Setting boundBox
	int boundBoxH = (int) ((float) boxSize*((float) 41/24));
	// event flag
	int flag = 0;
	//int transmission = 0;
	// auxiliary string for load/save
	std::string str;
	//chars de comparaçao
	int Por; int Up; int Down; int Left; int Right;
	
	// Configurar o Server
	trcontroller->configServer();

	// Thread for reciving clients and information
	std::thread *connection = new std::thread(&TRcontroller::checkConnection, trcontroller);

	// Start at title screen
	//titleScreen();
	while(rodando){
		std::map<int, std::string>::iterator com;
		for (com = trcontroller->get_commands().begin(); com!= trcontroller->get_commands().end(); ++com) {
			// Player has entered the server
			if(com->second.compare("new") == 0) {
				players.insert({com->first, std::shared_ptr<Player> (new Player(3*tileSize, 3*tileSize, boxSize, boundBoxH))});
				com->second = "9";
				players[com->first]->setCurrentMap(0);
			}

			// Player has left the server
			else if(com->second.compare("left") == 0) {
				players.erase(com->first);
				(trcontroller->get_commands()).erase(com->first);
			}

			// In case player in position "ouo" hasn't sent more commands
			else if(com->second.compare("9") == 0){
			       	continue;
			}

			// Processing commands from player in position "ouo"
			else {
				// Setting command flags
				Por = 0; Up = 0; Down = 0; Left = 0; Right = 0;
				if (com->second.compare("R") == 0) Right = 1;
				else if (com->second.compare("U") == 0) Up = 1;
				else if (com->second.compare("D") == 0) Down = 1;
				else if (com->second.compare("L") == 0) Left = 1;
				else if (com->second.compare("E") == 0) Por = 1;
				trcontroller->get_commands()[com->first] = "9";

				// mapa inicial
				collisioncontroller->set_map(mapVec[players[com->first]->getCurrentMap()]);

				// Movement and Collision Control
				collisioncontroller->move(*(players[com->first]), Up, Down, Left, Right);

				// Door Control
				// reset timer
				if (Por){
					for (int i = 0; i < portaVec.size(); i++){
						portacontroller->abre_fecha(*(portaVec[i]), *(players[com->first]), *mapVec[players[com->first]->getCurrentMap()], state, collisioncontroller->getCollisionMap());
					}
				}

				// Camera Control
				for (int i = 0; i < cameraVec.size(); i++){
					cameracontroller->visao(*mapVec[players[com->first]->getCurrentMap()], (*cameraVec[i]), *(players[com->first]));
					cameracontroller->deteccao(*mapVec[players[com->first]->getCurrentMap()], (*cameraVec[i]), *(players[com->first]));
				}

				// Events Control
				flag = event->checagem(*(players[com->first]));
				// mudanca de mapa
				if(flag>=0 && flag<=3) {
				}
				// fim do jogo
				if(flag == 4) {
					//titleScreen();
				}
				for (int i = 0; i < cameraVec.size(); i++){
					flag = event->checagem(*(players[com->first]), *(cameraVec[i]));
					// avistado pela camera (spawn no mapa 0)
					if(flag == 5) {
						viewer->renderExclamation(*(cameraVec[i]));
						for (int i = 0; i < portaVec.size(); i++){
							portaVec[i]->set_flag(0);
							portaVec[i]->atualiza_porta(collisioncontroller->getCollisionMap(), tileSize, *mapVec[players[com->first]->getCurrentMap()]);
						}
						break;
					}
				}
			}

		}

		// Save in file to transfer to viwer
		std::map<int, std::shared_ptr<Player>>::iterator pl;
		for (pl = players.begin(); pl != players.end(); ++pl){
			str.push_back('y'); 
			str+=std::to_string(pl->first);
			slcontroller->add(*(pl->second), str);
			str.pop_back(); str.pop_back();
		}
		for (int i = 0; i < portaVec.size(); i++){
			str.push_back('p'); 
			str+=std::to_string(i);
			slcontroller->add(*(portaVec[i]), str);
			str.pop_back(); str.pop_back();
		}
		for (int i = 0; i < cameraVec.size(); i++){
			str.push_back('c');
			str+=std::to_string(i);
			slcontroller->add(*(cameraVec[i]), str);
			str.pop_back(); str.pop_back();
		}
		trcontroller->sendState_server(slcontroller->get_file());
		slcontroller->clear();

		while (SDL_PollEvent(&evento)) {
			if (evento.type == SDL_QUIT) {
				rodando = false;
				trcontroller->set_flag(0);
				delete connection;
				//connection.join();
			}
		}

		SDL_Delay(20);
	}
}

void Controller::titleScreen(){
	int title = 0;
	viewer->renderMain();
	while (!title){
		while (SDL_PollEvent(&evento)){
			break;
		}
		if (evento.type == SDL_KEYDOWN && evento.key.keysym.scancode == SDL_SCANCODE_SPACE)
			title = 1;
	}
	return;
}
