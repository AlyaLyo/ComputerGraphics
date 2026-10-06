#include "BodyComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

BodyComponent::BodyComponent(Game* game, float radius, Vector4 color) :
	GameComponent(game),
	radius(radius),
	color(color),
	color2(color),
	materialAmbient(0.15f, 0.15f, 0.15f),
	materialSpecular(0.5f, 0.5f, 0.5f),
	materialShininess(32.0f)
{
	generateSphereGeometry();
	game->components.push_back(this);
}

BodyComponent::~BodyComponent()
{}

void BodyComponent::initialize()
{
	D3D_SHADER_MACRO PhongMacro[] = { "PHONG_LIGHTING", "1", nullptr, nullptr };

	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		PhongMacro /*macros*/,
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
			MessageBox(game->display->hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}
		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		PhongMacro /*macros*/,
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
		"COLOR",
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

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	//vertexBufDesc.ByteWidth = static_cast <UINT>(sizeof(Vector4) * std::size(points_) * 2);
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
	//indexBufDesc.ByteWidth = static_cast <UINT>(sizeof(int) * std::size(indeces_));
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

	D3D11_BUFFER_DESC lightBufDesc = {};
	lightBufDesc.Usage = D3D11_USAGE_DEFAULT;
	lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufDesc.CPUAccessFlags = 0;
	lightBufDesc.MiscFlags = 0;
	lightBufDesc.StructureByteStride = 0;
	lightBufDesc.ByteWidth = sizeof(CBLight);

	game->device->CreateBuffer(&lightBufDesc, nullptr, &cbLight);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	//rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	//rastDesc.FrontCounterClockwise = true;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;

	res = game->device->CreateRasterizerState(&rastDesc, &rastState);
}

void BodyComponent::draw()
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
	game->context->PSSetConstantBuffers(1, 1, &cbLight);

	game->context->DrawIndexed(indeces_.size(), 0, 0);
}

void BodyComponent::destroyResources()
{}

void BodyComponent::update()
{
	const Matrix world = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	Matrix worldViewProj = world * game->camera->getMatrix();
	
	CBPerObject cbData;
	cbData.worldViewProjection = worldViewProj.Transpose();
	cbData.world = world.Transpose();
	game->context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);

	CBLight lightData;

	Vector3 lightDir = Vector3(1.0f, 1.0f, 1.0f);
	lightDir.Normalize();
	lightData.lightDirection = Vector4(lightDir.x, lightDir.y, lightDir.z, 0.0f);

	lightData.lightColor = Vector4(1.0f, 0.95f, 0.9f, 0.0f);

	Vector3 camPos = game->camera->GetPosition();
	lightData.cameraPosition = Vector4(camPos.x, camPos.y, camPos.z, 0.0f);

	lightData.materialParams = Vector4(
		materialAmbient.x,
		materialAmbient.y,
		materialAmbient.z,
		materialShininess);

	lightData.materialSpecular = Vector4(
		materialSpecular.x,
		materialSpecular.y,
		materialSpecular.z,
		0.0f);

	//worldViewProj = worldViewProj.Transpose();
	//game->context->UpdateSubresource(cb, 0, nullptr, &worldViewProj, 0, 0);
	game->context->UpdateSubresource(cbLight, 0, nullptr, &lightData, 0, 0);
}

void BodyComponent::reload()
{}

void BodyComponent::generateSphereGeometry()
{
	//Point topPoint({ Vector4(0.0f, radius, 0.0f, 1.0f), color });
	Point topPoint;
	topPoint.position = Vector4(0.0f, radius, 0.0f, 1.0f);
	topPoint.color = color;
	topPoint.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);


	//Point bottomPoint({ Vector4(0.0f, -radius, 0.0f, 1.0f), color });
	Point bottomPoint;
	bottomPoint.position = Vector4(0.0f, -radius, 0.0f, 1.0f);
	bottomPoint.color = color;
	bottomPoint.normal = Vector4(0.0f, -1.0f, 0.0f, 0.0f);

	points_.push_back(topPoint);

	int sliceCount = 32;
	int stackCount = 32;
	const float phiStep = XM_PI / static_cast<float>(stackCount);
	const float thetaStep = XM_2PI / static_cast<float>(sliceCount);

	for (int i = 1; i <= stackCount - 1; ++i)
	{
		const float phi = static_cast<float>(i) * phiStep;

		for (int j = 0; j <= sliceCount; ++j)
		{
			const float theta = static_cast<float>(j) * thetaStep;
			Point point;
			/*
			point.position.x = radius * sinf(phi) * cosf(theta);
			point.position.y = radius * cosf(phi);
			point.position.z = radius * sinf(phi) * sinf(theta);
			point.position.w = 1.0f;
			point.color = (cosf(theta) >= 0.0f) ? color : color2;
			*/

			float nx = sinf(phi) * cosf(theta);
			float ny = cosf(phi);
			float nz = sinf(phi) * sinf(theta);

			point.position = Vector4(radius * nx, radius * ny, radius * nz, 1.0f);
			point.color = (cosf(theta) >= 0.0f) ? color : color2;
			point.normal = Vector4(nx, ny, nz, 0.0f);

			points_.push_back(point);
		}
	}

	points_.push_back(bottomPoint);

	for (int i = 1; i <= sliceCount; ++i)
	{
		indeces_.push_back(0);
		indeces_.push_back(i + 1);
		indeces_.push_back(i);
	}

	int baseIndex = 1;
	const int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			indeces_.push_back(baseIndex + i * ringVertexCount + j);
			indeces_.push_back(baseIndex + i * ringVertexCount + j + 1);
			indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indeces_.push_back(baseIndex + i * ringVertexCount + j + 1);
			indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	const int southPoleIndex = points_.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (int i = 0; i < sliceCount; ++i)
	{
		indeces_.push_back(southPoleIndex);
		indeces_.push_back(baseIndex + i);
		indeces_.push_back(baseIndex + i + 1);
	}
}