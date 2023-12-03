#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "Camera.h"
#include "DataTypes.h"
#include "Texture.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Mesh4AxisVertex;
	struct Vertex;
	struct Vertex_Out;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;
		void VertexTransformationFunction(const std::vector<Mesh>& meshes_in, std::vector<Mesh4AxisVertex>& meshes_out,const Camera camera);
		Vector2 ConvertNDCtoScreen(const Vector3& ndc, int screenWidth, int screenHeight)const;
		void ToggleZBuffer() { m_FinalColorEnabled = !m_FinalColorEnabled; };
		void ToggleNormalMap() { m_NormalMapEnabled = !m_NormalMapEnabled; };
		ColorRGB PixelShading(Vertex_Out& v, const Vector2& uvInterpolated);
		void CycleLightingMode();
		void RotateModel();
	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		//float* m_pDepthBufferPixels{};
		std::vector<std::vector<Vector2>> m_pTriangles;
		Camera m_Camera{};
		float m_AspectRatio;
		bool m_FinalColorEnabled = true;
		bool m_CanBeRotated = false;
		bool m_NormalMapEnabled = false;

		std::vector<Mesh> m_Meshes;
		std::vector<Mesh4AxisVertex> meshes_screen;

		Mesh m_Vehicle{};

		std::unique_ptr<dae::Texture> m_TextureVehicle = std::unique_ptr<dae::Texture>(dae::Texture::LoadFromFile("Resources/vehicle_diffuse.png")); 
		std::unique_ptr<dae::Texture> m_NormalMapVehicle = std::unique_ptr<dae::Texture>(dae::Texture::LoadFromFile("Resources/vehicle_normal.png"));
		std::unique_ptr<dae::Texture> m_SpecularColor = std::unique_ptr<dae::Texture>(dae::Texture::LoadFromFile("Resources/vehicle_specular.png"));
		std::unique_ptr<dae::Texture> m_GlosinessMap = std::unique_ptr<dae::Texture>(dae::Texture::LoadFromFile("Resources/vehicle_gloss.png")); 

		int m_Width{};
		int m_Height{};

		std::vector<float> m_pDepthBuffer;
	
		enum class LightingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined
		};

		LightingMode m_CurrentLightingMode = { LightingMode::ObservedArea };
		
		Matrix m_Translation;
	
	};
}
