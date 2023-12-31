//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>


using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height); 

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize Camera

	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	m_Camera.Initialize(m_AspectRatio, 45.f, { .0f,5.0f,64.f });
	m_pDepthBuffer.resize(m_Height * m_Width);
	m_FinalColorEnabled = true;


	Utils::ParseOBJ("Resources/vehicle.obj", m_Vehicle.vertices, m_Vehicle.indices);
	m_Vehicle.primitiveTopology = PrimitiveTopology::TriangleList;
	const float Ztranslation = 110.0f;
	const float Ytranslation = 6.f;

	m_Translation = Matrix::CreateTranslation(Vector3(0, Ytranslation, Ztranslation));

}

Renderer::~Renderer()
{

}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);

	if (m_CanBeRotated)
	{
		m_Vehicle.worldMatrix = Matrix::CreateRotationY(PI_DIV_2 * pTimer->GetTotal()) * m_Translation;
	}
	else
	{
		m_Vehicle.worldMatrix = m_Translation;
	}

}

void Renderer::Render()
{
	SDL_LockSurface(m_pBackBuffer);

	meshes_screen.clear();
	m_Meshes.clear();
	m_Meshes.push_back(m_Vehicle);
	VertexTransformationFunction(m_Meshes, meshes_screen, m_Camera);

	const unsigned int backgroundColor = 100;

	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(backgroundColor * 255),
		static_cast<uint8_t>(backgroundColor * 255),
		static_cast<uint8_t>(backgroundColor * 255))
	);

	// depth buffer is initialized with the maximum value of float
	std::fill(m_pDepthBuffer.begin(), m_pDepthBuffer.end(), std::numeric_limits<float>::max());

	const SDL_Rect clearRect = { 0, 0, m_Width, m_Height };
	//clearing the back buffer
	ColorRGB finalColor = { 0,0,0 };
	Triangle4 currentTriangle;

	for (int i = 0; i < m_Meshes[0].indices.size() - 2; i += 3)
	{
		if (m_Meshes[0].primitiveTopology == PrimitiveTopology::TriangleList)
		{
			currentTriangle =
			{
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 1]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 2]]
			};

		}
		else if (i % 2 == 0)
		{
			currentTriangle =
			{
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 1]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 2]]
			};
		}
		else
		{
			currentTriangle =
			{
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 2]],
				meshes_screen[0].vertices_out[m_Meshes[0].indices[i + 1]]
			};
		}

		//optimization

		int minX = static_cast<int>(std::min(currentTriangle.vertex0.position.x, std::min(currentTriangle.vertex1.position.x, currentTriangle.vertex2.position.x)) - 1);
		int maxX = static_cast<int>(std::max(currentTriangle.vertex0.position.x, std::max(currentTriangle.vertex1.position.x, currentTriangle.vertex2.position.x)) + 1);
		int minY = static_cast<int>(std::min(currentTriangle.vertex0.position.y, std::min(currentTriangle.vertex1.position.y, currentTriangle.vertex2.position.y)) - 1);
		int maxY = static_cast<int>(std::max(currentTriangle.vertex0.position.y, std::max(currentTriangle.vertex1.position.y, currentTriangle.vertex2.position.y)) + 1);

		// if statement of std::clamp from C++ 20
		minX = std::ranges::clamp(minX, 0, m_Width);
		maxX = std::ranges::clamp(maxX, 0, m_Width);
		minY = std::ranges::clamp(minY, 0, m_Height);
		maxY = std::ranges::clamp(maxY, 0, m_Height);

		for (int px = minX; px < maxX; ++px)
		{
			for (int py = minY; py < maxY; ++py)
			{
				Vertex_Out P = { {static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f,0,1} };

				Vector2 uvInterp = { 0,0 };
				float pixelDepth = 0;

				if (!Utils::IsPixelInterpolated(currentTriangle.vertex0, currentTriangle.vertex1, currentTriangle.vertex2, P, uvInterp, pixelDepth))
				{
					continue;
				}

				const int pixelIndex = { px + py * m_Width };

				if (pixelDepth > m_pDepthBuffer[pixelIndex]) continue;

				m_pDepthBuffer[pixelIndex] = pixelDepth;

				if (m_FinalColorEnabled)
				{
					finalColor = PixelShading(P, uvInterp);
				}
				if (!m_FinalColorEnabled)
				{
					const float a = Remap(pixelDepth, 0.985f,1.f, 0.f, .8f);

					finalColor = ColorRGB{ a, a, a };
				}
				//finalColor.MaxToOne();
				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Mesh>& meshes_in, std::vector<Mesh4AxisVertex>& meshes_out, const Camera camera)
{
	for (int i = 0; i < meshes_in.size(); i++)
	{
		std::vector<Vertex_Out> vertices_out;
		Mesh4AxisVertex newMesh;

		for (const auto& vertex : meshes_in[i].vertices)
		{
			Vertex_Out newVertex
			{
				Vector4{vertex.position.x,vertex.position.y,vertex.position.z,1},
				vertex.color,
				vertex.uv,
				vertex.normal,
				vertex.tangent,
				vertex.viewDirection
			};
			const Matrix worldViewProjectionMatrix{ meshes_in[i].worldMatrix * m_Camera.worldViewProectionMatrix };
			newVertex.position = worldViewProjectionMatrix.TransformPoint(newVertex.position);

			//perspective divide
			newVertex.position.x = newVertex.position.x / newVertex.position.w;
			newVertex.position.y = newVertex.position.y / newVertex.position.w;
			newVertex.position.z = newVertex.position.z / newVertex.position.w;

			// Convert from NDC to screen
			newVertex.position.x = ConvertNDCtoScreen(newVertex.position, m_Width, m_Height).x;
			newVertex.position.y = ConvertNDCtoScreen(newVertex.position, m_Width, m_Height).y;

			newVertex.uv = vertex.uv;

			newVertex.normal = m_Vehicle.worldMatrix.TransformVector(newVertex.normal);
			newVertex.tangent = m_Vehicle.worldMatrix.TransformVector(newVertex.tangent);

			newVertex.viewDirection = camera.origin - newVertex.position;
			newVertex.viewDirection.Normalize();

			vertices_out.push_back(newVertex);
		}
		newMesh =
		{
			vertices_out,
			meshes_in[i].indices,
			meshes_in[i].primitiveTopology

		};

		meshes_out.push_back(newMesh);
	}

}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
Vector2 Renderer::ConvertNDCtoScreen(const Vector3& ndc, int screenWidth, int screenHeight)const
{
	float screenSpaceX = (ndc.x + 1.0f) / 2.0f * screenWidth;
	float screenSpaceY = (1.0f - ndc.y) / 2.0f * screenHeight;
	return Vector2{ screenSpaceX, screenSpaceY };
}
ColorRGB Renderer::PixelShading(Vertex_Out& v, const Vector2& uvInterpolated)
{
	const Vector3 lightDirection = { 0.577f, -.577f, -.577f };
	const float lightIntensivity = 2.f;

	const int shiniessValue = 25;

	//normal mapping
	Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
	Matrix tangentSpaceAxis = Matrix{ v.tangent, binormal, v.normal, {0,0,0} };
	ColorRGB sampledNormal = m_NormalMapVehicle->Sample(uvInterpolated);

	sampledNormal = 2 * sampledNormal - ColorRGB{ 1,1,1 };

	const Vector3 resultNormal = tangentSpaceAxis.TransformVector(sampledNormal.r, sampledNormal.g, sampledNormal.b);

	if (m_NormalMapEnabled)
	{
		v.normal = resultNormal;
	}

	// lambert diffuse
	const auto& sampledColor = m_TextureVehicle->Sample(uvInterpolated);
	const ColorRGB diffuseColor = lightIntensivity * sampledColor;

	ColorRGB lambertFinalColor = diffuseColor / float(M_PI);

	float cosAngle = Vector3::Dot(v.normal, lightDirection);


	// phong 
	const Vector3 reflect = Vector3::Reflect(lightDirection, v.normal);
	const float cosAlpha = std::max(Vector3::Dot(reflect, v.viewDirection), 0.0f);

	const ColorRGB specularity = m_SpecularColor->Sample(uvInterpolated);
	float glosiness = m_GlosinessMap->Sample(uvInterpolated).r;

	const ColorRGB specularColor = specularity * powf(cosAlpha, glosiness * shiniessValue) * colors::White;
	const ColorRGB ambientOcclusion = { 0.05f, 0.05f,0.05f };

	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:

		if (cosAngle < 0) return { 0,0,0 };
		return ColorRGB{ cosAngle, cosAngle,cosAngle };

		break;

	case LightingMode::Diffuse:

		if (cosAngle < 0) return { 0,0,0 };

		return lambertFinalColor * ColorRGB{ cosAngle, cosAngle, cosAngle };

		break;

	case LightingMode::Specular:

		if (cosAngle < 0)return { 0,0,0 };
		return specularColor * ColorRGB{ cosAngle, cosAngle, cosAngle };

		break;

	case LightingMode::Combined:

		if (cosAngle < 0)return { 0,0,0 };

		return (lambertFinalColor + specularColor + ambientOcclusion) * ColorRGB(cosAngle, cosAngle, cosAngle);

		break;
	}
	return { 0,0,0 };
}
void Renderer::CycleLightingMode()
{
	int currentLightingMode = static_cast<int>(m_CurrentLightingMode);
	++currentLightingMode %= 4;
	m_CurrentLightingMode = LightingMode{ currentLightingMode };
}
void Renderer::RotateModel()
{
	m_CanBeRotated = !m_CanBeRotated;

}



