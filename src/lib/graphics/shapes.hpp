// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

namespace fcpp {

	//! @brief Supported shapes for representing nodes.
	enum class shape { plane, grid, ortho, cube, sphere };

	namespace internal {
		class Shapes {
        public:
			//! @brief Orthogonal axis' vertex data.
			static const float VERTEX_ORTHO[36];

			//! @brief Cube's vertex data.
			static const float VERTEX_CUBE[216];

            //! @brief Get the index of the specified shape.
            static const int getIndex(shape sh);
		};
	}
}

#endif // FCPP_GRAPHICS_SHAPES_H_