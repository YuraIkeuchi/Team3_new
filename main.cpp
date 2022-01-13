#include<Windows.h>
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
#include "Projector.h"
#include "Collision.h"
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
//イージング
float PI = 3.141592;
float aseInSine(const float x) {
	return 1 - cos((x * PI) / 2);
}

//ブロック置く処理
XMFLOAT3 sankaku(XMFLOAT3 screen, XMFLOAT3 Projector, XMFLOAT3 object) {
	XMFLOAT3 result = { screen.x,0,0 };
	float a, b, A, B;
	float add = 0.01f;

	a = fabsf(object.z - Projector.z + add);
	b = fabsf(object.y - Projector.y + add);
	A = fabsf(screen.z - Projector.z + add);

	B = A * (a / b);
	B += 1;
	B = B * object.z;
	result.z = B;

	a = fabsf(object.z - Projector.z + add);
	b = fabsf(object.x - Projector.x + add);
	A = fabsf(screen.z - Projector.z + add);

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
	const int OBJECT_NUM = 5;
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
	//影を作るためのブロック
	Object* Imageobject[OBJECT_NUM];
	for (int i = 0; i < _countof(Imageobject); i++) {
		if (!Imageobject[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		Imageobject[i] = Object::Create();
		Imageobject[i]->Update(matview);
	}

	XMFLOAT3 ImageObjectPosition[OBJECT_NUM];
	XMFLOAT4 ImageObjectColor[OBJECT_NUM];
	XMFLOAT3 Plus = { 0.0f,0.0f,0.0f };
	for (int i = 0; i < _countof(Imageobject); i++) {
		ImageObjectPosition[i] = Imageobject[i]->GetPosition();
		ImageObjectColor[i] = Imageobject[i]->GetColor();
		ImageObjectPosition[i] = { 0.0f,0.0f,-10.0f };
	}


	for (int i = 0; i < _countof(Imageobject); i++) {
		Imageobject[i]->SetPosition(ImageObjectPosition[i]);
	}
	//セットしたかどうかのフラグ
	int SetFlag[OBJECT_NUM] = { 0 };
	//ゲーム画面のブロック
	Object* Setobject[OBJECT_NUM];
	for (int i = 0; i < _countof(Setobject); i++) {
		if (!Setobject[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		Setobject[i] = Object::Create();
		Setobject[i]->Update(matview);
	}

	XMFLOAT3 SetobjectPosition[OBJECT_NUM];
	XMFLOAT4 SetobjectColor[OBJECT_NUM];
	int HitFlag[OBJECT_NUM] = { 0 };
	for (int i = 0; i < _countof(Setobject); i++) {
		SetobjectPosition[i] = Setobject[i]->GetPosition();
		SetobjectColor[i] = Setobject[i]->GetColor();
	}
	SetobjectPosition[0] = { -10.0f,-8.0f,50.0f };
	SetobjectPosition[1] = { 0.0f,-8.0f,50.0f };
	SetobjectPosition[2] = { 15.0f,5.0f,50.0f };
	SetobjectPosition[3] = { 10.0f,-8.0f,50.0f };
	SetobjectPosition[4] = { -15.0f,5.0f,50.0f };
	for (int i = 0; i < _countof(Setobject); i++) {
		Setobject[i]->SetPosition(SetobjectPosition[i]);
	}
#pragma endregion
#pragma region//後ろ
	Screen* screen;
	if (!screen->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	screen = Screen::Create();
	screen->Update(matview);

#pragma endregion
#pragma region//プロジェクター
	Projector* projector;
	if (!projector->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	 projector = Projector::Create();
	 projector->Update(matview);

#pragma endregion
#pragma region//プレイヤー変数
	//各プレイヤーの初期化
	Player* player;
	if (!player->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player = Player::Create();
	player->Update(matview);
	Player2* player2;
	if (!player2->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player2 = Player2::Create();
	player2->Update(matview);
	Player3* player3;
	if (!player3->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player3 = Player3::Create();
	player3->Update(matview);
	Player4* player4;
	if (!player4->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player4 = Player4::Create();
	player4->Update(matview);
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
	light->Update(matview);
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
#pragma region//カメラ
	XMVECTOR v0 = { 0,0,-120,0 };
	XMMATRIX rotM;// = XMMatrixIdentity();
	XMVECTOR eye2;
	XMVECTOR target2 = { PlayerPosition.x, 0,0,0};
	XMVECTOR up2 = { 0,0.0f,0,0 };
	
#pragma endregion
#pragma region//当たり判定
	Collision* Boxcollision = nullptr;

#pragma endregion
#pragma region//ループ処理

	while (true) {
		//ウィンドウメッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}
		for (int i = 0; i < _countof(Imageobject); i++) {
			ImageObjectPosition[i] = Imageobject[i]->GetPosition();
			ImageObjectColor[i] = Imageobject[i]->GetColor();
		}

		for (int i = 0; i < _countof(Setobject); i++) {
			SetobjectPosition[i] = Setobject[i]->GetPosition();
			SetobjectColor[i] = Setobject[i]->GetColor();
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
			//カメラ
			if (input->TriggerKey(DIK_M) && mode == 0 && modeflag == 1)
			{
				mode = 1;
			}

			else if (input->TriggerKey(DIK_M) && mode == 1 && modeflag == 0)
			{
				mode = 0;
			}

			//位置情報保存
			OldPlayerPosition.x = PlayerPosition.x;
			OldPlayerPosition.y = PlayerPosition.y;
			OldPlayerPosition.z = PlayerPosition.z;
			//プレイ中のプレイヤー移動
			if (mode == 0) {
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

				//ジャンプ処理
				if (input->TriggerKey(DIK_SPACE) && (JumpFlag == 0)) {
					JumpG = -1.0f;
					JumpFlag = 1;
				}
			}

			//設置画面
			if (mode == 1) {
				for (int i = 0; i < OBJECT_NUM; i++) {
					if (SetFlag[i] == 0) {
						//移動処理
						if (input->PushKey(DIK_UP)) {
							ImageObjectPosition[i].y += 0.5f;
						}

						if (input->PushKey(DIK_DOWN)) {
							ImageObjectPosition[i].y -= 0.5f;
						}

						if (input->PushKey(DIK_LEFT)) {
							ImageObjectPosition[i].x -= 0.5f;
							//PlayerRotation.z = 270;
						}
						if (input->PushKey(DIK_RIGHT)) {
							ImageObjectPosition[i].x += 0.5f;

						}
					}

					//ジャンプ処理
					if (input->TriggerKey(DIK_SPACE) && (SetFlag[i] == 0)) {
						SetFlag[i] = 1;
						break;
					}
				}
			}

			//ジャンプ処理
			PlayerPosition.y -= JumpG;
			JumpG += 0.025f;

			//アニメーションタイマー
			if (AnimetionTimer >= 8) {
				AnimetionCount++;
				AnimetionTimer = 0;
			}
			if (AnimetionCount == 4) {
				AnimetionCount = 0;
			}
	
		//カメラ移動
		//ゲーム画面
		if (mode == 0) {
			v0.m128_f32[0] -= 1.2f;
			v0.m128_f32[2] -= 0.5f;
			targetcamerapos.z += 0.7f;
			if (v0.m128_f32[0] <= 0.0f) {
				v0.m128_f32[0] = 0.0f;
			}
			if (v0.m128_f32[2] <= -50.0f) {
				v0.m128_f32[2] = -50.0f;
			}
			if (targetcamerapos.z >= 0.0f) {
				targetcamerapos.z = 0.0f;
			}
			if (v0.m128_f32[0] == 0.0f && v0.m128_f32[2] == -50.0f && targetcamerapos.z == 0.0f) {
				modeflag = 1;
			}
		}
		//設置画面
		if (mode == 1) {
			v0.m128_f32[0] += 1.2f;
			v0.m128_f32[2] += 0.5f;
			targetcamerapos.z = -15.0f;
			if (v0.m128_f32[0] >= 70.0f) {
				v0.m128_f32[0] = 70.0f;
			}
			if (v0.m128_f32[2] >= -10.0f) {
				v0.m128_f32[2] = -10.0f;
			}
			
			if (v0.m128_f32[0] == 70.0f && v0.m128_f32[2] == -10.0f) {
				modeflag = 0;
			}
		}
	
		//障害物との当たり判定
		for (int i = 0; i < OBJECT_NUM; i++) {
			if (SetobjectColor[i].w > 0.0f) {
				//playerとブロック左辺の当たり判定
				if (Boxcollision->BoxCollision_Left(PlayerPosition, { 7.5,5,4 }, SetobjectPosition[i], { 7.5,5,4 }) == TRUE) {
					PlayerPosition.x = OldPlayerPosition.x;
				}
				//playerとブロック右辺の当たり判定
				if (Boxcollision->BoxCollision_Right(PlayerPosition, { 7.5,5,4 }, SetobjectPosition[i], { 7.5,5,4 }) == TRUE) {
					PlayerPosition.x = OldPlayerPosition.x;
					//JumpG = 0.0f;
					//JumpG = 0.0f;
				}
				
				//playerとブロック下辺の当たり判定
				if (Boxcollision->BoxCollision_Down(PlayerPosition, { 3,5,4 }, SetobjectPosition[i], { 3,5,4 }) == TRUE) {
					PlayerPosition.y = OldPlayerPosition.y;
					JumpG = 0.0f;
				}

				//playerとブロック上辺の当たり判定
				if (Boxcollision->BoxCollision_Up(PlayerPosition, { 3,5,4 }, SetobjectPosition[i], { 3,5,4 }) == TRUE) {
					PlayerPosition.y = OldPlayerPosition.y;
					JumpG = 0.0f;
					JumpFlag = 0;
					HitFlag[i] = 1;
				} else {
					HitFlag[i] = 0;
				}

				}
			}
		

		//光とブロックの当たり判定
		for (int i = 0; i < _countof(Setobject); i++) {
			if (Boxcollision->CircleCollision(SetobjectPosition[i].x, SetobjectPosition[i].y, 3, LightPosition.x, LightPosition.y, 3) == 1) {
				SetobjectColor[i].w -= 0.01;
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
		for (int i = 0; i < _countof(Setobject); i++) {
			Setobject[i]->Update(matview);
			//ルートシグネチャの設定コマンド
			Setobject[i]->SetPosition(SetobjectPosition[i]);
			Setobject[i]->SetColor(SetobjectColor[i]);
		}

		for (int i = 0; i < _countof(Imageobject); i++) {
			Imageobject[i]->Update(matview);
			//ルートシグネチャの設定コマンド
			Imageobject[i]->SetPosition(ImageObjectPosition[i]);
			Imageobject[i]->SetColor(ImageObjectColor[i]);
		}

		//移動のやつ
		//カメラの注視点をプレイヤーの位置に固定
		target2.m128_f32[2] = 0;
		//カメラ追従用の処理
		target2.m128_f32[0] = PlayerPosition.x;
		target2.m128_f32[1] = 0;
		//行列を作り直す
		rotM = XMMatrixRotationX(XMConvertToRadians(angle));
		XMVECTOR v;
		v = XMVector3TransformNormal(v0, rotM);
		eye2 = target2 + v;
		matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

		background->Update();
		player->Update(matview);
		player2->Update(matview);
		player3->Update(matview);
		player4->Update(matview);
		light->Update(matview);
		screen->Update(matview);
		projector->Update(matview);
		player->SetPosition(PlayerPosition);
		player2->SetPosition(PlayerPosition);
		player3->SetPosition(PlayerPosition);
		player4->SetPosition(PlayerPosition);
		screen->SetPosition({PlayerPosition.x,0,320});
		projector->SetPosition({ PlayerPosition.x,-20,-50 });
		light->SetPosition(LightPosition);
		player->SetRotaition(PlayerRotation);
		player2->SetRotaition(PlayerRotation);
		player3->SetRotaition(PlayerRotation);
		player4->SetRotaition(PlayerRotation);
		
#pragma endregion
#pragma endregion
#pragma region//描画
		//描画コマンド
		dxCommon->PreDraw();
		////4.描画コマンドここから
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ImGui::Begin("test");
		if (ImGui::TreeNode("Debug"))
		{
			if (ImGui::TreeNode("Object0"))
			{
				ImGui::Text("SetFlag[0],%d", HitFlag[0]);
				ImGui::SliderFloat("Position.x", &SetobjectPosition[0].x, 50, -50);
				ImGui::SliderFloat("Position.y", &SetobjectPosition[0].y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Object1"))
			{
				ImGui::Text("SetFlag[1],%d", HitFlag[1]);
				ImGui::SliderFloat("Position.x", &SetobjectPosition[1].x, 50, -50);
				ImGui::SliderFloat("Position.y", &SetobjectPosition[1].y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("Object2"))
			{
				ImGui::Text("SetFlag[1],%d", HitFlag[2]);
				ImGui::SliderFloat("Position.x", &SetobjectPosition[2].x, 50, -50);
				ImGui::SliderFloat("Position.y", &SetobjectPosition[2].y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("Object3"))
			{
				ImGui::Text("SetFlag[1],%d", HitFlag[3]);
				ImGui::SliderFloat("Position.x", &SetobjectPosition[3].x, 50, -50);
				ImGui::SliderFloat("Position.y", &SetobjectPosition[3].y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Object4"))
			{
				ImGui::Text("SetFlag[1],%d", HitFlag[4]);
				ImGui::SliderFloat("Position.x", &SetobjectPosition[4].x, 50, -50);
				ImGui::SliderFloat("Position.y", &SetobjectPosition[4].y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Player"))
			{
				ImGui::Text("SetFlag[1],%d", SetFlag[1]);
				ImGui::SliderFloat("Position.x", &PlayerPosition.x, 50, -50);
				ImGui::SliderFloat("Position.y", &PlayerPosition.y, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
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
		Projector::PreDraw(dxCommon->GetCmdList());
		//背景
		if (Scene == gamePlay) {
			//background->Draw();
			screen->Draw();
			projector->Draw();
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
			for (int i = 0; i < _countof(Setobject); i++) {
				if (SetobjectColor[i].w > 0.0f) {
					Setobject[i]->Draw();
				}
			}

			for (int i = 0; i < _countof(Imageobject); i++) {
				//Imageobject[i]->Draw();
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
		Projector::PostDraw();
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
	delete projector;
	for (int i = 0; i < _countof(Imageobject); i++) {
		delete Imageobject[i];
	}
	winApp = nullptr;
	return 0;
#pragma endregion
}
#pragma endregion