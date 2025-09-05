#include "ui.h"

#include <regex>
#include <mutex>
#include <vector>
#include <format>
#include <d3d11.h>
#include <iostream> 
#include <filesystem>

#include "repr/Message.h"
#include "repr/Channel.h"
#include "client/client.h"

#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static POINTS g_Position{};
static HWND g_hWnd = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static bool isRunning = true;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CreateRenderTarget() {
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (!pBackBuffer) return;
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget() {
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool CreateDeviceD3D(HWND hWnd) {
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D() {
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

namespace jasper {
	namespace UI {
		Client* client;

		static Guild* curGuild = nullptr;
		static Channel* curChannel = nullptr;

		static Snowflake curGuildId;

		void init(Client* client) {
			UI::client = client;

			WNDCLASSEXW wc = {
				sizeof(wc),
				CS_CLASSDC,
				WndProc,
				0L,
				0L,
				GetModuleHandleA(0),
				nullptr, nullptr, nullptr, nullptr,
				L"Jasper",
				nullptr
			};
			RegisterClassExW(&wc);
			g_hWnd = CreateWindowExW(
				0,
				wc.lpszClassName,
				L"Jasper",
				WS_POPUP,
				100, 100,
				UI_WIDTH, UI_HEIGHT,
				nullptr, nullptr,
				wc.hInstance,
				nullptr
			);

			if (!CreateDeviceD3D(g_hWnd)) {
				CleanupDeviceD3D();
				UnregisterClassW(wc.lpszClassName, wc.hInstance);
				return;
			}

			ShowWindow(g_hWnd, SW_SHOWDEFAULT);
			UpdateWindow(g_hWnd);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			ImFontConfig cfg;

			{
				// TODO
				static const ImWchar ranges[] = { 0x20, 0xFFFF, 0 };
				io.Fonts->AddFontFromFileTTF("ui/fonts/Jetbrains Mono NL.ttf", 16.f, &cfg, ranges);
				cfg.MergeMode = true;
				io.Fonts->AddFontFromFileTTF("ui/fonts/Helvetica.ttf", 16.f, &cfg, ranges);
				io.Fonts->AddFontFromFileTTF("ui/fonts/Symbola.ttf", 16.f, &cfg, ranges);
			}

			io.Fonts->Build();
			io.IniFilename = nullptr;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.102f, 0.102f, 0.118f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0706f, 0.0706f, 0.0784f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.12f, 0.12f, 0.14f, 1.f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.22f, 0.25f, 1.f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.28f, 0.32f, 1.f);

			style.WindowPadding = ImVec2(5.f, 5.f);
			style.ScrollbarRounding = 3.f;
			
			ImGui_ImplWin32_Init(g_hWnd);
			ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

			while (isRunning) {
				MSG msg;
				while (PeekMessageA(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
					if (msg.message == WM_QUIT)
						isRunning = false;
				}
				if (!isRunning)
					break;

				if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
					Sleep(10);
					continue;
				}
				g_SwapChainOccluded = false;

				if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
					CleanupRenderTarget();
					g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
					g_ResizeWidth = g_ResizeHeight = 0;
					CreateRenderTarget();
				}

				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				{
					RECT rect;
					GetClientRect(g_hWnd, &rect);
					ImGui::SetNextWindowSize(ImVec2(
						(float)(rect.right - rect.left),
						(float)(rect.bottom - rect.top)
					));
					ImGui::SetNextWindowPos(ImVec2(0, 0));

					ImGui::Begin("Jasper", &isRunning, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
					render(io);
					ImGui::End();
				}

				ImGui::Render();
				static const float rgba[4] = { 0.f, 0.f, 0.f, 1.f };
				g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
				g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, rgba);
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				HRESULT hr = g_pSwapChain->Present(1, 0);
				g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
			}

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			CleanupDeviceD3D();
			DestroyWindow(g_hWnd);
			UnregisterClassW(wc.lpszClassName, wc.hInstance);

			ExitProcess(0);
		};

		std::string getShortName(const std::string& name) {
			std::string result;
			std::regex re("[^A-Za-z0-9]+");
	
			std::sregex_token_iterator it(name.begin(), name.end(), re, -1);
			std::sregex_token_iterator end;
	
			for (; it != end; ++it) {
				const std::string& part = it->str();
				if (part.empty())
					continue;
		
				result += part[0];
		
				for (size_t i = 1; i < part.size(); ++i) {
					if (isupper(part[i]))
						result += part[i];
				}
			}
	
			return result;
		}

		void renderGuildSidebar() {
			ImGuiStyle& style = ImGui::GetStyle();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.05f, .05f, .06f, 1.f));
			ImGui::BeginChild("##guildsSidebar", ImVec2(52.f, 0.f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PopStyleColor();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 6.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.f, 12.f));

			for (uint32_t i = 0; i < client->guildCache.size(); i++) {
				Guild* guild = client->guildCache.at(i);
				if (ImGui::Selectable(getShortName(guild->name.data).c_str(), curGuild == guild)) {
					curGuild = guild;

					if (guild->id != curGuildId) {
						curGuildId = guild->id;
						curChannel = nullptr;
					}
				}
			}

			ImGui::PopStyleVar(2);
			ImGui::EndChild();
		}

		void renderChannelSidebar() {
			ImGuiStyle& style = ImGui::GetStyle();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.07f, .07f, .08f, 1.f));
			ImGui::BeginChild("##channelsSidebar", ImVec2(220.f, 0.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(.12f, .12f, .14f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
			ImGui::Selectable(curGuild ? curGuild->name.data : ":D", false, ImGuiSelectableFlags_SpanAvailWidth);
			ImGui::PopStyleColor(2);

			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, 2.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 5.f));

			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(.2f, 0.22f, .25f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(.25f, 0.28f, .32f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.8f, .82f, .87f, 1.f));

			ImGui::BeginChild("##channels", ImVec2(0.f, 0.f));

			if (curGuild) {
				List<Channel*>& allChannels = curGuild->channels();
				
				List<Channel*> categories;
				List<Channel*> uncategorized;

				for (uint32_t i = 0; i < allChannels.size(); i++) {
					Channel* chan = allChannels[i];
					if (chan->type == CHANNEL_GUILD_CATEGORY)
						categories.push(chan);
					else if (chan->parentId == 0) {
						ImGui::TextUnformatted("#");
						ImGui::SameLine();
						if (ImGui::Selectable(chan->name, curChannel == chan))
							curChannel = chan;
						uncategorized.push(chan);
					}
				}

				for (uint32_t i = 0; i < categories.size(); i++) {
					Channel* cat = categories[i];

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.588f, .588f, .588f, 1.f));
					ImGui::TextUnformatted(cat->name);
					ImGui::PopStyleColor();

					ImGui::Indent(10.f);
					List<Channel*>& channels = cat->channels();
					for (uint32_t j = 0; j < channels.size(); j++) {
						Channel* chan = channels[j];

						ImGui::TextUnformatted("#");
						ImGui::SameLine();
						if (ImGui::Selectable(chan->name, curChannel == chan))
							curChannel = chan;
					}
					ImGui::Unindent(10.f);
				}

			} else
				ImGui::TextUnformatted("\n\n\n\n\n\n\n\n\n\n             :C");

			ImGui::EndChild();

			ImGui::PopStyleColor(4);
			ImGui::PopStyleVar(2);
			ImGui::EndChild();
		}

		void render(ImGuiIO& io) {
			renderGuildSidebar();
			ImGui::SameLine(0, 0);
			renderChannelSidebar();
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			ImGui::BeginChild("##channel", ImVec2(0, 0), false);

			if (curChannel) {
				ImGui::TextUnformatted("#");
				ImGui::SameLine();
				ImGui::TextUnformatted(curChannel->name);
				ImGui::Separator();
				
				ImGui::TextUnformatted("fuck u");
			} else {
				ImGui::TextUnformatted("# :P");
				ImGui::Separator();
				ImGui::TextUnformatted("⋆.˚ ☾⭒.˚ lovre (˶˃ ᵕ ˂˶)\n — 9:36 AM\nhmm not sure\nCant spell jasper without BLOATFREE");
			}

			ImGui::EndChild();
		}
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam);
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_LBUTTONDOWN: {
		g_Position = MAKEPOINTS(lParam);
		return 0;
	}
	case WM_MOUSEMOVE: {
		if (wParam == MK_LBUTTON) {
			const auto points = MAKEPOINTS(lParam);
			auto rect = RECT{ };

			GetWindowRect(g_hWnd, &rect);

			rect.left += points.x - g_Position.x;
			rect.top += points.y - g_Position.y;

			if (g_Position.x >= 0 && g_Position.x <= UI_WIDTH && g_Position.y >= 0 && g_Position.y <= 19)
				SetWindowPos(
					g_hWnd,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}
