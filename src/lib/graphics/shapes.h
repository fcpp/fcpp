// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

namespace fcpp {

	//! @brief Supported shapes for representing nodes.
	enum class shape { line, square, ortho, cube, sphere };

	namespace internal {
		class Shapes {
        public:
			//! @brief Line's vertex data.
			static const float VERTEX_LINE[12];

			//! @brief Square's vertex data.
			static const float VERTEX_SQUARE[24];

			//! @brief Square's index data.
			static const int INDEX_SQUARE[6];

			//! @brief Orthogonal axis' vertex data.
			static const float VERTEX_ORTHO[36];

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