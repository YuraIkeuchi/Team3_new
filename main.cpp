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
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//テクスチャの最大枚数
const int SpriteSRVCount = 512;

int collision2(const float& X1, const float& Y1, const float& R1, const float& X2, const float& Y2, const float& R2) {
	int a = X1 - X2;
	int b = Y1 - Y2;
	int distance = sqrtf(a * a + b * b);
	int radius = R1 + R2;
	if (distance <= radius) {
		return TRUE;
	}
}

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

	//描画初期化処理
	//頂点データ3点分の座標
//#pragma regin 描画初期化処理
	//頂点データ構造体
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
	//スプライト共通データ
	if (!Sprite::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 0;
	}
	int map[3][3] = {
		0,0,0,
		1,0,1,
		0,0,1
	};

	const int SpriteMax = 10;
	Sprite* sprite[SpriteMax] = { nullptr };
	// スプライト共通テクスチャ読み込み
	/*SpriteCommonLoadTexture(spriteCommon, 0, L"Resources/texture.png", dxCommon->GetDev());
	SpriteCommonLoadTexture(spriteCommon, 1, L"Resources/house.png", dxCommon->GetDev());*/
	Sprite::LoadTexture(0, L"Resources/Title.png");
	Sprite::LoadTexture(1, L"Resources/END.png");
	sprite[0] = Sprite::Create(0, { 0.0f,0.0f });
	sprite[1] = Sprite::Create(1, { 0.0f,0.0f });
	//オーディオ
	const int AudioMax = 3;
	Audio* audio = new Audio;
	if (!audio->Initialize()) {
		assert(0);
		return 1;
	}
	audio->LoadSound(0, "Resources/Sound/kadai_BGM.wav");
	audio->LoopWave(0, 0.5f);
	Object* object[OBJECT_NUM];
	for (int i = 0; i < _countof(object); i++) {
		if (!object[i]->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
			assert(0);
			return 1;
		}
		object[i] = Object::Create();
		object[i]->Update();
	}

	Object* player;
	if (!player->StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height)) {
		assert(0);
		return 1;
	}
	player = Object::Create();
	player->Update();
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

	//キー処理
	//入力の初期化
	input = new Input();
	input->Initialize(winApp);
	XMFLOAT3 ObjectPosition[OBJECT_NUM];

	XMFLOAT3 Plus = { 0.0f,0.0f,0.0f };
	for (int i = 0; i < _countof(object); i++) {
		ObjectPosition[i] = object[i]->GetPosition();
	}

	ObjectPosition[0] = { -20.0f,-10.0f,0.0f };
	ObjectPosition[1] = { -5.0f,0.0f,0.0f };
	for (int i = 0; i < _countof(object); i++) {
		object[i]->SetPosition(ObjectPosition[i]);
	}

	XMFLOAT3 PlayerPosition;
	PlayerPosition = { 0.0f,0.0f,0.0f };
	PlayerPosition = player->GetPosition();

	player->SetPosition(PlayerPosition);
	XMFLOAT2 SpritePosition = sprite[0]->GetPosition();
	int HitFlag = 0;
	int RightHitFlag = 0;
	int LeftHitFlag = 0;
	int UpHitFlag = 0;
	int DownHitFlag = 0;
	float JumpG = 0.0f;
	int Scene = 0;
	enum Scene {
		title,
		gamePlay,
		gameOver,
		gameClear
	};
	while (true) {
		//ウィンドウメッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}
		for (int i = 0; i < _countof(object); i++) {
			ObjectPosition[i] = object[i]->GetPosition();
		}
		PlayerPosition = player->GetPosition();
		SpritePosition = sprite[0]->GetPosition();
		//毎フレーム処理

		//キーの更新
		input->Update();

		if (Scene == title) {
			if (input->TriggerKey(DIK_SPACE)) {
				Scene = gamePlay;
			}

		}
		
		if (Scene == gamePlay) {
			if (input->PushKey(DIK_UP)) {
				PlayerPosition.y += 0.5f;
			}

			if (input->PushKey(DIK_DOWN) && DownHitFlag == 0) {
				PlayerPosition.y -= 0.5f;
			}

			if (input->PushKey(DIK_LEFT) && LeftHitFlag == 0) {
				PlayerPosition.x -= 0.5f;
			}
			if (input->PushKey(DIK_RIGHT) && RightHitFlag == 0) {
				PlayerPosition.x += 0.5f;
			}

			//ジャンプ処理
			if (input->TriggerKey(DIK_SPACE)) {
				JumpG = -0.8f;
			}


			PlayerPosition.y -= JumpG;
			JumpG += 0.025f;

			if (PlayerPosition.y <= -10.0f) {
				JumpG = 0.0f;
				PlayerPosition.y = -10.0f;
			}

			//障害物との当たり判定
			for (int i = 0; i < OBJECT_NUM; i++) {
				//上
				if ((ObjectPosition[i].y - PlayerPosition.y <= 5.2) && (ObjectPosition[i].y - PlayerPosition.y >= 1.0)
					&& (ObjectPosition[i].x - PlayerPosition.x <= 4.8) && (ObjectPosition[i].x - PlayerPosition.x >= -4.8)) {
					JumpG = JumpG * -1;
				}

				//下
				if ((ObjectPosition[i].y - PlayerPosition.y >= -5.2) && (ObjectPosition[i].y - PlayerPosition.y <= -1.0)
					&& (ObjectPosition[i].x - PlayerPosition.x <= 4.8) && (ObjectPosition[i].x - PlayerPosition.x >= -4.8)) {
					JumpG = 0.0f;
					DownHitFlag = 1;
					if (PlayerPosition.y <= ObjectPosition[i].y / 2) {
						PlayerPosition.y = ObjectPosition[i].y / 2;
					}
				} else {
					DownHitFlag = 0;
				}

				//右
				if ((ObjectPosition[i].x - PlayerPosition.x <= 5) && (ObjectPosition[i].x - PlayerPosition.x >= 4)
					&& (ObjectPosition[i].y - PlayerPosition.y <= 5) && (ObjectPosition[i].y - PlayerPosition.y >= -5)) {
					RightHitFlag = 1;
					break;
				} else {
					RightHitFlag = 0;
				}

				//左
				if ((ObjectPosition[i].x - PlayerPosition.x >= -5) && (ObjectPosition[i].x - PlayerPosition.x <= -4) &&
					(ObjectPosition[i].y - PlayerPosition.y <= 5) && (ObjectPosition[i].y - PlayerPosition.y >= -5)) {
					LeftHitFlag = 1;
					break;
				} else {
					LeftHitFlag = 0;
				}
			}

			if (input->TriggerKey(DIK_R)) {
				Scene = gameClear;
			}
		}

		if (Scene == gameClear) {
			if (input->TriggerKey(DIK_S)) {
				Scene = title;
			}
		}
		for (int i = 0; i < _countof(object); i++) {
			object[i]->Update();
			//ルートシグネチャの設定コマンド
			object[i]->SetPosition(ObjectPosition[i]);
		}

		player->Update();
		player->SetPosition(PlayerPosition);

		//びょうがこまんど
		//x,y座標のデバッグログ
		wchar_t str[256];

		swprintf_s(str, L"ObjectPosition[1].x - PlayerPosition.x:%f\n", ObjectPosition[1].x - PlayerPosition.x);
		OutputDebugString(str);
		swprintf_s(str, L"ObjectPosition[0].x - PlayerPosition.x:%f\n", ObjectPosition[0].x - PlayerPosition.x);
		OutputDebugString(str);

		/*swprintf_s(str, L"ObjectPosition[1].y - ObjectPosition[0].y:%f\n", ObjectPosition[1].y - ObjectPosition[0].y);
		OutputDebugString(str);*/
		//描画コマンド
		dxCommon->PreDraw();
		////4.描画コマンドここから
		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//4.描画コマンドここまで
#pragma endregion
#pragma region//描画
		//背景スプライト描画前処理
		Sprite::PreDraw(dxCommon->GetCmdList());
		if (Scene == title) {
			sprite[0]->Draw();
		}
		Sprite::PostDraw();
		Object::PreDraw(dxCommon->GetCmdList());
		//背景
		if (Scene == gamePlay) {
			player->Draw();
			for (int i = 0; i < _countof(object); i++) {
				object[i]->Draw();
			}
		}
		Sprite::PreDraw(dxCommon->GetCmdList());
		if (Scene == gameClear) {
			sprite[1]->Draw();
		}
	
		Sprite::PostDraw();
		// 3Dオブジェクト描画後処理
		Object::PostDraw();
	
#pragma endregion
#pragma region//ゲーム外処理２
		dxCommon->PostDraw();
	}
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
	for (int i = 0; i < _countof(object); i++) {
		delete object[i];
	}
	winApp = nullptr;
	return 0;
}