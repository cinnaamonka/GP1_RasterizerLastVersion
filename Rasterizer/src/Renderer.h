#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

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
		void VertexTransformationFunction(const std::vector<Mesh>& meshes_in, std::vector<Mesh4AxisVertex>& meshes_out) const;
		Vector2 ConvertNDCtoScreen(const Vector3& ndc, int screenWidth, int screenHeight)const;
		void ToggleShadows() { m_FinalColorEnabled = !m_FinalColorEnabled; };

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		//float* m_pDepthBufferPixels{};
		std::vector<std::vector<Vector2>> m_pTriangles;
		Camera m_Camera{};
		float m_AspectRatio;
		bool m_FinalColorEnabled;

		std::vector<Mesh> m_Meshes;
		std::vector<Mesh4AxisVertex> meshes_screen;

		Mesh m_Vehicle{};

		Texture* m_TextureVehicle;

		int m_Width{};
		int m_Height{};

		std::vector<float> m_pDepthBuffer;
	
	
	};
}
