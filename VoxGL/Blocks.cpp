#include "Blocks.hpp"

std::shared_ptr<BlockTexture> getBlockTexture(BlockID id) {
	switch (id) {
	case blockID<Blocks::Grass>() : {
		const static std::shared_ptr<BlockTexture> grass = std::make_shared<BlockTexture>(2, 3, 1, 1, 1, 1);
		return grass;
	}
	case blockID<Blocks::Dirt>() : {
		const static std::shared_ptr<BlockTexture> dirt = std::make_shared<BlockTexture>(3, 3, 3, 3, 3, 3);
		return dirt;
	}
	case blockID<Blocks::Stone>() : {
		const static std::shared_ptr<BlockTexture> stone = std::make_shared<BlockTexture>(4, 4, 4, 4, 4, 4);
		return stone;
	}
	default: {
		const static std::shared_ptr<BlockTexture> no_texture = std::make_shared<BlockTexture>(0, 0, 0, 0, 0, 0);
		return no_texture;
	}
	}	
}
