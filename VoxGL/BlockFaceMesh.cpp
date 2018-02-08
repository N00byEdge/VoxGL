#include "BlockFaceMesh.hpp"

static MeshPoint::WorldPos const FrontBottomLeft{0, 0, 0};
static MeshPoint::WorldPos const FrontBottomRight{1, 0, 0};
static MeshPoint::WorldPos const FrontTopLeft{0, 0, 1};
static MeshPoint::WorldPos const FrontTopRight{1, 0, 1};
static MeshPoint::WorldPos const BackBottomLeft{0, 1, 0};
static MeshPoint::WorldPos const BackBottomRight{1, 1, 0};
static MeshPoint::WorldPos const BackTopLeft{0, 1, 1};
static MeshPoint::WorldPos const BackTopRight{1, 1, 1};

static std::vector<MeshPoint::TextPos> const TexturePoints{
  {1.0f / TextureLength, 1.0f / TextureLength}, {1.0f / TextureLength, 0}, {0, 0}, {0, 1.0f / TextureLength}
};
static std::vector<unsigned> const Indices{1, 2, 3, 3, 0, 1};

MeshData BasicBlockFaceMesh(glm::vec3 const at, int const textId, BlockSide const side) {
  MeshData ret{{}, Indices};
  auto const idX = textId % TextureLength;
  auto const idY = textId / TextureLength;

  MeshPoint::TextPos const textOffset{static_cast<float>(idX) / TextureLength, static_cast<float>(idY) / TextureLength};

  ret.vertices = {
    {{}, TexturePoints[0] + textOffset},
    {{}, TexturePoints[1] + textOffset},
    {{}, TexturePoints[2] + textOffset},
    {{}, TexturePoints[3] + textOffset}
  };

  switch(side) {
  case BlockSide::Front:
    ret.vertices[0].loc = FrontBottomRight + at;
    ret.vertices[1].loc = FrontTopRight + at;
    ret.vertices[2].loc = FrontTopLeft + at;
    ret.vertices[3].loc = FrontBottomLeft + at;
    break;

  case BlockSide::Back:
    ret.vertices[0].loc = BackBottomLeft + at;
    ret.vertices[1].loc = BackTopLeft + at;
    ret.vertices[2].loc = BackTopRight + at;
    ret.vertices[3].loc = BackBottomRight + at;
    break;

  case BlockSide::Top:
    ret.vertices[0].loc = FrontTopRight + at;
    ret.vertices[1].loc = BackTopRight + at;
    ret.vertices[2].loc = BackTopLeft + at;
    ret.vertices[3].loc = FrontTopLeft + at;
    break;

  case BlockSide::Bottom:
    ret.vertices[0].loc = BackBottomRight + at;
    ret.vertices[1].loc = FrontBottomRight + at;
    ret.vertices[2].loc = FrontBottomLeft + at;
    ret.vertices[3].loc = BackBottomLeft + at;
    break;

  case BlockSide::Left:
    ret.vertices[0].loc = FrontBottomLeft + at;
    ret.vertices[1].loc = FrontTopLeft + at;
    ret.vertices[2].loc = BackTopLeft + at;
    ret.vertices[3].loc = BackBottomLeft + at;
    break;

  case BlockSide::Right:
    ret.vertices[0].loc = BackBottomRight + at;
    ret.vertices[1].loc = BackTopRight + at;
    ret.vertices[2].loc = FrontTopRight + at;
    ret.vertices[3].loc = FrontBottomRight + at;
    break;
  }

  return ret;
}
