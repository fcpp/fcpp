// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

namespace fcpp {

	//! @brief Supported shapes for representing nodes.
	enum class shape { cube, sphere, SIZE };

	//! @brief Supported pointers to vertex buffers.
	enum class vertex { font, singleLine, star, plane, grid, cube, sphere, SIZE };

	//! @brief Supported pointers to index buffers.
	enum class index { plane, gridNorm, gridHigh, SIZE };

	namespace internal {
		class Shapes {
        public:
			//! @brief Cube's vertex data.
			static const float VERTEX_CUBE[216];
		};
	}
}

#endif // FCPP_GRAPHICS_SHAPES_H_
