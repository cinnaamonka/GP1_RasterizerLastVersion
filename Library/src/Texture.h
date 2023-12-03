#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include "Vector3.h"
namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;
		Vector3 SampleNormalMap(const Vector2& uv) const;
		class ReadEmptytexture : public std::exception
		{
		public:
			 virtual const char* what() const throw()
			{
				return "Texture not loaded";
			}
		};
	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}