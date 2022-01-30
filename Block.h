#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>

/// 3D�I�u�W�F�N�g
class Block
{
private: // �G�C���A�X
	// Microsoft::WRL::���ȗ�
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::���ȗ�
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: // �T�u�N���X
	// ���_�f�[�^�\����
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT3 normal; // �@���x�N�g��
		XMFLOAT2 uv;  // uv���W
	};

	// �萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferData
	{
		XMFLOAT4 color;	// �F (RGBA)
		XMMATRIX mat;	// �R�c�ϊ��s��
	};

private: // �萔
	static const int division = 50;					// ������
	static const float radius;				// ��ʂ̔��a
	static const float prizmHeight;			// ���̍���
	static const int planeCount = division * 2 + division * 2;		// �ʂ̐�
	static const int vertexCount = planeCount * 3;		// ���_��

public: // �ÓI�����o�֐�
	/// �ÓI������
	static bool StaticInitialize(ID3D12Device* device, int window_width, int window_height);

	/// �`��O����
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);

	/// �`��㏈��
	static void PostDraw();

	/// 3D�I�u�W�F�N�g����
	static Block* Create();

	/// ���_���W�̎擾
	static const XMFLOAT3& GetEye() { return eye; }

	/// ���_���W�̐ݒ�
	static void SetEye(XMFLOAT3 eye);

	/// �����_���W�̎擾
	static const XMFLOAT3& GetTarget() { return target; }

	/// �����_���W�̐ݒ�
	static void SetTarget(XMFLOAT3 target);

	/// �x�N�g���ɂ��ړ�
	static void CameraMoveVector(XMFLOAT3 move);

private: // �ÓI�����o�ϐ�
	static ID3D12Device* device;
	static UINT descriptorHandleIncrementSize;
	static ID3D12GraphicsCommandList* cmdList;
	static ComPtr<ID3D12RootSignature> rootsignature;
	static ComPtr<ID3D12PipelineState> pipelinestate;
	static ComPtr<ID3D12DescriptorHeap> descHeap;
	static ComPtr<ID3D12Resource> vertBuff;
	static ComPtr<ID3D12Resource> indexBuff;
	static ComPtr<ID3D12Resource> texbuff;
	static CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	static CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	static XMMATRIX matView;
	static XMMATRIX matProjection;
	static XMFLOAT3 eye;
	static XMFLOAT3 target;
	static XMFLOAT3 up;
	static D3D12_VERTEX_BUFFER_VIEW vbView;
	static D3D12_INDEX_BUFFER_VIEW ibView;
	static VertexPosNormalUv vertices[vertexCount];
	static unsigned short indices[planeCount * 3];

private:// �ÓI�����o�֐�
	/// �f�X�N���v�^�q�[�v�̏�����
	static bool InitializeDescriptorHeap();

	/// �J����������
	static void InitializeCamera(int window_width, int window_height);

	/// �O���t�B�b�N�p�C�v���C������
	static bool InitializeGraphicsPipeline();

	/// �e�N�X�`���ǂݍ���
	static bool LoadTexture();

	/// ���f���쐬
	static void CreateModel();

	/// �r���[�s����X�V
	static void UpdateViewMatrix();

public: // �����o�֐�//
	//������
	bool Initialize();
	/// ���t���[������
	void Update(XMMATRIX& matView);

	/// �`��
	void Draw();

	/// ���W�̎擾
	const XMFLOAT3& GetPosition() { return position; }
	/// ��]�̎擾
	const XMFLOAT3& GetRotation() { return  rotation; }
	/// �傫���̎擾
	const XMFLOAT3& GetScale() { return  scale; }
	/// �F�̎擾
	const XMFLOAT4& GetColor() { return  color; }

	/// ���W�̐ݒ�
	void SetPosition(XMFLOAT3 position) { this->position = position; }
	/// ��]�̐ݒ�
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }
	/// �傫���̐ݒ�
	void SetScale(XMFLOAT3 scale) { this->scale = scale; }
	/// �F�̐ݒ�
	void SetColor(XMFLOAT4 color) { this->color = color; }
	//�J���������̈ʒu�Ɉړ������鏈��
	static void SetCameraPosition(XMFLOAT3 position, XMFLOAT3 targetposition);
private: // �����o�ϐ�
	ComPtr<ID3D12Resource> constBuff; // �萔�o�b�t�@
	// �F
	XMFLOAT4 color = { 0,0,0,1.0 };
	// ���[�J���X�P�[��
	XMFLOAT3 scale = { 1.0,1.0,0.2 };
	// X,Y,Z�����̃��[�J����]�p
	XMFLOAT3 rotation = { 0,0,0 };
	// ���[�J�����W
	XMFLOAT3 position = { 0,0,120 };
	// ���[�J�����[���h�ϊ��s��
	XMMATRIX matWorld;
	// �e�I�u�W�F�N�g
	Block* parent = nullptr;
};
