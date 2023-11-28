#pragma once
#include <cassert>
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

//#define DISABLE_OBJ

namespace dae
{
	namespace Utils
	{

		bool IsPixelInterpolated(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Vertex_Out& pixelVector, Vector2& uvInterpolated, float& pixelDepth)
		{
			const Vector2 edge = v1.position.GetXY() - v0.position.GetXY();// V1 - V0
			const Vector2 edge1 = v2.position.GetXY() - v1.position.GetXY();// V2 - V1
			const Vector2 edge2 = v0.position.GetXY() - v2.position.GetXY();

			const Vector2 pointToVertex = pixelVector.position.GetXY() - v0.position.GetXY();// P - V0
			const Vector2 pointToVertex1 = pixelVector.position.GetXY() - v1.position.GetXY();// P - V1
			const Vector2 pointToVertex2 = pixelVector.position.GetXY() - v2.position.GetXY();// P - V2

			// Calculate 2D cross products (signed areas)

			float cross1 = Vector2::Cross(pointToVertex2, edge2);	if (cross1 > 0) return false;

			float cross0 = Vector2::Cross(pointToVertex1, edge1);	if (cross0 > 0) return false;

			float cross2 = Vector2::Cross(pointToVertex, edge);	if (cross2 > 0) return false;

			// Check the signs of the cross products

			const float totalParallelogramArea = cross0 + cross1 + cross2;

			const float W0 = cross0 / totalParallelogramArea;
			const float W1 = cross1 / totalParallelogramArea;
			const float W2 = cross2 / totalParallelogramArea;

			//interpolate through the depth values
			pixelDepth = 1 /
				(W0 / v0.position.z +
					W1 / v1.position.z +
					W2 / v2.position.z);

			if (pixelDepth < 0 || pixelDepth > 1) return false;// culling

			//interpolate through the depth values

			float interpolatedDepth = 1 /
				(W0 / v0.position.w +
					W1 / v1.position.w +
					W2 / v2.position.w);

			uvInterpolated = (v0.uv * W0 / v0.position.w +
				v1.uv * W1 / v1.position.w +
				v2.uv * W2 / v2.position.w) * interpolatedDepth;

			const Vector3 interpolatedNormal = (v0.normal * W0 / v0.position.w +
				v1.normal * W1 / v1.position.w +
				v2.normal * W2 / v2.position.w) * interpolatedDepth;


			pixelVector.color = (v0.color * W0 / v0.position.w +
				v1.color * W1 / v1.position.w +
				v2.color * W2 / v2.position.w) * interpolatedDepth;

			pixelVector.tangent = (v0.tangent * W0 / v0.position.w +
				v1.tangent * W1 / v1.position.w +
				v2.tangent * W2 / v2.position.w) * interpolatedDepth;

			pixelVector.viewDirection = (v0.viewDirection * W0 / v0.position.w +
				v1.viewDirection * W1 / v1.position.w +
				v2.viewDirection * W2 / v2.position.w) * interpolatedDepth;

			return true;
		}


		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
#ifdef DISABLE_OBJ

			//TODO: Enable the code below after uncommenting all the vertex attributes of DataTypes::Vertex
			// >> Comment/Remove '#define DISABLE_OBJ'
			assert(false && "OBJ PARSER not enabled! Check the comments in Utils::ParseOBJ");

#else

			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if (flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
#endif
		}
#pragma warning(pop)
	}
}