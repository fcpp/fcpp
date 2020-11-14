// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

namespace fcpp {

	//! @brief Supported shapes for representing nodes.
	enum class shape { ortho, grid, cube, sphere };

	namespace internal {
		class Shapes {
        public:
			//! @brief Orthogonal axis' vertex data.
			static const float VERTEX_ORTHO[36];

			//! @brief Grid line vertex data.
			static const float VERTEX_GRIDLINE[12];

			//! @brief Cube's vertex data.
			static const float VERTEX_CUBE[216];

			/*
            //Get the vertex data of the specified shape.
            static const float* getVertex(shape sh);

            //Get the index data of the specified shape.
            static const int* getIndex(shape sh);
			*/
		};
	}
}

#endif // FCPP_GRAPHICS_SHAPES_H_