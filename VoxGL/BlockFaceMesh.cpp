#include "BlockFaceMesh.hpp"

const static MeshPoint::WorldPos frontBottomLeft  { 0, 0, 0 };
const static MeshPoint::WorldPos frontBottomRight { 1, 0, 0 };
const static MeshPoint::WorldPos frontTopLeft     { 0, 0, 1 };
const static MeshPoint::WorldPos frontTopRight    { 1, 0, 1 };
const static MeshPoint::WorldPos backBottomLeft   { 0, 1, 0 };
const static MeshPoint::WorldPos backBottomRight  { 1, 1, 0 };
const static MeshPoint::WorldPos backTopLeft      { 0, 1, 1 };
const static MeshPoint::WorldPos backTopRight     { 1, 1, 1 };

const static std::vector <MeshPoint::TextPos> texturePoints { { 1.0f/textureLength, 1.0f / textureLength }, { 1.0f / textureLength,  0 }, { 0,  0 }, { 0, 1.0f / textureLength } };
const static std::vector <unsigned> indices { 1, 2, 3, 3, 0, 1 };

MeshData basicBlockFaceMesh(glm::vec3 at, int textID, BlockSide side) {
	MeshData ret{ {}, indices };
	int idX = textID % textureLength;
	int idY = textID / textureLength;

	MeshPoint::TextPos textOffset { (float)idX / textureLength, (float)idY / textureLength };

	ret.vertices = {
		{ {}, texturePoints[0] + textOffset },
		{ {}, texturePoints[1] + textOffset },
		{ {}, texturePoints[2] + textOffset },
		{ {}, texturePoints[3] + textOffset }
	};

	switch (side) {

	case BlockSide::Front:
		ret.vertices[0].loc = frontBottomRight + at;
		ret.vertices[1].loc = frontTopRight + at;
		ret.vertices[2].loc = frontTopLeft + at;
		ret.vertices[3].loc = frontBottomLeft + at;
		break;

	case BlockSide::Back:
		ret.vertices[0].loc = backBottomLeft + at;
		ret.vertices[1].loc = backTopLeft + at;
		ret.vertices[2].loc = backTopRight + at;
		ret.vertices[3].loc = backBottomRight + at;
		break;

	case BlockSide::Top:
		ret.vertices[0].loc = frontTopRight + at;
		ret.vertices[1].loc = backTopRight + at;
		ret.vertices[2].loc = backTopLeft + at;
		ret.vertices[3].loc = frontTopLeft + at;
		break;

	case BlockSide::Bottom:
		ret.vertices[0].loc = backBottomRight + at;
		ret.vertices[1].loc = frontBottomRight + at;
		ret.vertices[2].loc = frontBottomLeft + at;
		ret.vertices[3].loc = backBottomLeft + at;
		break;

	case BlockSide::Left:
		ret.vertices[0].loc = frontBottomLeft + at;
		ret.vertices[1].loc = frontTopLeft + at;
		ret.vertices[2].loc = backTopLeft + at;
		ret.vertices[3].loc = backBottomLeft + at;
		break;

	case BlockSide::Right:
		ret.vertices[0].loc = backBottomRight + at;
		ret.vertices[1].loc = backTopRight + at;
		ret.vertices[2].loc = frontTopRight + at;
		ret.vertices[3].loc = frontBottomRight + at;
		break;
	}

	return ret;
}
