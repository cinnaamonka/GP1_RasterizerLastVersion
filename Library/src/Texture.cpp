#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		
		 SDL_Surface* pSurface = IMG_Load(path.c_str());
		
		 Texture* textureObject = new Texture(pSurface);
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		return textureObject;
	}
	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		// Ensure that the texture has a valid surface
		if (!m_pSurface)
		{
			// Handle error or return a default color
			return ColorRGB(0.0f, 0.0f, 0.0f);
		}

		// Convert UV coordinates to pixel coordinates
		int x = static_cast<int>(uv.x * m_pSurface->w);
		int y = static_cast<int>(uv.y * m_pSurface->h);

		// Clamp pixel coordinates to valid range
		x = std::max(0, std::min(x, m_pSurface->w - 1));
		y = std::max(0, std::min(y, m_pSurface->h - 1));

		// Calculate the index of the pixel in the single array
		const uint32_t pixel = m_pSurfacePixels[y * m_pSurface->w + x];
		// Read the color of the pixel
		Uint8 r, g, b;
		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		// Remap color values to [0, 1] range
		float normalizedR = static_cast<float>(r) / 255.0f;
		float normalizedG = static_cast<float>(g) / 255.0f;
		float normalizedB = static_cast<float>(b) / 255.0f;

		return ColorRGB(normalizedR, normalizedG, normalizedB);
	}

}