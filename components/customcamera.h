namespace CustomCamera {
	bool bReset = false;
	Car* pTargetPlayerVehicle = nullptr;

	const float fPanSpeedBase = 0.005;

	double fMouse[2] = {};
	NyaVec3 vPos = {0, 0, 0};
	NyaVec3 vPosAfterLook = {0, 0, 0};
	NyaVec3 vPosChange = {0, 0, 0};
	float fLookatOffset = 0.7;
	float fFollowOffset = 1.7;
	NyaVec3 vLastPlayerPosition = {0, 0, 0};
	float fMouseRotateSpeed = 1;
	float fStringDistanceClose = 2;
	float fStringDistanceFar = 3;
	float fStringVelocityMult = 1;
	float fStringResetTime = 2;
	float fStringCorrectionMult = 0.5;
	float fStringMaxYDiff = -0.5;

	double fMouseTimer = -1;

	NyaVec3 GetCarPosition(Car* ply) {
		// center the car by bounds on the Y axis, center by wheel positions on the Z axis

		auto mat = pMyPlugin->carAvatar->physicsState.worldMatrix;
		auto v = mat.p;
		//v += mat.x * (ply->bounds.max.x + ply->bounds.min.x) * 0.5;
		v += mat.y * (ply->bounds.max.y + ply->bounds.min.y) * 0.5;
		//v += mat.z * (ply->bounds.max.z + ply->bounds.min.z) * 0.5;

		NyaVec3 susp1, susp2;
		ply->tyres[0].hub->getBasePosition(&susp1);
		ply->tyres[2].hub->getBasePosition(&susp2);
		v += mat.z * (susp1.z + susp2.z) * 0.5;

		return v;
	}

	double GetStringDistance(Car* ply) {
		auto bound = std::abs((ply->bounds.max.z - ply->bounds.min.z)) * 0.5;
		if (pMyPlugin->sim->cameraManager->persistanceCameraMode.lastDrivableCameraMode == DrivableCamera::eChase) {
			return bound * fStringDistanceClose;
		}
		else {
			return bound * fStringDistanceFar;
		}
	}

	NyaVec3 GetLookatOffset(Car* ply) {
		float bound = std::abs((ply->bounds.max.y - ply->bounds.min.y)) * 0.5;
		return {0, bound * fLookatOffset, 0};
	}

	NyaVec3 GetFollowOffset(Car* ply) {
		float bound = std::abs((ply->bounds.max.y - ply->bounds.min.y)) * 0.5;
		return {0, bound * fFollowOffset, 0};
	}

	NyaVec3* GetTargetPosition(Car* ply) {
		if (!ply) return nullptr;

		static NyaVec3 vec;
		vec = GetCarPosition(ply);
		vec += GetLookatOffset(ply);
		return &vec;
	}

	NyaVec3* GetFollowPosition(Car* ply) {
		if (!ply) return nullptr;

		static NyaVec3 vec;
		vec = GetCarPosition(ply);
		vec += GetFollowOffset(ply);
		return &vec;
	}

	void SetRotation(Camera* pCam) {
		auto plyPos = GetTargetPosition(pTargetPlayerVehicle);
		if (!plyPos) return;

		auto lookat = vPos - *plyPos;
		lookat.Normalize();
		auto mat = NyaMat4x4::LookAt(lookat);
		mat.p = vPos;
		//mat = WorldToRenderMatrix(mat);
		pCam->matrix = mat;
	}

	void SetRotationAfterLook(Camera* pCam) {
		auto plyPos = GetTargetPosition(pTargetPlayerVehicle);
		if (!plyPos) return;

		auto lookat = vPosAfterLook - *plyPos;
		lookat.Normalize();
		auto mat = NyaMat4x4::LookAt(lookat);
		mat.p = vPosAfterLook;
		//mat = WorldToRenderMatrix(mat);
		pCam->matrix = mat;
	}

	void DoCamString() {
		auto ply = pTargetPlayerVehicle;
		auto maxDist = GetStringDistance(ply);

		auto plyPos = GetFollowPosition(ply);
		auto lookatFront = -(vPos - *plyPos);
		auto dist = lookatFront.length();
		lookatFront.Normalize();
		vPos += lookatFront * dist;
		vPos -= lookatFront * maxDist;
	}

	void DoMovement(Camera* pCam) {
		auto player = pTargetPlayerVehicle;
		if (!player) return;

		auto mat = pCam->matrix;
		vPosChange = mat.x * fMouse[0] * fMouseRotateSpeed * fPanSpeedBase;
		vPosChange += mat.y * fMouse[1] * -fMouseRotateSpeed * fPanSpeedBase;
		vPos -= vPosChange;

		NyaVec3 currPos = GetCarPosition(player);

		auto velocity = currPos - vLastPlayerPosition;
		velocity *= fStringVelocityMult;

		vPos -= vLastPlayerPosition;
		vPos += currPos;
		vPos -= velocity;
		DoCamString();

		auto lookat = GetTargetPosition(player);
		if (vPos.y - lookat->y < fStringMaxYDiff) {
			vPos.y = lookat->y + fStringMaxYDiff;
		}

		vLastPlayerPosition = currPos;

		mat.p = vPos;
		pCam->matrix = mat;
	}

	void SetCameraToDefaultPos(Car* ply) {
		NyaMat4x4 mat;
		ply->body->getWorldMatrix(&mat, 0.0);
		mat.p = GetCarPosition(ply);
		vPos = vLastPlayerPosition = mat.p;
		vPos += GetFollowOffset(ply);
		vPos -= mat.z * GetStringDistance(ply);
		fMouseTimer = -1;
		fMouse[0] = 0;
		fMouse[1] = 0;
	}

	void ProcessCam(Camera* cam, double delta) {
		//if (auto sensitivity = GetGameSettingByName("MouseSensitivity")) {
		//	fMouseRotateSpeed = std::lerp(0.25, 4.0, *(int*)sensitivity->value / 100.0);
		//}

		fMouseTimer -= delta;

		if (!cam) return;

		auto follow = pTargetPlayerVehicle;
		if (!pTargetPlayerVehicle) {
			bReset = true;
			pTargetPlayerVehicle = follow = pMyPlugin->car;
			if (!pTargetPlayerVehicle) return;
		}
		if (!follow) {
			bReset = true;
			return;
		}
		if ((vLastPlayerPosition - GetCarPosition(pTargetPlayerVehicle)).length() > 10) {
			bReset = true;
		}
		if (bReset) {
			SetCameraToDefaultPos(pTargetPlayerVehicle);
		}
		bReset = false;

		vPosChange = {0,0,0};
		SetRotation(cam);
		DoMovement(cam);
		SetRotation(cam);

		bool lookBack = pMyPlugin->car->controlsProvider->getAction(DriverActions::eLookingBack);
		bool lookLeft = pMyPlugin->car->controlsProvider->getAction(DriverActions::eLookingLeft);
		bool lookRight = pMyPlugin->car->controlsProvider->getAction(DriverActions::eLookingRight);
		if (lookLeft && lookRight) lookBack = true;

		if (lookBack) {
			auto carPos = GetCarPosition(pTargetPlayerVehicle);
			vPosAfterLook = vPos;
			vPosAfterLook -= carPos;
			vPosAfterLook.x *= -1;
			vPosAfterLook.z *= -1;
			vPosAfterLook += carPos;
			SetRotationAfterLook(cam);
		}
		else if (lookRight) {
			auto carMat = pMyPlugin->carAvatar->physicsState.worldMatrix;
			vPosAfterLook = GetCarPosition(pTargetPlayerVehicle);
			vPosAfterLook += carMat.x * GetStringDistance(pTargetPlayerVehicle);
			vPosAfterLook += carMat.y * GetFollowOffset(pTargetPlayerVehicle).y;
			SetRotationAfterLook(cam);
		}
		else if (lookLeft) {
			auto carMat = pMyPlugin->carAvatar->physicsState.worldMatrix;
			vPosAfterLook = GetCarPosition(pTargetPlayerVehicle);
			vPosAfterLook -= carMat.x * GetStringDistance(pTargetPlayerVehicle);
			vPosAfterLook += carMat.y * GetFollowOffset(pTargetPlayerVehicle).y;
			SetRotationAfterLook(cam);
		}

		if (fMouse[0] != 0.0 || fMouse[1] != 0.0) {
			fMouseTimer = fStringResetTime;
		}
		fMouse[0] = 0;
		fMouse[1] = 0;
	}

	void SetTargetCar(Car* pCar) {
		if (!pCar) {
			pTargetPlayerVehicle = nullptr;
			return;
		}
		if (pCar != pTargetPlayerVehicle) bReset = true;
		pTargetPlayerVehicle = pCar;
	}
}