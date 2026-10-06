#include "PickupComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace std;
using namespace Assimp;

PickupComponent::PickupComponent(Game* game, string name, float boundingRadius, Vector3 position) :
	GameComponent(game),
	boundingRadius(boundingRadius),
	position(position)
{
	modelPath = "./Models/" + name + ".fbx";
	texturePath = L"./Textures/";
	texturePath.append(name.begin(), name.end());
	texturePath += L".dds";

	game->components.push_back(this);
}

PickupComponent::~PickupComponent()
{}

void PickupComponent::initialize()
{
	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(L"./Shaders/MyVerySecondShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderByteCode,
		&errorVertexCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(game->display->hWnd, L"MyVerySecondShader.hlsl", L"Missing Shader File", MB_OK);
		}
		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVerySecondShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/, "PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShaderByteCode,
		&errorPixelCode);

	game->device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	game->device->CreatePixelShader(
		pixelShaderByteCode->GetBufferPointer(),
		pixelShaderByteCode->GetBufferSize(),
		nullptr, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
	D3D11_INPUT_ELEMENT_DESC {
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		0,
		D3D11_INPUT_PER_VERTEX_DATA,
		0},
	D3D11_INPUT_ELEMENT_DESC {
		"TEXCOORD",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA,
		0},
	D3D11_INPUT_ELEMENT_DESC {
		"NORMAL",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA,
		0}
	};

	game->device->CreateInputLayout(
		inputElements,
		3,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	loadModel(modelPath);
	loadTexture(texturePath);

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Point) * std::size(points_);


	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points_.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	game->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces_);

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces_.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	game->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	strides[0] = 48;
	offsets[0] = 0;

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;
	constBufDesc.ByteWidth = sizeof(CBPerObject);

	game->device->CreateBuffer(&constBufDesc, nullptr, &cb);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = true;
	rastDesc.DepthClipEnable = true;

	res = game->device->CreateRasterizerState(&rastDesc, &rastState);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	game->device->CreateSamplerState(&samplerDesc, &samplerState);
}

void PickupComponent::draw()
{ 
	game->context->RSSetState(rastState);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(game->display->clientWidth);
	viewport.Height = static_cast<float>(game->display->clientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	game->context->RSSetViewports(1, &viewport);

	game->context->IASetInputLayout(layout);
	game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game->context->VSSetShader(vertexShader, nullptr, 0);
	game->context->VSSetConstantBuffers(0, 1, &cb);
	game->context->PSSetShader(pixelShader, nullptr, 0);
	game->context->PSSetShaderResources(0, 1, &textureSRV);
	game->context->PSSetSamplers(0, 1, &samplerState);

	game->context->DrawIndexed(indeces_.size(), 0, 0);
}

void PickupComponent::destroyResources()
{}

void PickupComponent::update()
{

	const Matrix world =
		Matrix::CreateScale(boundingRadius) *
		Matrix::CreateFromQuaternion(rotation) *
		Matrix::CreateTranslation(position);

	Matrix worldViewProj = world * game->camera->getMatrix();
	CBPerObject cbData;

	cbData.worldViewProj = worldViewProj.Transpose();
	cbData.world = world.Transpose();
	game->context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);
}

void PickupComponent::reload()
{
}

void PickupComponent::loadModel(const string& path)
{
	Importer importer;

	unsigned int flags =
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_OptimizeMeshes;

	const aiScene* scene = importer.ReadFile(path, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
		std::cerr << "Failed to load model: " << path << std::endl;
		return;
	}

	points_.clear();
	indeces_.clear();

	UINT vertexOffset = 0;

	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];

		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
			Point point;
			point.position = Vector4(
				mesh->mVertices[v].x,
				mesh->mVertices[v].y,
				mesh->mVertices[v].z,
				1.0f
			);
			point.uv = Vector4(
				mesh->mTextureCoords[0][v].x,
				mesh->mTextureCoords[0][v].y,
				0.0f,
				0.0f
			);
			point.normal = Vector4(
				mesh->mNormals[v].x,
				mesh->mNormals[v].y,
				mesh->mNormals[v].z,
				0.0f
			);
			points_.push_back(point);
		}

		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			const aiFace& face = mesh->mFaces[f];
			for (unsigned int i = 0; i < face.mNumIndices; ++i)
			{
				indeces_.push_back(face.mIndices[i] + vertexOffset);
			}
		}

		vertexOffset += mesh->mNumVertices;
	}

	Vector3 center = Vector3::Zero;
	for (const auto& v : points_)
	{
		center += Vector3(v.position.x, v.position.y, v.position.z);
	}
	center /= static_cast<float>(points_.size());

	float maxRadius = 0.0f;
	for (const auto& v : points_)
	{
		Vector3 offset = Vector3(v.position.x, v.position.y, v.position.z) - center;
		float dist = offset.Length();
		if (dist > maxRadius)
			maxRadius = dist;
	}

	if (maxRadius > 0.0001f)
	{
		for (auto& v : points_)
		{
			v.position.x -= center.x;
			v.position.y -= center.y;
			v.position.z -= center.z;
			v.position.x /= maxRadius;
			v.position.y /= maxRadius;
			v.position.z /= maxRadius;
		}
	}

	if (rotation != Quaternion::Identity)
	{
		Matrix rotMatrix = Matrix::CreateFromQuaternion(rotation);
		for (auto& v : points_)
		{
			Vector3 pos(v.position.x, v.position.y, v.position.z);
			pos = Vector3::Transform(pos, rotMatrix);
			v.position.x = pos.x;
			v.position.y = pos.y;
			v.position.z = pos.z;

			Vector3 norm(v.normal.x, v.normal.y, v.normal.z);
			norm = Vector3::TransformNormal(norm, rotMatrix);
			v.normal.x = norm.x;
			v.normal.y = norm.y;
			v.normal.z = norm.z;
		}
		rotation = Quaternion::Identity;
	}

	float minY = FLT_MAX;
	for (const auto& v : points_)
	{
		if (v.position.y < minY)
			minY = v.position.y;
	}

	if (minY < 0.0f)
	{
		for (auto& v : points_)
		{
			v.position.y -= minY;
		}
	}
}

void PickupComponent::loadTexture(const wstring& path)
{
	HRESULT hr = CreateDDSTextureFromFile(
		game->device.Get(),
		path.c_str(),
		nullptr,
		&textureSRV
	);

	if (FAILED(hr))
	{
		std::cerr << "Failed to load texture: ";
		std::wcerr << path << L" HRESULT: 0x" << std::hex << hr << std::endl;
	}

	std::wcout << L"Texture loaded: " << path << std::endl;
}