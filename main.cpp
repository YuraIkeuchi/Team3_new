﻿#include<Windows.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<vector>
#include<string>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#define DIRECTiNPUT_VERSION 

#include<DirectXTex.h>
#include<wrl.h>
#include <stdio.h>

//#include"Header1.hlsli"
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Sprite.h"
#include "Audio.h"
#include "Object3d.h"
#include "Player.h"
#include "Player2.h"
#include "Player3.h"
#include "Player4.h"
#include "backGround.h"
#include "Light.h"
#include "Screen.h"
#include "DirectXTex/d3dx12.h"

#include "imgui\imgui.h"
#include <cassert>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma region//関数
#pragma region 当たり判定用の構造体と関数
struct BoxVertex
{
	float Up;
	float Down;
	float Left;
	float Right;
};

bool BoxCollision_Down(XMFLOAT3 object1, XMFLOAT3 radius1, XMFLOAT3 object2, XMFLOAT3 radius2) {
	BoxVertex Object1;
	BoxVertex Object2;

	//object1の右頂点
	Object1.Right = object1.x + radius1.x * 2;
	//object1の左頂点
	Object1.Left = object1.x;
	//object1の上頂点
	Object1.Up = object1.y + radius1.y;
	//object1の下頂点
	Object1.Down = object1.y - radius1.y;

	//object1の左頂点
	Object2.Left = object2.x - 10;
	//object1の右頂点
	Object2.Right = object2.x + radius2.x * 2;
	//object1の下頂点
	Object2.Down = object2.y - radius2.y;
	//object1の上頂点
	Object2.Up = Object2.Down + 1;
	//→１と左２　→２と左１　上１と下２　上２と下１
	return Object1.Right > Object2.Left && Object2.Right > Object1.Left && Object1.Up > Object2.Down && Object2.Up > Object1.Down;
}

bool BoxCollision_Up(XMFLOAT3 object1, XMFLOAT3 radius1, XMFLOAT3 object2, XMFLOAT3 radius2) {
	BoxVertex Object1;
	BoxVertex Object2;

	//object1の右頂点
	Object1.Right = object1.x + radius1.x * 2;
	//object1の左頂点
	Object1.Left = object1.x;
	//object1の上頂点
	Object1.Up = object1.y + radius1.y;
	//object1の下頂点
	Object1.Down = object1.y - radius1.y;

	//object1の左頂点
	Object2.Left = object2.x - 10;
	//object1の右頂点
	Object2.Right = object2.x + radius2.x * 2;
	//object1の上頂点
	Object2.Up = object2.y + radius2.y;
	//object1の下頂点
	Object2.Down = Object2.Up - 1;
	//→１と左２　→２と左１　上１と下２　上２と下１
	return Object1.Right > Object2.Left && Object2.Right > Object1.Left && Object1.Up > Object2.Down && Object2.Up > Object1.Down;
}


bool BoxCollision_Left(XMFLOAT3 object1, XMFLOAT3 radius1, XMFLOAT3 object2, XMFLOAT3 radius2) {
	BoxVertex Object1;
	BoxVertex Object2;

	//object1の右頂点
	Object1.Right = object1.x + radius1.x * 2;
	//object1の左頂点
	Object1.Left = object1.x;
	//object1の上頂点
	Object1.Up = object1.y + radius1.y;
	//object1の下頂点
	Object1.Down = object1.y - radius1.y;

	//object1の左頂点
	Object2.Left = object2.x - 10;
	//object1の右頂点
	Object2.Right = Object2.Left + 1;
	//object1の下頂点
	Object2.Down = object2.y - radius2.y + 0.5;
	//object1の上頂点
	Object2.Up = object2.y + radius2.y - 0.5;

	//→１と左２　→２と左１　上１と下２　上２と下１
	return Object1.Right > Object2.Left && Object2.Right > Object1.Left && Object1.Up > Object2.Down && Object2.Up > Object1.Down;
}

bool BoxCollision_Right(XMFLOAT3 object1, XMFLOAT3 radius1, XMFLOAT3 object2, XMFLOAT3 radius2) {
	BoxVertex Object1;
	BoxVertex Object2;

	//object1の右頂点
	Object1.Right = object1.x + radius1.x * 2;
	//object1の左頂点
	Object1.Left = object1.x;
	//object1の上頂点
	Object1.Up = object1.y + radius1.y;
	//object1の下頂点
	Object1.Down = object1.y - radius1.y;

	//object1の右頂点
	Object2.Right = object2.x + radius2.x * 2;
	//object1の左頂点
	Object2.Left = Object2.Right - 11;
	//object1の下頂点
	Object2.Down = object2.y - radius2.y + 0.5;
	//object1の上頂点
	Object2.Up = object2.y + radius2.y - 0.5;

	//→１と左２　→２と左１　上１と下２　上２と下１
	return Object1.Right > Object2.Left && Object2.Right > Object1.Left && Object1.Up > Object2.Down && Object2.Up > Object1.Down;
}
#pragma endregion
int collision2(const float& X1, const float& Y1, const float& R1, const float& X2, const float& Y2, const float& R2) {
	int a = X1 - X2;
	int b = Y1 - Y2;
	int distance = sqrtf(a * a + b * b);
	int radius = R1 + R2;
	if (distance <= radius) {
		return TRUE;
	}
}

//イージング
float PI = 3.141592;
float aseInSine(const float x) {
	return 1 - cos((x * PI) / 2);
}

XMFLOAT3 sankaku(XMFLOAT3 screen, XMFLOAT3 Screen, XMFLOAT3 object) {
	XMFLOAT3 result = { screen.x,0,0 };
	float a, b, A, B;
	float add = 0.01f;

	a = fabsf(object.z - Screen.z + add);
	b = fabsf(object.y - Screen.y + add);
	A = fabsf(screen.z - Screen.z + add);

	B = A * (a / b);
	B += 1;
	B = B * object.z;
	result.z = B;

	a = fabsf(object.z - Screen.z + add);
	b = fabsf(object.x - Screen.x + add);
	A = fabsf(screen.z - Screen.z + add);

	B = A * (a / b);
	B += 1;
	B = B * object.y;
	result.y = B;

	return result;
}

#pragma endregion
#pragma region//WinMain
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//ポインタ置き場
	Input* input = nullptr;
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	winApp = new WinApp();
	winApp->Initialize();
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	MSG msg{};
	//でバッグレイヤーのやつ
#ifdef _DEBUG
#endif
	//directx初期化
	HRESULT result;
#pragma region//構造体
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	//定数バッファ用データ構造体
	struct ConstBufferData {
		XMFLOAT4 color;
		XMMATRIX mat;
	};
	const int constantBufferNum = 128;
	const int OBJECT_NUM = 2;
#pragma endregion
#pragma region//行列
	//射影変換行列の作り
	XMMATRIX matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)WinApp::window_width / WinApp::window_height, 0.1f, 1000.0f);

	//ビュー変換行列
	XMMATRIX matview;
	XMFLOAT3 eye(0, 0, -100);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//ワールド変換行列
	XMMATRIX matworld0;
	matworld0 = XMMatrixIdentity();
	XMFLOAT3 position;
	position = { 0.0f,0.0f,0.0f };
	//2
	XMMATRIX matRot1;
	XMMATRIX matTrans1;
	XMMATRIX matworld1;
	matRot1 = XMMatrixRotationY(XMConvertToRadians(90.0f));
	matTrans1 = XMMatrixTranslation(-20.0f, 0, 0);
	matworld1 = matRot1 * matTrans1;
	//カメラの回転角
	float angle = 0.0f;
#pragma endregion
#pragma region//スプライト共通データ
	if (!Sprite::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 0;
	}

	const int SpriteMax = 10;
	Sprite* sprite[SpriteMax] = { nullptr };
	// スプライト共通テクスチャ読み込み
	/*SpriteCommonLoadTexture(spriteCommon, 0, L"Resources/texture.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spriteCommon, 1, L"Resources/house.png", dxCommon->GetDev());*/
	Sprite::LoadTexture(0, L"Resources/Title.png");
	Sprite::LoadTexture(1, L"Resources/END.png");
	sprite[0] = Sprite::Create(0, { 0.0f,0.0f });
	sprite[1] = Sprite::Create(1, { 0.0f,0.0f });
#pragma endregion
#pragma region//オーディオ
	const int AudioMax = 3;
	Audio* audio = new Audio;
	if (!audio->Initialize()) {
		assert(0);
		return 1;
	}
	audio->LoadSound(0, "Resources/Sound/kadai_BGM.wav");
	audio->LoopWave(0, 0.5f);
#pragma endregion
#pragma region//デスクリプタやシェーダリソースビュー
	//定数バッファ用でスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap>basicDescHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = constantBufferNum + 1;

	result = dxCommon->GetDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};

	auto basicHeapHandle0 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		0, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//2
	auto basicHeapHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
		1, dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//シェーダリソースビューのアドレス計算処理
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	//ハンドルのアドレスを進める
	cpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	gpuDescHandleSRV.ptr += constantBufferNum * dxCommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#pragma endregion
#pragma region//キー処理
	//入力の初期化
	input = new Input();
	input->Initialize(winApp);
#pragma endregion
#pragma region//障害物変数
	Object* object[OBJECT_NUM];
	for (int i = 0; i < _countof(object); i++) {
		if (!object[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		object[i] = Object::Create();
		object[i]->Update();
	}

	XMFLOAT3 ObjectPosition[OBJECT_NUM];
	XMFLOAT4 ObjectColor[OBJECT_NUM];
	XMFLOAT3 Plus = { 0.0f,0.0f,0.0f };
	for (int i = 0; i < _countof(object); i++) {
		ObjectPosition[i] = object[i]->GetPosition();
		ObjectColor[i] = object[i]->GetColor();
	}

	ObjectPosition[0] = { -20.0f,-8.0f,20.0f };
	ObjectPosition[1] = { -5.0f,-8.0f,20.0f };
	for (int i = 0; i < _countof(object); i++) {
		object[i]->SetPosition(ObjectPosition[i]);
	}
#pragma endregion
#pragma region//後ろ
	Screen* screen;
	if (!screen->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	screen = Screen::Create();
	screen->Update();

#pragma endregion
#pragma region//プレイヤー変数
	//各プレイヤーの初期化
	Player* player;
	if (!player->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player = Player::Create();
	player->Update();
	Player2* player2;
	if (!player2->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player2 = Player2::Create();
	player2->Update();
	Player3* player3;
	if (!player3->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player3 = Player3::Create();
	player3->Update();
	Player4* player4;
	if (!player4->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player4 = Player4::Create();
	player4->Update();
	XMFLOAT3 PlayerPosition;
	PlayerPosition = { 0.0f,0.0f,0.0f };
	PlayerPosition = player->GetPosition();
	XMFLOAT3 OldPlayerPosition;
	XMFLOAT3 PlayerRotation;
	PlayerRotation = player->GetRotaition();
	player->SetPosition(PlayerPosition);
	XMFLOAT2 SpritePosition = sprite[0]->GetPosition();
	//ジャンプ変数
	int JumpFlag = 0;
	float JumpG = 0.0f;

	XMFLOAT3 camerapos = { 0.0f,0.0f, -50.0f };
	XMFLOAT3 targetcamerapos = { 0,0,0 };
	int mode = 0;//0:ゲームモード 1:設置モード
	int modeflag = 0;

	//アニメーション
	int AnimetionTimer = 0;
	int AnimetionCount = 0;
#pragma endregion
#pragma region//ライト変数
	Light* light;
	if (!light->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	light = Light::Create();
	light->Update();
	XMFLOAT3 LightPosition;
	LightPosition = light->GetRotaition();
	light->SetPosition(LightPosition);
#pragma endregion
#pragma region//背景
	BackGround* background;
	if (!background->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	//XMFLOAT3 BackPosition;
	//BackPosition = background->GetPosition();

	background = BackGround::Create();
	background->Update();
#pragma endregion
#pragma region//スクロール変数
	XMFLOAT3 ScrollPosition = { 0.0f,0.0f,0.0f };
#pragma endregion
#pragma region//シーン変数
	int Scene = 0;
	enum Scene {
		title,
		gamePlay,
		gameOver,
		gameClear
	};
#pragma endregion
#pragma region//ループ処理

	while (true) {
		//ウィンドウメッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}
		for (int i = 0; i < _countof(object); i++) {
			ObjectPosition[i] = object[i]->GetPosition();
			ObjectColor[i] = object[i]->GetColor();
		}
		PlayerPosition = player->GetPosition();
		LightPosition = light->GetPosition();
		SpritePosition = sprite[0]->GetPosition();
#pragma region//更新処理
		//キーの更新
		input->Update();
#pragma region//タイトル
		if (Scene == title) {
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = gamePlay;
			}
		}
#pragma endregion
#pragma region//ゲームプレイ中
		if (Scene == gamePlay) {
			//位置情報保存
			OldPlayerPosition.x = PlayerPosition.x;
			OldPlayerPosition.y = PlayerPosition.y;
			OldPlayerPosition.z = PlayerPosition.z;
			//移動処理
			if (input->PushKey(DIK_UP)) {
				PlayerPosition.y += 0.5f;
			}

			if (input->PushKey(DIK_DOWN)) {
				PlayerPosition.y -= 0.5f;
			}

			if (input->PushKey(DIK_LEFT)) {
				PlayerPosition.x -= 0.5f;
				//PlayerRotation.z = 270;
				AnimetionTimer++;
			}
			if (input->PushKey(DIK_RIGHT)) {
				PlayerPosition.x += 0.5f;
				/*PlayerRotation.z = 180;*/
				AnimetionTimer++;
			}

			if (input->PushKey(DIK_S)) {
				LightPosition.y -= 0.1;
			}

			if (input->PushKey(DIK_W)) {
				LightPosition.y += 0.1;
			}

			if (input->PushKey(DIK_A)) {
				LightPosition.x -= 0.1;
			}

			if (input->PushKey(DIK_D)) {
				LightPosition.x += 0.1;
			}

			//ジャンプ処理
			if (input->TriggerKey(DIK_SPACE) && (JumpFlag == 0)) {
				JumpG = -0.8f;
				JumpFlag = 1;
			}
			PlayerPosition.y -= JumpG;
			JumpG += 0.025f;

			if (AnimetionTimer >= 8) {
				AnimetionCount++;
				AnimetionTimer = 0;
			}
			if (AnimetionCount == 4) {
				AnimetionCount = 0;
			}
		//カメラ
		if (input->TriggerKey(DIK_M) && mode == 0 && modeflag == 1)
		{
			mode = 1;
		}

		else if (input->TriggerKey(DIK_M) && mode == 1 && modeflag == 0)
		{
			mode = 0;
		}

		//ゲーム画面
		if (mode == 0) {
			camerapos.x -= 1.2f;
			camerapos.z -= 0.5f;
			targetcamerapos.z += 0.7f;
			if (camerapos.x <= 0.0f) {
				camerapos.x = 0.0f;
			}
			if (camerapos.z <= -50.0f) {
				camerapos.z = -50.0f;
			}
			if (targetcamerapos.z >= 0.0f) {
				targetcamerapos.z = 0.0f;
			}
			if (camerapos.x == 0.0f && camerapos.z == -50.0f && targetcamerapos.z == 0.0f) {
				modeflag = 1;
			}
		}
		//設置画面
		if (mode == 1) {
			camerapos.x += 1.2f;
			camerapos.z += 0.5f;
			targetcamerapos.z = -15.0f;
			if (camerapos.x >= 70.0f) {
				camerapos.x = 70.0f;
			}
			if (camerapos.z >= -10.0f) {
				camerapos.z = -10.0f;
			}
			/*		if (targetcamerapos.z >= -0.01f) {
						targetcamerapos.z = -0.01f;
					}*/
			if (camerapos.x == 70.0f && camerapos.z == -10.0f) {
				modeflag = 0;
			}
		}
	
			//障害物との当たり判定
		for (int i = 0; i < OBJECT_NUM; i++) {
			if (ObjectColor[i].w > 0.0f) {
				//playerとブロック左辺の当たり判定
				if (BoxCollision_Left(PlayerPosition, { 2,4,4 }, ObjectPosition[i], { 6,4,4 }) == TRUE) {
					PlayerPosition.x = OldPlayerPosition.x;
					//JumpG = 0.0f;
				}

				//playerとブロック右辺の当たり判定
				if (BoxCollision_Right(PlayerPosition, { 2,4,4 }, ObjectPosition[i], { 6,4,4 }) == TRUE) {
					PlayerPosition.x = OldPlayerPosition.x;
					//JumpG = 0.0f;
				}
				//playerとブロック下辺の当たり判定
				if (BoxCollision_Down(PlayerPosition, { 2,4,4 }, ObjectPosition[i], { 6,4,4 }) == TRUE) {
					PlayerPosition.y = OldPlayerPosition.y;
					JumpG = 0.0f;
				}

				//playerとブロック上辺の当たり判定
				if (BoxCollision_Up(PlayerPosition, { 2,4,4 }, ObjectPosition[i], { 6,4,4 }) == TRUE) {
					PlayerPosition.y = OldPlayerPosition.y;
					JumpG = 0.0f;
					JumpFlag = 0;
				}
			}
		}

		//光とブロックの当たり判定
		for (int i = 0; i < _countof(object); i++) {
			if (collision2(ObjectPosition[i].x, ObjectPosition[i].y, 3, LightPosition.x, LightPosition.y, 3) == 1) {
				ObjectColor[i].w -= 0.01;
			}
		}

			if (input->TriggerKey(DIK_R)) {
				Scene = gameClear;
			}
		}
#pragma endregion
#pragma region//クリア
		if (Scene == gameClear) {
			if (input->TriggerKey(DIK_S)) {
				Scene = title;
			}
		}
#pragma endregion
#pragma region//Update
		for (int i = 0; i < _countof(object); i++) {
			object[i]->Update();
			//ルートシグネチャの設定コマンド
			object[i]->SetPosition(ObjectPosition[i]);
			object[i]->SetColor(ObjectColor[i]);
		}
		background->Update();
		player->Update();
		player2->Update();
		player3->Update();
		player4->Update();
		light->Update();
		screen->Update();
		player->SetPosition(PlayerPosition);
		player2->SetPosition(PlayerPosition);
		player3->SetPosition(PlayerPosition);
		player4->SetPosition(PlayerPosition);
		light->SetPosition(LightPosition);
		player->SetRotaition(PlayerRotation);
		player2->SetRotaition(PlayerRotation);
		player3->SetRotaition(PlayerRotation);
		player4->SetRotaition(PlayerRotation);
		Object::SetCameraPosition(camerapos, targetcamerapos);
		Player::SetCameraPosition(camerapos, targetcamerapos);
		Player2::SetCameraPosition(camerapos, targetcamerapos);
		Player3::SetCameraPosition(camerapos, targetcamerapos);
		Player4::SetCameraPosition(camerapos, targetcamerapos);
		Light::SetCameraPosition(camerapos, targetcamerapos);
		Screen::SetCameraPosition(camerapos, targetcamerapos);
#pragma endregion
#pragma endregion
#pragma region//描画
		//描画コマンド
		//x,y座標のデバッグログ
		wchar_t str[256];

		swprintf_s(str, L"ObjectPosition[1].x - PlayerPosition.x:%f\n", ObjectPosition[1].x - PlayerPosition.x);
		OutputDebugString(str);
		swprintf_s(str, L"ObjectPosition[0].x - PlayerPosition.x:%f\n", ObjectPosition[0].x - PlayerPosition.x);
		OutputDebugString(str);
		swprintf_s(str, L"JumpG:%f\n", JumpG);
		OutputDebugString(str);
		dxCommon->PreDraw();
		////4.描画コマンドここから
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ImGui::Begin("test");
		if (ImGui::TreeNode("Debug"))
		{
			if (ImGui::TreeNode("Player"))
			{
				ImGui::Indent();
				ImGui::SliderFloat("Position.x", &PlayerPosition.x, 50, -50);
				ImGui::SliderFloat("Position.y", &PlayerPosition.y, 50, -50);
				ImGui::SliderFloat("JumpG", &JumpG, 50, -50);
				ImGui::SliderFloat("Color", &ObjectColor[0].w, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::Indent();
		ImGui::Unindent();
		ImGui::End();
		//背景スプライト描画前処理
		Sprite::PreDraw(dxCommon->GetCmdList());
		if (Scene == title) {
			sprite[0]->Draw();
		}
		Sprite::PostDraw();
		Object::PreDraw(dxCommon->GetCmdList());
		Player::PreDraw(dxCommon->GetCmdList());
		Player2::PreDraw(dxCommon->GetCmdList());
		Player3::PreDraw(dxCommon->GetCmdList());
		Player4::PreDraw(dxCommon->GetCmdList());
		Light::PreDraw(dxCommon->GetCmdList());
		BackGround::PreDraw(dxCommon->GetCmdList());
		Screen::PreDraw(dxCommon->GetCmdList());
		//背景
		if (Scene == gamePlay) {
			//background->Draw();
			screen->Draw();
			//light->Draw();
			if (AnimetionCount == 0) {
				player->Draw();
			}
			else if (AnimetionCount == 1) {
				player2->Draw();
			}
			else if (AnimetionCount == 2) {
				player3->Draw();
			}
			else if (AnimetionCount == 3) {
				player4->Draw();
			}
			for (int i = 0; i < _countof(object); i++) {
				if (ObjectColor[i].w > 0.0f) {
					object[i]->Draw();
				}
			}
		
		}
	
		Sprite::PreDraw(dxCommon->GetCmdList());
		if (Scene == gameClear) {
			sprite[1]->Draw();
		}
	
		Sprite::PostDraw();
		// 3Dオブジェクト描画後処理
		Object::PostDraw();
		Player::PostDraw();
		Player2::PostDraw();
		Player3::PostDraw();
		Player4::PostDraw();
		Light::PostDraw();
		BackGround::PostDraw();
		Screen::PostDraw();
		dxCommon->PostDraw();
	}
#pragma endregion
#pragma endregion
#pragma region//開放処理
	winApp->Finalize();
	for (int i = 0; i < SpriteMax; i++) {
		delete sprite[i];
	}
	//入力開放
	delete input;
	delete audio;
	delete winApp;
	delete dxCommon;
	delete player;
	delete light;
	delete screen;
	for (int i = 0; i < _countof(object); i++) {
		delete object[i];
	}
	winApp = nullptr;
	return 0;
#pragma endregion
}
#pragma endregion