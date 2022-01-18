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

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Sprite.h"
#include "Audio.h"
#include "Object3d.h"
#include "PlayerLeftWalk.h"
#include "PlayerLeftWalk2.h"
#include "PlayerLeftWalk3.h"
#include "PlayerLeftWalk4.h"
#include "PlayerRightWalk.h"
#include "PlayerRightWalk2.h"
#include "PlayerRightWalk3.h"
#include "PlayerRightWalk4.h"
#include "backGround.h"
#include "Light.h"
#include "Screen.h"
#include "Projector.h"
#include "Collision.h"
#include "LightSource.h"
#include "Item.h"
#include "Goal.h"
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

	const int SpriteMax = 20;
	const int SpriteNumberMax = 10;
	Sprite* sprite[SpriteMax] = { nullptr };
	// スプライト共通テクスチャ読み込み
	/*SpriteCommonLoadTexture(spriteCommon, 0, L"Resources/texture.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spriteCommon, 1, L"Resources/house.png", dxCommon->GetDev());*/
	Sprite::LoadTexture(0, L"Resources/Title.png");
	Sprite::LoadTexture(1, L"Resources/END.png");
	Sprite::LoadTexture(2, L"Resources/ItemUI.png");
	Sprite::LoadTexture(3, L"Resources/0.png");
	Sprite::LoadTexture(4, L"Resources/1.png");
	Sprite::LoadTexture(5, L"Resources/2.png");
	Sprite::LoadTexture(6, L"Resources/3.png");
	Sprite::LoadTexture(7, L"Resources/4.png");
	Sprite::LoadTexture(8, L"Resources/5.png");
	Sprite::LoadTexture(9, L"Resources/6.png");
	Sprite::LoadTexture(10, L"Resources/7.png");
	Sprite::LoadTexture(11, L"Resources/8.png");
	Sprite::LoadTexture(12, L"Resources/9.png");
	Sprite::LoadTexture(13, L"Resources/STAGECLEAR.png");
	Sprite::LoadTexture(14, L"Resources/GAMECLEAR.png");
	Sprite::LoadTexture(15, L"Resources/GAMEOVER.png");
	Sprite::LoadTexture(16, L"Resources/EXPLATION.png");
	sprite[0] = Sprite::Create(0, { 0.0f,0.0f });
	sprite[1] = Sprite::Create(1, { 0.0f,0.0f });
	sprite[2] = Sprite::Create(2, { 0.0f,0.0f });
	sprite[3] = Sprite::Create(13, { 0.0f,0.0f });
	sprite[4] = Sprite::Create(14, { 0.0f,0.0f });
	sprite[5] = Sprite::Create(15, { 0.0f,0.0f });
	sprite[6] = Sprite::Create(16, { 0.0f,0.0f });
	Sprite* spriteNumber[SpriteNumberMax] = { nullptr };
	spriteNumber[0] = Sprite::Create(3, { 0.0f,0.0f });
	spriteNumber[1] = Sprite::Create(4, { 0.0f,0.0f });
	spriteNumber[2] = Sprite::Create(5, { 0.0f,0.0f });
	spriteNumber[3] = Sprite::Create(6, { 0.0f,0.0f });
	spriteNumber[4] = Sprite::Create(7, { 0.0f,0.0f });
	spriteNumber[5] = Sprite::Create(8, { 0.0f,0.0f });
	spriteNumber[6] = Sprite::Create(9, { 0.0f,0.0f });
	spriteNumber[7] = Sprite::Create(10, { 0.0f,0.0f });
	spriteNumber[8] = Sprite::Create(11, { 0.0f,0.0f });
	spriteNumber[9] = Sprite::Create(12, { 0.0f,0.0f });
	for (int i = 0; i < SpriteNumberMax; i++) {
		spriteNumber[i]->SetColor({ 0.0f,0.0f,0.0f,1.0f });
		spriteNumber[i]->SetSize({ 120.0f,120.0f });
		spriteNumber[i]->SetPosition({ 210,0 });
	}
	//sprite[2]->SetPosition({ 200,20 });
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
	for (int i = 0; i < _countof(Imageobject); i++) {
		ImageObjectPosition[i] = Imageobject[i]->GetPosition();
		ImageObjectColor[i] = Imageobject[i]->GetColor();
		ImageObjectPosition[i] = { 0.0f,0.0f,70.0f };
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
	for (int i = 0; i < _countof(Setobject); i++) {
		SetobjectPosition[i] = { 0.0f,400.0f,0.0f };
		SetobjectColor[i] = Setobject[i]->GetColor();

	}
	for (int i = 0; i < _countof(Setobject); i++) {
		Setobject[i]->SetPosition(SetobjectPosition[i]);
	}
	//目印のマーク
	Object* Markobject[OBJECT_NUM];
	for (int i = 0; i < _countof(Markobject); i++) {
		if (!Markobject[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		Markobject[i] = Object::Create();
		Markobject[i]->Update(matview);
	}

	XMFLOAT3 MarkobjectPosition[OBJECT_NUM];
	XMFLOAT4 MarkobjectColor[OBJECT_NUM];
	for (int i = 0; i < _countof(Markobject); i++) {
		MarkobjectColor[i] = Imageobject[i]->GetColor();
		MarkobjectPosition[i] = { 0.0f,0.0f,134.0f };
	}
	for (int i = 0; i < _countof(Markobject); i++) {
		Markobject[i]->SetPosition(MarkobjectPosition[i]);
	}

	//普通のブロック
	const int FIELD_NUM = 20;
	Object* Fieldobject[FIELD_NUM];
	XMFLOAT3 FieldobjectPosition[FIELD_NUM];
	XMFLOAT4 FieldobjectColor[FIELD_NUM];
	for (int i = 0; i < _countof(Fieldobject); i++) {
		if (!Fieldobject[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		Fieldobject[i] = Object::Create();
		Fieldobject[i]->Update(matview);
	
	}

	for (int i = 0; i < 4; i++) {
		FieldobjectPosition[i] = { -150 + ((float)i * 10),0,134 };
		Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
	}

	for (int i = 4; i < 10; i++) {
		FieldobjectPosition[i] = { -100 + ((float)i * 10),0,134 };
		Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
	}

	for (int i = 10; i < 20; i++) {
		FieldobjectPosition[i] = { -80 + ((float)i * 10),-70,134 };
		Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
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
	XMFLOAT3 screenPos = screen->GetPosition();
#pragma endregion
#pragma region//プロジェクター
	Projector* projector;
	if (!projector->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	 projector = Projector::Create();
	 projector->Update(matview);

	 XMFLOAT3 projectorPos = projector->GetPosition();
#pragma endregion
#pragma region//光源
	 LightSource* lightSource;
	 if (!lightSource->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		 assert(0);
		 return 1;
	 }
	 lightSource = LightSource::Create();
	 lightSource->Update(matview);

	 XMFLOAT3 lightSourcePos = lightSource->GetPosition();
#pragma endregion
#pragma region//プレイヤー変数
#pragma region//左向きのプレイヤー変数
	//各プレイヤーの初期化
	PlayerLeftWalk* playerleftwalk;
	if (!playerleftwalk->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerleftwalk = PlayerLeftWalk::Create();
	playerleftwalk->Update(matview);
	PlayerLeftWalk2* playerleftwalk2;
	if (!playerleftwalk2->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerleftwalk2 = PlayerLeftWalk2::Create();
	playerleftwalk2->Update(matview);
	PlayerLeftWalk3* playerleftwalk3;
	if (!playerleftwalk3->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerleftwalk3 = PlayerLeftWalk3::Create();
	playerleftwalk3->Update(matview);
	PlayerLeftWalk4* playerleftwalk4;
	if (!playerleftwalk4->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerleftwalk4 = PlayerLeftWalk4::Create();
	playerleftwalk4->Update(matview);
#pragma endregion
#pragma region//右向きのプレイヤー変数
	//各プレイヤーの初期化
	PlayerRightWalk* playerrightwalk;
	if (!playerrightwalk->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerrightwalk = PlayerRightWalk::Create();
	playerrightwalk->Update(matview);
	PlayerRightWalk2* playerrightwalk2;
	if (!playerrightwalk2->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerrightwalk2 = PlayerRightWalk2::Create();
	playerrightwalk2->Update(matview);
	PlayerRightWalk3* playerrightwalk3;
	if (!playerrightwalk3->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerrightwalk3 = PlayerRightWalk3::Create();
	playerrightwalk3->Update(matview);
	PlayerRightWalk4* playerrightwalk4;
	if (!playerrightwalk4->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	playerrightwalk4 = PlayerRightWalk4::Create();
	playerrightwalk4->Update(matview);
#pragma endregion
	XMFLOAT3 PlayerPosition;
	PlayerPosition = { -140.0f,10.0f,135.0f };
	
	XMFLOAT3 OldPlayerPosition;
	XMFLOAT3 PlayerRotation;
	PlayerRotation = playerleftwalk->GetRotaition();
	playerleftwalk->SetPosition(PlayerPosition);
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
	int PlayerAlive = 1;
	int PlayerDirectionNumber = 0;
	int PlayerHitNumber = 0;
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
#pragma region//アイテム変数
	const int Item_NUM = 10;
	Item* item[Item_NUM];
	XMFLOAT3 ItemPosition[Item_NUM];
	int ItemAlive[Item_NUM];
	int ItemCount = 0;

	for (int i = 0; i < Item_NUM; i++) {
		if (!item[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		item[i] = Item::Create();
		item[i]->Update(matview);
		ItemPosition[i] = { 0,300,134 };
		ItemAlive[i] = 1;
		item[i]->SetPosition(ItemPosition[i]);
	}

	ItemPosition[0] = { -120,10,134 };
	ItemPosition[1] = { -40,10,134 };
#pragma endregion
#pragma region//ゴール
	Goal* goal;
	if (!goal->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	goal = Goal::Create();
	goal->Update(matview);
	XMFLOAT3 GoalPosition = { 110.0f,-60.0f,134.0f };
	goal->SetPosition(GoalPosition);
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
#pragma region//シーン変数
	int Scene = 0;
	int StageNumber = 1;
	int ResetFlag = 0;
	int SpaceCount = 0;
	enum Scene {
		title,
		explation,
		gamePlay,
		gameOver,
		stageClear,
		gameClear
	};
#pragma endregion
#pragma region//カメラ
	XMVECTOR v0 = { 0,0,-50,0 };
	XMMATRIX rotM;// = XMMatrixIdentity();
	XMVECTOR eye2;
	XMVECTOR target2 = { PlayerPosition.x, 0,0,0};
	XMVECTOR up2 = { 0,0.0f,0,0 };
#pragma endregion
#pragma region//当たり判定
	Collision* Boxcollision = nullptr;

#pragma endregion
#pragma region//ループ処理
	XMFLOAT3 kage[4];
	while (true) {
		//ウィンドウメッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}
		for (int i = 0; i < _countof(Imageobject); i++) {
			ImageObjectPosition[i] = Imageobject[i]->GetPosition();
			if (ItemCount != 0) {
				Imageobject[i]->SetColor({ 0.0f,1.0f,0.0f,1.0 });
			} else {
				Imageobject[i]->SetColor({ 1.0f,0.0f,0.0f,1.0 });
			}
			Imageobject[i]->SetScale({ 1.0f,1.0f,1.0f});
		}

		for (int i = 0; i < _countof(Setobject); i++) {
			SetobjectPosition[i] = Setobject[i]->GetPosition();
			SetobjectColor[i] = Setobject[i]->GetColor();
		}

		for (int i = 0; i < _countof(Markobject); i++) {
			Markobject[i]->SetColor({ 1.0f,0.0f,0.0f,1.0 });
		}

		for (int i = 0; i < _countof(Fieldobject); i++) {
			Fieldobject[i]->SetColor({ 0.3f,0.3f,0.3f,1.0f });
		}
		LightPosition = light->GetPosition();
		SpritePosition = sprite[0]->GetPosition();
		
#pragma region//更新処理
		//キーの更新
		input->Update();
#pragma region//タイトル
		if (Scene == title) {
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = explation;
				SpaceCount++;
			}
		}
#pragma endregion
#pragma region//説明
		if (Scene == explation) {
			if (input->TriggerKey(DIK_SPACE)) {
				SpaceCount++;
			}

			if (SpaceCount == 3) {
				Scene = gamePlay;
				SpaceCount = 0;
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

			if (mode == 0) {
				v0.m128_f32[2] += 1.5f;
				v0.m128_f32[1] -= 1.0f;
				if (v0.m128_f32[2] >= -20.0f) {
					v0.m128_f32[2] = -20.0f;
				}
				if (v0.m128_f32[1] <= 0.0f) {
					v0.m128_f32[1] = 0.0f;
				}
				if (v0.m128_f32[2] == -20.0f && v0.m128_f32[1] == 0.0f) {
					modeflag = 1;
				}
			}
			if (mode == 1) {
				v0.m128_f32[2] -= 1.5f;
				v0.m128_f32[1] += 1.0f;
				if (v0.m128_f32[2] <= -140.0f) {
					v0.m128_f32[2] = -140.0f;
				}
				if (v0.m128_f32[1] >= 60.0f) {
					v0.m128_f32[1] = 60.0f;
				}
				if (v0.m128_f32[2] == -140.0f && v0.m128_f32[1] == 60.0f) {
					modeflag = 0;
				}
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
					AnimetionTimer++;
					PlayerDirectionNumber = 1;
				}
				if (input->PushKey(DIK_RIGHT)) {
					PlayerPosition.x += 0.5f;
					AnimetionTimer++;
					PlayerDirectionNumber = 0;
				}

				//ジャンプ処理
				if (input->TriggerKey(DIK_SPACE) && (JumpFlag == 0)) {
					JumpG = -1.0f;
					JumpFlag = 1;
				}

				//ジャンプ処理
				PlayerPosition.y -= JumpG;
				JumpG += 0.025f;
			}

			//設置画面
			if (mode == 1) {
				XMFLOAT3 temp[4];
				for (int i = 0; i < 4; i++) {
					temp[i] = { 0.0f,0.0f,screenPos.z };
					kage[i] = { 0.0f,0.0f,screenPos.z };
				}

				for (int i = 0; i < OBJECT_NUM; i++) {
					float a1;
					float a2;
					float b;
					float c;

					float A;
					float B;
					float C;
					float boxRadius = 1.0f;
					a1 = (ImageObjectPosition[i].z - boxRadius) - projectorPos.z;
					a2 = (ImageObjectPosition[i].z + boxRadius) - projectorPos.z;

					b = boxRadius + (projectorPos.y - ImageObjectPosition[i].y);

					c = (ImageObjectPosition[i].y + boxRadius) - projectorPos.y;

					A = screenPos.z - projectorPos.z;

					if (abs(ImageObjectPosition[i].y - projectorPos.y) <= boxRadius) {
						B = (b * A) / a1;
						C = (c * A) / a1;
						float lineY = projectorPos.y - B;
						float lineLength = B + C;
						temp[0].y = lineY;
						temp[1].y = lineLength;
					}

					else if (ImageObjectPosition[i].y < projectorPos.y) {
						B = (b * A) / a1;
						C = (c * A) / a2;
						float lineY = projectorPos.y - B;
						float lineLength = B + C;
						temp[0].y = lineY;
						temp[1].y = lineLength;
					} else if (ImageObjectPosition[i].y > projectorPos.y) {
						B = (b * A) / a2;
						C = (c * A) / a1;
						float lineY = projectorPos.y - B;
						float lineLength = B + C;
						temp[0].y = lineY;
						temp[1].y = lineLength;
					}

					a1 = (ImageObjectPosition[i].z - boxRadius) - projectorPos.z;
					a2 = (ImageObjectPosition[i].z + boxRadius) - projectorPos.z;

					b = boxRadius + (projectorPos.x - ImageObjectPosition[i].x);

					c = (ImageObjectPosition[i].x + boxRadius) - projectorPos.x;

					A = screenPos.z - projectorPos.z;

					if (abs(ImageObjectPosition[i].x - projectorPos.x) <= boxRadius) {
						B = (b * A) / a1;
						C = (c * A) / a1;
						float lineY = projectorPos.x - B;
						float lineLength = B + C;
						temp[2].x = lineY;
						temp[3].x = lineLength;
					} else if (ImageObjectPosition[i].x < projectorPos.x) {
						B = (b * A) / a1;
						C = (c * A) / a2;
						float lineY = projectorPos.x - B;
						float lineLength = B + C;
						temp[2].x = lineY;
						temp[3].x = lineLength;
					} else if (ImageObjectPosition[i].x > projectorPos.x) {
						B = (b * A) / a2;
						C = (c * A) / a1;
						float lineY = projectorPos.x - B;
						float lineLength = B + C;
						temp[2].x = lineY;
						temp[3].x = lineLength;
					}
				}
				kage[0] = { temp[2].x,temp[0].y,screenPos.z - 10.0f };
				kage[1] = { temp[3].x,temp[0].y,screenPos.z - 10.0f };
				kage[2] = { temp[3].x,temp[1].y,screenPos.z - 10.0f };
				kage[3] = { temp[2].x,temp[1].y,screenPos.z - 10.0f };

				float kageXR = (kage[1].x - kage[0].x) / 2;
				float kageYR = (kage[3].y - kage[0].y) / 2;
				kage[0].x = kage[0].x + kageXR;
				kage[0].y = kage[0].y + kageYR;
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
						MarkobjectPosition[i].x = kage[0].x;
						MarkobjectPosition[i].y = kage[0].y;
						MarkobjectPosition[i].z = 134.0f;
					}

					//ブロックを置く処理
					if (input->TriggerKey(DIK_SPACE) && (SetFlag[i] == 0) && (ItemCount != 0)) {
						ItemCount--;
						SetFlag[i] = 1;
						SetobjectPosition[i].x = kage[0].x;
						SetobjectPosition[i].y = kage[0].y;
						SetobjectPosition[i].z = 134.0f;
						break;
					}
				}
			}

			//アニメーションタイマー
			if (AnimetionTimer >= 8) {
				AnimetionCount++;
				AnimetionTimer = 0;
			}
			if (AnimetionCount == 4) {
				AnimetionCount = 0;
			}

			//カメラ移動
			//障害物との当たり判定
			for (int i = 0; i < OBJECT_NUM; i++) {
				if (SetFlag[i] == 1) {
					if (SetobjectColor[i].w > 0.0f) {
						//playerとブロック左辺の当たり判定
						if (Boxcollision->BoxCollision_Left(PlayerPosition, { 7.5,5.4,4 }, SetobjectPosition[i], { 7.5,4,4 }) == TRUE) {
							PlayerPosition.x = OldPlayerPosition.x;
						}
						//playerとブロック右辺の当たり判定
						if (Boxcollision->BoxCollision_Right(PlayerPosition, { 7.5,5.4,4 }, SetobjectPosition[i], { 7.5,4,4 }) == TRUE) {
							PlayerPosition.x = OldPlayerPosition.x;
						}

						//playerとブロック下辺の当たり判定
						if (Boxcollision->BoxCollision_Down(PlayerPosition, { 3,5.3,4 }, SetobjectPosition[i], { 3,5.3,4 }) == TRUE) {
							PlayerPosition.y = OldPlayerPosition.y;
							JumpG = 0.0f;
						}

						//playerとブロック上辺の当たり判定(高いところからだと死)
						if (Boxcollision->BoxCollision_Up(PlayerPosition, { 3.2,5.2,4 }, SetobjectPosition[i], { 3.2,5.2,4 }) == TRUE) {
							PlayerPosition.y = OldPlayerPosition.y;
							if (JumpG >= 1.85f) {
								PlayerAlive = 0;
							} else {
								JumpG = 0.0f;
							}
							JumpFlag = 0;
						}
					}
				}
			}

			for (int i = 0; i < FIELD_NUM; i++) {
				//playerとブロック左辺の当たり判定
				if (Boxcollision->BoxCollision_Left(PlayerPosition, { 7.5,5.4,4 }, FieldobjectPosition[i], { 7.5,5.4,4 }) == TRUE) {
					PlayerHitNumber = 1;
					PlayerPosition.x = OldPlayerPosition.x;
				}
				//playerとブロック右辺の当たり判定
				if (Boxcollision->BoxCollision_Right(PlayerPosition, { 7.5,5.4,4 }, FieldobjectPosition[i], { 7.5,5.4,4 }) == TRUE) {
					PlayerHitNumber = 2;
					PlayerPosition.x = OldPlayerPosition.x;
				}

				//playerとブロック下辺の当たり判定
				if (Boxcollision->BoxCollision_Down(PlayerPosition, { 3,5.3,4 }, FieldobjectPosition[i], { 3,5.3,4 }) == TRUE) {
					PlayerHitNumber = 3;
					PlayerPosition.y = OldPlayerPosition.y;
					JumpG = 0.0f;
				}

				//playerとブロック上辺の当たり判定(高いところからだと死)
				if (Boxcollision->BoxCollision_Up(PlayerPosition, { 3.2,5.2,4 }, FieldobjectPosition[i], { 3.2,5.2,4 }) == TRUE) {
					PlayerHitNumber = 4;
					PlayerPosition.y = OldPlayerPosition.y;
					JumpFlag = 0;
					if (JumpG >= 1.85f) {
						PlayerAlive = 0;
					} else {
						JumpG = 0.0f;
					}
				}
			}

			//光とブロックの当たり判定
			for (int i = 0; i < _countof(Setobject); i++) {
				if (SetFlag[i] == 1) {
					if (Boxcollision->CircleCollision(SetobjectPosition[i].x, SetobjectPosition[i].y, 3, LightPosition.x, LightPosition.y, 3) == 1) {
						//SetobjectColor[i].w -= 0.01;
					}
				}
			}

			//アイテム取得判定
			for (int i = 0; i < Item_NUM; i++) {
				if (ItemAlive[i] == 1) {
					if (Boxcollision->CircleCollision(PlayerPosition.x, PlayerPosition.y, 3, ItemPosition[i].x, ItemPosition[i].y, 3) == 1) {
						ItemCount++;
						ItemAlive[i] = 0;
					}
				}
			}

			//ゴール判定
			if (Boxcollision->CircleCollision(PlayerPosition.x, PlayerPosition.y, 3, GoalPosition.x, GoalPosition.y, 3) == 1) {
				StageNumber++;
				if (StageNumber != 3) {
					Scene = stageClear;
				} else {
					Scene = gameClear;
				}
			}

			//画面外で死
			if (PlayerPosition.y <= -130) {
				PlayerAlive = 0;
			}

			if (PlayerAlive == 0) {
				Scene = gameOver;
			}
		}

#pragma region//ステージクリア
		if (Scene == stageClear) {
			ResetFlag = 1;
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = gamePlay;
				ResetFlag = 0;
			
			}
		}
#pragma endregion
#pragma region//ゲームオーバー
		if (Scene == gameOver) {
			ResetFlag = 1;
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = gamePlay;
				ResetFlag = 0;
			}
		}
#pragma endregion
#pragma endregion
#pragma region//クリア
		if (Scene == gameClear) {
			StageNumber = 1;
			ResetFlag = 1;
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = title;
				
				ResetFlag = 0;
				SpaceCount = 0;
			}
		}

		if (ResetFlag == 1) {
			//各変数初期化
			JumpG = 0.0f;
			PlayerAlive = 1;
			PlayerDirectionNumber = 0;
			ItemCount = 0;
			for (int i = 0; i < _countof(Setobject); i++) {
				SetobjectPosition[i] = { 0.0f,400.0f,0.0f };
				SetFlag[i] = 0;
			}

			for (int i = 0; i < _countof(Markobject); i++) {
				MarkobjectPosition[i] = { 0.0f,0.0f,134.0f };
			}

			for (int i = 0; i < Item_NUM; i++) {
				ItemPosition[i] = { 0,300,134 };
				ItemAlive[i] = 1;
				item[i]->SetPosition(ItemPosition[i]);
			}
			for (int i = 0; i < _countof(Imageobject); i++) {
				ImageObjectPosition[i] = { 0.0f,0.0f,70.0f };
			}

			for (int i = 0; i < _countof(Fieldobject); i++) {
				FieldobjectPosition[i] = { 0.0f,400.0f,0.0f };
			}

			if (StageNumber == 1) {
				PlayerPosition = { -140.0f,20.0f,135.0f };
				for (int i = 0; i < 4; i++) {
					FieldobjectPosition[i] = { -150 + ((float)i * 10),0,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				for (int i = 4; i < 10; i++) {
					FieldobjectPosition[i] = { -100 + ((float)i * 10),0,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				for (int i = 10; i < 20; i++) {
					FieldobjectPosition[i] = { -80 + ((float)i * 10),-70,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				ItemPosition[0] = { -120,10,134 };
				ItemPosition[1] = { -40,10,134 };
			}

			else if (StageNumber == 2) {
				PlayerPosition = { -140.0f,20.0f,135.0f };
				for (int i = 0; i < 5; i++) {
					FieldobjectPosition[i] = { -150 + ((float)i * 10),0,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				for (int i = 5; i < 10; i++) {
					FieldobjectPosition[i] = { -70 + ((float)i * 10),40,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				for (int i = 10; i < 15; i++) {
					FieldobjectPosition[i] = { -20 + ((float)i * 10),-70,134 };
					Fieldobject[i]->SetPosition({ FieldobjectPosition[i] });
				}

				ItemPosition[0] = { -120,10,134 };
				ItemPosition[1] = { -70,25,134 };
				ItemPosition[2] = { 30,50,134 };
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

		for (int i = 0; i < _countof(Markobject); i++) {
			Markobject[i]->Update(matview);
			//ルートシグネチャの設定コマンド
			Markobject[i]->SetPosition(MarkobjectPosition[i]);
			Markobject[i]->SetColor(MarkobjectColor[i]);
		}

		for (int i = 0; i < _countof(Fieldobject); i++) {
			Fieldobject[i]->Update(matview);
		}

		for (int i = 0; i < Item_NUM; i++) {
			item[i]->Update(matview);
			item[i]->SetPosition(ItemPosition[i]);
		}
		//移動のやつ
		//カメラの注視点をプレイヤーの位置に固定
		target2.m128_f32[2] = 0;
		//カメラ追従用の処理
		target2.m128_f32[0] = 0;
		target2.m128_f32[1] = 0;
		//行列を作り直す
		rotM = XMMatrixRotationX(XMConvertToRadians(angle));
		XMVECTOR v;
		v = XMVector3TransformNormal(v0, rotM);
		eye2 = target2 + v;
		matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

		background->Update();
		playerleftwalk->Update(matview);
		playerleftwalk2->Update(matview);
		playerleftwalk3->Update(matview);
		playerleftwalk4->Update(matview);
		playerrightwalk->Update(matview);
		playerrightwalk2->Update(matview);
		playerrightwalk3->Update(matview);
		playerrightwalk4->Update(matview);
		light->Update(matview);
		screen->Update(matview);
		projector->Update(matview);
		goal->Update(matview);
		lightSource->Update(matview);
		playerleftwalk->SetPosition(PlayerPosition);
		playerleftwalk2->SetPosition(PlayerPosition);
		playerleftwalk3->SetPosition(PlayerPosition);
		playerleftwalk4->SetPosition(PlayerPosition);
		playerrightwalk->SetPosition(PlayerPosition);
		playerrightwalk2->SetPosition(PlayerPosition);
		playerrightwalk3->SetPosition(PlayerPosition);
		playerrightwalk4->SetPosition(PlayerPosition);
		screen->SetPosition({0,50,400});
		projector->SetPosition({ 0,-20,-50 });
		goal->SetPosition(GoalPosition);
		lightSource->SetPosition({ 0,0,180 });
		light->SetPosition(LightPosition);
	
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
			if (ImGui::TreeNode("Player"))
			{
				ImGui::Text("stageNumber,%d", StageNumber);
				ImGui::Text("ResetFlag,%d",ResetFlag);
				ImGui::Text("SpaceCOunt,%d", SpaceCount);
				ImGui::SliderFloat("Position.x", &PlayerPosition.x, 50, -50);
				ImGui::SliderFloat("Position.y", &PlayerPosition.y, 50, -50);
				ImGui::SliderFloat("Position.z", &PlayerPosition.z, 50, -50);
				ImGui::SliderFloat("JumpG", &JumpG, 50, -50);
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
		//背景スプライト描画前処理
		Sprite::PreDraw(dxCommon->GetCmdList());
	
		Sprite::PostDraw();
		Object::PreDraw(dxCommon->GetCmdList());
		PlayerLeftWalk::PreDraw(dxCommon->GetCmdList());
		PlayerLeftWalk2::PreDraw(dxCommon->GetCmdList());
		PlayerLeftWalk3::PreDraw(dxCommon->GetCmdList());
		PlayerLeftWalk4::PreDraw(dxCommon->GetCmdList());
		PlayerRightWalk::PreDraw(dxCommon->GetCmdList());
		PlayerRightWalk2::PreDraw(dxCommon->GetCmdList());
		PlayerRightWalk3::PreDraw(dxCommon->GetCmdList());
		PlayerRightWalk4::PreDraw(dxCommon->GetCmdList());
		Light::PreDraw(dxCommon->GetCmdList());
		BackGround::PreDraw(dxCommon->GetCmdList());
		Screen::PreDraw(dxCommon->GetCmdList());
		Projector::PreDraw(dxCommon->GetCmdList());
		Goal::PreDraw(dxCommon->GetCmdList());
		LightSource::PreDraw(dxCommon->GetCmdList());
		Item::PreDraw(dxCommon->GetCmdList());
		//背景
		if (Scene == gamePlay) {
			//background->Draw();
			//スクリーン
			screen->Draw();
			//プロジェクター
			projector->Draw();
			//プレイヤー
			if (AnimetionCount == 0) {
				if (PlayerDirectionNumber == 0) {
					playerrightwalk->Draw();
				}
				else if (PlayerDirectionNumber == 1) {
					playerleftwalk->Draw();
				}
			}
			else if (AnimetionCount == 1) {
				if (PlayerDirectionNumber == 0) {
					playerrightwalk2->Draw();
				} else if (PlayerDirectionNumber == 1) {
					playerleftwalk2->Draw();
				}
			}
			else if (AnimetionCount == 2) {
				if (PlayerDirectionNumber == 0) {
					playerrightwalk3->Draw();
				} else if (PlayerDirectionNumber == 1) {
					playerleftwalk3->Draw();
				}
			}
			else if (AnimetionCount == 3) {
				if (PlayerDirectionNumber == 0) {
					playerrightwalk4->Draw();
				} else if (PlayerDirectionNumber == 1) {
					playerleftwalk4->Draw();
				}
			}
			//アイテム
			for (int i = 0; i < Item_NUM; i++) {
				if (ItemAlive[i] == 1) {
					item[i]->Draw();
				}
			}
			//ゴール
			goal->Draw();
			//ブロック
			for (int i = 0; i < _countof(Fieldobject); i++) {
				Fieldobject[i]->Draw();
			}
			for (int i = 0; i < _countof(Setobject); i++) {
				if (SetFlag[i] == 1) {
					if (SetobjectColor[i].w > 0.0f) {
						Setobject[i]->Draw();
					}
				} 
				else if((SetFlag[i] == 0) && (mode == 1) && (ItemCount != 0)) {
					Markobject[i]->Draw();
				}
			}

			if (mode == 1) {
				for (int i = 0; i < _countof(Imageobject); i++) {
					Imageobject[i]->Draw();
				}
				lightSource->Draw();
			}
		}
	
		Sprite::PreDraw(dxCommon->GetCmdList());
		if (Scene == title) {
			sprite[0]->Draw();
		}

		else if (Scene == explation) {
			sprite[6]->Draw();
		}

		else if (Scene == gamePlay) {
			sprite[2]->Draw();
			spriteNumber[ItemCount]->Draw();
		}

		else if (Scene == stageClear) {
			sprite[3]->Draw();
		}

		else if (Scene == gameClear) {
			sprite[4]->Draw();
		}
	
		else if (Scene == gameOver) {
			sprite[5]->Draw();
		}
		Sprite::PostDraw();
		// 3Dオブジェクト描画後処理
		Object::PostDraw();
		PlayerLeftWalk::PostDraw();
		PlayerLeftWalk2::PostDraw();
		PlayerLeftWalk3::PostDraw();
		PlayerLeftWalk4::PostDraw();
		PlayerRightWalk::PostDraw();
		PlayerRightWalk2::PostDraw();
		PlayerRightWalk3::PostDraw();
		PlayerRightWalk4::PostDraw();
		Light::PostDraw();
		BackGround::PostDraw();
		Screen::PostDraw();
		Projector::PostDraw();
		Goal::PostDraw();
		LightSource::PostDraw();
		Item::PostDraw();
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
	delete playerleftwalk;
	delete playerleftwalk2;
	delete playerleftwalk3;
	delete playerleftwalk4;
	delete playerrightwalk;
	delete playerrightwalk2;
	delete playerrightwalk3;
	delete playerrightwalk4;
	delete light;
	for (int i = 0; i < Item_NUM; i++) {
		delete item[i];
	}
	delete screen;
	delete projector;
	delete lightSource;
	delete goal;
	for (int i = 0; i < _countof(Imageobject); i++) {
		delete Imageobject[i];
	}
	for (int i = 0; i < _countof(Markobject); i++) {
		delete Markobject[i];
	}
	for (int i = 0; i < _countof(Fieldobject); i++) {
		delete Fieldobject[i];
	}
	winApp = nullptr;
	return 0;
#pragma endregion
}
#pragma endregion