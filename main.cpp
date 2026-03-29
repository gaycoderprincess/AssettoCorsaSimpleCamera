#include <windows.h>
#include <format>
#include <codecvt>
#include <filesystem>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "nya_commonmath.h"

#include "ac.h"

void OnPluginStartup();
#include "util.h"

#include "components/customcamera.h"

bool IsChaseCamera() {
	if (pMyPlugin->sim->cameraManager->mode != CameraMode::eDrivable) return false;
	auto mode = pMyPlugin->sim->cameraManager->persistanceCameraMode.lastDrivableCameraMode;
	return mode == DrivableCamera::eChase || mode == DrivableCamera::eChase2;
}

auto renderHooked_orig = (void(__fastcall*)(Game*, GameObject*, float))nullptr;
void __fastcall renderHooked(Game* pThis, GameObject* o, float dt) {
	if (IsChaseCamera()) {
		if (!pMyPlugin->car->ksPhysics->hasSessionStarted(0.0)) {
			CustomCamera::bReset = true;
		}
		CustomCamera::ProcessCam(pMyPlugin->sim->sceneCamera, dt);
	}
	else {
		CustomCamera::bReset = true;
	}
	renderHooked_orig(pThis, o, dt);
}

void OnPluginStartup() {
	if (std::filesystem::exists("plugins/AssettoCorsaSimpleCamera_gcp.toml")) {
		auto config = toml::parse_file("plugins/AssettoCorsaSimpleCamera_gcp.toml");
		CustomCamera::fLookatOffset = config["point_offset"].value_or(CustomCamera::fLookatOffset);
		CustomCamera::fFollowOffset = config["follow_offset"].value_or(CustomCamera::fFollowOffset);
		CustomCamera::fStringDistanceFar = config["distance_far"].value_or(CustomCamera::fStringDistanceFar);
		CustomCamera::fStringDistanceClose = config["distance_close"].value_or(CustomCamera::fStringDistanceClose);
	}

	renderHooked_orig = (void(__fastcall*)(Game*, GameObject*, float))NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x2428B8, &renderHooked);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x15AE310) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using the latest x64 Steam release (.exe size of 22890776 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}
		} break;
		default:
			break;
	}
	return TRUE;
}