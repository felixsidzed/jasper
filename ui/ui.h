#pragma once

#include "imgui/imgui.h"

#define UI_WIDTH 800
#define UI_HEIGHT 450

namespace jasper {
	class Client;

	namespace UI {
		void init(Client* client);
		void render(ImGuiIO& io);
	}
}
